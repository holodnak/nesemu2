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

#include "mappers/fds/calls.h"
#include "mappers/fds/hle.h"

HLECALL(readpads)
{
	int i;
	u8 data,tmp1,tmp2,tmp3,tmp4;

	data = cpu_read(0xFB);
	cpu_write(0x4016,data + 1);
	cpu_write(0x4016,data);
	for(tmp1=tmp2=tmp3=tmp4=0,i=0;i<8;i++) {
		tmp1 <<= 1;
		tmp2 <<= 1;
		tmp3 <<= 1;
		tmp4 <<= 1;
		data = cpu_read(0x4016);
		tmp1 |= data & 1;
		tmp2 |= (data >> 1) & 1;
		data = cpu_read(0x4017);
		tmp3 |= data & 1;
		tmp4 |= (data >> 1) & 1;
	}
	cpu_write(0xF5,tmp1);
	cpu_write(0x00,tmp2);
	cpu_write(0xF6,tmp3);
	cpu_write(0x01,tmp4);
	nes->cpu.flags.z = 1;
	nes->cpu.flags.n = 0;
}

HLECALL(readverifypads)
{
	u8 tmp[4];

	//read initial pad state
	hle_readpads();

	//save initial pad state
	tmp[0] = cpu_read(0xF5);
	tmp[1] = cpu_read(0x00);
	tmp[2] = cpu_read(0xF6);
	tmp[3] = cpu_read(0x01);

	//keep looping until reads match
	for(;;) {

		//read pad data
		hle_readpads();

		//check for match
		if(tmp[0] == cpu_read(0xF5) &&
			tmp[1] == cpu_read(0x00) &&
			tmp[2] == cpu_read(0xF6) &&
			tmp[3] == cpu_read(0x01))
			break;

		//save pad state
		tmp[0] = cpu_read(0xF5);
		tmp[1] = cpu_read(0x00);
		tmp[2] = cpu_read(0xF6);
		tmp[3] = cpu_read(0x01);
	}
	log_printf("readverifypads\n");
}

HLECALL(orpads)
{
	cpu_write(0xF5,cpu_read(0x00) | cpu_read(0xF5));
	cpu_write(0xF6,cpu_read(0x01) | cpu_read(0xF6));
}

HLECALL(downpads)
{
	int i;

	for(i = 1; i >= 0; i--) {
		nes->cpu.a = cpu_read(0xF5 + i);
		nes->cpu.y = nes->cpu.a;
		nes->cpu.a ^= cpu_read(0xF7 + i);
		nes->cpu.a &= cpu_read(0xF5 + i);
		cpu_write(0xF5 + i,nes->cpu.a);
		cpu_write(0xF7 + i,nes->cpu.y);
	}
}

HLECALL(readordownpads)
{
	hle_readpads();
	hle_orpads();
	hle_downpads();
	log_hle("readordownpads\n");
}

HLECALL(readdownverifypads)
{
	hle_readverifypads();
	hle_downpads();
}

HLECALL(readordownverifypads)
{
	hle_readverifypads();
	hle_orpads();
	hle_downpads();
}

HLECALL(readdownexppads)
{
	hle_readpads();
	//now read expansion port!
	hle_downpads();
//log_printf("readdownexppads not implemented\n");
}

HLECALL(readkeyboard)
{
	log_printf("readkeyboard not implemented\n");
}
