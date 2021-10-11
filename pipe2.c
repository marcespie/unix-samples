// always minimize includes, since C has one single namespace
#include <unistd.h>
#include <err.h>	// this is more or less standard these days
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

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

void
perform_computation(int i, int fd)
{
	char *res;

	asprintf(&res, "Square of %d is %d\n", i, i * i);
	safe_write(fd, res, strlen(res));
}

// lift the handling code into a separate function
void 
decode_status(int status)
{
	if (WIFEXITED(status)) {
		int rc = WEXITSTATUS(status);
		if (rc == 0)
			return;
		else {
			fprintf(stderr, "Child exited with exit(%d)\n", rc);
			exit(1);
		}
	} else if (WIFSIGNALED(status)) {
		int sig = WTERMSIG(status);
		fprintf(stderr, "Child exited with signal %d (%s)\n", 
		    sig, strsignal(sig));
	} else {
		// under normal circumstances, you either get a signal
		// or an exit status
		fprintf(stderr, "This should never happen: %d\n", status);
	}
	exit(1);
}

#define MYSIZE 1024
void
print_from(int fd)
{
	char buf[MYSIZE];
	while (1) {
		ssize_t r = read(fd, buf, MYSIZE);
		if (r == -1)
			err(1, "read");
		if (r == 0)
			break;
		safe_write(1, buf, r);
	}
}

int
main()
{
	int i = 15; // XXX implicit communication
	// since fork() gives you a CC of the parent,
	// -> you get implicit parameter passing to the child
	// explicit communication from child to father

	int rc, pip[2];
	rc = pipe(pip);
	// mnemotechnic: stdin is fd 0, stdout is fd 1
	// so pip[0] is the read side and pip[1] is the write side
	if (rc == -1)
		err(1, "pipe");
	int pid = fork();
	switch(pid) {
	case -1: 
		err(1, "fork");
	case 0:
		// XXX ALWAYS close the side of the pipe you don't want
		if (close(pip[1]) == -1)
			err(1, "close");
		perform_computation(i, pip[0]);
		exit(0);
	default:
		if (close(pip[0]) == -1)
			err(1, "close");
		break;
	}
	// get the result and print it
	print_from(pip[1]);
	if (close(pip[1]) == -1)
		err(1, "close");

	// this is purely the father's code
	int status;
	// always check *all* syscalls for errors
	// always use waitpid *even if you have one single child*
	// (you never know when you're going to reuse your code)
	int r = waitpid(pid, &status, 0);
	if (r == -1)
		err(1, "waitpid");
	// decode status
	decode_status(status);
	exit(0);
}

