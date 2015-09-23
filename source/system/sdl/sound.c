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

#include <SDL/SDL.h>
#include "types.h"
#include "misc/log.h"
#include "misc/config.h"
#include "misc/memutil.h"
#include "system/sound.h"

static int sound_samplerate = 44100;

// number of samples per SDL callback
#define SDL_XFER_SAMPLES    512
#define MAX_AUDIO_LATENCY       4

static int sdl_xfer_samples = SDL_XFER_SAMPLES;
static int stream_in_initialized = 0;
static int stream_loop = 0;

static int              initialized_audio = 0;
static int              buf_locked;

static s8             *stream_buffer;
static volatile s32   stream_playpos;

static int           stream_buffer_size;
static int           stream_buffer_in;

// buffer over/underflow counts
static int              buffer_underflows;
static int              buffer_overflows;

// sound enable
static int snd_enabled = 1;

static int          sdl_create_buffers(void);
static void         sdl_destroy_buffers(void);
static void         sdl_cleanup_audio();
static void         SDLCALL sdl_callback(void *userdata, Uint8 *stream, int len);

static void sdl_callback(void *userdata, Uint8 *stream, int len)
{
	int len1, len2, sb_in;

	sb_in = stream_buffer_in;
	if (stream_loop)
		sb_in += stream_buffer_size;

	if (sb_in < (stream_playpos+len))
	{
		return;
	}
	else if ((stream_playpos+len) > stream_buffer_size)
	{
		len1 = stream_buffer_size - stream_playpos;
		len2 = len - len1;
	}
	else
	{
		len1 = len;
		len2 = 0;
	}

	if (snd_enabled)
	{
		memcpy(stream, stream_buffer + stream_playpos, len1);
		memset(stream_buffer + stream_playpos, 0, len1); // no longer needed
		if (len2)
		{
			memcpy(stream+len1, stream_buffer, len2);
			memset(stream_buffer, 0, len2); // no longer needed
		}

	}
	else
	{
		memset(stream, 0, len);
	}

	// move the play cursor
	stream_playpos += len1 + len2;
	if (stream_playpos >= stream_buffer_size)
	{
		stream_playpos -= stream_buffer_size;
		stream_loop = 0;
	}
}

//============================================================
//  lock_buffer
//============================================================
static INLINE int lock_buffer(long offset, long size, void **buffer1, long *length1, void **buffer2, long *length2)
{
//	volatile long pstart, pend, lstart, lend;

	if (!buf_locked)
	{
/*		if (machine.video().throttled())
		{
			pstart = stream_playpos;
			pend = (pstart + sdl_xfer_samples);
			lstart = offset;
			lend = lstart+size;
			while (((pstart >= lstart) && (pstart <= lend)) ||
					((pend >= lstart) && (pend <= lend)))
			{
				pstart = stream_playpos;
				pend = pstart + sdl_xfer_samples;
			}
		}*/

		SDL_LockAudio();
		buf_locked++;
	}

	// init lengths
	*length1 = *length2 = 0;

	if ((offset + size) > stream_buffer_size)
	{
		// 2-piece case
		*length1 = stream_buffer_size - offset;
		*buffer1 = &stream_buffer[offset];
		*length2 = size - *length1;
		*buffer2 = stream_buffer;
	}
	else
	{
		// normal 1-piece case
		*length1 = size;
		*buffer1 = &stream_buffer[offset];
	}

	return 0;
}

//============================================================
//  unlock_buffer
//============================================================
static INLINE void unlock_buffer(void)
{
	buf_locked--;
	if(buf_locked == 0)
		SDL_UnlockAudio();
}

static INLINE void copy_sample_data(s16 *data, int bytes_to_copy)
{
	void *buffer1, *buffer2 = (void *)NULL;
	long length1, length2;
	int cur_bytes;

	// attempt to lock the stream buffer
	if (lock_buffer(stream_buffer_in, bytes_to_copy, &buffer1, &length1, &buffer2, &length2) < 0)
	{
		buffer_underflows++;
		return;
	}

	// adjust the input pointer
	stream_buffer_in += bytes_to_copy;
	if (stream_buffer_in >= stream_buffer_size)
	{
		stream_buffer_in -= stream_buffer_size;
		stream_loop = 1;
	}

	// copy the first chunk
	cur_bytes = (bytes_to_copy > length1) ? length1 : bytes_to_copy;
	memcpy(buffer1, data, cur_bytes);

	// adjust for the number of bytes
	bytes_to_copy -= cur_bytes;
	data = (s16 *)((u8 *)data + cur_bytes);

	// copy the second chunk
	if (bytes_to_copy != 0)
	{
		cur_bytes = (bytes_to_copy > length2) ? length2 : bytes_to_copy;
		memcpy(buffer2, data, cur_bytes);
	}

	// unlock
	unlock_buffer();
}

