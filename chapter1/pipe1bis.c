// the use of fdopen to get "nice" handles 
// (caveats galore: you have to be more or less CERTAIN your 
// read/writes are small enough)
#include <err.h>	// this is more or less standard these days
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "myfuncs.h"

void
perform_computation(int i, FILE *f)
{
	char *res;

	fprintf(f, "Square of %d is %d\n", i, i * i);
	// XXX this is a pipe, we want to make sure the parent gets it
	fflush(f); 
}

void
file_copy(FILE *in, FILE *out)
{
	int c;
	while (1) {
		c = fgetc(in);
		if (c == -1)
			break;
		fputc(c, out);
	}
	if (ferror(in))
		errx(1, "Fatal error while reading from pipe");
}

int
main()
{
	int i = 15; 

	int rc, pip[2];
	errwrap(rc = pipe(pip));
	int pid;

	errwrap(pid = fork());

	if (pid == 0) {
		errwrap(close(pip[1]));
		FILE *f = fdopen(pip[0], "w");
		// XXX the libc often defines errno stuff for "non syscalls"
		if (!f)
			err(1, "fdopen");
		perform_computation(i, f);
		exit(0);
	}
	// father
	errwrap(close(pip[0]));

	FILE *in = fdopen(pip[1], "r");
	if (!in)
		err(1, "fdopen");

	file_copy(in, stdout);

	// and it also "transparently" passes thru errno from syscalls
	// (fclose calls close)
	if (fclose(in) != 0)
		err(1, "fclose");

	int status, r;

	errwrap(r = waitpid(pid, &status, 0));
	exit(bad_status(status, pid) ? 1 : 0);
}

