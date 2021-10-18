// some fairly horrible code that works because of how stdio works
#include <err.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void
perform_computation(int i)
{
	// there's already Square is in the buffer
	printf("%d is %d\n", i, i * i);
}

bool 
bad_status(int status)
{
	if (WIFEXITED(status)) {
		int rc = WEXITSTATUS(status);
		if (rc == 0)
			return false;
		else
			fprintf(stderr, "Child exited with exit(%d)\n", rc);
	} else if (WIFSIGNALED(status)) {
		int sig = WTERMSIG(status);
		fprintf(stderr, "Child exited with signal %d (%s)\n", 
		    sig, strsignal(sig));
	} else
		fprintf(stderr, "This should never happen: %d\n", status);
	return true;
}

int
main()
{
	// XXX exercise C buffering: by default stdout is line-buffered on
	// a tty, and fully buffered on a fd, so stuff printed BEFORE the fork
	// *doesn't* go out and gets duplicated
	printf("Square of ");

	for (int i = 15; i < 35; i += 2) {
		int pid = fork();
		switch(pid) {
		case -1: 
			err(1, "fork");
		case 0:
			perform_computation(i);
			exit(0);
		}
	}

	// parent
	int rc = 0; // by default we succeed

	int status, r;
	while ((r = wait(&status)) != -1)
		if (bad_status(status))
			rc = 1;
	if (errno != ECHILD)
		err(1, "wait");

	// XXX and of course, we still have "Square of" to print in the
	// main process
	// but exit is not the syscall! it does flush stdout and friends
	// before exiting
	_exit(rc);
}
