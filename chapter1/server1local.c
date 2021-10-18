#include <stdbool.h>
#include <err.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "myfuncs.h"

// how to bind a unix domain socket...
// rules are somewhat drastic:
// - don't create sockets on NFS filesystems
// - names can only be 104 characters long
// - the socket (path) "stays around" when you die unless you clean it up

// hence the global
struct sockaddr_un path;

// along with the cleanup stuff for signals and exit
void
byebyesig(int sig)
{
	unlink(path.sun_path);
	signal(sig, SIG_DFL);
	kill(getpid(), sig);
}

void
byebye()
{
	unlink(path.sun_path);
}

int
main(int argc, char *argv[])
{
	if (argc < 2)
		errx(1, "pass socket name as a parameter");

	if (strlen(argv[1])+1 > sizeof(path.sun_path))
		errx(1, "socket name too long");

	int s;

	// unix domain sockets show up on the filesystem
	errwrap(s = socket(AF_UNIX, SOCK_STREAM, 0));


	// XXX the sun_len field is NOT portable, linux doesn't have it
#ifndef __linux__
	path.sun_len = sizeof(path);
#endif
	path.sun_family = AF_UNIX;
	strcpy(path.sun_path, argv[1]);

	// we're ready to bind so, get ready to clean up in advance
	// those are the 4 usual signals for "normal" process termination
	signal(SIGHUP, byebyesig);
	signal(SIGINT, byebyesig);
	signal(SIGQUIT, byebyesig);
	signal(SIGTERM, byebyesig);
	atexit(byebye);

	errwrap(bind(s, (const struct sockaddr *)&path, sizeof(path)));
	// by default, the local socket will be accessible by anyone
	// this is the place to change the rights: nobody can connect
	// before listen is in effect, AND you can't change the modes
	// before the name is in the filesystem
	errwrap(chmod(path.sun_path, 0600));
	// (note that we would expect fchmod to work, but it does not!
	// the socket implementation is different, and the vfs ops for
	// the "file" do NOT include fchmod, so we have to use the name
	// note that there's no race here, since we are in full control
	// of the name (hopefully)

	// there are macros for modes in sys/stat.h, but frankly, 0600
	// is colloquial and understood by any unix programmer.
	// like 0 and 1 for standard input/output, almost no-one uses
	// these macros ;)

	errwrap(listen(s, 128));

	// IP sockets are accessible through telnet(1) or nc(1),
	// Unix domain sockets can be talked to with nc -U
	while (1) {
		int fd;

		errwrap(fd = accept(s, NULL, 0));
		FILE *f = fdopen(fd, "w");
		if (!f)
			err(1, "fdopen");

		fprintf(f, "The time is %zd\r\n", (ssize_t)(time(NULL)));
		fclose(f);
	}
}
