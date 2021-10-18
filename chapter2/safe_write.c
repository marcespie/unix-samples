#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <err.h>

#include "foo.h"

void
safe_write(int fd, 
    const void *buf,
    size_t sz)
{
	const char *b = buf;
	while (sz != 0) {
		ssize_t r;
		errwrap(r = write(fd, b, sz));
		assert(r <= sz);
		b += r;
		sz -= r;
		// loop invariant: we have to write [buf, buf+sz[ to fd
	}
}

