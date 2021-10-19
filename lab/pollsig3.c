// sigaction doesn't affect things
// poll gets interrupted by signals, if used correctly
// note that linux introduced ppoll to have finer grained
// behavior !
#include <poll.h>
#include <err.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "myfuncs.h"

void
reaper(int sig)
{
}

int
main()
{
	struct pollfd pfd[1];

	int pip[2];
	errwrap(pipe(pip));

	int pid;

	errwrap(pid = fork());
	if (pid == 0) {
		sleep(5);
		fprintf(stderr, "Okay thxbye\n");
		exit(0);
	}
	struct sigaction sa;
	sa.sa_handler = reaper;
	sa.sa_flags = SA_RESTART;
	errwrap(sigaction(SIGCHLD, &sa, NULL));

	// now we "wait"

	pfd[0].fd = pip[0];
	pfd[0].events = POLLIN;
	int n = poll(pfd, 1, INFTIM);
	if (n == -1 && errno == EINTR) {
		fprintf(stderr, "Everything is okay\n");
	}
}
