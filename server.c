/*
 * IPMic, The IP Microphone project
 * Copyright (C) 2016  Ricardo Biehl Pasquali <rbpoficial@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>

#include "network.h"
#include "audio.h"

long long xrun_count;

netlayer_t nl;

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
 * We'll have 44100 Bytes per second ( 2Bytes (16bit) * 1Channel * 22050Hz ).
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
		printf("usage: cmd <PCMname> <PCMformat> <channels> <rate> "
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

	if((nl = netlayer_new(SOCK_DGRAM, port, NULL)) == -1)
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
		if((len = netlayer_recv(nl, (void*) buf, alp.psize_ib, 0))
	!= alp.psize_ib)
		{
			if(len == -1)
			{
				if(errno == EAGAIN)
					continue;

				perror("Error in `main()`, recv() ");
				goto _go_free_buf;
			}
			else
				continue;
		}

		if(len == 0)
			goto _go_free_buf;

		if((err = audiolayer_writei(al, (const void*) buf,
	alp.psize_if)) < 0)
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

			goto _go_free_buf;
		}

	}

_go_free_buf:
	free(buf);

_go_netlayer_close:
	netlayer_close(nl);

	audiolayer_free(al);

	printf("underrun = %lld\n", xrun_count);

	return 1;
}
