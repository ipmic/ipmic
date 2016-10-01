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

/* stdio.h  for debug purposes */
#include <stdio.h>
#include <stdlib.h>

#include "audio.h"

static audiolayer_t *al;
audioparam_t alp;

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

#ifndef _TINYALSA

static int
set_hw_params(void)
{
	int err;

	snd_pcm_hw_params_t *hw_params;

	snd_pcm_hw_params_alloca(&hw_params);
	snd_pcm_hw_params_any(al, hw_params);

	/* Set access */
	snd_pcm_hw_params_set_access(al, hw_params,
	SND_PCM_ACCESS_RW_INTERLEAVED);

	/* Set format */
	snd_pcm_hw_params_set_format(al, hw_params, SND_PCM_FORMAT_S16_LE);

	/* Set number of channels */
	snd_pcm_hw_params_set_channels(al, hw_params, DEFAULT_CHANNELS);

	/* Set rate */
	snd_pcm_hw_params_set_rate(al, hw_params, DEFAULT_RATE, 0);

	/* Set period size from parameters */
	snd_pcm_hw_params_set_period_size(al, hw_params, alp.period_size, 0);

	if((err = snd_pcm_hw_params(al, hw_params)) < 0)
		return -1;

	return 0;
}

#endif /* ifndef _TINYALSA */

int
audiolayer_open(void)
{
#ifdef _TINYALSA
	struct pcm_config pcmconf;

	/* Set format, channels, rate, period size and period count */
	pcmconf.channels = DEFAULT_CHANNELS;
	pcmconf.format = PCM_FORMAT_S16_LE;
	pcmconf.rate = DEFAULT_RATE;
	pcmconf.period_size = alp.period_size; /* in frames */
	pcmconf.period_count = 2;
	pcmconf.start_threshold = 0;
	pcmconf.stop_threshold = 0;
	pcmconf.silence_threshold = 0;

	al = pcm_open(0, 0, (alp.type == AL_CAPTURE ? PCM_IN : PCM_OUT),
	&pcmconf);

	if(!pcm_is_ready(al))
	{
		fprintf(stderr, "tinyalsa Error: %s\n", pcm_get_error(al));
		goto _go_close_pcm; /* NOTE is this necessary? */
	}

	return 0;
_go_close_pcm:
	pcm_close(al);
	return -1;
#else
	if(snd_pcm_open(&al, "ipmic", (alp.type == AL_CAPTURE ?
	SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK), 0) < 0)
		return -1;
	if(set_hw_params() == -1)
		goto _go_close_pcm;

	return 0;
_go_close_pcm:
	snd_pcm_close(al);
	return -1;
#endif
}

int
audiolayer_close(void)
{
#ifdef _TINYALSA
	pcm_close(al);
#else
	snd_pcm_close(al);
#endif

	return 0;
}

int
audiolayer_read(void *buffer, unsigned int frames)
{
	int ret;

	ret = frames;
#ifdef _TINYALSA
	if(pcm_read(al, buffer, frames_to_bytes(frames)))
		return -1;
#else
	int res;

	ret = frames;

	while(frames)
	{
		if((res = snd_pcm_readi(al, buffer, frames)) < 0)
			return res;
		frames -= res;
		buffer += frames_to_bytes(res);
	}
#endif

	return ret;
}

int
audiolayer_write(const void *buffer, unsigned int frames)
{
	int ret;

	ret = frames;
#ifdef _TINYALSA
	if(pcm_write(al, buffer, frames_to_bytes(frames)))
		return -1;
#else
	int res;

	while(frames)
	{
		if((res = snd_pcm_writei(al, buffer, frames)) < 0)
			return res;
		frames -= res;
		buffer += frames_to_bytes(res);
	}
#endif

	return ret;
}

void
audiolayer_prepare(void)
{
#ifdef _TINYALSA
	pcm_prepare(al);
#else
	snd_pcm_prepare(al);
#endif
}
