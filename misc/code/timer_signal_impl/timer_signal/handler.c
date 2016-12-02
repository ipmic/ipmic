// IPMic timer-signal (the signal handler)

/* for laypeople:
 * o A "timer" here is something that sends a timer-signal to a system process
 *   (thread) every timer-expiration (this last defined by a timeout).
 * o A timer-signal causes signal handler to be executed :-) */

#include <assert.h>
#include <signal.h>
#include <string.h> /* memset() */

/* we expect in audio.h:
 *   `long` -> `snd_pcm_sframes_t`  (frames counting, Ex: `frames_to_adjust`)
 *   `audio_get_current_frame()` -> snd_pcm_avail()
 *   `_extern_ frame_nsecs`  (how many nanoseconds a frame has)
 *   `FRAME_ADJUST_NSECS`  How many nanoseconds should a "frame adjust" be
 */
#include "audio.h"
#include "timer.h"
#include "wakeup_servers.h"

/* signal action structure */
static struct sigaction sact;

/* frame range which we expect to be when signal handler run */
static long expected_frame_min;
static long expected_frame_max;

/* current round count  (round = audio period) */
static unsigned long round;
/* last round which we have adjusted timer-expiration */
static unsigned long last_adjust_round;

/**
 *                           >> THE SIGNAL HANDLER <<
 * @param snum  signal number
 * @param sinf  signal info (man 2 sigaction)
 * @param uctx  user context (not used)
 */
static void
sigusr2_handler (int snum, siginfo_t *sinf, void *uctx) {
	/* digest: here we get current frame with audio_get_current_frame()
	 *         and syncronize timer-expiration */

	/* in pratice:
	 * > First check (assert) if an expiration has occured "in the middle".
	 * > Get current audio frame and check if it is between acceptable
	 *   range (`expected_frame_min` and `expected_frame_max`).
	 * > Verify if timer was adjusted in last round. If yes, restore it
	 *   (or no, see ALLOW_MULTIPLE_ADJUSTS below).
	 * > [forget this] Check if we're about to be context-switched
	 * > Require each server to mix its own audio into a buffer.
	 *
	 * NOTES: We must send a broadcast signal to all servers waiting
	 *        for audio packet from network. And then require that each
	 *        server mixes its own audio data into mmaped sound-playback
	 *        buffer.
	 *        >> implementing what was mentioned above could be
	 *           done with mutexes, conds?  NO! (see WHY_NOT_PTHREADS.txt)
	 */

	long current_frame; /* result of snd_pcm_avail() */
	long frames_to_adjust; /* number of frames to adjust timer
				  (if necessary) */

	/* assertion -- no expiration has occured! */
	assert(timer_get_expirations() == 0);

	/* get current frame with --> snd_pcm_avail() */
	if ((current_frame = audio_get_current_frame()) < 0) {
		/* XXXX: an error here may be literally fatal!
		 *       at the moment I don't know how to recover so we must
		 *       break here! */
		exit(1);
	}

	/* and here comes the handler's main task: adjust (if necessary) timer's
	 * interrupt-time.
	 * we have two implementations
	 * 1st: ALLOW_MULTIPLE_ADJUSTS
	 *	if timer interval was already adjusted in previous round and
	 *	we still are out of expected range, we adjust timer interval
	 *	again.
	 * 2nd: DOESN'T ALLOW_MULTIPLE_ADJUSTS
	 *	if timer interval was already adjusted in previous round and
	 *	we still are out of expected range, we do nothing!
	 * what implementation is most efficient? ___test miss ratio!___
	 *
	 * Another thing that needs to be mentioned here is that setting timer
	 * from signal handler ( timer_settime() ) is safe, as described in
	 * signal (7) manual:
	 * <http://man7.org/linux/man-pages/man7/signal.7.html> */
#ifdef ALLOW_MULTIPLE_ADJUSTS
	if (current_frame < expected_frame_min ||
	    current_frame > expected_frame_max) {
		/* here we must adjust timer's interrupt-time */
		frames_to_adjust = current_frame - expected_frame;
		timer_adjust_interval(frames_to_adjust * FRAME_ADJUST_NSECS);
		last_adjust_round = round;
	} else if (last_adjust_round == (round - 1))
		/* if we've adjusted last time, now we can restore to normal
		 * interval */
		timer_restore_interval();
#else /* << allow only one adjust! */
	if (last_adjust_round == (round - 1)) {
		if (current_frame > expected_frame_min &&
		    current_frame < expected_frame_max)
			timer_restore_interval();
	} else {
		if (current_frame < expected_frame_min ||
		    current_frame > expected_frame_max) {
			/* here we must adjust timer's interrupt-time */
			frames_to_adjust = current_frame - expected_frame;
			timer_adjust_interval(frames_to_adjust *
					      FRAME_ADJUST_NSECS);
			last_adjust_round = round;
		}
	}
#endif

	/* check if we're about to be context-switched */
	// nothing TODO?

	/* require each server to mix its own audio into a buffer */
	wakeup_servers(); /* see wakeup_servers.c */

	round++;
	return;
}

/**
 * @param efmin  value defining minimum value acceptable inside current_frame
 * 		 when timer interrupt occurs
 * @param efmax  value defining maximum value acceptable inside current_frame
 * 		 when timer interrupt occurs
 */
int
handler_setup (long efmin, long efmax) {
	expected_frame_min = efmin;
	expected_frame_max = efmax;

	/* initialize sigaction struct */
	memset(&sact, 0, sizeof(struct sigaction));
	sact.sa_sigaction = sigusr2_handler;
	sact.sa_flags = SA_SIGINFO;

	if ( sigaction(SIGUSR2, (const struct sigaction*) &sact, NULL) == -1 ) {
		// we die here ...
		return -1;
	}
	/* timer is not initialized yet and SIGUSR2 may not arrives here ... */

	// NOTE more to do?
	return 0;
}
