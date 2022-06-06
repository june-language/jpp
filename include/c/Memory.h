#ifndef vm_memory_h
#define vm_memory_h

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t JuneMemMult8Roundup(size_t sz);
void *JuneMemAlloc(size_t sz);
void JuneMemFree(void *ptr, size_t sz);

#ifdef __cplusplus
}
#endif

#endif
