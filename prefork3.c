// monitoring
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
	errx(1, "Usage: prefork1 [-d] service");
}

int 
create_server(const char *service, bool debug)
{
	struct addrinfo hints, *res, *res0;
	int error;
	int s = -1;

	memset(&hints, 0, sizeof(hints));
	// XXX we don't specify the family
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	error = getaddrinfo(NULL, service, &hints, &res0);
	if (error)
		errx(1, "%s", gai_strerror(error));
	for (res = res0; res; res = res->ai_next) {
		s = socket(res->ai_family, res->ai_socktype,
		    res->ai_protocol);
		if (s == -1)
			continue;
		if (bind(s, res->ai_addr, res->ai_addrlen) == -1) {
			warn("bind");
			close(s);
			s = -1;
			continue;
		}
		break;
	}

	if (s == -1)
		errx(1, "Couldn't bind any socket");

	printf("Success! bound %s address\n",
	    (res->ai_family == AF_INET ? "IPv4": "IPv6"));
	freeaddrinfo(res0);
	if (debug) {
		int p = 1;
		errwrap(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &p, sizeof(p)));
	}

	errwrap(listen(s, 128));
	return s;
}

#define MAXBUF 1024
void 
run_bc(int fd)
{
	int pip[2];
	errwrap(pipe(pip));

	int pid;
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
		eclose(pip[1]);
		if (fd != 1) {
			errwrap(dup2(fd, 1));
			eclose(fd);
		}
		if (pip[0] != 0) {
			errwrap(dup2(pip[0], 0));
			eclose(pip[0]);
		}
		execlp("bc", "bc", NULL);
		err(1, "exec");
	}
}

void
child_services(int s, int pip[])
{
	int pid;
	errwrap(pid = fork());
	if (pid == 0) {
		int fd;

		eclose(pip[0]);
		errwrap(fd = accept(s, NULL, 0));
		// tell monitor we're running
		char p[] = {'0'};
		errwrap(write(pip[1], p, 1));

		run_bc(fd);
	}
}

volatile sig_atomic_t got_sig = 0;

void
child_notify(int sig)
{
	got_sig = 1;
}

#define MAXCLIENTS_INIT 8
#define THRESHOLD 5
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
	int s = create_server(argv[0], debug);


	int pip[2];
	errwrap(pipe(pip));

	signal(SIGCHLD, child_notify);

	// initial pool of clients
	int maxclients = 0;
	int avail = 0;
	int i;
	for (i = 0; i != MAXCLIENTS_INIT; i++) {
		child_services(s, pip);
		avail++;
		maxclients++;
	}


	struct pollfd fds[1];
	fds[0].fd = pip[0];
	fds[0].events = POLLIN;

	while (true) {
		char buf[MAXBUF];
		printf("Avail/total: %d/%d\n", avail, maxclients);
		int n = poll(fds, 1, INFTIM);
		while (true) {
			got_sig = 0;

			int pid, status;
			errwrap(pid = wait3(&status, WNOHANG, NULL));
			if (pid == 0)
				break;
			(void)bad_status(status, pid);
			child_services(s, pip);
			avail++;
		}
		if (n == 1) {
			ssize_t r = read(pip[0], buf, sizeof buf);
			avail -= r;
			if (avail >= THRESHOLD) 
				continue;

			for (i = 0; i != 5; i++) {
				child_services(s, pip);
				avail++;
				maxclients++;
			}
		}
	}
}
