// the signals that go with tty handling and bg job
#include <termios.h>
#include <signal.h>
#include <err.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "foo.h"


// the terminal discipline to restore
struct termios basetty;

// vs our terminal discipline
struct termios mytty;

void
save_data()
{
}

// all the signal handlers
void
byebye(int sig)
{
	save_data();
	signal(sig, SIG_DFL);
	errwrap(tcsetattr(0, TCSANOW, &basetty));
	kill(getpid(), sig);
}

void
tstp_handler(int sig)
{
	int save_errno = errno;

	signal(sig, SIG_DFL);
	errwrap(tcsetattr(0, TCSANOW, &basetty));
	kill(getpid(), sig);
	errno = save_errno;
}

void
tcont_handler(int sig)
{
	int save_errno = errno;

	signal(SIGTSTP, tstp_handler);
	errwrap(tcsetattr(0, TCSANOW, &mytty));
	errno = save_errno;
}

#define MYSIZE 1024

int
main()
{
	if (!isatty(0)) {
		fprintf(stderr, "This example programs shows tty behavior\n");
		exit(0);
	}
	// save basic tty stuff for later
	errwrap(tcgetattr(0, &basetty));
	errwrap(tcgetattr(0, &mytty));

	// obvious signals where we need to change/restore behavior
	signal(SIGTSTP, tstp_handler);
	signal(SIGCONT, tcont_handler);

	// all the signals that "kill" your program, so you should save
	// data AND restore the tty
	signal(SIGINT, byebye);
	signal(SIGQUIT, byebye);
	signal(SIGTERM, byebye);
	signal(SIGHUP, byebye);

	// XXX a lot of modern shells may handle tty modes for us, but
	// it's by no way guaranteed, so to be fully compliant we must
	// restore the tty on TSTP, and restore it on CONT.


	// tweak stuff we want (e.g., echo off)
	mytty.c_lflag &= ~ECHO;
	errwrap(tcsetattr(0, TCSANOW, &mytty));


	char buffer[MYSIZE];

	while (true) {
		ssize_t r;
		errwrap(r = read(0, buffer, sizeof buffer));
		if (r == 0)
			break;
		safe_write(1, buffer, r);
	}
}
