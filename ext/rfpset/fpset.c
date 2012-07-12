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

typedef struct {
  size_t size;
  size_t reserved_size;
  char data[0];
} blob;


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

blob* blob_make_test(char * c_str) {
  blob* new_blob = blob_make(strlen(c_str) + 1);
  new_blob->size = strlen(c_str) + 1;
  memcpy(new_blob->data, c_str, strlen(c_str) + 1);
  return new_blob;
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

  return 0;
}
