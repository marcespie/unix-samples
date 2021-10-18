// avoiding fork: using poll
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "myfuncs.h"

void
usage()
{
	errx(1, "Usage: server5 [-d] service");
}

// Unix has two mechanisms to handle multiple available fds: poll is
// way simpler to use (select has a large number of gotcha, especially
// when you need large values of fd)
#define MAXSERV 10
struct pollfd servers[MAXSERV];
size_t nservers;

// note that for simple "system" programs, pollfd is enough to record what
// we need to do... in more complex examples, we'll have a data structure
// related to each fd we want to poll() on the side.


bool
create_servers(const char *service, bool debug)
{
	struct addrinfo hints, *res, *res0;
	int error;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	nservers = 0;
	error = getaddrinfo(NULL, service, &hints, &res0);
	if (error)
		errx(1, "%s", gai_strerror(error));
	for (res = res0; res; res = res->ai_next) {
		int s = socket(res->ai_family, res->ai_socktype,
		    res->ai_protocol);
		if (s == -1)
			continue;
		if (bind(s, res->ai_addr, res->ai_addrlen) == -1) {
			warn("bind");
			close(s);
			continue;
		}
		printf("Success! bound %s address\n",
		    (res->ai_family == AF_INET ? "IPv4": "IPv6"));
		if (debug) {
			int p = 1;
			errwrap(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, 
			    &p, sizeof(p)));
		}
		errwrap(listen(s, 128));

		servers[nservers].fd = s;
		servers[nservers].events = POLLIN;
		nservers++;
		if (nservers > MAXSERV)
			errx(1, "Too many servers");
	}

	freeaddrinfo(res0);
	return nservers != 0;
}


void 
reaper(int sig)
{
	int save_errno = errno;
	int pid, status;
	while ((pid = wait3(&status, WNOHANG, NULL)) >= 0) {
		bad_status(status, pid);
	}
	if (pid == -1 && errno != ECHILD)
		err(1, "wait3");
	errno = save_errno;
}

void
server_code(int s)
{
}

int
main(int argc, char *argv[])
{

	bool debug = false;
	int c;
	while ((c = getopt(argc, argv, "d")) != -1) {
		switch(c) {
			case 'd':
				debug = true;
				break;
			default:
				usage();
		}
	}
	argc -= optind;
	argv += optind;

	if (argc != 1)
		usage();
    	if (!create_servers(argv[0], debug))
		errx(1, "Couldn't create any server");

	signal(SIGCHLD, reaper);
	while (1) {
		int n = poll(servers, nservers, INFTIM); 
		// XXX we have a signal handler for SIGCHLD
		// this is the first time we encounter a system call
		// that might be interrupted by a signal (and it *will*
		// be interrupted, so we have to provide for that in order
		// to continue..)
		// IF the blocking call was allowed to sleep, THEN the
		// dead child would become a zombie, until a new accept
		// happens.
		if (n == -1 && errno == EINTR)
			continue;

		int i;
		// XXX on large pollfd arrays, we use n to optimize: get out
		// of the loop when we have the correct number of polls.
		for (i = 0; i != nservers; i++) {
			if (!(servers[i].revents & POLLIN))
				continue;

			int fd;
			errwrap(fd = accept(servers[i].fd, NULL, 0));
			int pid;
			errwrap(pid = fork());
			if (pid == 0) {
				if (fd != 1) {
					errwrap(dup2(fd, 1));
					errwrap(close(fd));
				}
				execlp("date", "date", NULL);
				err(1, "exec");
			}
			errwrap(close(fd));
		}
	}
}
