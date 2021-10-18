// inject fault (hung child) to see how we fare
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
	if (rand() % 5 == 0) {
		close(fd);
		sleep(2000);
	}
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

	int pip[2];
	errwrap(pipe(pip));

	int i;
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

	while (true) {
		struct data d;
		ssize_t r;
		errwrap(r = read(pip[1], &d, sizeof d));
		if (r == 0) 
			break;
		struct myproc *p = lookup(d.pid);
		p->result = d.result;
		printf("Result for v=%d is %d\n",
		    p->v, p->result);
	}

	errwrap(close(pip[1]));
	int rc = 0;
	for (i = 0; i != MYPROCS; i++) {
		int status;
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
