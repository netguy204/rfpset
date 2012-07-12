#include "fpset.h"
#include <ruby.h>

/**
 * IMROVEMENT IDEAS:
 * 
 * A RSTRING is pretty similar to a blob. We could write routines to
 * sort the RARRAY in place and emit the RSTRING directly. That would
 * cut memory usage in half.
 *
 */
static VALUE rfpset_spit_array(VALUE self, VALUE array, VALUE filename) {
  FILE* out = fopen(RSTRING_PTR(filename), "w");
  if(out == NULL) return rb_fix_new(-1);

  long ii; 
  long size = RARRAY_LEN(array);
  blob** blobs = malloc(sizeof(blob*) * size);

  // slurp out the values
  VALUE* values = RARRAY_PTR(array);
  for(ii = 0; ii < size; ++ii) {
    long len = RSTRING_LEN(values[ii]);
    blob* new_blob = blob_make(len);
    blobs[ii] = blob_fill(new_blob, RSTRING_PTR(values[ii]), len);
  }

  // sort them in our special way
  blob_sort_array(blobs, size);

  // spit them at the disk, freeing as we go
  blob* last_blob = NULL;
  for(ii = 0; ii < size; ++ii) {
    if(!last_blob
       || blobs[ii]->size != last_blob->size
       || memcmp(blobs[ii]->data, last_blob->data, last_blob->size) != 0) {
      // this blob is unique. Write it
      blob_write(out, blobs[ii]);

      if(last_blob) free(last_blob);
      last_blob = blobs[ii];
    } else {
      free(blobs[ii]);
    }
  }

  if(last_blob) free(last_blob);
  free(blobs);

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

  int num_results;
  blob** blobs = blob_intersect_files(files, file_count, &num_results);

  // close the files
  for(ii = 0; ii < file_count; ++ii) {
    fclose(files[ii]);
  }

  // build a return value for ruby
  VALUE array = rb_ary_new();

  for(ii = 0; ii < num_results; ++ii) {
    rb_ary_push(array, rb_str_new(blobs[ii]->data, blobs[ii]->size));
    free(blobs[ii]);
  }
  free(blobs);

  return array;
}

void Init_rfpset() {
  VALUE klass = rb_define_class("FPSetInternal", rb_cObject);
  rb_define_singleton_method(klass, "spit_array", rfpset_spit_array, 2);
  rb_define_singleton_method(klass, "slurp_array", rfpset_slurp_array, 1);
  rb_define_singleton_method(klass, "intersect_files", rfpset_intersect_files, 1);
}
