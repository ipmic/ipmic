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
#include <unistd.h>

#include "audio.h"
#include "network.h"

/* At the moment we don't have a version */
#define VERSION "0.0"

extern audioparam_t alp;
extern netparam_t nlp;

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
	alp.type == AL_PLAYBACK ? "server" : "client",
	DEFAULT_CHANNELS, DEFAULT_RATE, alp.period_size, alp.psize_ib, nlp.port,
	nlp.addr);
}
