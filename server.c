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

extern long long xrun_count;

extern audioparam_t alp;
extern netparam_t nlp;

extern int keep_running;

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
	alp.buffer_size = frames_to_bytes(alp.period_size);
	nlp.socket_type = DEFAULT_SOCKETTYPE;
	nlp.port = atoi(argv[++argidx]);
	nlp.addr = NULL;

	if(common_init(&buf) == -1)
		return 1;

	while(keep_running)
	{
		if((len = netlayer_recv(buf, alp.buffer_size, 0)) < 0)
		{
			if(len == -1 && errno != EAGAIN)
			{
				fprintf(stderr, ">> Error in "
				"netlayer_recv()\n");
				sleeponesec();
			}
			continue;
		}

		if((err = audiolayer_write((const void*) buf,
		bytes_to_frames(alp.buffer_size))) < 0)
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

	return common_finit(buf);
}
