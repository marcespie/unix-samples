#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <err.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>

#include "foo.h"

void
create_servers(struct myfds *myfds, const char *service, bool debug,
	size_t capacity)
{
	struct addrinfo hints, *res, *res0;
	int error;

	myfds->n = 0;
	myfds->servers = 0;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	error = getaddrinfo(NULL, service, &hints, &res0);
	if (error)
		errx(1, "%s", gai_strerror(error));

	myfds->pfds  = ereallocarray(NULL, sizeof(struct pollfd), capacity);
	myfds->capacity = capacity;
	myfds->n = 0;
	myfds->servers = 0;
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

		myfds->pfds[myfds->servers].fd = s;
		myfds->pfds[myfds->servers].events = POLLIN;
		myfds->servers++;
	}

	freeaddrinfo(res0);
	myfds->n = myfds->servers;
}

