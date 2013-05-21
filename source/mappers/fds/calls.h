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

//disk.c
HLECALL(loadbootfiles);
HLECALL(loadfiles);
HLECALL(writefile);
HLECALL(getdiskinfo);
HLECALL(checkfilecount);
HLECALL(adjustfilecount);
HLECALL(setfilecount1);
HLECALL(setfilecount);

//diskutil.c
HLECALL(xferdone);

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
HLECALL(unk_EC22);

//misc.c
HLECALL(delay132);
HLECALL(delayms);
HLECALL(vintwait);

//vector.c
HLECALL(nmi);
HLECALL(irq);
HLECALL(reset);

#endif
