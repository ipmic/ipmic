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

/* Client side */

#include <stdio.h>
#include <stdlib.h>

#include "network.h"
#include "audio.h"
#include "common.h"

long long xrun_count;

extern audioparam_t alp;
extern netparam_t nlp;

int
main(int argc, char **argv)
{
	int8_t *buf;
	ssize_t len;
	int err;

	int index = 1;

	if(argc < 8)
	{
		printf("usage: cmd <PCMname> <PCMformat> <channels> <rate> "
		"<period_size> <port> <address>\n");
		return 1;
	}

	/* Set audio and network parameters from command line */
	alp.name = argv[index++];
	alp.format = argv[index++];
	alp.channels = atoi(argv[index++]);
	alp.rate = atoi(argv[index++]);
	alp.period_size = atoi(argv[index++]);
	alp.capture = 1;
	nlp.socket_type = SOCK_DGRAM;
	nlp.port = atoi(argv[index++]);
	nlp.addr = argv[index++];

	/* Open audio layer */
	if(audiolayer_open() == -1)
		goto _go_audiolayer_close;

	/* Open network layer */
	if(netlayer_open() == -1)
		goto _go_netlayer_close;

	/* Allocate memory for buffer */
	if((buf = malloc(alp.psize_ib)) == NULL)
	{
		printf("Error while allocating buffer!\n");
		goto _go_netlayer_close;
	}

	/* Print info on terminal */
	printf("channels %d\nrate %d\nperiod size %d (in frames)\n",
	alp.channels, alp.rate, alp.psize_if);

	while(1)
	{
		if((err = audiolayer_readi(buf, alp.psize_if)) != alp.psize_if)
		{
			if(err == -EAGAIN)
				continue;

			if(err == -EPIPE)
			{
				xrun_count++;
				audiolayer_prepare();
				continue;
			}

			printf("Error in snd_pcm_readi(): %s\n",
			snd_strerror(err));
			sleeponesec();
			continue;
		}

		if((len = netlayer_send(buf, alp.psize_ib, 0)) != alp.psize_ib)
		{
			if(len == -1 && errno != EAGAIN)
			{
				perror("Error in main(), send()");
				sleeponesec();
			}
			continue;
		}
	}

_go_free_buf:
	free(buf);

_go_netlayer_close:
	netlayer_close();

_go_audiolayer_close:
	audiolayer_close();

	printf("overrun = %lld\n", xrun_count);

	return 1;
}
