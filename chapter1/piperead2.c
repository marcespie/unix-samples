// stdin to the rescue
#include <err.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>

#include "myfuncs.h"

void
usage()
{
	errx(1, "Usage: piperead2 file.gz");
}

// this is just the wrapper to go stdio (fdopen)
FILE * 
compressed_read(const char *filename, int *r)
{
	int pid;
	int pip[2];
	errwrap(pipe(pip));
	errwrap(pid = fork());
	if (pid == 0) {
		eclose(pip[0]);
		if (pip[1] != 1) {
			errwrap(dup2(pip[1], 1));
			eclose(pip[1]);
		}
		execlp("gzip", "gzip", "-c", "-d", "--", filename, (void *)0);
		err(1, "execl");
	}

	*r = pid;

	eclose(pip[1]);

	FILE *f = fdopen(pip[0], "r");
	if (!f)
		err(1, "fdopen");
	return f;
}

int
main(int argc, char *argv[]) {
	if (argc != 2)
		usage();

	int pid;
	FILE *f = compressed_read(argv[1], &pid);

	while (true) {
		int c = fgetc(f);
		if (c == EOF)
			break;
		putchar(c);
	}

	if (ferror(f))
		errx(1, "error on closing file");

	fclose(f);

	int status;
	errwrap(waitpid(pid, &status, 0));
	(void)(bad_status(status, pid));
}

