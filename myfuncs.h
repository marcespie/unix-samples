#ifndef MYFUNCS_H 
#define MYFUNCS_H 1

extern void safe_write(int, const void *, size_t);
extern bool bad_status(int, int);

// a "simple" wrapper for syscall error handling as a macro
// the do {} while (0) trick allows errwrap(x) to be used alone,
// or as a "simple expression in a test
// #x is text verbatim
#define errwrap(x) \
	do { \
		if ((x) == -1) err(1, #x); \
	} while (0)

#endif
