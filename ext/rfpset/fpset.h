#ifndef FPSET_H
#define FPSET_H

#include <stdio.h>

typedef struct {
  size_t size;
  size_t reserved_size;
  char data[0];
} blob;

blob* blob_make(size_t reserved_size);
blob* blob_fill(blob* dst, char * src, size_t count);

blob* blob_read(FILE* src, blob* datum);
int blob_write(FILE* dst, blob* datum);
void blob_print(FILE* out, blob* datum);
int blob_sort_array(blob** blobs, size_t count);

blob** blob_intersect_files(FILE** files, int file_count, int* result_count);
#endif
