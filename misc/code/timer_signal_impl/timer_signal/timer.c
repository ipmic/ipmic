// IPMic timer-signal (the timer management)
// Copyright (C) 2016  Ricardo Biehl Pasquali <rbpoficial@gmail.com>

/* making it clear:
 * > the syncing procedure should be made every "period clock".
 * > "period clock" is represented by timer-expiration.
 */

/*   link with -lrt */
#include <signal.h>
#include <time.h>

static struct sigevent sigev = {
	.sigev_notify = SIGEV_SIGNAL,
	.sigev_signo = SIGUSR2,
	.sigev_value = NULL, /* value to pass to handler */
};
static timer_t timerid;

static struct itimerspec default_interval;


static inline void
do_nsec_add (long nsecs, struct timespec *ts) {
	/* TODO here we add nsecs to ts */
	
}

/* timer API: */

int
timer_get_expirations (void) {
	return timer_getoverrun(timerid);
}

int
timer_adjust_interval (long nsecs) {
	struct itimerspec timerspec;

	timerspec = default_interval;
	do_nsec_add(nsecs, &timerspec.it_value); /* inline function */

	return timer_settime(timerid, 0, &timerspec, NULL);
}

int
timer_restore_interval (void) {
	/* flag TIMER_ABSTIME may be interesting (man 2 timer_settime) */
	return timer_settime(timerid, 0, &default_interval, NULL);
}

int
timer_setup (long nsecs) {
	struct timespec ts;

	if (timer_create(CLOCK_MONOTONIC, &sigev, &timerid) == -1)
		return -1;

	ts.tv_sec = 0; /* `tv_sec` field is not used */
	ts.tv_nsec = nsecs;

	default_interval.it_interval = ts;
	default_interval.it_value = ts;

	return 0;
}
