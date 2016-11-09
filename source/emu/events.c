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

#include "emu/events.h"
#include "emu/emu.h"
#include "misc/log.h"
#include "misc/config.h"
#include "misc/paths.h"
#include "misc/memutil.h"
#include "system/video.h"
#include "system/sound.h"
#include "nes/nes.h"
#include "nes/nes.h"
#include "nes/state/state.h"
#include "mappers/mapperid.h"

static void setfullscreen(int fs)
{
	video_kill();
	sound_pause();
	config_set_bool("video.fullscreen",fs);
	sound_play();
	video_init();
	ppu_sync();
}

int emu_event(int id,void *data)
{
	int ret = 0;
	char dest[1024];
	char *str;

	switch(id) {

		case E_QUIT:
			quit++;
			break;

		case E_LOADROM:
			if((ret = nes_load((char*)data)) == 0) {
				nes_reset(1);
				running = config_get_bool("video.pause_on_load") ? 0 : 1;
			}
			else
				ret = 1;
			break;

		case E_LOADPATCH:
			if(nes->cart == 0)
				break;
			str = mem_strdup(nes->cart->filename);
			if((ret = nes_load_patched(str,(char*)data)) == 0) {
				nes_reset(1);
			}
			mem_free(str);
			break;

		case E_UNLOAD:
			nes_unload();
			break;

		case E_SOFTRESET:
			nes_reset(0);
			break;

		case E_HARDRESET:
			nes_reset(1);
			break;

		case E_LOADSTATE:
			if(nes->cart == 0)
				break;
			paths_makestatefilename(nes->cart->filename,dest,1024);
			nes_loadstate(dest);
			break;

		case E_SAVESTATE:
			if (nes == 0 || nes->cart == 0) {
				log_printf("emu_event:  cannot load state, no rom loaded\n");
				break;
			}
			paths_makestatefilename(nes->cart->filename,dest,1024);
			nes_savestate(dest);
			break;

		case E_RECORDMOVIE:
			log_printf("emu_event:  recording movie from frame %d\n",nes->ppu.frames);
			movie_record();
			break;

		case E_PLAYMOVIE:
			movie_play();
			log_printf("emu_event:  playing movie from frame %d\n",nes->ppu.frames);
			break;

		case E_STOPMOVIE:
			movie_stop();
			log_printf("emu_event:  stopping movie at frame %d\n",nes->ppu.frames);
			break;

		case E_LOADMOVIE:
			movie_load((char*)data);
			break;

		case E_SAVEMOVIE:
			movie_save((char*)data);
			break;

		case E_FLIPDISK:
			if(nes->cart == 0)
				break;
			if((nes->cart->mapperid & B_TYPEMASK) == B_FDS) {
				u8 d[4] = {0,0,0,0};

				nes->mapper->state(CFG_SAVE,d);
				if(d[0] == 0xFF)
					d[0] = 0;
				else {
//					d[0] ^= 1;
					d[0] = (d[0] + 1) & 3;
				}
				nes->mapper->state(CFG_LOAD,d);
				log_printf("disk inserted!  side = %d\n",d[0]);
			}
			else
				log_printf("cannot flip disk.  not fds.\n");
			break;

		case E_DUMPDISK:
			if(nes->cart == 0)
				break;
			if((nes->cart->mapperid & B_TYPEMASK) == B_FDS) {
				FILE *fp;

				log_printf("dumping disk as dump.fds\n");
				if((fp = fopen("dump.fds","wb")) != 0) {
					fwrite(nes->cart->disk.data,1,nes->cart->disk.size,fp);
					fclose(fp);
				}
			}
			break;

		case E_TOGGLERUNNING:
			running ^= 1;
			break;

		case E_PAUSE:
			running = 0;
			break;

		case E_UNPAUSE:
			running = 1;
			break;

		case E_TOGGLEFULLSCREEN:
			setfullscreen(config_get_bool("video.fullscreen") ^ 1);
			break;

		case E_FULLSCREEN:
			setfullscreen(1);
			break;

		case E_WINDOWED:
			setfullscreen(0);
			break;

		//unhandled event
		default:
			log_printf("emu_event:  unhandled event id %d\n",id);
			ret = -1;
			break;
	}
	return(ret);
}
