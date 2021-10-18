#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "myfuncs.h"

void
safe_write(int fd, 
    const void *buf, // ALWAYS annotate your functions to be const correct
    size_t sz)
{
	// XXX stdlib wants void *, but we need char* for pointer
	// arithmetic
	const char *b = buf;
	while (sz != 0) {
		ssize_t r;
		errwrap(r = write(fd, b, sz));
		assert(r <= sz);
		b += r;
		sz -= r;
		// loop invariant: we have to write [buf, buf+sz[ to fd
	}
	// loop termination: sz becomes zero eventually
	// write will *never* returns 0
}

// NEVER put magic numbers directly in the code, ALWAYS name them
#define MYSIZE 1024
void
print_from(int fd)
{
	char buf[MYSIZE];
	while (1) {
		ssize_t r;
		errwrap(r = read(fd, buf, MYSIZE));
		if (r == 0)
			break;
		// by contrast standard output is ALWAYS fd 1
		// some people use STDOUT_FILENO, but unless your code
		// is really confusing, it's not really needed
		safe_write(1, buf, r);
	}
}

bool 
bad_status(int status, int pid)
{
	if (WIFEXITED(status)) {
		int rc = WEXITSTATUS(status);
		if (rc == 0)
			return false;
		else
			fprintf(stderr, "Child %d exited with exit(%d)\n", 
			    pid, rc);
	} else if (WIFSIGNALED(status)) {
		int sig = WTERMSIG(status);
		fprintf(stderr, "Child %d exited with signal %d (%s)\n", 
		    pid, sig, strsignal(sig));
	} else
		// under normal circumstances, you either get a signal
		// or an exit status
		fprintf(stderr, "This should never happen: child %d returned status=%d\n", pid, status);
	return true;
}

