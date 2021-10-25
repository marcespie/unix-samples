// the weird semantics of shutdown
#include <err.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "myfuncs.h"


// you should run tcpdump on the side
// something like: tcpdump -i lo0
int
main()
{
	struct addrinfo hints, *res, *res0;
	int error;
	int s = -1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	error = getaddrinfo("localhost", "ssh", &hints, &res0);
	if (error)
		errx(1, "%s", gai_strerror(error));
	for (res = res0; res; res = res->ai_next) {
		s = socket(res->ai_family, res->ai_socktype,
		    res->ai_protocol);
		if (s == -1)
			continue;
		if (connect(s, res->ai_addr, res->ai_addrlen) == -1) {
			warn("connect");
			close(s);
			s = -1;
			continue;
		}
		break;
	}
	if (s == -1)
		fprintf(stderr, "no ssh running here\n");
	int pid;

	errwrap(pid = fork());
	// so this is the fun thing: most fds in the system are shared
	// between processes: when you call close(), the underlying 
	// file/socket does NOT get closed
	if (pid == 0) {
		shutdown(s, SHUT_WR);
		exit(0);
	}
	pause();
}

