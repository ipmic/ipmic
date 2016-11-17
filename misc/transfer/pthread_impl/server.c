// IPMic pthread_impl (server side)

#include "../transfer_common.h"

extern int sfd; /* socket file descriptor */
extern unsigned long misses; /* misses count */
extern int keep_running; /* keep looping? (see while loop in main ) */

int
main (int argc, char **argv) {
	while (keep_running) {
		pthread_cond_wait()
	}
}
