// use a pipe to talk back to the parent
#include <unistd.h>
#include <err.h>	// this is more or less standard these days
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "myfuncs.h"

void
perform_computation(int i, int fd)
{
	char *res;

	asprintf(&res, "Square of %d is %d\n", i, i * i);
	safe_write(fd, res, strlen(res));
}

int
main()
{
	int i = 15; 

	int rc, pip[2];
	errwrap(rc = pipe(pip));
	// mnemonic: stdin is fd 0, stdout is fd 1
	// so pip[0] is the read side and pip[1] is the write side
	int pid;

	// note that we're no longer printing anything (directly) in
	// our child, so we no longer need fflush()!
	errwrap(pid = fork());

	// XXX ALWAYS close the side of the pipe you don't want
	// so we "orient pipes", here child -> parent (we still)
	// communicate implicitly what we want from the parent
	if (pid == 0) {
		errwrap(close(pip[1]));
		// in case we don't exec(), there is no need to redirect
		// (dup the write side of the pipe to stdout),
		// since we can pass the fd we want as a parameter
		perform_computation(i, pip[0]);
		exit(0);
	}
	// father
	errwrap(close(pip[0]));

	// get the result and print it
	print_from(pip[1]);


	// note that we "kind of" know about child death in two ways
	// - child closed the pipe.
	// - child exited
	// -> so if we got consistent data, things are probably okay
	// -> AND we can check child status (exit(0)) as well
	errwrap(close(pip[1]));

	int status, r;

	errwrap(r = waitpid(pid, &status, 0));
	exit(bad_status(status, pid) ? 1 : 0);
}

