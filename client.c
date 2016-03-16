#include <stdio.h>
#include <stdlib.h>

#include "network.h"
#include "audio.h"

long long xrun_count;

NetLayer *nl;

AudioL *al;

AudioP alp;

int
main(int argc, char **argv)
{
	if(argc < 8)
	{
		printf("usage: cmd <PCMname> <PCMformat> <channels> <rate> "
	"<period_size> <port>\n <address>\n");
		return 1;
	}

	int8_t *buf;
	ssize_t len;
	int err;

	int port;
	char *addr;

	int index = 1;

	alp.name = argv[index++];
	alp.format = argv[index++];
	alp.channels = atoi(argv[index++]);
	alp.rate = atoi(argv[index++]);
	alp.period_size = atoi(argv[index++]);

	port = atoi(argv[index++]);
	addr = argv[index++];

	alp.capture = 1;

	if((al = audiolayer_new(&alp)) == NULL)
		return 1;

	if((nl = netlayer_new(SOCK_DGRAM, port, addr)) == NULL)
		goto _go_free0;

	if((buf = malloc(alp.psize_ib)) == NULL)
	{
		printf("Error while alloc buf!\n");
		goto _go_free1;
	}

	printf("channels %d\nrate %d\nperiod size %d (in frames)\n",
	alp.channels, alp.rate, alp.psize_if);

	while(1)
	{
		if((err = audiolayer_readi(al, buf, alp.psize_if)) < 0)
		{
			if(err == -EAGAIN)
				continue;
			else
			if(err == -EPIPE)
			{
				xrun_count++;
				snd_pcm_prepare(al->handle);
				continue;
			}
			else
				printf("Error in `snd_pcm_readi()`: %s\n",
	snd_strerror(err));
			goto _go_free2;
		}

		if((len = netlayer_send(nl->socket_fd, buf,
	err * alp.fsize_ib, 0)) == -1)
		{
			if(errno == EAGAIN)
				continue;

			perror("Error in `main()`, send()");
			goto _go_free2;
		}
	}

_go_free2:
	free(buf);

_go_free1:
	netlayer_free(nl);

_go_free0:
	audiolayer_free(al);

	printf("overrun = %lld\n", xrun_count);

	return 1;
}
