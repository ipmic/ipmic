// IPMic select() test (server side)

/* FD_SETSIZE = 1024 (No problem) */

#include <fcntl.h>
#include <stdio.h> /* for debug */
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

#include "common.h"
#include "../transfer_common.h"

#define BUFFER_SIZE 1024

#define packet_has_expired(ptime, ltime) \
	( ptime.tv_sec < ltime.tv_sec || \
	  (ptime.tv_sec == ltime.tv_sec && ptime.tv_usec < ltime.tv_usec) )

static struct timeval select_timeout; /* it's natural latency
					 (see select_client.c) */

extern int sfd; /* socket file descriptor */
extern unsigned long misses; /* misses count */
extern int keep_running; /* keep looping? (see while loop in main ) */

static unsigned long drops = 0; /* how many packets were dropped */

/* this function gets select_timeout (natural latency) from client */
void
get_timeout (void) {
	char buffer[sizeof(struct timeval)];

	/* this read() below blocks */
	if (read(sfd, buffer, sizeof(struct timeval)) == sizeof(struct timeval))
		memcpy(&select_timeout, buffer, sizeof(struct timeval));
	else
		select_timeout.tv_sec = select_timeout.tv_usec = 0;

	return;
}

int
main (void) {
	fd_set rfds;
	struct timeval select_timeout_arg; /* argument to pass to select() */

	int retval;
	struct timeval last_timeout = {0, 0}; /* initialize it to zero */

	char buffer[BUFFER_SIZE];  /*  ->--.  */
	struct timeval packet_time;/*  -<--'  */

	/* prepare file descriptor set */
	FD_ZERO(&rfds);
	/* initialize socket in blocking mode and bind it to a local port */
	if (network_open(NULL) == -1)
		return 1;
	FD_SET(sfd, &rfds); /* add socket to fdset */
	/* set signal handler */
	register_sigint_handler();

	/* get timeout value from peer */
	get_timeout();
	if (!(select_timeout.tv_sec || select_timeout.tv_usec))
		goto _go_close_socket;

	/* set socket in non-blocking mode */
	if (fcntl(sfd, F_SETFD, fcntl(sfd, F_GETFD, 0) | O_NONBLOCK) == -1)
		goto _go_close_socket;

	printf("natural latency = %ld seconds and %ld microseconds\n",
	       select_timeout.tv_sec, select_timeout.tv_usec);

	/* introduce a delay here and (maybe) we won't have packets missing.
	 * here we add some microseconds foreseeing network latency that may
	 * come
	 *   >> see `select_timeout` variable in select_client.c */
	usleep(10000 + CLIENT_SELECT_TIMEOUT);

	while (keep_running) {
		/* call select */
		/* NOTE in an audio application we must synchronize playback
		 * with select() >>> snd_pcm_delay/avail()? <<< */
		memcpy(&select_timeout_arg, &select_timeout,
		       sizeof(struct timeval));
		retval = select(sfd + 1, &rfds, NULL, NULL,
				&select_timeout_arg);
		if (retval == -1) {
			perror("select()");
			goto _go_print_misses;
		} else if (!retval) {
			gettimeofday(&last_timeout, NULL);
			print_misses_each_interval();
			/* here we expect other side has evicted the packet */
			misses++;
			continue; /* skip read() (see NOTE above) */
		}

		/* call read */
		retval = read(sfd, buffer, BUFFER_SIZE);
		if (retval == -1) {
			perror("read()");
			goto _go_print_misses;
		} else if (!retval || retval < sizeof(struct timeval)) {
			printf("no data received\n");
			continue;
		}

		/* sleep for remaining time  Is it necessary?
		 * In an audio application for example we would get
		 * current frame with `snd_pcm_avail()` and wait until
		 * period ends (in this case it should be done only in first
		 * time) */
		usleep(select_timeout_arg.tv_usec);

		/* let's handle data  first checking if packet has expired */
		memcpy(&packet_time, buffer, sizeof(struct timeval));
		if (packet_has_expired(packet_time, last_timeout)) {
			drops++;
			continue;
		}

		/* =========================================
		 * =AND=HERE=AN=APPLICATION=HANDLE=ITS=DATA=
		 * ========================================= */
	}

_go_print_misses:
	printf("drops = %ld\n", drops);
	if (misses)
		printf("misses = %ld\n", misses);
	else
		printf("No misses :-)\n");
_go_close_socket:
	close(sfd);
	return 1;
}
