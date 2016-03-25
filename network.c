#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "network.h"

#define DEFAULT_FAMILY AF_INET

int socket_priority = 6; /* 6 = high priority */

netlayer_t
netlayer_new(int socket_type, uint16_t port, const char *addr)
{
	netlayer_t nl;

	struct sockaddr_in socket_addr;

	int tmpsfd;

	if((nl = socket(DEFAULT_FAMILY, socket_type, 0)) == -1)
		goto _go_return_err;

	socket_addr.sin_family = DEFAULT_FAMILY;
	socket_addr.sin_port = (in_port_t) htons(port);

	/* Only works with UDP, TCP opens a new socket with accept()
	 * possible solution is move it down :-) */
	setsockopt(nl, SOL_SOCKET, SO_PRIORITY, &socket_priority, sizeof(int));

	if(addr == NULL) /* if NULL we're in server mode */
	{
		socket_addr.sin_addr.s_addr = INADDR_ANY;

		if(bind(nl, (struct sockaddr*) &socket_addr,
	sizeof(struct sockaddr_in)) == -1)
			goto _go_return_err;

		if(socket_type == SOCK_STREAM)
		{
			if(listen(nl, 1) == -1)
				goto _go_return_err;

			if((tmpsfd = accept(nl, NULL, NULL)) == -1)
				goto _go_return_err;

			close(nl);
			nl = tmpsfd;

			/* set TCP_NODELAY option */
			setsockopt(nl, IPPROTO_TCP, TCP_NODELAY, NULL, 0);
		}
	}
	else /* else we're in client mode */
	{
		if((socket_addr.sin_addr.s_addr = inet_addr(addr)) == -1)
			goto _go_return_err;

		if(connect(nl, (struct sockaddr*) &socket_addr,
	sizeof(struct sockaddr_in)) == -1)
			goto _go_return_err;
	}

	return nl;

_go_return_err:
	return -1;
}

inline int
netlayer_close(netlayer_t nl)
{
	return close(nl);
}

/*
 * The `recv()` function masked
 */
inline ssize_t
netlayer_recv(netlayer_t sockfd, void *buf, size_t len, int flags)
{
	return recv(sockfd, buf, len, flags);
}

/*
 * The `send()` function masked
 */
inline ssize_t
netlayer_send(netlayer_t sockfd, const void *buf, size_t len, int flags)
{
	return send(sockfd, buf, len, flags);
}
