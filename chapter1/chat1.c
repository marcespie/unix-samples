// a simple interactive chat bot
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
	errx(1, "Usage: chat1 [-d] service");
}

#define MAXFDS 1024

// we store all the poll stuff in a single array
// [0, nservers[ -> server fds
// [nservers, nfd[ -> client fds

struct pollfd fds[MAXFDS];
size_t nservers;
size_t nfds;

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

		fds[nservers].fd = s;
		fds[nservers].events = POLLIN;
		nservers++;
		if (nservers > MAXFDS)
			errx(1, "Too many servers");
	}

	nfds = nservers;
	freeaddrinfo(res0);
	return nservers != 0;
}

#define MAXSIG 25
#define MAXBUF 1024

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

	// XXX there are lots of things we don't do yet, such as handling
	// disconnecting clients, or properly buffering messages to pass them
	// off correctly
	while (1) {
		char signature[MAXSIG];
		int n = poll(fds, nfds, INFTIM); 

		int i;
		for (i = 0; i != nservers; i++) {
			if (!(fds[i].revents & POLLIN))
				continue;

			int fd;
			errwrap(fd = accept(fds[i].fd, NULL, 0));
			fds[nfds].fd = fd;
			fds[nfds].events = POLLIN;
			nfds++;
			if (nfds == MAXFDS)
				errx(1, "Too many clients");
			snprintf(signature, MAXSIG, "Hello %d\n", fd);
			safe_write(fd, signature, strlen(signature));
		}
		for (i = nservers; i != nfds; i++) {
			if (fds[i].revents & POLLIN) {
				char buffer[MAXBUF];
				ssize_t n = read(fds[i].fd, buffer, 
				    sizeof buffer);
				if (n == -1)
					err(1, "read");
				int j;
				snprintf(signature, MAXSIG, "%d: ", fds[i].fd);
				for (j = nservers; j != nfds; j++) {
					// don't echo our own messages
					if (i == j)
						continue;
					safe_write(fds[j].fd, signature, 
					    strlen(signature));
					safe_write(fds[j].fd, buffer, n);
				}
			}
		}
	}
}
