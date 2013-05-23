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

#ifndef __fds__calls_h__
#define __fds__calls_h__

#include "nes/nes.h"
#include "misc/log.h"

#define HLECALL(name) void hle_ ## name ()

typedef struct fds_disk_header_s {
	u8 blockid;
	u8 ident[14];
	u8 manufacturer_code;
	u8 name[4];
	u8 version;
	u8 diskside;
	u8 disknum;
	u8 extra[2];
	u8 bootid;
	u8 data[31];
	u8 numfiles;
} fds_disk_header_t;

typedef struct fds_file_header_s {
	u8 unk;
	u8 blockid;
	u8 index;
//start of file header on disk
	u8 fileid;
	u8 name[8];
	u16 loadaddr;
	u16 filesize;
	u8 loadarea;
//end of file header on disk
} fds_file_header_t;

typedef struct fds_file_header2_s {
	u8 fileid;
	u8 name[8];
	u16 loadaddr;
	u16 filesize;
	u8 loadarea;
	u16 srcaddr;
	u8 srcarea;
} fds_file_header2_t;

//disk.c
HLECALL(loadbootfiles);
HLECALL(loadfiles);
HLECALL(writefile);
HLECALL(appendfile);
HLECALL(getdiskinfo);
HLECALL(checkfilecount);
HLECALL(adjustfilecount);
HLECALL(setfilecount1);
HLECALL(setfilecount);

//diskutil.c
HLECALL(xferdone);
HLECALL(checkdiskheader);
HLECALL(getnumfiles);
HLECALL(checkblocktype);
HLECALL(filematchtest);
HLECALL(loaddata);

//ppu.c
HLECALL(enpf);
HLECALL(dispf);
HLECALL(enobj);
HLECALL(disobj);
HLECALL(enpfobj);
HLECALL(dispfobj);
HLECALL(setscroll);
HLECALL(spritedma);

//vram.c
HLECALL(vramfill);
HLECALL(vramstructwrite);
HLECALL(writevrambuffer);
HLECALL(readvrambuffer);
HLECALL(preparevramstring);
HLECALL(preparevramstrings);
HLECALL(getvrambufferbyte);
HLECALL(loadtileset);

//pads.c
HLECALL(readpads);
HLECALL(readverifypads);
HLECALL(orpads);
HLECALL(downpads);
HLECALL(readordownpads);
HLECALL(readdownverifypads);
HLECALL(readordownverifypads);
HLECALL(readdownexppads);
HLECALL(readkeyboard);

//util.c
HLECALL(counterlogic);
HLECALL(random);
HLECALL(fetchdirectptr);
HLECALL(jumpengine);
HLECALL(memfill);
HLECALL(pixel2nam);
HLECALL(nam2pixel);
HLECALL(gethcparam);

//misc.c
HLECALL(delay132);
HLECALL(delayms);
HLECALL(vintwait);
HLECALL(unk_EC22);

//vector.c
HLECALL(nmi);
HLECALL(irq);
HLECALL(reset);

#endif
