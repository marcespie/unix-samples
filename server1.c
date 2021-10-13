#include <stdbool.h>
#include <err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "myfuncs.h"

int
main(int argc, char *argv[])
{
	// argv[0] is the program name
	if (argc < 2)
		errx(1, "pass port number as a parameter");
	// XXX we should properly use strtol and check for error
	int port = atoi(argv[1]);
	// (but in this case, zero is not a valid value)
	if (port <= 0)
		errx(1, "%s is not a valid port", argv[1]);

	if (port < 1024 && getuid() != 0)
		errx(1, "only root can bind port # <1024");

	// old-style server setup 
	// DO NOT DO THINGS THAT WAY IN NEW CODE!!! It's just there
	// to explain things "the old way"
	int s;

	errwrap(s = socket(AF_INET, SOCK_STREAM, 0));
	//                 ^IPv4    ^TCP


	struct sockaddr_in ipv4;
	// XXX the sin_len field is NOT portable, linux doesn't have it
#ifndef __linux__
	ipv4.sin_len = sizeof(ipv4);
#endif
	ipv4.sin_family = AF_INET;
	// endianess: network info is often passed from host to host,
	// so by convention, socket addresses are "network ready"
	// use host to network short (e.g., short == 16 bits in this context)
	ipv4.sin_port = htons(port);
	// usually we want to listen on "any" available ipv4 interface
	ipv4.sin_addr.s_addr = INADDR_ANY;
	// this is OO in C
	errwrap(bind(s, (const struct sockaddr *)&ipv4, sizeof(ipv4)));
	//              ^ horrible cast

	errwrap(listen(s, 128));
	//                ^ nothing really cares about this
	// (the kernel will choose a proper queue size)

	// server loop, sequential client service
	while (1) {
		int fd;

		// note that errors will kill the server dead!
		errwrap(fd = accept(s, NULL, 0));
		// a few points of note
		// 1/ we can get the client address, but here we don't care
		// the correct way is to use sockaddr_storage, so that it
		// can evolve later
		// 2/ if we lag, it's not a big issue: the kernel establishes
		// connections for us.
		FILE *f = fdopen(fd, "w");
		if (!f)
			err(1, "fdopen");

		fprintf(f, "The time is %zd\r\n", (ssize_t)(time(NULL)));
		fclose(f);
	}
}
