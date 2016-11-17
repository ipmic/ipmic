// IPMic select() test (common)

  /* client's select timeout is the timeout to write data in network
   * socket    It needs to be smaller than NATURAL_LATENCY */
#define CLIENT_SELECT_TIMEOUT  5000
  /* natural latency    In audio it's aka "time to record or play audio"
   * HACK: Do not use values greater than 999999 */
#define NATURAL_LATENCY 50000 /* 0.05 second */
