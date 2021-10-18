#include <stdlib.h>
#include <err.h>

#include "foo.h"

void *
emalloc(size_t sz)
{
	void *ptr = malloc(sz);
	if (!ptr)
		err(1, " in malloc(%zu)", sz);
	return ptr;
}

void *
erealloc(void *p, size_t sz)
{
	void *ptr = realloc(p, sz);
	if (!ptr)
		err(1, " in realloc(%zu)", sz);
	return ptr;
}

void *
ecalloc(size_t sz1, size_t sz2)
{
	void *ptr = calloc(sz1, sz2);
	if (!ptr)
		err(1, " in calloc(%zu, %zu)", sz1, sz2);
	return ptr;
}

void *
ereallocarray(void *p, size_t sz1, size_t sz2)
{
	void *ptr = reallocarray(p, sz1, sz2);
	if (!ptr)
		err(1, " in reallocarray(%zu, %zu)", sz1, sz2);
	return ptr;
}
