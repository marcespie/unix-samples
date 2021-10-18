// setsockopt + debug mode
#include <stdbool.h>
#include <err.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <getopt.h>

#include "myfuncs.h"

void
usage()
{
	errx(1, "Usage: server3 [-d] port");
}

int 
create_server(int port, bool debug)
{
	// old-style server setup 
	// DO NOT DO THINGS THAT WAY IN NEW CODE!!! It's just there
	// to explain things
	int s;

	errwrap(s = socket(AF_INET, SOCK_STREAM, 0));

	// SO.... normally the kernel keeps the server socket around
	// for ~2 mn, just in case it needs to re-send the final FIN to
	// a client (so that the client doesn't get confused because the
	// sequence numbers MUST match)
	//
	// it's possible to disable that behavior BEFORE creating the server
	// note that this actually BREAKS TCP/IP slightly so this is debug
	// behavior!
	if (debug) {
		int p = 1;
		errwrap(setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &p, sizeof(p)));
	}

	struct sockaddr_in ipv4;
#ifndef __linux__
	ipv4.sin_len = sizeof(ipv4);
#endif
	ipv4.sin_family = AF_INET;
	ipv4.sin_port = htons(port);
	ipv4.sin_addr.s_addr = INADDR_ANY;
	errwrap(bind(s, (const struct sockaddr *)&ipv4, sizeof(ipv4)));

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
	// XXX we should properly use strtol and check for error
	int port = atoi(argv[0]);
	if (port == 0)
		errx(1, "%s is not a valid port", argv[0]);

	if (port < 1024 && getuid() != 0)
		errx(1, "only root can bind port # <1024");

	int s = create_server(port, debug);
	signal(SIGCHLD, SIG_IGN);
	while (1) {
		int fd;

		// note that errors will kill the server dead!
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
