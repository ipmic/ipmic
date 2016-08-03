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

#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "audio.h"
#include "network.h"

/* At the moment we don't have a version */
#define VERSION "0.0"

long long xrun_count;

extern audioparam_t alp;
extern netparam_t nlp;

int keep_running = 1;

static void
signal_handler(int signum)
{
	keep_running = 0;
	return;
}

static int
go_realtime(void)
{
	int max_pri;
	struct sched_param sp;

	if(sched_getparam(0, &sp))
		goto _go_err;

	max_pri = sched_get_priority_max(SCHED_FIFO);
	sp.sched_priority = max_pri;

	if(sched_setscheduler(0, SCHED_FIFO, &sp))
		goto _go_err;

	return 0;

_go_err:
	printf(">> Warning: We're not realtime! Are you root?\n");
	return -1;
}

static const struct sigaction sact = {
	signal_handler,
	0,
	0,
	0,
};

static void
register_signals(void)
{
	sigaction(SIGINT, &sact, NULL);
	return;
}

void
sleeponesec(void)
{
	printf("Sleeping one second ...\n");
	sleep(1);
}

void
print_general_info(void)
{
	printf("IPMic version %s (%s mode)\n"
	"channels %d\n"
	"rate %d\n"
	"period size %d (in frames)\n"
	"period size %d (in bytes)\n"
	"network port %d\n"
	"network address %s\n\n",
	VERSION,
	alp.type == AL_PLAYBACK ? "server" : "client", DEFAULT_CHANNELS,
	DEFAULT_RATE, alp.period_size, frames_to_bytes(alp.period_size),
	nlp.port, nlp.addr);
}

int
common_finit(void *buf)
{
	free(buf);
	audiolayer_close();
	netlayer_close();

	printf("%s: %d\n", (alp.type == AL_PLAYBACK ? "underrun" : "overrun"),
	xrun_count);

	if(!keep_running)
	{
		printf("A signal has caught! Ending ...\n");
		return 0;
	}

	return 1;
}

int
common_init(void **buf)
{
	/* Open network layer */
	if(netlayer_open() == -1)
	{
		fprintf(stderr, ">> Error in netlayer_open()\n");
		return -1;
	}

	/* Open audio layer */
	if(audiolayer_open() == -1)
	{
		fprintf(stderr, ">> Error in audiolayer_open()\n");
		goto _go_netlayer_close;
	}

	/* Allocate memory for buffer */
	if((*buf = malloc(nlp.buffer_size)) == NULL)
		goto _go_audiolayer_close;

	/* Print info on terminal */
	print_general_info();

	/* Try to go realtime :-) */
	go_realtime();

	register_signals();

	return 0;

_go_audiolayer_close:
	audiolayer_close();
_go_netlayer_close:
	netlayer_close();

	return -1;
}
