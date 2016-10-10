// IPMic select() test (client side)

#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <netinet/ip.h>
#include <stdio.h> /* for debug */
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#define PORT_TO_CONNECT 8080

struct sockaddr_in peer_saddr = {
	AF_INET,
};

int sfd;

struct timeval natural_latency; /* for example: time to record an audio
				   packet from a microphone.
				   here it's introduced artificially */

unsigned long misses = 0; /* how many packets doesn't arrive in time */

int
socket_init (const char *addr) {
	/* set port and address */
	peer_saddr.sin_port = htons(PORT_TO_CONNECT);
	if (!inet_aton(addr, (struct in_addr*) &peer_saddr.sin_addr))
		return -1;

	/* open socket and connect it to `peer_saddr` */
	if ((sfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		return -1;
	if (connect(sfd, (const struct sockaddr*) &peer_saddr,
	    sizeof(struct sockaddr_in)) == -1)
		goto _go_close_socket;

	return 0;
_go_close_socket:
	close(sfd);
	return -1;
}

int
main (int argc, char **argv) {
	fd_set wfds;
	struct timeval select_timeout;
	struct timeval select_timeout_arg;

	int retval;
	struct timeval last_timeout = {0, 0}; /* initialize to zero */

	struct timeval buffer; /* stores the time when packet is sent */

	if (argc < 2)
		return 1;

	/* prepare file descriptor set */
	FD_ZERO(&wfds);

	/* open socket and connect it to a remote address and port */
	if (socket_init(argv[1]) == -1)
		return 1;

	/* define and send our natural latency */
	natural_latency.tv_sec = 0;
	natural_latency.tv_usec = 50000; /* 0.05 second */
	write(sfd, (const void*) &natural_latency, sizeof(struct timeval));

	/* set socket in non-blocking mode */
	if (fcntl(sfd, F_SETFD, fcntl(sfd, F_GETFD, 0) | O_NONBLOCK) == -1)
		goto _go_close_socket;

	FD_SET(sfd, &wfds);

	/* define select timeout */
	select_timeout.tv_sec = 0;
	select_timeout.tv_usec = 10000; /* this should be considered in server's
					 delay */

	while (1) {
		/* generate natural latency */
		usleep(natural_latency.tv_usec);

		/* call select */
		memcpy(&select_timeout_arg, &select_timeout,
		       sizeof(struct timeval));
		assert(select_timeout_arg.tv_usec ||
		       select_timeout_arg.tv_sec);
		retval = select(sfd + 1, NULL, &wfds, NULL,
				&select_timeout_arg);
		if (retval == -1) {
			perror("select()");
			goto _go_close_socket;
		} else if (!retval) {
			misses++;
			gettimeofday(&last_timeout, NULL);
			printf("[miss] packet dropped\n"
			       "       misses = %ld\n", misses);
			continue;
		}

		/* call write */
		gettimeofday(&buffer, NULL);
		retval = write(sfd, (const void*) &buffer,
			       sizeof(struct timeval));
		if (retval == -1) {
			perror("write()");
			goto _go_close_socket;
		} else if (!retval || retval < sizeof(struct timeval)) {
			printf("no data sent\n");
			continue;
		}
	}

_go_close_socket:
	close(sfd);
	return 1;
}

#if 0
	/* We cannot send timeout minus any value because it will create a
	   growing latency */
	int usecs_diff = USECS_DIFF;

	/* the code below do the timeout minus USECS_DIFF */
	if (!timeout.tv_sec) { /* if second value is zero */
		if (timeout.tv_usec < 1000) /* there remain only a few usecs */
			exit(1);
		timeout.tv_usec -= usecs_diff;
	} else if (timeout.tv_usec < usecs_diff) { /* if usecs is less than diff
*/		usecs_diff -= timeout.tv_usec;
		timeout.tv_sec--;
		timeout.tv_usec += 1000000 - usecs_diff;
	} else /* if usecs is not less than diff */
		timeout.tv_usec -= usecs_diff;
#endif
