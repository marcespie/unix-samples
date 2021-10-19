#ifndef MYFUNCS_H 
#define MYFUNCS_H 1

// a "simple" wrapper for syscall error handling as a macro
// the do {} while (0) trick allows errwrap(x) to be used alone,
// or as a "simple expression in a test
// #x is text verbatim
#define errwrap(x) \
	do { \
		if ((x) == -1) err(1, #x); \
	} while (0)

#endif

#define eclose(x) errwrap(close(x))

// XXX weird OS compat
#ifndef INFTIM
#define INFTIM (-1)
#endif
