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

audiolayer_t *al;

audioparam_t alp;

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
