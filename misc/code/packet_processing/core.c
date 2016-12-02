// IPMic packet processing (core)

#include <string.h>
#include <time.h>

#include "core.h"

#define packet_has_expired(ptime, ctime) \
	( ptime.tv_sec < ctime.tv_sec || \
	  (ptime.tv_sec == ctime.tv_sec && ptime.tv_nsec < ctime.tv_nsec) )

#define PACKET_HEADER_SIZE (sizeof(struct timespec)) /* we could add something
							else here */


/**
 * Return 0 on success or a negative error code
 * @param buffer  packet buffer
 * @param len  packet len
 */
/* NOTE should __current_time__ be an argument? */
int
packet_is_ok (void *buffer, size_t len) {
	struct timespec ctime; /* __current_time__ */
	struct timespec ptime; /* packet-sent-time */

	if (len < PACKET_HEADER_SIZE)
		return -ERROR_HDRINV;

	/* check if packet has expired */
	memcpy((void*) &ptime, (const void*) buffer, sizeof(struct timespec));
	clock_gettime(CLOCK_MONOTONIC_RAW, &ctime); /* get current time */
	if (packet_has_expired(ptime, ctime))
		return -ERROR_TSINV;

	/* if header grows, we simply add size in PACKET_HEADER_SIZE and
	 * increments buffer with an offset */

	return 0;
}
