#include "config.h"

#if !HAVE_REALLOCARRAY
extern void * reallocarray(void *, size_t, size_t);
#endif
