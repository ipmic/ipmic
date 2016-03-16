#include <stdio.h>
#include <stdlib.h>

#include "network.h"
#include "audio.h"

long long xrun_count;

NetLayer *nl;

AudioL *al;

AudioP alp;

/*
 * See an example:
 *
 * If we have
 *  ______ ________ _______
 * |sample|channels|   rate|
 * | 16bit|     1ch|22050Hz|
 *
 * We'll have 44100 Bytes per second ( 2Bytes (16bit) * 2Channels * 22050Hz ).
 *
 * If we split a second of record in 20 parts
 *
 * ( 44100 Bytes/s  /  20 parts )  =  ( 2205 Bytes  /  1/20s )
 *
 * We'll have 2205 Bytes per 1/20 second.
 */

int
main(int argc, char **argv)
{
	int8_t *buf;
	ssize_t len;
	int err;

	int port;

	int index = 1;

	if(argc < 7)
	{
		printf("usage: cmd <PCMname> <PCMformat> <channels> <rate>"
		"<period_size> <port>\n");
		return 1;
	}

	alp.name = argv[index++];
	alp.format = argv[index++];
	alp.channels = atoi(argv[index++]);
	alp.rate = atoi(argv[index++]);
	alp.period_size = atoi(argv[index++]);

	port = atoi(argv[index++]);

	alp.capture = 0;

	if((al = audiolayer_new(&alp)) == NULL)
		return 1;

	if((nl = netlayer_new(SOCK_DGRAM, port, NULL)) == NULL)
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
		if((len = netlayer_recv(nl->socket_fd, (void*) buf,
	alp.psize_ib, 0)) == -1)
		{
			if(errno == EAGAIN)
				continue;

			perror("Error in `main()`, recv() ");
			goto _go_free2;
		}

		if(len == 0)
			goto _go_free2;

		if((err = audiolayer_writei(al, (const void*) buf,
	len / alp.fsize_ib)) < 0)
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
				printf("Error in `snd_pcm_writei()`: %s\n",
	snd_strerror(err));

			goto _go_free2;
		}

	}

_go_free2:
	free(buf);

_go_free1:
	netlayer_free(nl);

_go_free0:
	audiolayer_free(al);

	printf("underrun = %lld\n", xrun_count);

	return 1;
}
