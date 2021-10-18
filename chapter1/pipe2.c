// actually send structured data back
#include <assert.h>
#include <err.h>	
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "myfuncs.h"

// we're getting closer to actual code

// message passing through pipes: we can pass binary data without issue
// it's *more annoying* to debug than text data, but we don't have to
// do *anything* to it (same box, same endianess, no alignment issue)

// we send back our pid (one single pipe) to differentiate results
struct data {
	int pid;
	int result;
};

// if things are small enough, there won't be *any* interleaving
// it's *guaranteed* that small writes make it thru a pipe
void
perform_computation(int pid, int v, int fd)
{
	struct data d;
	d.pid = pid;
	d.result = v * v;
	// safe_write is not strictly necessary, but hey belt & suspendwer
	safe_write(fd, &d, sizeof d);
}


// choice: for longer/more complex computations, we can associate one
// pipe *per process*, but we have to either poll the pipes, OR get the
// result from each child sequentially
// one single pipe + demultiplexing means we get data easily


// systems programming involves data structures and algorithms:
// we need some kind of registry per-pid to remember what child is handling
// which computation
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

	int pip[2];
	errwrap(pipe(pip));

	int i;
	// straightforward: start all computations, and record them
	for (i = 0; i != MYPROCS; i++) {
		struct myproc *p = allprocs + i;
		int v = rand() % 200;
		int pid;
		errwrap(pid = fork());
		if (pid == 0) {
			errwrap(close(pip[1]));
			perform_computation(getpid(), v, pip[0]);
			exit(0);
		}
		// father
		p->pid = pid;
		p->v = v;
		p->result = -1;
	}
	errwrap(close(pip[0]));

	// we actually get errors in two ways:
	// - we read the results from pipe, and missing pid will stay
	// with result == -1
	while (true) {
		struct data d;
		ssize_t r;
		errwrap(r = read(pip[1], &d, sizeof d));
		if (r == 0) // it's often simpler to code "exit loop in
			    // the middle" rather than add arbitrary bool
			break;
		// match pipe data to required computation
		struct myproc *p = lookup(d.pid);
		p->result = d.result;
	}

	errwrap(close(pip[1]));
	// - or we get the error when we read the status from our dead child
	// (note that exit from child + no result is the "normal" case
	int rc = 0;
	int status, pid;
	while ((pid = wait(&status)) != -1) {
		if (bad_status(status, pid))
			rc = 1;
		struct myproc *p = lookup(pid);
		p->pid = -1;
	}
	if (errno != ECHILD)
		err(1, "wait");

	// new part: check what part of our data structure is still unfilled
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

