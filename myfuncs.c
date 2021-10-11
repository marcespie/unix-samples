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
		ssize_t r = write(fd, b, sz);
		if (r == -1)
			err(1, "write");
		assert(r <= sz);
		b += r;
		sz -= r;
		// loop invariant: we have to write [buf, buf+sz[ to fd
	}
	// loop termination: sz becomes zero eventually
	// write will *never* returns 0
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

