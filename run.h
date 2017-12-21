#ifndef _M_RUN_H_
#define _M_RUN_H_

#include <sys/types.h>

//Medata data block size
#define META_SIZE (sizeof(struct metadata) - 4)

typedef struct metadata *p_meta;

//Metadata
struct metadata
{
  size_t size;
  p_meta next;
  p_meta prev;
  int free;

  // A pointer tto the data section
  char data[1];
};

extern void *base;

int availableSize(p_meta index);
void* m_malloc(size_t size);
void m_free(void* ptr);
void* m_realloc(void* ptr, size_t size);
p_meta find_meta(p_meta *last, size_t size);

#endif
