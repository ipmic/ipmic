// IPMic transfer (common part)
// this code contains network opening, signal handler and misses count

#include <arpa/inet.h>
#include <netinet/ip.h>
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

/* each  X  misses we print number of misses in terminal */
#define MISSES_INTERVAL_TO_PRINT  20
/* port to bind (server) or connect (client) */
#define PORT_TO_BIND  8080

int sfd;
unsigned long misses = 0; /* how many packets doesn't arrive in time */
int keep_running = 1;

static struct sockaddr_in saddr = {
	AF_INET, /* aka IP family */
};

/* signal handler */
static void
on_sigint (int foo) {
	keep_running = 0;
}
static const struct sigaction signal_action = {
	.sa_handler = on_sigint,
	.sa_mask = 0,
	.sa_flags = 0,
};

int
network_open (const char *addr) {
	/* set port and address */
	saddr.sin_port = htons(PORT_TO_BIND);
	if (addr) { /* client mode */
		if (!inet_aton(addr, (struct in_addr*) &saddr.sin_addr))
			return -1;
	} else /* server mode */
		saddr.sin_addr.s_addr = INADDR_ANY;

	/* open socket and connect it to `peer_saddr` */
	if ((sfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		return -1;
	if (addr) { /* client mode */
		if (connect(sfd, (const struct sockaddr*) &saddr,
		    sizeof(struct sockaddr_in)) == -1)
			goto _go_close_socket;
	} else { /* server mode */
		if (bind(sfd, (const struct sockaddr*) &saddr,
		    sizeof(struct sockaddr_in)) == -1)
			goto _go_close_socket;
	}

	return 0;
_go_close_socket:
	close(sfd);
	return -1;
}

void
register_sigint_handler (void) {
	sigaction(SIGINT, &signal_action, NULL);
}

void
print_misses_each_interval (void) {
	if (!misses)
		printf("[miss] first miss occurred\n");
	else if (misses % MISSES_INTERVAL_TO_PRINT == 0)
		printf("misses = %ld\n", misses);
}
