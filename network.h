#include <stdlib.h>
#include <netinet/in.h>

typedef int netlayer_t;

netlayer_t
netlayer_new(int, uint16_t, const char*);

inline int
netlayer_close(netlayer_t);

inline ssize_t
netlayer_recv(netlayer_t, void*, size_t, int);

inline ssize_t
netlayer_send(netlayer_t, const void*, size_t, int);
