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

/* Server side */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "network.h"
#include "audio.h"
#include "common.h"

long long xrun_count;

extern audioparam_t alp;
extern netparam_t nlp;

int
main(int argc, char **argv)
{
	void *buf;
	int len;
	int err;

	int argidx = 0;

	if(argc < 3)
	{
		printf("usage: cmd <period_size(in frames)> <port>\n");
		return 1;
	}

	/* Set audio and network parameters from command line */
	alp.type = AL_PLAYBACK; /* server will play sound */
	alp.period_size = atoi(argv[++argidx]);
	nlp.socket_type = DEFAULT_SOCKETTYPE;
	nlp.port = atoi(argv[++argidx]);
	nlp.addr = NULL;

	/* Try to go realtime :-) */
	go_realtime();

	/* Open network layer */
	if(netlayer_open() == -1)
	{
		fprintf(stderr, ">> Error in netlayer_open()\n");
		return 1;
	}

	/* Open audio layer */
	if(audiolayer_open() == -1)
	{
		fprintf(stderr, ">> Error in audiolayer_open()\n");
		goto _go_netlayer_close;
	}

	/* Allocate memory for buffer */
	if((buf = malloc(alp.psize_ib)) == NULL)
		goto _go_audiolayer_close;

	/* Print info on terminal */
	print_general_info(); /* see common.c */

	while(1)
	{
		if((len = netlayer_recv(buf, alp.psize_ib, 0)) != alp.psize_ib)
		{
			if(len == -1 && errno != EAGAIN)
			{
				fprintf(stderr, ">> Error in "
				"netlayer_recv()\n");
				sleeponesec();
			}
			continue;
		}

		if((err = audiolayer_write((const void*) buf, alp.period_size))
		!= alp.period_size)
		{
			if(err == -EAGAIN)
				continue;

			if(err == -EPIPE)
			{
				xrun_count++;
				audiolayer_prepare();
				continue;
			}

			fprintf(stderr, ">> Error in audiolayer_write()\n");
			sleeponesec();
			continue;
		}
	}

_go_free_buf:
	free(buf);
_go_audiolayer_close:
	audiolayer_close();
_go_netlayer_close:
	netlayer_close();

	printf("underrun = %lld\n", xrun_count);

	return 1;
}
