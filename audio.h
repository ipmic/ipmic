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

typedef snd_pcm_t audiolayer_t;
typedef struct audiolayerparams_t audioparam_t;

struct audiolayerparams_t
{
	char *name;
	char *format;
	int channels;
	int rate;
	int period_size; /* in frames */
	int capture; /* is capture device? (bool) If 0 it's playback. */

/* internal defined: */
	snd_pcm_format_t pcm_format;
	size_t ssize_ib; /* sample size (in bytes) */
	size_t fsize_ib; /* frame size (in bytes) */
	snd_pcm_uframes_t psize_if; /* period size (in frames) */
	size_t psize_ib; /* period size (in bytes) */
};

int
audiolayer_open(void);

int
audiolayer_close(void);

snd_pcm_sframes_t
audiolayer_readi(void*, snd_pcm_uframes_t);

snd_pcm_sframes_t
audiolayer_writei(const void*, snd_pcm_uframes_t);

inline void
audiolayer_prepare(void);
