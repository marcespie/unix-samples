#include <unistd.h>
#include <err.h>	
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <errno.h>

#include "myfuncs.h"

struct data {
	int pid;
	int result;
};

// this is how actual code usually works
void
perform_computation(int pid, int v, int fd)
{
	struct data d;
	d.pid = pid;
	d.result = v * v;
	safe_write(fd, &d, sizeof d);
}

struct myproc {
	int pid;
	int v;
	int result;
};

#define MYPROCS 10
struct myproc allprocs[MYPROCS];

struct myproc * 
lookup(int pid)
{
	int i;
	for (i = 0; i != MYPROCS; i++)
		if (allprocs[i].pid == pid)
			return allprocs+i;
	fprintf(stderr, "Fatal error: pid %d not found\n", pid);
	exit(1);
}

int
main()
{

	int r, pip[2];
	r = pipe(pip);
	// mnemotechnic: stdin is fd 0, stdout is fd 1
	// so pip[0] is the read side and pip[1] is the write side
	if (r == -1)
		err(1, "pipe");

	int i;
	for (i = 0; i != MYPROCS; i++) {
		int pid = fork();
		int v = rand() % 200;
		switch(pid) {
		case -1: 
			err(1, "fork");
		case 0:
			// XXX ALWAYS close the side of the pipe you don't want
			if (close(pip[1]) == -1)
				err(1, "close");
			perform_computation(getpid(), v, pip[0]);
			exit(0);
		}
		allprocs[i].pid = pid;
		allprocs[i].v = v;
		allprocs[i].result = -1;
	}
	// father side
	if (close(pip[0]) == -1)
		err(1, "close");

	while (true) {
		struct data d;
		ssize_t r = read(pip[1], &d, sizeof d);
		if (r == -1)
			err(1, "read from pipe");
		if (r == 0)
			break;
		struct myproc *p = lookup(d.pid);
		p->result = d.result;
	}

	if (close(pip[1]) == -1)
		err(1, "close");

	int rc = 0;
	int status;
	while ((r = wait(&status)) != -1) {
		if (bad_status(status, r))
			rc = 1;
		struct myproc *p = lookup(r);
		p->pid = -1;
	}
	if (errno != ECHILD) // okay we reaped every child
		err(1, "wait");

	for (i = 0; i != MYPROCS; i++) {
		if (allprocs[i].pid != -1) {
			fprintf(stderr, "error: we did not reap %d\n",
			    allprocs[i].pid);
		}
		if (allprocs[i].result == -1) {
			fprintf(stderr, "error: no result for v=%d\n", 
			    allprocs[i].v);
		}
		printf("Result for v=%d is %d\n",
		    allprocs[i].v, allprocs[i].result);
	}
	exit(rc);
}

