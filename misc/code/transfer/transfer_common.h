// IPMic transfer (common part)

extern int sfd; /* socket file descriptor */
extern unsigned long misses; /* misses count */
extern int keep_running; /* keep looping? (see while loop in main ) */

int
network_open (const char*);

void
register_sigint_handler (void);

void
print_misses_each_interval (void);