static INLINE void update_audio_stream(s16 *buffer, int samples_this_frame)
{
	// if nothing to do, don't do it
	if (sound_samplerate != 0 && stream_buffer)
	{
		int bytes_this_frame = samples_this_frame * sizeof(s16);
		int play_position, write_position, stream_in;
		int orig_write; // used in LOG

		play_position = stream_playpos;

		write_position = stream_playpos + ((sound_samplerate / 60) * sizeof(s16));
		orig_write = write_position;

		if (!stream_in_initialized)
		{
			stream_in = stream_buffer_in = (write_position + stream_buffer_size);

			log_printf("stream_in = %d, ", (int)stream_in);
			log_printf("stream_playpos = %d, ", (int)stream_playpos);
			log_printf("write_position = %d\n", (int)write_position);

			// start playing
			SDL_PauseAudio(0);

			stream_in_initialized = 1;
		}
		else
		{
			// normalize the stream in position
			stream_in = stream_buffer_in;
			if (stream_in < write_position && stream_loop == 1)
				stream_in += stream_buffer_size;

			// now we should have, in order:
			//    <------pp---wp---si--------------->

			// if we're between play and write positions, then bump forward, but only in full chunks
			while (stream_in < write_position)
			{
//				log_printf("Underflow: PP=%d  WP=%d(%d)  SI=%d(%d)  BTF=%d\n", (int)play_position, (int)write_position, (int)orig_write, (int)stream_in, (int)stream_buffer_in, (int)bytes_this_frame);
				buffer_underflows++;
				stream_in += bytes_this_frame;
			}

			// if we're going to overlap the play position, just skip this chunk
			if (stream_in + bytes_this_frame > play_position + stream_buffer_size)
			{
//				log_printf("Overflow: PP=%d  WP=%d(%d)  SI=%d(%d)  BTF=%d\n", (int)play_position, (int)write_position, (int)orig_write, (int)stream_in, (int)stream_buffer_in, (int)bytes_this_frame);
				buffer_overflows++;
				return;
			}
		}

		if (stream_in >= stream_buffer_size)
		{
			stream_in -= stream_buffer_size;
			stream_loop = 1;
		}

		// now we know where to copy; let's do it
		stream_buffer_in = stream_in;
		copy_sample_data(buffer, bytes_this_frame);
	}
}


static void sdl_cleanup_audio()
{
	// if nothing to do, don't do it
	if (sound_samplerate == 0)
		return;

	// kill the buffers and dsound
	sdl_destroy_buffers();

	// print out over/underflow stats
	if(buffer_overflows || buffer_underflows)
		log_printf("sdl_cleanup_audio:  overflows=%d underflows=%d\n", buffer_overflows, buffer_underflows);
}

//============================================================
//  dsound_create_buffers
//============================================================

static int sdl_create_buffers(void)
{
	log_printf("sdl_create_buffers:  creating stream buffer of %u bytes\n", stream_buffer_size);
	stream_buffer = (s8*)mem_alloc(stream_buffer_size);
	stream_playpos = 0;
	buf_locked = 0;
	return 0;
}

//============================================================
//  sdl_destroy_buffers
//============================================================

static void sdl_destroy_buffers(void)
{
	// release the buffer
	if(stream_buffer)
		mem_free(stream_buffer);
	stream_buffer = NULL;
}

int sound_init()
{
	int audio_latency;
	SDL_AudioSpec aspec, obtained;
	char audio_driver[16] = "";

	if(config_get_bool("sound.enabled") == 0)
		return(0);

	if(initialized_audio) {
		sound_kill();
	}

	log_printf("sound_init: Start initialization\n");
	SDL_AudioDriverName(audio_driver,sizeof(audio_driver));
	log_printf("sound_init: Driver is %s\n",audio_driver);
	initialized_audio = 0;
	sdl_xfer_samples = SDL_XFER_SAMPLES;
	stream_in_initialized = 0;
	stream_loop = 0;

	// set up the audio specs
	aspec.freq = sound_samplerate;
	aspec.format = AUDIO_S16SYS;    // keep endian independent
	aspec.channels = 1;
	aspec.samples = sdl_xfer_samples;
	aspec.callback = sdl_callback;
	aspec.userdata = 0;

	if(SDL_OpenAudio(&aspec,&obtained) < 0)
		goto cant_start_audio;

	initialized_audio = 1;
	snd_enabled = 1;

	log_printf("sound_init: frequency: %d, channels: %d, samples: %d\n",obtained.freq,obtained.channels,obtained.samples);

	sdl_xfer_samples = obtained.samples;

	audio_latency = 1;

	// compute the buffer sizes
	stream_buffer_size = sound_samplerate * sizeof(s16) * audio_latency / MAX_AUDIO_LATENCY;
	stream_buffer_size = (stream_buffer_size / 1024) * 1024;
	if(stream_buffer_size < 1024)
		stream_buffer_size = 1024;

	// create the buffers
	if (sdl_create_buffers())
		goto cant_create_buffers;

	log_printf("sound_init: End initialization, frame latency = %d\n",audio_latency);
	return(0);

	// error handling
cant_create_buffers:
cant_start_audio:
	log_printf("sound_init: Initialization failed. SDL error: %s\n", SDL_GetError());
	return(1);
}

void sound_kill()
{
	if(initialized_audio) {
		log_printf("sound_kill: closing audio\n");
		SDL_CloseAudio();
		sdl_cleanup_audio();
	}
}

void sound_play()
{
	if(initialized_audio)
		SDL_PauseAudio(0);
}

void sound_pause()
{
	if(initialized_audio)
		SDL_PauseAudio(1);
}

void sound_update(void *buf,int size)
{
	if(initialized_audio)
		update_audio_stream((s16*)buf,size);
}

void sound_setfps(int fps)
{

}
