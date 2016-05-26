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

static int
set_hw_params(void)
{
	int err;

	snd_pcm_hw_params_t *hw_params;

	snd_pcm_hw_params_alloca(&hw_params);

	snd_pcm_hw_params_any(al, hw_params);

	snd_pcm_hw_params_set_access(al, hw_params,
	SND_PCM_ACCESS_RW_INTERLEAVED);

	/* Translate string into sample format and set it */
	alp.pcm_format = snd_pcm_format_value(alp.format);
	snd_pcm_hw_params_set_format(al, hw_params, alp.pcm_format);

	/* Set number of channels */
	snd_pcm_hw_params_set_channels(al, hw_params, alp.channels);

	/* Set rate */
	snd_pcm_hw_params_set_rate_near(al, hw_params, &alp.rate, NULL);

	/* Get period size from parameters and set it */
	alp.psize_if = alp.period_size;
	snd_pcm_hw_params_set_period_size_near(al, hw_params, &alp.psize_if,
	NULL);

	if((err = snd_pcm_hw_params(al, hw_params)) < 0)
		return -1;

	if((alp.ssize_ib = snd_pcm_format_size(alp.pcm_format, 1)) < 0)
		return -1;

	/* Calculate frame and period size (in bytes) to know it in the future
	*/
	alp.fsize_ib = alp.ssize_ib * alp.channels;
	alp.psize_ib = alp.psize_if * alp.fsize_ib;

	return 0;
}

int
audiolayer_open(void)
{
	if(snd_pcm_open(&al, alp.name, alp.type, 0) < 0
	|| set_hw_params() == -1)
		return -1;

	return 0;
}

int
audiolayer_close(void)
{
	snd_pcm_close(al);

	#if 0
	/* Is it wrong!? */
	_go_free:
	free(al);
	#endif

	return 0;
}

snd_pcm_sframes_t
audiolayer_readi(void *buffer, snd_pcm_uframes_t size)
{
	snd_pcm_sframes_t res, ret;

	ret = size;

	while(size)
	{
		if((res = snd_pcm_readi(al, buffer, size)) < 0)
			return res;
		size -= res;
		buffer += res * alp.fsize_ib;
	}

	return ret;
}

snd_pcm_sframes_t
audiolayer_writei(const void *buffer, snd_pcm_uframes_t size)
{
	snd_pcm_sframes_t res, ret;

	ret = size;

	while(size)
	{
		if((res = snd_pcm_writei(al, buffer, size)) < 0)
			return res;
		size -= res;
		buffer += res * alp.fsize_ib;
	}

	return ret;
}

inline void
audiolayer_prepare(void)
{
	snd_pcm_prepare(al);
}
