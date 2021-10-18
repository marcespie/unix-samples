#ifndef FOO_H
#define FOO_H
// we gain a library that contains useful functions that are not quite
// standard.

// we don't want to write a full configure so we do it manually
#include "config.h"

#if !HAVE_REALLOCARRAY
// reallocarray (from OpenBSD) works like realloc, except it does handle
// the product nmemb * szmemb  without any problem wrt overflows
extern void *reallocarray(void *, size_t, size_t);
#endif

#if !HAVE_STRLCPY
// strlcpy/cat are less error prone than strcpy/cat and (almost) fool-proof
extern size_t strlcpy(char *, const char *, size_t);
#endif

#if !HAVE_STRLCAT
// strlcpy/cat are less error prone than strcpy/cat and (almost) fool-proof
extern size_t strlcat(char *, const char *, size_t);
#endif
// some wrappers for usual C memory functions that just exit when
// something goes wrong
extern void *emalloc(size_t);
extern void *ereallocarray(void *, size_t, size_t);
extern void *erealloc(void *, size_t);
extern void *ecalloc(size_t, size_t);

// we also keep the ever so useful error wrappers for syscalls, and we
// extend them a bit
#define errwrap(x) \
	do { \
		if ((x) == -1) err(1, #x); \
	} while (0)

#define eclose(x) errwrap(close(x))
#define epipe(x) errwrap(pipe(x))

// the size/capacity idiom for servers
// we assume we won't remove servers, so the struct
// will contain [0, servers[ : sockets for server
// [servers, n[: sockets for client
struct myfds {
	struct pollfd *pfds;
	size_t servers;
	size_t n;
	size_t capacity;
};

extern void create_servers(struct myfds *, const char *, bool, size_t);
extern void safe_write(int, const void *, size_t);
#endif
