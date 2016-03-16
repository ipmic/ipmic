#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "network.h"

#define DEFAULT_FAMILY AF_INET

/**
 * @p socket_type example: SOCK_STREAM, SOCK_DGRAM
 * @p port port number
 * @p addr if NULL: socket_addr = 0; else: socket_addr = addr; (almost it)
 */
NetLayer*
netlayer_new(int socket_type, uint16_t port, const char *addr)
{
	NetLayer *nl;

	int tmpsfd;

	if((nl = malloc(sizeof(NetLayer))) == NULL)
		return NULL;

	nl->socket_type = socket_type;

	if((nl->socket_fd = socket(DEFAULT_FAMILY, nl->socket_type, 0)) == -1)
		goto _go_free;

	nl->socket_addr.sin_family = DEFAULT_FAMILY;
	nl->socket_addr.sin_port = (in_port_t) htons(port);

	if(nl->socket_type == SOCK_STREAM)
		setsockopt(nl->socket_fd, IPPROTO_TCP, TCP_NODELAY, NULL, 0);

	if(addr == NULL) /* server mode? */
	{
		nl->socket_addr.sin_addr.s_addr = INADDR_ANY;

		if(bind(nl->socket_fd, (struct sockaddr*) &nl->socket_addr,
	sizeof(struct sockaddr_in)) == -1)
			goto _go_close;

		if(nl->socket_type == SOCK_STREAM)
		{
			if(listen(nl->socket_fd, 1) == -1)
				goto _go_close;

			if((tmpsfd = accept(nl->socket_fd, NULL, NULL)) == -1)
				goto _go_close;

			close(nl->socket_fd);
			nl->socket_fd = tmpsfd;
		}
	}
	else /* client mode */
	{
		if((nl->socket_addr.sin_addr.s_addr = inet_addr(addr)) == -1)
			goto _go_close;

		if(connect(nl->socket_fd, (struct sockaddr*) &nl->socket_addr,
	sizeof(struct sockaddr_in)) == -1)
			goto _go_close;
	}

	return nl;

// exit with error
_go_close:
	close(nl->socket_fd);

_go_free:
	free(nl);

	return NULL;
}

int
netlayer_free(NetLayer *nl)
{
	if(nl == NULL)
		return -1;

	close(nl->socket_fd);

	free(nl);

	return 0;
}

/*
 * The `recv()` function masked
 */
inline ssize_t
netlayer_recv(int sockfd, void *buf, size_t len, int flags)
{
	return recv(sockfd, buf, len, flags);
}

/*
 * The `send()` function masked
 */
inline ssize_t
netlayer_send(int sockfd, const void *buf, size_t len, int flags)
{
	return send(sockfd, buf, len, flags);
}
