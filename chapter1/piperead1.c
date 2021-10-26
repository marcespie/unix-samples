// simplest way to grab a compressed file

#include <err.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

#include "myfuncs.h"

void
usage()
{
	errx(1, "Usage: piperead1 file.gz");
}

// XXX this function returns two things so that we can use waitpid
int 
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
		// XXX -- to stop option processing
		// XXX (void*)0 because NULL is not necessarily the right size
		execlp("gzip", "gzip", "-c", "-d", "--", filename, (void *)0);
		err(1, "execl");
	}

	*r = pid;

	eclose(pip[1]);

	return pip[0];
}

#define MYBUF 1024
int
main(int argc, char *argv[]) {
	if (argc != 2)
		usage();

	int pid;
	int fd = compressed_read(argv[1], &pid);

	while (true) {
		char buffer[MYBUF];
		ssize_t r;
		errwrap(r = read(fd, buffer, sizeof buffer));
		if (r == 0)
			break; // EOF
		// here we do any processing we want on the buffer
		safe_write(1, buffer, r);
	}
	// note we get TWO errors: one because of the close
	eclose(fd);

	int status;
	// AND the wait status from our child
	errwrap(waitpid(pid, &status, 0));
	(void)(bad_status(status, pid));
}

