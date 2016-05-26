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

#define DEFAULT_PCMNAME "ipmicpcm"
#define DEFAULT_PCMFORMAT "S16_LE"
#define DEFAULT_PCMCHANNELS 1
#define DEFAULT_PCMRATE 22050
/* PCM default period size (in frames)
 * 1225 F/ 1/18s    or    2450 B/ 1/18s
 */
#define DEFAULT_PCMPSIZE 1225

/* Default is UDP */
#define DEFAULT_SOCKETTYPE SOCK_DGRAM

void
sleeponesec(void);

void
print_general_info(void);
