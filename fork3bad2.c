// what goes wrong when you do separate writes
#include <unistd.h>
#include <err.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

void
perform_computation(int i)
{
	// do something horrible where interleaving may happen
	char *res;

	asprintf(&res, "Square of %d is %d\n", i, i * i);

	// print string manually, one char at a time
	while (*res) {
		write(1, res, 1);
		res++;
	}
}

// we have to tweak decode_status to keep going
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
	// this normally exits with ECHILD, when we reaped every child
	if (errno != ECHILD)
		err(1, "wait");

	exit(rc);
}
