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

/* CURIOUS ...
 *
 * If we have [2 channels] with [16-bit sample] in a [48000 rate] ...
 *
 * our Frame is [2 channels] * [2 bytes (16-bit sample)] = [4 bytes]
 *
 * .-----------------------------.
 * | Our total = 192 kilobytes/s |
 * `-----------------------------´
 */

static int
set_hw_params(AudioL *al, AudioP *ap)
{
	int err;

	snd_pcm_hw_params_t *hw_params;

	snd_pcm_hw_params_alloca(&hw_params);

	snd_pcm_hw_params_any(al->handle, hw_params);

	snd_pcm_hw_params_set_access(al->handle, hw_params,
	SND_PCM_ACCESS_RW_INTERLEAVED);

	/* translate string into format value */
	ap->pcm_format = snd_pcm_format_value(ap->format);

	snd_pcm_hw_params_set_format(al->handle, hw_params, ap->pcm_format);

	snd_pcm_hw_params_set_channels(al->handle, hw_params, ap->channels);

	snd_pcm_hw_params_set_rate_near(al->handle, hw_params, &ap->rate, NULL);

	ap->psize_if = ap->period_size;

	snd_pcm_hw_params_set_period_size_near(al->handle, hw_params,
	&ap->psize_if, NULL);

	if((err = snd_pcm_hw_params(al->handle, hw_params)) < 0)
		return -1;

	if((ap->ssize_ib = snd_pcm_format_size(ap->pcm_format, 1)) < 0)
		return -1;

/* calc frame size (in bytes) */
	ap->fsize_ib = ap->ssize_ib * ap->channels;
/* calc period size (in bytes) */
	ap->psize_ib = ap->psize_if * ap->fsize_ib;

	return 0;
}

AudioL*
audiolayer_new(AudioP *ap)
{
	if(ap == NULL)
		return NULL;

	AudioL *al;

	int err;

	if((al = malloc(sizeof(AudioL))) == NULL)
		return NULL;

	if((err = snd_pcm_open(&al->handle, ap->name,
	(ap->capture ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK),
	0)) < 0)
		goto _go_free;

	if(set_hw_params(al, ap) == -1)
		goto _go_close;

	return al;

	_go_close:
	snd_pcm_close(al->handle);

	_go_free:
	free(al);

	return NULL;
}

int
audiolayer_free(AudioL *al)
{
	if(al == NULL)
		return -1;

	snd_pcm_close(al->handle);

	free(al);

	return 0;
}

inline snd_pcm_sframes_t
audiolayer_readi(AudioL *al, void *buffer, snd_pcm_uframes_t size)
{
	return snd_pcm_readi(al->handle, buffer, size);
}

inline snd_pcm_uframes_t
audiolayer_writei(AudioL *al, const void *buffer, snd_pcm_uframes_t size)
{
	return snd_pcm_writei(al->handle, buffer, size);
}

inline ssize_t
audiolayer_frames_to_bytes(AudioL *al, snd_pcm_sframes_t frames)
{
	return snd_pcm_frames_to_bytes(al->handle, frames);
}
