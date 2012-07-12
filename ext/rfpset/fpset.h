#ifndef FPSET_H
#define FPSET_H

blob* blob_make(size_t reserved_size);
blob* blob_read(FILE* src, blob* datum);
int blob_write(FILE* dst, blob* datum);
void blob_print(FILE* out, blob* datum);
int blob_sort_array(blob** blobs, size_t count);

#endif
