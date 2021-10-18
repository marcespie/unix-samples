// some more simple puzzle pieces with fork
#include <err.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

// we moved some pieces into a separate file
// old school: do not have includes that include other stuff
// having everything laid out "flat" makes it much easier to
// (later) handle namespace issues, where you need to rework includes
// to have less collisions (one of the curses of C)
#include "myfuncs.h"

void
perform_computation(int i)
{
	printf("Square of %d is %d\n", i, i * i);
}

int
main()
{
	// IF we do anything with stdio before a fork, we
	// should flush so that we don't get duplicate buffers:
	// fork doesn't know anything about stdin
	fflush(NULL);
	for (int i = 15; i < 35; i += 2) {
		int pid;
		errwrap(pid = fork());
		if (pid == 0) {
			perform_computation(i);
			exit(0);
		}
	}

	// parent
	int rc = 0;

	int status, pid;
	while ((pid = wait(&status)) != -1)
		if (bad_status(status, pid))
			rc = 1;
	if (errno != ECHILD)
		err(1, "wait");

	exit(rc);
}
