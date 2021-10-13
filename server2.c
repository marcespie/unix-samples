// first use of exec with redirection
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

#include "myfuncs.h"

int 
create_server(int port)
{
	// old-style server setup 
	// DO NOT DO THINGS THAT WAY IN NEW CODE!!! It's just there
	// to explain things
	int s;

	errwrap(s = socket(AF_INET, SOCK_STREAM, 0));


	struct sockaddr_in ipv4;
	// XXX the sin_len field is NOT portable, linux doesn't have it
#ifndef __linux__
	ipv4.sin_len = sizeof(ipv4);
#endif
	ipv4.sin_family = AF_INET;
	// endianness: network info is often passed from host to host,
	// so by convention, socket addresses are "network ready"
	// use host to network short (e.g., short == 16 bits in this context)
	ipv4.sin_port = htons(port);
	// usually we want to listen on "any" available ipv4 interface
	ipv4.sin_addr.s_addr = INADDR_ANY;
	errwrap(bind(s, (const struct sockaddr *)&ipv4, sizeof(ipv4)));

	errwrap(listen(s, 128));
	return s;
}

int
main(int argc, char *argv[])
{
	if (argc < 2)
		errx(1, "pass port number as a parameter");
	int port = atoi(argv[1]);
	if (port <= 0)
		errx(1, "%s is not a valid port", argv[1]);

	if (port < 1024 && getuid() != 0)
		errx(1, "only root can bind port # <1024");

	int s = create_server(port);
	// server loop, sequential client service
	// XXX in this very specific case, we don't care about our children
	// this avoids creating zombies entirely
	signal(SIGCHLD, SIG_IGN);
	while (1) {
		int fd;

		errwrap(fd = accept(s, NULL, 0));
		// let's delegate the proper printing of time to an external
		// program
		// so we have to fork
		int pid;
		errwrap(pid = fork());
		if (pid == 0) {
			// since this is external we need to redirect
			if (fd != 1) {
				errwrap(dup2(fd, 1));
				// close the original fd to be "clean"
				// date won't care but it's still a good idea
				// to avoid having too many fds
				errwrap(close(fd));
			}
			execlp("date", "date", NULL);
			// if exec succeeds it never returns
			err(1, "exec");
		}
		// XXX if we forget to close our fd the
		// client will never finish: a network connection
		// only gets closed when every fd that refers to it is gone.
		errwrap(close(fd));
	}
}
