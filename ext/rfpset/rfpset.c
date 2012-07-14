#include <ruby.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  size_t size;
  size_t reserved_size;
  char data[0];
} blob;

typedef char* bytes;

/**
 * Read COUNT bytes from SRC into DATA. Return 1 if all bytes were
 * read successfully or 0 if it wasn't possible to read the requested
 * number of bytes.
 */
int read_confidently(FILE* src, size_t count, bytes data) {
  return fread(data, count, 1, src);
}

blob* blob_make(size_t reserved_size) {
  blob* new_blob = malloc(sizeof(blob) + reserved_size);
  new_blob->size = 0;
  new_blob->reserved_size = reserved_size;
  return new_blob;
}

blob* blob_ensure_reserved_size(blob* datum, size_t reserved_size) {
  if(reserved_size > datum->reserved_size) {
    datum = realloc(datum, reserved_size);
    datum->reserved_size = reserved_size;
  }
  return datum;
}

/**
 * Read a blob from SRC into DATUM. May realloc DATUM- if not large
 * enough to contain the blob so the returned value should always be
 * used in place of DATUM. Returns null if read was impossible.
 */
blob* blob_read(FILE* src, blob* datum) {
  if(!read_confidently(src, sizeof(size_t), (char*)datum)) return NULL;
  datum = blob_ensure_reserved_size(datum, datum->size);
  if(!read_confidently(src, datum->size, datum->data)) return NULL;
  return datum;
}

/**
 * Writes DATUM to DST. Returns bytes written if success or 0 if
 * failure. (A blob always has size > 0)
 */
int rstring_write(FILE* dst, VALUE string) {
  size_t size = RSTRING_LEN(string);
  fwrite(&size, sizeof(size_t), 1, dst);
  return fwrite(RSTRING_PTR(string), size, 1, dst);
}

int rstring_compare(const void* va, const void* vb) {
  VALUE a = *(VALUE*)va;
  VALUE b = *(VALUE*)vb;

  size_t size_a = RSTRING_LEN(a);
  size_t size_b = RSTRING_LEN(b);

  if(size_a < size_b) return -1;
  if(size_a > size_b) return 1;
  return memcmp(RSTRING_PTR(a), RSTRING_PTR(b), size_a);
}

int blob_compare(const void * va, const void * vb) {
  blob* a = *(blob**)va;
  blob* b = *(blob**)vb;

  if(a->size < b->size) return -1;
  if(a->size > b->size) return 1;
  return memcmp(a->data, b->data, a->size);
}

/**
 * Sort an array of blobs in place
 */
int rstring_sort_array(VALUE* strings, size_t count) {
  qsort(strings, count, sizeof(VALUE), rstring_compare);
}

VALUE blob_intersect_files(FILE** files, int file_count) {
  VALUE result = rb_ary_new();

  if(file_count == 0) return result;

  int master_idx = 0;
  int ii = 0;
  blob* master_blob = blob_make(512);
  blob* next_blob = blob_make(512);

  // bootstrap
  master_blob = blob_read(files[0], master_blob);

  // until a file runs out of data
  while(1) {
    int all_match = 1;
    int end_of_file = 0;

    for(ii = 0; ii < file_count; ++ii) {
      if(ii == master_idx) continue;

      // read blobs from this file until they aren't less than the
      // master blob
      int compare_result = 0;
      while(1) {
        next_blob = blob_read(files[ii], next_blob);
        if(next_blob == NULL) {
          end_of_file = 1;
          break;
        } else {
          compare_result = blob_compare(&next_blob, &master_blob);
          if(compare_result >= 0) break;
        }
      }

      // if any file ever reaches the end while we're looking it means
      // that we've found the entire intersection
      if(end_of_file) {
        all_match = 0;
        break;
      }

      // if we ever get a non-zero compare result then that means the
      // current candidate is a failure and we have a new candidate to
      // try
      if(compare_result != 0) {
        all_match = 0;
        break;
      }
    }

    // finish bailing out on end of file
    if(end_of_file) break;

    // store the match if we had one
    if(all_match) {
      rb_ary_push(result, rb_str_new(master_blob->data,
                                     master_blob->size));
    } else {
      // if we didn't have a match then whichever blob failed first
      // becomes the new master and we try again
      blob* temp = master_blob;
      master_blob = next_blob;
      next_blob = temp;
      master_idx = ii;
    }
  }

  free(master_blob);
  free(next_blob);

  return result;
}

static VALUE rfpset_spit_array(VALUE self, VALUE array, VALUE filename) {
  FILE* out = fopen(RSTRING_PTR(filename), "w");
  if(out == NULL) return rb_fix_new(-1);

  long ii; 
  long size = RARRAY_LEN(array);

  // sort the array in place
  VALUE* values = RARRAY_PTR(array);
  rstring_sort_array(values, size);

  // spit them at the disk, freeing as we go
  VALUE last_value = 0;
  for(ii = 0; ii < size; ++ii) {
    if(!last_value
       || RSTRING_LEN(values[ii]) != RSTRING_LEN(last_value)
       || memcmp(RSTRING_PTR(values[ii]), RSTRING_PTR(last_value),
                 RSTRING_LEN(last_value)) != 0) {
      // this blob is unique. Write it
      rstring_write(out, values[ii]);
      last_value = values[ii];
    }
  }

  fclose(out);

  // return the number of blobs written
  return rb_fix_new(size);
}

VALUE rfpset_slurp_array(VALUE self, VALUE filename) {
  FILE* in = fopen(RSTRING_PTR(filename), "r");
  if(in == NULL) return rb_fix_new(-1);

  VALUE array = rb_ary_new();

  blob* next_blob = blob_make(512);
  while((next_blob = blob_read(in, next_blob)) != NULL) {
    rb_ary_push(array, rb_str_new(next_blob->data, next_blob->size));
  }
  fclose(in);
  free(next_blob);

  return array;
}

VALUE rfpset_intersect_files(VALUE self, VALUE filenames) {
  long file_count = RARRAY_LEN(filenames);
  int ii;

  FILE** files = malloc(sizeof(FILE*) * file_count);
  VALUE* values = RARRAY_PTR(filenames);

  // open all the files
  for(ii = 0; ii < file_count; ++ii) {
    const char* name = RSTRING_PTR(values[ii]);
    FILE* file = fopen(name, "r");
    if(file == NULL) break; // failure!
    files[ii] = file;
  }

  // make sure they all opened
  if(ii < file_count) {
    // close them all
    int jj;
    for(jj = 0; jj < ii; ++jj) {
      fclose(files[jj]);
    }
    return rb_fix_new(-1);
  }

  VALUE array = blob_intersect_files(files, file_count);

  // close the files
  for(ii = 0; ii < file_count; ++ii) {
    fclose(files[ii]);
  }

  return array;
}

void Init_rfpset() {
  VALUE klass = rb_define_class("FPSetInternal", rb_cObject);
  rb_define_singleton_method(klass, "spit_array", rfpset_spit_array, 2);
  rb_define_singleton_method(klass, "slurp_array", rfpset_slurp_array, 1);
  rb_define_singleton_method(klass, "intersect_files", rfpset_intersect_files, 1);
}
