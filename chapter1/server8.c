// sigaction to avoid EINTR
#include <stdbool.h>
#include <err.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <poll.h>

#include "myfuncs.h"

void
usage()
{
	errx(1, "Usage: server5 [-d] service");
}

#define MAXSERV 10
struct pollfd servers[MAXSERV];
size_t nservers;

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

	struct sigaction sa;
	sa.sa_handler = reaper;
	(void)sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sigaction(SIGCHLD, &sa, NULL);
	while (1) {
		// with SA_RESTART set, the kernel will automatically restart
		// poll... minutia: process is sleeping on poll, child dies
		// SIGCHLD gets posted, we interrupt the system call to go
		// thru the SIGCHLD handler. On return from reaper(), kernel
		// restarts poll automatically.

		// XXX there might be some shennanigans with properly restarting
		// with a partial timeout but since we specified infinite 
		// timeout, this doesn't affect us.
		int n = poll(servers, nservers, INFTIM); 

		int i;
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
