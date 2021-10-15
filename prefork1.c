// prefork example
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

// one fairly popular technique is the "prefork" technique:
// instead of fork()ing on every client connection, which is
// slow, we fork ahead of time (so we already paid the piper)
// and we call accept in each child: one of the children will
// get the accept and handle the call.
void
child_services(int s)
{
	int pid;
	errwrap(pid = fork());
	if (pid == 0) {
		int fd;

		errwrap(fd = accept(s, NULL, 0));
		if (fd != 1) {
			errwrap(dup2(fd, 1));
			errwrap(close(fd));
		}
		execlp("date", "date", NULL);
		err(1, "exec");
	}
}

#define MAXCLIENTS 25
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

	// initial pool of clients
	int i;

	for (i = 0; i != MAXCLIENTS; i++)
		child_services(s);

	while (true) {
		// and of course we need to replenish each connection that ends
		int pid, status;
		errwrap(pid = wait(&status));
		(void)bad_status(status, pid);
		child_services(s);
	}
}
// further notes for reflexion:
// - since each accept is independent, supporting multiple address spaces
// is simple: we just need to allocate some clients for each socket
// - finding out the proper pool size is tricky. One thing we can do is
// monitor what's going on: create a pipe, and have each child write a bit
// of info when it becomes active... Tracking pids for each pool becomes
// more or less necessary (in any case like we talked about loooong ago, we
// may have other processes around, so sooner or later, we need to catch
// only "our" processes.
// The main code can then call poll() on the pipe (it will get interrupted
// by signals remember ?) and so we can easily know the occupancy ratio of
// each pool.
// (okay, please be careful when handling signals around blocking calls,
// it is fairly easy to "lose" a signal -- well, not quite lose, but have
// it arrive at the "wrong time", right after you figured you didn't get
// any signal...  interaction between blocking calls like poll and wait
// and signals is one of the trickiest part of basic Unix programming!
