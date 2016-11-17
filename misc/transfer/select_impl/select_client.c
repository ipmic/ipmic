// IPMic select() test (client side)

#include <fcntl.h>
#include <stdio.h> /* for debug */
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#include "common.h"
#include "../transfer_common.h"

static struct timeval natural_latency; /* for example: time to record an audio
					  packet from a microphone.
					  here it's introduced artificially */

extern int sfd; /* socket file descriptor */
extern unsigned long misses; /* misses count */
extern int keep_running; /* keep looping? (see while loop in main ) */

int
main (int argc, char **argv) {
	fd_set wfds;
	struct timeval select_timeout;
	struct timeval select_timeout_arg;

	int retval;
	struct timeval last_timeout = {0, 0}; /* initialize it to zero */

	struct timeval buffer; /* stores the time when packet is sent */

	if (argc < 2) {
		printf("usage: %s <address>\n", argv[0]);
		return 1;
	}

	/* prepare file descriptor set */
	FD_ZERO(&wfds);
	/* open socket and connect it to a remote address and port */
	if (network_open(argv[1]) == -1)
		return 1;
	FD_SET(sfd, &wfds); /* add socket to fdset */
	/* set signal handler */
	register_sigint_handler();

	/* define and send our natural latency */
	natural_latency.tv_sec = 0;
	natural_latency.tv_usec = NATURAL_LATENCY;
	write(sfd, (const void*) &natural_latency, sizeof(struct timeval));

	/* set socket in non-blocking mode */
	if (fcntl(sfd, F_SETFD, fcntl(sfd, F_GETFD, 0) | O_NONBLOCK) == -1)
		goto _go_close_socket;

	/* define select timeout */
	select_timeout.tv_sec = 0;
	/* this should be considered in server's delay */
	select_timeout.tv_usec = CLIENT_SELECT_TIMEOUT;

	while (keep_running) {
		/* generate natural latency */
		usleep(natural_latency.tv_usec);

		/* call select */
		memcpy(&select_timeout_arg, &select_timeout,
		       sizeof(struct timeval));
		retval = select(sfd + 1, NULL, &wfds, NULL,
				&select_timeout_arg);
		if (retval == -1) {
			perror("select()");
			goto _go_print_misses;
		} else if (!retval) {
			gettimeofday(&last_timeout, NULL);
			print_misses_each_interval();
			/* we expect other side will evict a packet */
			misses++;
			continue; /* skip write()
				     an overrun will occur if it's an audio
				     application */
		}

		/* call write */
		gettimeofday(&buffer, NULL);
		retval = write(sfd, (const void*) &buffer,
			       sizeof(struct timeval));
		if (retval == -1) {
			perror("write()");
			goto _go_print_misses;
		} else if (!retval || retval < sizeof(struct timeval)) {
			printf("no data sent\n");
			continue;
		}
	}

_go_print_misses:
	if (misses)
		printf("misses = %ld\n", misses);
	else
		printf("No misses :-)\n");
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
