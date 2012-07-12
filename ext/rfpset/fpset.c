/** fpset
 *
 * fast-persistent sets
 * 
 * A super specialized implementation of a set and set intersection
 * for large sets of binary blob objects.
 */
#include "fpset.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  if(datum->reserved_size < reserved_size) {
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
int blob_write(FILE* dst, blob* datum) {
  fwrite(datum, sizeof(size_t), 1, dst);
  fwrite(datum->data, datum->size, 1, dst);
}

void blob_print(FILE* out, blob* datum) {
  fprintf(out, "<blob size: %ld reserved_size: %ld data: %s>\n",
          datum->size, datum->reserved_size, datum->data);
}

int blob_compare(const void* va, const void* vb) {
  blob* a = *(blob**)va;
  blob* b = *(blob**)vb;

  if(a->size < b->size) return -1;
  if(a->size > b->size) return 1;
  return memcmp(a->data, b->data, a->size);
}

/**
 * Sort an array of blobs in place
 */
int blob_sort_array(blob** blobs, size_t count) {
  qsort(blobs, count, sizeof(blob*), blob_compare);
}

blob* blob_fill(blob* dst, char * src, size_t count) {
  dst = blob_ensure_reserved_size(dst, count);
  dst->size = count;
  memcpy(dst->data, src, count);
  return dst;
}

blob* blob_copy(blob* src) {
  blob* new_blob = blob_make(src->size);
  return blob_fill(new_blob, src->data, src->size);
}

blob** blob_intersect_files(FILE** files, int file_count, int* result_count) {
  if(file_count == 0) {
    *result_count = 0;
    return NULL;
  }

  int master_idx = 0;
  int ii = 0;
  blob* master_blob = blob_make(512);
  blob* next_blob = blob_make(512);

  int result_capacity = 1024;
  *result_count = 0;
  blob** result = malloc(sizeof(blob*) * result_capacity);

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
      // resize our result array if we need to
      if(*result_count == result_capacity) {
        result = realloc(result, result_capacity * 2);
        result_capacity *= 2;
      }

      result[(*result_count)++] = blob_copy(master_blob);
    } else {
      // if we didn't have a match then whichever blob failed first
      // becomes the new master and we try again
      blob* temp = master_blob;
      master_blob = next_blob;
      next_blob = temp;
      master_idx = ii;
    }
  }

  return result;
}

blob* blob_make_test(char * c_str) {
  blob* new_blob = blob_make(strlen(c_str) + 1);
  return blob_fill(new_blob, c_str, strlen(c_str) + 1);
}

int main(int argc, char ** argv) {
  blob* test1 = blob_make_test("hello worldzz");
  blob* test2 = blob_make_test("goodbye world");

  blob* blobs[2];
  blobs[0] = test1;
  blobs[1] = test2;

  blob_print(stdout, blobs[0]);
  blob_sort_array(blobs, 2);
  blob_print(stdout, blobs[0]);

  FILE * test = fopen("test.dat", "w");
  blob_write(test, blobs[0]);
  blob_write(test, blobs[1]);
  fclose(test);

  test = fopen("test.dat", "r");
  blob* read_blob = blob_make(5);
  read_blob = blob_read(test, read_blob);
  blob_print(stdout, read_blob);
  read_blob = blob_read(test, read_blob);
  blob_print(stdout, read_blob);
  fclose(test);

  test = fopen("test2.dat", "w");
  blob_write(test, test2);
  fclose(test);

  FILE* files[2];
  files[0] = fopen("test.dat", "r");
  files[1] = fopen("test2.dat", "r");

  int found;
  blob** intersect = blob_intersect_files(files, 2, &found);
  printf("found %d intersects\n", found);
  blob_print(stdout, intersect[0]);
  //blob_print(stdout, intersect[1]);

  return 0;
}
