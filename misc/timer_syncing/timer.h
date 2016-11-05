// IPMic timer syncing (timer)

/* invoke timer_getoverrun() */
int
timer_get_expirations (void);
/** adjust timer interval for a possible synchronization with another source
 * @param nsecs  nanoseconds to combine with default interval */
int
timer_adjust_interval (long);
/** restore timer to default interval */
int
timer_restore_interval (void);
/** setup the timer
 * @param nsecs  interval between timer expirations */
int
timer_setup (long);
