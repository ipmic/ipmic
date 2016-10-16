// IPMic select() test (common header)

// here is a list of defines that can be personalized
  /* port to bind (server) or connect (client) */
#define PORT_TO_BIND  8080
  /* client's select timeout is the timeout to write data in network
   * socket */
#define CLIENT_SELECT_TIMEOUT  5000
  /* each  X  misses we print number of misses in terminal */
#define MISSES_INTERVAL_TO_PRINT  20
  /* natural latency    In audio it's aka "time to record or play audio"
   * HACK: Do not use values greater than 999999 */
#define NATURAL_LATENCY 50000 /* 0.05 second */
// ----------

int
network_open (const char*);

void
register_sigint_handler (void);

void
print_misses_each_interval (void);
