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

// this is how actual code usually works

// message passing through pipes: we can pass binary data without issue
// it's *more annoying* to debug than text data, but we don't have to
// do *anything* to it (same box, same endianess, no alignment issue
struct data {
	int pid;
	int result;
};

// if things are small enough, there won't be *any* interleaving
// it's *guaranteed that small writes make it thru a pipe
void
perform_computation(int pid, int v, int fd)
{
	struct data d;
	d.pid = pid;
	d.result = v * v;
	safe_write(fd, &d, sizeof d);
}

// so we send back our pid (one single pipe) to differentiate results
struct myproc {
	int pid;
	int v;
	int result;
};

// choice: for longer/more complex computations, we can associate one
// pipe *per process*, but it becomes harder to figure out when things
// happen


// we generally need some simple way to associate pid to actual computation
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
	}

	if (close(pip[1]) == -1)
		err(1, "close");

	// - or we get the error when we read the status from our dead child
	// (this will most often match)
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
		struct myproc *p = allprocs+i;
		if (p->pid != -1) {
			fprintf(stderr, "error: we did not reap %d\n",
			    p->pid);
		}
		if (p->result == -1) {
			fprintf(stderr, "error: no result for v=%d\n", 
			    p->v);
		}
		printf("Result for v=%d is %d\n",
		    p->v, p->result);
	}
	exit(rc);
}

