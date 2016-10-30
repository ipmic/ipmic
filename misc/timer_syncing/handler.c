// IPMic timer syncing (handler)

#include <signal.h>
#include <string.h>

/* in audio.h:
 *   `long` -> `snd_pcm_sframes_t`  'frames count
 *   `audio_get_current_frame()` -> snd_pcm_avail()
 *   `_extern_ frame_nsecs`  'how many nanoseconds a frame has
 */
#include "audio.h"

/* signal action structure */
static struct sigaction sact;

/* frame which we expect to be when timer interrupt happens */
static long expected_frame = 0;

/* last round which we needed to adjust timer's interrupt-time */
static unsigned long last_adjust_round;
/* current round count */
static unsigned long round;

/**
 * @param snum  signal number
 * @param sinf  signal info (man 2 sigaction)
 * @param uctx  user context (not used?)
 */
static void
sigusr2_handler (int snum, siginfo_t *sinf, void *uctx) {
	/* here we get current frame with snd_pcm_avail()
	 * and syncronize with timer signal/interrupt */

	long current_frame; /* result of snd_pcm_avail() */

	if ((current_frame = audio_get_current_frame()) < 0) {
		// TODO what happens when an error occurs here?
	}

	/* and here comes the handler's main task:
	 * adjust (if necessary) timer's interrupt-time */
	/* it's so accurate! we need a margin ... */
	if (current_frame != expected_frame) {
		/* here we should adjust timer's interrupt-time */
	}

	return;
}

int
handler_setup (void) {
	/* initialize sigaction struct */
	memset(&sact, 0, sizeof(struct sigaction));
	sact.sa_sigaction = sigusr2_handler;
	sact.sa_flags = SA_NODEFER | SA_SIGINFO;

	if ( sigaction(SIGUSR2, (const struct sigaction*) &sact, NULL) == -1 ) {
		// NOTE we die here ...
		return -1;
	}
	/* timer is not initialized yet, ok! */

	// NOTE more to do?
	return 0;
}
