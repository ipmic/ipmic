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

#include <alsa/asoundlib.h>

typedef struct audiolayer_t AudioL;
typedef struct audiolayerparams_t AudioP;

struct audiolayerparams_t
{
	char *name;
	char *format;
	int channels;
	int rate;
	int period_size; /* in frames */
	int capture; /* is capture device? (bool) */

/* internal defined: */
	snd_pcm_format_t pcm_format;
	size_t ssize_ib; /* sample size (in bytes) */
	size_t fsize_ib; /* frame size (in bytes) */
	snd_pcm_uframes_t psize_if; /* period size (in frames) */
	size_t psize_ib; /* period size (in bytes) */
};

struct audiolayer_t
{
	snd_pcm_t *handle;
};

AudioL*
audiolayer_new(AudioP*);

int
audiolayer_free(AudioL*);

inline snd_pcm_sframes_t
audiolayer_readi(AudioL*, void*, snd_pcm_uframes_t);

inline snd_pcm_uframes_t
audiolayer_writei(AudioL*, const void*, snd_pcm_uframes_t);

inline ssize_t
audiolayer_frames_to_bytes(AudioL*, snd_pcm_sframes_t);
