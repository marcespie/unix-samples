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

// one cool thing with having one pipe is that we can display/process
// results on the fly

struct data {
	int pid;
	int result;
};

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
	// should never happen
	fprintf(stderr, "Fatal error: pid %d not found\n", pid);
	exit(1);
}

int
main()
{

	int r, pip[2];
	r = pipe(pip);
	if (r == -1)
		err(1, "pipe");

	int i;
	// straightforward: start all computations, and record them
	for (i = 0; i != MYPROCS; i++) {
		struct myproc *p = allprocs + i;
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
		p->pid = pid;
		p->v = v;
		p->result = -1;
	}
	// father side
	if (close(pip[0]) == -1)
		err(1, "close");

	// we actually get errors in two ways:
	// - we read the results from pipe, and missing pid will stay
	// with result == -1
	while (true) {
		struct data d;
		ssize_t r = read(pip[1], &d, sizeof d);
		if (r == -1)
			err(1, "read from pipe");
		if (r == 0)
			break;
		struct myproc *p = lookup(d.pid);
		p->result = d.result;
		printf("Result for v=%d is %d\n", p->v, p->result);
	}

	if (close(pip[1]) == -1)
		err(1, "close");

	// so at this point we got all results we possibly could
	// AND we have a table of *all* pids we want to handle
	// so we can got back to waitpid, with a twist
	int rc = 0;
	int status;
	for (i = 0; i != MYPROCS; i++) {
		struct myproc *p = allprocs+i;

		int flags = 0;
		if (p->result == -1) {
			fprintf(stderr, "error: no result for v=%d\n", 
			    p->v);
		    	flags = WNOHANG;
		}

		int r = waitpid(p->pid, &status, flags);
		if (r == -1)
			err(1, "waitpid(%d)", p->pid);
		if (r == 0)
			fprintf(stderr, "error: we did not reap %d\n",
			    p->pid);
		else if (bad_status(status, r))
			rc = 1;
	}
	exit(rc);
}

