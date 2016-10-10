// IPMic select() test (server side)

/* FD_SETSIZE = 1024 (No problem) */

#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <netinet/ip.h>
#include <stdio.h> /* for debug */
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define PORT_TO_BIND 8080
#define BUFFER_SIZE 1024

struct sockaddr_in local_saddr = {
	AF_INET,
};

int sfd;

struct timeval select_timeout; /* it's natural latency
				  (see select_client.c) */

unsigned long misses = 0; /* how many packets doesn't arrive in time */

int
socket_init (void) {
	/* set port and address */
	local_saddr.sin_port = htons(PORT_TO_BIND);
	local_saddr.sin_addr.s_addr = INADDR_ANY;

	/* open socket and bint it to local port */
	if ((sfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		return -1;
	if (bind(sfd, (const struct sockaddr*) &local_saddr,
		 sizeof(struct sockaddr_in)) == -1)
		goto _go_close_socket;

	return 0;
_go_close_socket:
	close(sfd);
	return -1;
}

/* this function gets select_timeout (natural latency) from client */
void
get_timeout (void) {
	char buffer[sizeof(struct timeval)];

	/* this read() below blocks */
	if (read(sfd, buffer, sizeof(struct timeval)) != sizeof(struct timeval))
		select_timeout.tv_sec = select_timeout.tv_usec = 0;
	else
		memcpy(&select_timeout, buffer, sizeof(struct timeval));

	return;
}

int
main (int argc, char **argv) {
	fd_set rfds;

	struct timeval select_timeout_arg; /* argument to pass to select() */

	int retval;
	struct timeval last_timeout = {0, 0}; /* initialize to zero */

	char buffer[BUFFER_SIZE];
	struct timeval packet_time;

	/* prepare file descriptor set */
	FD_ZERO(&rfds);

	/* initialize socket in blocking mode and bind it to a local port */
	if (socket_init() == -1)
		return 1;

	/* get timeout value from peer */
	get_timeout();
	if (!(select_timeout.tv_sec || select_timeout.tv_usec))
		goto _go_close_socket;

	/* set socket in non-blocking mode */
	if (fcntl(sfd, F_SETFD, fcntl(sfd, F_GETFD, 0) | O_NONBLOCK) == -1)
		goto _go_close_socket;

	FD_SET(sfd, &rfds);

	printf("natural latency = %ld seconds and %ld microseconds\n",
	       select_timeout.tv_sec, select_timeout.tv_usec);

	/* introduce a delay here and (maybe) we won't have packets missing */
	usleep(20000); /* see select_timeout variable in select_client.c */

	while (1) {
		/* call select */
		memcpy(&select_timeout_arg, &select_timeout,
		       sizeof(struct timeval));
		assert(select_timeout_arg.tv_usec ||
		       select_timeout_arg.tv_sec);
		retval = select(sfd + 1, &rfds, NULL, NULL,
				&select_timeout_arg);
		if (retval == -1) {
			perror("select()");
			goto _go_close_socket;
		} else if (!retval) {
			/* here we expect other side has evicted the packet */
			misses++;
			gettimeofday(&last_timeout, NULL);
			printf("[miss] packet dropped\n"
			       "       misses = %ld\n", misses);
			continue;
		}

		/* call read */
		retval = read(sfd, buffer, BUFFER_SIZE);
		if (retval == -1) {
			perror("read()");
			goto _go_close_socket;
		} else if (!retval || retval < sizeof(struct timeval)) {
			printf("no data received\n");
			continue;
		}

		usleep(select_timeout_arg.tv_usec); /* remaining time */

		/* let's handle data  first checking if packet has expired */
		memcpy(&packet_time, buffer, sizeof(struct timeval));
		if (packet_time.tv_sec < last_timeout.tv_sec ||
		    (packet_time.tv_sec == last_timeout.tv_sec &&
		     packet_time.tv_usec < last_timeout.tv_usec))
			continue;
	}

_go_close_socket:
	close(sfd);
	return 1;
}
