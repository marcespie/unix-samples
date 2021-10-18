// modern internet: binding all address families
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
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
	errx(1, "Usage: server5 [-d] service");
}

// we pass a callback to the getaddrinfo loop which will be the server loop
// otherwise, we could store sockets in a table in order to do a pool on
// them (or something)
bool
create_servers(const char *service, bool debug, void (*server_loop)(int))
{
	bool success = false;
	struct addrinfo hints, *res, *res0;
	int error;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

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

		int pid;
		errwrap(pid = fork());
		if (pid == 0) {
			server_loop(s);
			exit(1);
		}
		close(s);
		success = true;
	}

	freeaddrinfo(res0);
	return success;
}


// this is the first time we get an actual process tree
// main controller with two children: IPv4 server/IPv6 server
// both run accept() in a loop and delegate each connection to a separate
// child that execs date (or something else)

// this architecture works just fine for servers where clients don't
// interact with each other...

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
	signal(SIGCHLD, reaper);
	while (1) {
		int fd;

		errwrap(fd = accept(s, NULL, 0));
		int pid;
		errwrap(pid = fork());
		if (pid == 0) {
			// if we want we could put a more interactive program
			// but beware that telnet(1) sends crlf (as per the
			// telnet protocol) and a lot of unix-y programs
			// expect a single lf at the end of each line...
			// so this is (more or less) a complicated mess to
			// handle !
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
    	if (create_servers(argv[0], debug, server_code)) {
		// pause is an old function (see sigsuspend)
		// but it's not really going away any time soon
		// we will get woken up if one of our servers goes kaput
		pause();
	} else {
		errx(1, "Couldn't create any server");
	}
}
