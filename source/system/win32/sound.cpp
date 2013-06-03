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
#include <dsound.h>

extern "C" {
	#include "misc/log.h"
	#include "system/sound.h"
	#include "system/win32/mainwnd.h"
}

static LPDIRECTSOUND DirectSound;
static LPDIRECTSOUNDBUFFER PrimaryBuffer;
static LPDIRECTSOUNDBUFFER Buffer;
static unsigned long next_pos;
static int isEnabled;
static int			buflen;
static unsigned long		MHz;
static unsigned long		LockSize;
static int			BufPos;

static int sound_bps = 16;
static int sound_samplerate = 44100;
static int sound_fragsize = 1024;//SOUND_HZ / 60;
static void (*audio_callback)(void *buffer, int length) = 0;
static int soundinited = 0;

#define	FREQ		44100
#define	BITS		16
#define	FRAMEBUF	4
const	unsigned int	LOCK_SIZE = FREQ * (BITS / 8);

#define Try(action,errormsg) \
	do {\
		if(FAILED(action)) {\
			Stop();\
			Start();\
			if(FAILED(action)) {\
				sound_pause();\
				MessageBox(hWnd,errormsg,"nesemu2",MB_OK | MB_ICONERROR);\
				return;\
			}\
		}\
	} while (false)

void Stop();

void	Start()
{
	WAVEFORMATEX WFX;
	DSBUFFERDESC DSBD;

	if(DirectSound == 0)
		return;

	ZeroMemory(&DSBD, sizeof(DSBUFFERDESC));
	DSBD.dwSize = sizeof(DSBUFFERDESC);
	DSBD.dwFlags = DSBCAPS_PRIMARYBUFFER;
	DSBD.dwBufferBytes = 0;
	DSBD.lpwfxFormat = NULL;
	if(FAILED(DirectSound->CreateSoundBuffer(&DSBD, &PrimaryBuffer, NULL))) {
		Stop();
		MessageBox(hWnd,"Failed to create primary buffer!","nesemu2",MB_OK);
		return;
	}
	ZeroMemory(&WFX, sizeof(WAVEFORMATEX));
	WFX.wFormatTag = WAVE_FORMAT_PCM;
	WFX.nChannels = 1;
	WFX.nSamplesPerSec = FREQ;
	WFX.wBitsPerSample = BITS;
	WFX.nBlockAlign = WFX.wBitsPerSample / 8 * WFX.nChannels;
	WFX.nAvgBytesPerSec = WFX.nSamplesPerSec * WFX.nBlockAlign;
	if(FAILED(PrimaryBuffer->SetFormat(&WFX))) {
		Stop();
		MessageBox(hWnd,"Failed to set output format!","nesemu2", MB_OK);
		return;
	}
	if(FAILED(PrimaryBuffer->Play(0, 0, DSBPLAY_LOOPING))) {
		Stop();
		MessageBox(hWnd,"Failed to start playing primary buffer!","nesemu2", MB_OK);
		return;
	}
	DSBD.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_LOCSOFTWARE | DSBCAPS_GETCURRENTPOSITION2;
	DSBD.dwBufferBytes = LockSize * FRAMEBUF;
	DSBD.lpwfxFormat = &WFX;
	if(FAILED(DirectSound->CreateSoundBuffer(&DSBD, &Buffer, NULL))) {
		Stop();
		MessageBox(hWnd,"Failed to create secondary buffer!","nesemu2",MB_OK);
		return;
	}
	log_printf("dsound init ok, %ihz, %i bits",WFX.nSamplesPerSec,WFX.wBitsPerSample);
}

void	Stop (void)
{
	if(Buffer) {
		sound_pause();
		Buffer->Release();
		Buffer = 0;
	}
	if(PrimaryBuffer) {
		PrimaryBuffer->Stop();
		PrimaryBuffer->Release();
		PrimaryBuffer = 0;
	}
}

int sound_init()
{
	DirectSound	= 0;
	PrimaryBuffer	= 0;
	Buffer		= 0;
	isEnabled	= 0;
	MHz		= 1;
	LockSize	= 1;
	buflen		= 0;
	BufPos		= 0;
	next_pos	= 0;
	LockSize = LOCK_SIZE / 60;
	buflen = LockSize / (BITS / 8);
	if(FAILED(DirectSoundCreate(&DSDEVID_DefaultPlayback, &DirectSound, NULL))) {
		sound_kill();
		MessageBox(hWnd,"Error creating DirectSound interface","nesemu2",MB_OK);
		return(1);
	}
	if(FAILED(DirectSound->SetCooperativeLevel(hWnd, DSSCL_PRIORITY))) {
		sound_kill();
		MessageBox(hWnd,"Error setting DirectSound cooperative level","nesemu2", MB_OK);
		return(1);
	}
	sound_play();
	return(0);
}

void sound_kill()
{
	Stop();
	if(DirectSound) {
		DirectSound->Release();
		DirectSound = 0;
	}
}

void sound_play()
{
	LPVOID bufPtr;
	DWORD bufBytes;

	if(isEnabled)
		return;
	if(Buffer == 0) {
		Start();
		if(Buffer == 0)
			return;
	}
	Try(Buffer->Lock(0,0,&bufPtr,&bufBytes,NULL,0,DSBLOCK_ENTIREBUFFER),"Error locking sound buffer");
	ZeroMemory(bufPtr,bufBytes);
	Try(Buffer->Unlock(bufPtr,bufBytes,NULL,0),"Error unlocking sound buffer");
	isEnabled = TRUE;
	Try(Buffer->Play(0,0,DSBPLAY_LOOPING),"Unable to start playing sound buffer");
	next_pos = 0;
}

void sound_pause()
{
	if(isEnabled == 0)
		return;
	isEnabled = 0;
	if(Buffer)
		Buffer->Stop();
}

void sound_setcallback(void (*cb)(void *buffer, int length))
{
	audio_callback = cb;
}

void sound_update(void *buffer,int length)
{
	LPVOID bufPtr;
	DWORD bufBytes;
	unsigned long rpos, wpos;

	do {
		Sleep(1);
		if(isEnabled == 0)
			return;
		Try(Buffer->GetCurrentPosition(&rpos, &wpos),"Error getting audio position");
		rpos /= LockSize;
		wpos /= LockSize;
		if (wpos < rpos)
			wpos += FRAMEBUF;
	} while ((rpos <= next_pos) && (next_pos <= wpos));
	if(isEnabled) {
		Try(Buffer->Lock(next_pos * LockSize,LockSize,&bufPtr,&bufBytes,NULL,0,0),"Error locking sound buffer");
		memcpy(bufPtr, buffer, bufBytes);
		Try(Buffer->Unlock(bufPtr,bufBytes,NULL,0),"Error unlocking sound buffer");
		next_pos = (next_pos + 1) % FRAMEBUF;
	}
}
