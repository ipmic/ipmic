#include <stdio.h>
#include <stdlib.h>

#include "network.h"
#include "audio.h"

long long xrun_count;

netlayer_t nl;

AudioL *al;

AudioP alp;

int
main(int argc, char **argv)
{
	if(argc < 8)
	{
		printf("usage: cmd <PCMname> <PCMformat> <channels> <rate> "
	"<period_size> <port> <address>\n");
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

	if((nl = netlayer_new(SOCK_DGRAM, port, addr)) == -1)
		goto _go_netlayer_close;

	if((buf = malloc(alp.psize_ib)) == NULL)
	{
		printf("Error while alloc buf!\n");
		goto _go_netlayer_close;
	}

	printf("channels %d\nrate %d\nperiod size %d (in frames)\n",
	alp.channels, alp.rate, alp.psize_if);

	while(1)
	{
		if((err = audiolayer_readi(al, buf, alp.psize_if)) !=
	alp.psize_if)
		{
			if(err < 0)
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
					puts(snd_strerror(err));

				goto _go_free_buf;
			}
			else
				continue;
		}

		if((len = netlayer_send(nl, buf, alp.psize_ib, 0)) == -1)
		{
			if(errno == EAGAIN)
				continue;

			perror("Error in `main()`, send()");
			goto _go_free_buf;
		}
	}

_go_free_buf:
	free(buf);

_go_netlayer_close:
	netlayer_close(nl);

	audiolayer_free(al);

	printf("overrun = %lld\n", xrun_count);

	return 1;
}
