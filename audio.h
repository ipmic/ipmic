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

#ifndef IPMIC_AUDIO_H
#define IPMIC_AUDIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define IPMIC_DEFAULT_CHANNELS 2
#define IPMIC_DEFAULT_RATE 48000

/** Describes whether an audio device is an input or output.
 * Used when opening a device.
 * */

enum ipmic_device_type
{
	/** The device is an input device (e.g. microphone) */
	IPMIC_DEVICE_INPUT,
	/** The device is an output device (e.g. speaker) */
	IPMIC_DEVICE_OUTPUT
};

/** Describes behaviour of a PCM.
 * */

struct ipmic_device_params
{
	/** Describes whether or not device is an input */
	enum ipmic_device_type type;
	/** The number of frames in a PCM period */
	unsigned int period_size;
	/** The number of frames in a PCM buffer */
	unsigned int buffer_size;
	/** The number of frames per second */
	unsigned int rate;
	/** The number of channels per frame */
	unsigned int channels;
};

/** Represents an audio input or output.
 * */

struct ipmic_device;

/** Opens up a audio device.
 * @param params A structure containing
 *  parameters affecting how the audio
 *  device will run.
 * @returns On success, a pointer to an audio device.
 *  On failure, NULL.
 * */

struct ipmic_device *
ipmic_device_open(const struct ipmic_device_params *params);

/** Closes an audio device opened with @ipmic_device_open.
 * @param device An initialized audio device.
 *  This parameter may be NULL.
 * */

void
ipmic_device_close(struct ipmic_device *device);

/** Reads frames from an audio device.
 * @param device An audio device opened
 *  with @ref ipmic_device_open.
 * @param frame_array A series of interleaved audio samples.
 * @param frame_count The number of frames in @p frame_array.
 *  This value should not be greater than INT_MAX.
 * @retruns On success, the number of frames read.
 *  On failure, a negative error code.
 * */

int
ipmic_device_read(struct ipmic_device *device,
                  int16_t *frame_array,
                  unsigned int frame_count);

/** Writes frames to an audio device.
 * @param device An audio device opened
 *  with @ref ipmic_device_open.
 * @param frame_array A series of interleaved audio samples.
 * @param frame_count The number of frames in @p frame_array.
 *  This value should not be greater than INT_MAX.
 * @retruns On success, the number of frames written.
 *  On failure, a negative error code.
 * */

int
ipmic_device_write(struct ipmic_device *device,
                   const int16_t *frame_array,
                   unsigned int frame_count);

/** Prepares an audio device for input or output operations.
 * @param device An audio device opened
 *  with @ref ipmic_device_open.
 * @returns Zero on success, a negative error code on failure.
 * */

int
ipmic_device_prepare(struct ipmic_device *device);

#define AL_PLAYBACK IPMIC_DEVICE_OUTPUT

#define AL_CAPTURE IPMIC_DEVICE_INPUT

#define DEFAULT_RATE IPMIC_DEFAULT_RATE

#define DEFAULT_CHANNELS IPMIC_DEFAULT_CHANNELS

#define bytes_to_frames(bytes) (bytes / 2 * IPMIC_DEFAULT_CHANNELS)

#define frames_to_bytes(frames) (frames * 2 * IPMIC_DEFAULT_CHANNELS)

typedef struct ipmic_device_params audioparam_t;

extern struct ipmic_device_params alp;

extern struct ipmic_device *ipmic_snd_input;

extern struct ipmic_device *ipmic_snd_output;

int
audiolayer_open(void);

void
audiolayer_close(void);

int
audiolayer_read(void *, unsigned int);

int
audiolayer_write(const void *, unsigned int);

int
audiolayer_prepare(void);

#ifdef __cplusplus
} /* extern "C" { */
#endif /* __cplusplus */

#endif /* IPMIC_AUDIO_H */

