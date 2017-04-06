#!/usr/bin/make -f

# IPMic, The IP Microphone project
# Copyright (C) 2016  Ricardo Biehl Pasquali <rbpoficial@gmail.com>
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# Variables defined in this file
# are typed in lower case if they
# are defined within the Makefile
# and typed in upper case if they
# are defined automatically by Make.

# This makefile contains
# four top level targets:
#   all - builds all programs
#   clean - removes all temporary files
#   check - runs all tests
#   test - same as 'check'
#   install - installs the programs

# Variables that may affect the building
# of the project are:
#   use_tinyalsa - if set, causes the audio layer
#                  to use tinyalsa instead of alsa-lib
#   prefix - determines where the default installs paths
#            are (for example, '/usr' or '/usr/local').
#            this path should not end in a '/'
#   bindir - determins where the programs will be installed
#            to. if this variable is directly set, 'prefix'
#            will have no affect on it.

# Variables may be use using:
#   make use_tinyalsa=1
#   make prefix=/usr
#   make bindir=~/.local/bin

prefix ?= /usr/local
bindir ?= $(prefix)/bin

CFLAGS += -Wall -Wextra

objfiles += audio.o
objfiles += common.o
objfiles += network.o

ifdef use_tinyalsa
objfiles += tinyalsa.o
CFLAGS += -D_TINYALSA
else
LDLIBS += -lasound
endif

.PHONY: all
all: server client

server: server.o

client: client.o

server client: $(objfiles)

audio.o: audio.c audio.h tinyalsa.h

common.o: common.c common.h audio.h network.h

network.o: network.c network.h

.PHONY: clean
clean:
	$(RM) $(objfiles)

.PHONY: test check
test check:

.PHONY: install
install:
	install --directory $(bindir)/
	install server $(bindir)/
	install client $(bindir)/

