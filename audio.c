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

#include "audio.h"

#include <stdlib.h>

#ifndef IPMIC_RELEASE
/* for debugging */
#include <stdio.h>
#include <stdarg.h>
#endif /* IPMIC_RELEASE */

#ifdef IPMIC_WITH_TINYALSA
#include "tinyalsa.h"
#else /* IPMIC_WITH_TINYALSA */
#include <alsa/asoundlib.h>
#endif /* IPMIC_WITH_TINYALSA */

#ifndef IPMIC_RELEASE
void
ipmic_debug(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
}
#else /* IPMIC_RELEASE */
void
ipmic_debug(const char *fmt, ...)
{
	(void) fmt;
}
#endif /* IPMIC_RELEASE */

#ifdef IPMIC_WITH_TINYALSA

struct ipmic_device
{
	/** A TinyALSA PCM */
	struct pcm *pcm;
};

struct ipmic_device *
ipmic_device_open(const struct ipmic_device_params *params)
{
	struct pcm *pcm;
	struct pcm_config pcm_config;
	int pcm_flags;
	struct ipmic_device *dev;

	pcm_config.channels = params->channels;
	pcm_config.rate = params->rate;
	pcm_config.format = PCM_FORMAT_S16_LE;
	pcm_config.period_size = params->period_size;
	pcm_config.period_count = 4;
	pcm_config.start_threshold = 0;
	pcm_config.stop_threshold = 0;
	pcm_config.silence_threshold = 0;

	if (params->type == IPMIC_DEVICE_INPUT)
		pcm_flags = PCM_IN;
	else
		pcm_flags = PCM_OUT;

	pcm = pcm_open(0, 0, pcm_flags, &pcm_config);
	if (pcm == NULL)
	{
		ipmic_debug("%s: failed to malloc PCM\n", __func__);
		return NULL;
	}

	if (!pcm_is_ready(pcm))
	{
		ipmic_debug("%s: failed to open PCM\n", __func__);
		ipmic_debug("  %s\n", pcm_get_error(pcm));
		pcm_close(pcm);
		return NULL;
	}

	dev = malloc(sizeof(*dev));
	if (dev == NULL)
	{
		ipmic_debug("%s: failed to malloc device\n", __func__);
		return NULL;
	}

	dev->pcm = pcm;

	return dev;
}

void
ipmic_device_close(struct ipmic_device *device)
{
	if (device == NULL)
	{
		ipmic_debug("%s: received NULL device\n", __func__);
		return;
	}

	if (device->pcm == NULL)
	{
		ipmic_debug("%s: received NULL device pcm\n", __func__);
		return;
	}

	pcm_close(device->pcm);

	free(device);
}

int
ipmic_device_prepare(struct ipmic_device *device)
{
	return pcm_prepare(device->pcm);
}

int
ipmic_device_read(struct ipmic_device *device,
                  int16_t *frame_array,
                  unsigned int frame_count)
{
	return pcm_read(device->pcm, frame_array, frame_count);
}

int
ipmic_device_write(struct ipmic_device *device,
                   const int16_t *frame_array,
                   unsigned int frame_count)
{
	return pcm_write(device->pcm, frame_array, frame_count);
}

#else /* IPMIC_WITH_TINYALSA */

struct ipmic_device {
	/** An alsa-lib pcm */
	snd_pcm_t *pcm;
};

int
ipmic_device_set_params(struct ipmic_device *device,
                        const struct ipmic_device_params *params)
{
	snd_pcm_hw_params_t *hw_params;

	snd_pcm_hw_params_alloca(&hw_params);

	snd_pcm_hw_params_any(device->pcm, hw_params);
	snd_pcm_hw_params_set_access(device->pcm, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(device->pcm, hw_params, SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_channels(device->pcm, hw_params, params->channels);
	snd_pcm_hw_params_set_rate(device->pcm, hw_params, params->rate, 0);
	snd_pcm_hw_params_set_period_size(device->pcm, hw_params, params->period_size, 0);
	snd_pcm_hw_params_set_periods(device->pcm, hw_params, 2, 0);

	return snd_pcm_hw_params(device->pcm, hw_params);
}

struct ipmic_device *
ipmic_device_open(const struct ipmic_device_params *params)
{
	int err;
	snd_pcm_t *pcm;
	snd_pcm_stream_t pcm_stream;
	struct ipmic_device *dev;

	if (params->type == IPMIC_DEVICE_INPUT)
		pcm_stream = SND_PCM_STREAM_CAPTURE;
	else
		pcm_stream = SND_PCM_STREAM_PLAYBACK;

	err = snd_pcm_open(&pcm, "default", pcm_stream, /* mode */ 0);
	if (err != 0)
	{
		ipmic_debug("%s: failed to open pcm\n", __func__);
		ipmic_debug("  %s\n", snd_strerror(err));
		return NULL;
	}

	dev = malloc(sizeof(*dev));
	if (dev == NULL)
	{
		err = errno;
		ipmic_debug("%s: failed to malloc device\n", __func__);
		ipmic_debug("  %s\n", strerror(err));
		return NULL;
	}

	dev->pcm = pcm;

	err = ipmic_device_set_params(dev, params);
	if (err != 0)
	{
		ipmic_debug("%s: failed to set device parameters\n", __func__);
		ipmic_debug("  %s\n", snd_strerror(err));
		ipmic_device_close(dev);
		return NULL;
	}

	return dev;
}

void
ipmic_device_close(struct ipmic_device *device)
{
	if ((device != NULL) && (device->pcm != NULL))
		snd_pcm_close(device->pcm);
	free(device);
}

int
ipmic_device_prepare(struct ipmic_device *device)
{
	return snd_pcm_prepare(device->pcm);
}

int
ipmic_device_read(struct ipmic_device *device,
                  int16_t *frame_array,
                  unsigned int frame_count)
{
	return snd_pcm_readi(device->pcm, frame_array, frame_count);
}

int
ipmic_device_write(struct ipmic_device *device,
                   const int16_t *frame_array,
                   unsigned int frame_count)
{
	return snd_pcm_writei(device->pcm, frame_array, frame_count);
}

#endif /* IPMIC_WITH_TINYALSA */

struct ipmic_device_params alp;

struct ipmic_device *ipmic_default_device;

int
audiolayer_open(void)
{
	alp.channels = IPMIC_DEFAULT_CHANNELS;
	alp.rate = IPMIC_DEFAULT_RATE;
	ipmic_default_device = ipmic_device_open(&alp);
	if (ipmic_default_device == NULL)
		return -1;
	ipmic_device_prepare(ipmic_default_device);
	return 0;
}

void
audiolayer_close(void)
{
	ipmic_device_close(ipmic_default_device);
	ipmic_default_device = NULL;
}

int
audiolayer_prepare(void)
{
	return ipmic_device_prepare(ipmic_default_device);
}

int
audiolayer_write(const void *frame_array, unsigned int frame_count)
{
	return ipmic_device_write(ipmic_default_device, frame_array, frame_count);
}

int
audiolayer_read(void *frame_array, unsigned int frame_count)
{
	return ipmic_device_read(ipmic_default_device, frame_array, frame_count);
}

