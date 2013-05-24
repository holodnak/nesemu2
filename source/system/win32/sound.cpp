/***************************************************************************
 *   Copyright (C) 2013 by James Holodnak                                  *
 *   jamesholodnak@gmail.com                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <windows.h>

extern "C" {
	#include "system/sound.h"
}

static int sound_bps = 16;
static int sound_samplerate = 44100;
static int sound_fragsize = 1024;//SOUND_HZ / 60;
static void (*audio_callback)(void *buffer, int length) = 0;
static int soundinited = 0;

int sound_init()
{
//	log_printf("dsound init ok, %dhz, %d bits\n",sound_samplerate,sound_bps);
	return(0);
}

void sound_kill()
{
	if(soundinited) {
	}
	soundinited = 0;
}

void sound_play()
{
}

void sound_pause()
{
}

void sound_setcallback(void (*cb)(void *buffer, int length))
{
	audio_callback = cb;
}

void sound_update()
{
}

void sound_lock()
{
}

void sound_unlock()
{
}
