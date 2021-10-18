// an extra stage to run bc thru telnet
// XXX this code actually has a subtle bug!
// can you guess what it is without peeking at the next program ?
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
	errx(1, "Usage: server9 [-d] service");
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

#define MAXBUF 1024

// so we got our socket, we want to run bc BUT it disagrees
// with telnet (doesn't expect crlf on input
// so we wrap it as strip_cr | bc
// like in a real shell pipe, what we care about is when bc
// exits, so we fork TWICE, and we only monitor bc.

// note that telnet has other "fun" characteristics (try ^C for instance)
// so we should properly filter more stuff

// XXX note that my "standard" bc(1) has a small bug: if you enter quit
// on the terminal, since things are buffered per-line, you will get your
// exit as expected. BUT through a socket, it requires some extra input to
// actually quit, as the standard input is no longer line-buffered, and
// somehow the parser doesn't quite cope.
void
go_run_command(int fd)
{
	int pid;
	errwrap(pid = fork());
	if (pid != 0) 
		return;

	int pip[2];
	errwrap(pipe(pip));
	errwrap(pid = fork());
	if (pid == 0) {
		eclose(pip[0]);
		// read from socket, strip cr, pass to bc
		char buffer[MAXBUF];
		while (true) {
			ssize_t r;
			errwrap(r = read(fd, buffer, sizeof buffer));
			if (r == 0)
				break;
			size_t i, j;
			// copy in place since we shrink the buffer
			for (i = 0, j = 0; i != r; i++)
				if (buffer[i] != '\r')
					buffer[j++] = buffer[i];
			safe_write(pip[1], buffer, j);
		}
		eclose(pip[1]);
		eclose(fd);
		exit(0);

	} else {
		errwrap(close(pip[1]));
		if (fd != 1) {
			errwrap(dup2(fd, 1));
			errwrap(close(fd));
		}
		if (pip[0] != 0) {
			errwrap(dup2(pip[0], 0));
			errwrap(close(pip[0]));
		}
		execlp("bc", "bc", NULL);
		err(1, "exec");
	}
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
		int n = poll(servers, nservers, INFTIM); 

		int i;
		for (i = 0; i != nservers; i++) {
			if (!(servers[i].revents & POLLIN))
				continue;

			int fd;
			errwrap(fd = accept(servers[i].fd, NULL, 0));
			go_run_command(fd); // this does not return
			errwrap(close(fd));
		}
	}
}
