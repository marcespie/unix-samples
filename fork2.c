// always minimize includes, since C has one single namespace
#include <unistd.h>
#include <err.h>	// this is more or less standard these days
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void
perform_computation(int i)
{
	printf("Square of %d is %d\n", i, i * i);
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

int
main()
{
	int i = 15; // XXX implicit communication
	// since fork() gives you a CC of the parent,
	// -> you get implicit parameter passing to the child
	int pid = fork();
	switch(pid) {
	case -1: 
		err(1, "fork");
	case 0:
		perform_computation(i);
		exit(0);
	default:
		break;
	}
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

