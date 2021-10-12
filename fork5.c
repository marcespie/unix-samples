#include <unistd.h>
#include <err.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

void
perform_computation(int i)
{
	printf("Square of %d is %d\n", i, i * i);
}

int
main()
{
	int i = 15; 
	int pid = fork();
	switch(pid) {
	case -1: 
		err(1, "fork");
	case 0:
		perform_computation(i);
		exit(0);
	}
	// father
	int status;
	int r = waitpid(pid, &status, 0);
	if (r == -1)
		err(1, "waitpid");
	// in some cases we want to transparently pass the child's 
	// code to the father...
	if (WIFEXITED(status)) {
		int rc = WEXITSTATUS(status);
		if (rc == 0) {
			exit(0);
		} else {
			fprintf(stderr, "Child exited with exit(%d)\n", rc);
			exit(rc);
		}
	} else if (WIFSIGNALED(status)) {
		int sig = WTERMSIG(status);
		fprintf(stderr, "Child exited with signal %d (%s)\n", 
		    sig, strsignal(sig));
		kill(getpid(), sig);
	} else {
		fprintf(stderr, "This should never happen: %d\n", status);
		// XXX that's the case we can't do
		exit(1);
	}
}

