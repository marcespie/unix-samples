// lifting out the error handling code into a separate function
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void
perform_computation(int i)
{
	printf("Square of %d is %d\n", i, i * i);
}

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
	// parent
	int status;
	int r = waitpid(pid, &status, 0);
	if (r == -1)
		err(1, "waitpid");
	decode_status(status);
	exit(0);
}

