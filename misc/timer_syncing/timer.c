// IPMic timer syncing (timer)

/* -> syncing process is made every "period clock".
 * -> "period clock" is each time we receive a signal.
 */

/*   link with -lrt */
#include <signal.h>
#include <time.h>

static struct sigevent sigev = {
	.sigev_notify = SIGEV_SIGNAL,
	.sigev_signo = SIGUSR2,
	.sigev_value = NULL, /* TODO value to pass to handler */
};
static timer_t timerid;

/*
 * it's like I love prototypes :-)
 */
/* update timer interval for a possible synchronization with another source */
int
timer_update_interval(struct itimerspec);
/* create a new timer */
int
timer_new(void);
// timer_create(CLOCK_MONOTONIC, struct sigevent *, timer_t *);
