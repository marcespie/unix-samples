// simple usage of fork and decoding of status

// always minimize includes, since C has one single namespace
//
// to sort or not to sort ?
// ISO C includes can be sorted, and most system includes as well
// beware of the sys/ directory which often has specific ordering rules
// (watch the SYNOPSIS)
#include <err.h>	// this is more or less standard these days
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
	// this is purely the parent's code
	int status;
	// always check *all* syscalls for errors
	// always use waitpid *even if you have one single child*
	// (you never know when you're going to reuse your code)
	int r = waitpid(pid, &status, 0);
	if (r == -1)
		err(1, "waitpid");
	// decode status
	// some similar code is *required* each time you interact with
	// children
	// -> we have some simple communication back from child to parent
	// which is whether we succeeded or not
	if (WIFEXITED(status)) {
		int rc = WEXITSTATUS(status);
		if (rc == 0) {
			// that case is 100% silent because 
			exit(0);	// everything is okay
		} else {
			fprintf(stderr, "Child exited with exit(%d)\n", rc);
			exit(1);
		}
	} else if (WIFSIGNALED(status)) {
		int sig = WTERMSIG(status);
		fprintf(stderr, "Child exited with signal %d (%s)\n", 
		    sig, strsignal(sig));
		exit(1);
	} else {
		// under normal circumstances, you either get a signal
		// or an exit status
		fprintf(stderr, "This should never happen: %d\n", status);
		exit(1);
	}
}

