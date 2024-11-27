#ifndef MYMALLOC_H
#define MYMALLOC_H

#include <stddef.h>

void* myMalloc(size_t size);
void  myFree(void* ptr);
void* myRealloc(void* ptr, size_t size);

#ifdef USE_MY_MALLOC
#define malloc(size) myMalloc(size)
#define free(ptr)    myFree(ptr)
#define realloc(ptr, size) myRealloc(ptr, size)
#endif

#endif 
