// modern internet !
#include <stdbool.h>
#include <err.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <getopt.h>
#include <string.h>

#include "myfuncs.h"

void
usage()
{
	errx(1, "Usage: server4 [-d] service");
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

	// note we may still pass a port name (string)
	// OR a service (equivalent to getservbyname, cf
	// /etc/services)
	error = getaddrinfo(NULL, service, &hints, &res0);
	if (error)
		errx(1, "%s", gai_strerror(error));
	// this is (more or less) the simplest way to use getaddrinfo,
	// because we're creating a single server on the first address
	// and we don't really do anything with fun errors.
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

	// show what family we bound... if we reuse the same port
	// without -d, we may switch from IPv4 to IPv6
	//
	// Specific family depends a lot on OS configs: some OSes
	// have IPv4 "passthrough" from V6, others will prefer V4/V6
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
	signal(SIGCHLD, SIG_IGN);
	while (1) {
		int fd;

		errwrap(fd = accept(s, NULL, 0));
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
