// some more puzzle pieces with fork
#include <unistd.h>
#include <err.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
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
	int rc = 0;

	int status, pid;
	while ((pid = wait(&status)) != -1)
		if (bad_status(status, pid))
			rc = 1;
	if (errno != ECHILD)
		err(1, "wait");

	exit(rc);
}
