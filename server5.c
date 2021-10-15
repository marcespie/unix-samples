// with an actual signal handler
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

#include "myfuncs.h"

void
usage()
{
	errx(1, "Usage: server5 [-d] service");
}

int 
create_server(const char *service, bool debug)
{
	struct addrinfo hints, *res, *res0;
	int error;
	int s = -1;

	memset(&hints, 0, sizeof(hints));
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

// signals handlers have a LOT of restrictions
// - you can't really use the libc... if you don't play with buffering
// modes, printing to stderr is okay (because there is NO buffering, so
// no shared memory with the main process)
// - system calls are generally okay, but you must preserve the global errno
// - ALSO: if you want to modify ANY variable that's outside of the handler's
// scope (globals...), that variable MUST be volatile sig_atomic_t
// (volatile so that the compiler knows something funky is going on and WILL
// actually obey read/writes instead of optimizing them away, and
// sig_atomic_t, because otherwise your handler may come in the middle
// of the main program writing something to that variable, which may have
// some VERY funny consequences.

// also note: if you have global data structures that signal handlers want
// to peek at, you can protect them by blocking signals temporarily.
// See sigprocmask(2) for starters.
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
	// XXX if we don't do this, one of the "main" program syscall
	// may error out with ECHILD, which would be strange !
	errno = save_errno;
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
	signal(SIGCHLD, reaper);
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
