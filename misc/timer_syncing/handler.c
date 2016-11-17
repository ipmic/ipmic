// IPMic timer syncing (handler)

#include <assert.h>
#include <signal.h>
#include <string.h> /* memset() */

/* in audio.h:
 *   `long` -> `snd_pcm_sframes_t`  'frames count
 *   `audio_get_current_frame()` -> snd_pcm_avail()
 *   `_extern_ frame_nsecs`  'how many nanoseconds a frame has
 *   `FRAME_ADJUST_NSECS`  How many nanoseconds should a frame adjust be
 */
#include "audio.h"
#include "timer.h"

/* signal action structure */
static struct sigaction sact;

/* frame which we expect to be when timer interrupt happens */
static long expected_frame_min;
static long expected_frame_max;

/* current round count  (round = period) */
static unsigned long round;
/* last round which we needed to adjust timer's interrupt-time */
static unsigned long last_adjust_round;

/**
 * @param snum  signal number
 * @param sinf  signal info (man 2 sigaction)
 * @param uctx  user context (not used)
 */
static void
sigusr2_handler (int snum, siginfo_t *sinf, void *uctx) {
	/* here we get current frame with audio_get_current_frame()
	 * and syncronize timer signal/interrupt */

	/* in IPMic:
	 * >  first check (assert) if an expiration has occured "in the middle"
	 * >  get current audio frame and check if it is between acceptable
	 *    limits (`expected_frame_min` and `expected_frame_max`)
	 * >  verificate if timer was adjusted in last round. if yes, restore it
	 * >  check if we're about to be context-switched
	 * >  require each server to mix its own audio into a buffer
	 *
	 * NOTES: we must send a broadcast signal to all servers waiting
	 *        for audio packet from network.
	 *        and then require that each server mixes its own audio data
	 *        into mmaped sound-playback buffer.
	 *        >>implementing what was mentioned above could be
	 *          done with mutexes, rwlocks?
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
	 * from signal handler ( timer_settime() ) is safe, as described in:
	 * <http://pubs.opengroup.org/onlinepubs/9699919799/functions/
	 * V2_chap02.html> */
#define ALLOW_MULTIPLE_ADJUSTS
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
	// TODO

	/* require each server to mix its own audio into a buffer */
	/* ok, I know that calling pthread_cond_broadcast() in a signal handler 
	 * is unsafe, but we don't expect a call to pthread_cond_wait() here.
	 * Every call to pthread_cond_wait() must occur exactly after this
	 * signal handler ends.
	 */
	// TODO

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
	sact.sa_flags = SA_NODEFER | SA_SIGINFO;

	if ( sigaction(SIGUSR2, (const struct sigaction*) &sact, NULL) == -1 ) {
		// we die here ...
		return -1;
	}
	/* timer is not initialized yet! ok?!
	 * we should not expect for SIGUSR2 signal here ... */

	// NOTE more to do?
	return 0;
}
