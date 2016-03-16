#include <netinet/in.h>

typedef struct oc_netlayer_t
{
	int socket_type;
	int socket_fd;
	struct sockaddr_in socket_addr;
} NetLayer;

NetLayer*
netlayer_new(int, uint16_t, const char*);

int
netlayer_free(NetLayer*);

inline ssize_t
netlayer_recv(int, void*, size_t, int);
