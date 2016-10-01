/*
 * IPMic, The IP Microphone project
 * Copyright (C) 2016  Ricardo Biehl Pasquali <rbpoficial@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "network.h"

#define DEFAULT_FAMILY AF_INET

netparam_t nlp;
static netlayer_t nl;

//int socket_priority = 6;  6 = high priority (TCP only)

int
netlayer_open (void)
{
	struct sockaddr_in socket_addr;
	int tmp_sfd; /* for TCP protocol */

	if ((nl = socket(DEFAULT_FAMILY, nlp.socket_type, 0)) == -1)
		return -1;

	socket_addr.sin_family = DEFAULT_FAMILY;
	socket_addr.sin_port = htons(nlp.port);

	if (nlp.addr == NULL) { /* if NULL we're in server mode */
		socket_addr.sin_addr.s_addr = INADDR_ANY;

		if (bind(nl, (struct sockaddr*) &socket_addr,
		    sizeof(struct sockaddr_in)) == -1)
			goto _go_close_socket;

		if (nlp.socket_type == SOCK_STREAM) {
			if (listen(nl, 1) == -1)
				goto _go_close_socket;
			if ((tmp_sfd = accept(nl, NULL, NULL)) == -1)
				goto _go_close_socket;

			close(nl);
			nl = tmp_sfd;
		}
	}
	else { /* else we're in client mode */
		if ((socket_addr.sin_addr.s_addr = inet_addr(nlp.addr)) == -1)
			goto _go_close_socket;

		if (connect(nl, (struct sockaddr*) &socket_addr,
		    sizeof(struct sockaddr_in)) == -1)
			goto _go_close_socket;
	}

	if(nlp.socket_type == SOCK_STREAM)
		/* set TCP_NODELAY option */
		setsockopt(nl, IPPROTO_TCP, TCP_NODELAY, NULL, 0);

	//setsockopt(nl, SOL_SOCKET, SO_PRIORITY, &socket_priority,sizeof(int));

	return 0;

_go_close_socket:
	close(nl);
	return -1;
}

int
netlayer_close (void)
{
	return close(nl);
}

ssize_t
netlayer_recv (void *buf, size_t len, int flags)
{
	ssize_t res, ret;

	ret = len;

	while (len) {
		if((res = read(nl, buf, len)) < 0)
			return res; /* `res` serves to parse result */
		len -= res;
		buf += res;
	}

	return ret;
}

ssize_t
netlayer_send (const void *buf, size_t len, int flags)
{
	ssize_t res, ret;

	ret = len;

	while (len) {
		if ((res = write(nl, buf, len)) < 0)
			return res;
		len -= res;
		buf += res;
	}

	return ret;
}
