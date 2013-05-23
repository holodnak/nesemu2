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
#include "mappers/fds/fds.h"
#include "mappers/fds/hle.h"

HLECALL(loadbootfiles)
{
	fds_disk_header_t disk_header;
	fds_file_header_t file_header;
	int i,j,pos;
	u8 loadedfiles = 0;

	//change reset action
	cpu_write(0x103,0x53);

	//leet hax
	log_hle("loadbootfiles:  setting diskside to 0\n");
	diskside = 0;
	pos = 0;

	//load disk header
	pos = diskside * 65500;
	memcpy(&disk_header,nes.cart->disk.data + pos,sizeof(fds_disk_header_t));
	pos += 57;

	//load boot files
	for(i=0;i<disk_header.numfiles;i++) {
		char fn[10] = "        \0";

		memcpy(&file_header,nes.cart->disk.data + pos,sizeof(fds_file_header_t));
		pos += 17;
		if(file_header.fileid <= disk_header.bootid) {
			memcpy(fn,file_header.name,8);
			log_hle("loadbootfiles: loading file '%s', id %d, size $%X, load addr $%04X\n",fn,file_header.fileid,file_header.filesize,file_header.loadaddr);
			loadedfiles++;

			//load into cpu
			if(file_header.loadarea == 0) {
				for(j=0;j<file_header.filesize;j++)
					cpu_write(j + file_header.loadaddr,nes.cart->disk.data[pos + j + 1]);
			}

			//load into ppu
			else {
				for(j=0;j<file_header.filesize;j++)
					ppu_memwrite(j + file_header.loadaddr,nes.cart->disk.data[pos + j + 1]);
			}
		}
		pos += file_header.filesize;
	}

	//put loaded files into y register
	nes.cpu.y = loadedfiles;

	//put error code in accumulator
	nes.cpu.a = 0;
}

HLECALL(loadfiles)
{
	fds_disk_header_t disk_header;
	fds_file_header_t file_header;
	u8 diskid,loadedfiles = 0;
	u32 addr1,addr2;
	int pos,i,j,k;
	u8 fileidlist[0x20];

	//bios needs these flags set
	nes.cpu.flags.z = 1;
	nes.cpu.flags.n = 0;
	nes.cpu.flags.v = 0;
	nes.cpu.flags.c = 0;
	nes.cpu.a = 0;
	nes.cpu.y = 0;

	if(diskside == 0xFF) {
		log_hle("loadfiles:  hax:  disk not inserted, setting to 0\n");
		diskside = 0;
//		log_hle("loadfiles:  aborting, disk not inserted\n");
//		return;
	}

	addr1 = hle_getparam(0);
	addr2 = hle_getparam(1);
	hle_fixretaddr(2);

	log_hle("loadfiles: addr1,2 = $%04X, $%04X\n",addr1,addr2);

	//get diskid
	diskid = cpu_read(addr1);

	//load file id list
	for(i=0;i<20;i++) {
		fileidlist[i] = cpu_read(addr2 + i);
		log_hle("loadfiles: id list[%d] = $%02X\n",i,fileidlist[i]);
		if(fileidlist[i] == 0xFF)
			break;
	}

	log_hle("loadfiles: disk id required = %d, disknum = %d\n",diskid,diskside);

	//read in disk header
	pos = diskside * 65500;
	memcpy(&disk_header,nes.cart->disk.data + pos,sizeof(fds_disk_header_t));
	pos += 57;

	//$FF indicates to load system boot files
	if(fileidlist[0] == 0xFF) {
		hle_loadbootfiles();
		return;
	}

	//load files from id list
	for(i=0;i<disk_header.numfiles;i++) {
		char fn[10] = "        \0";
		int wasloaded = 0;

		memcpy(&file_header,nes.cart->disk.data + pos,sizeof(fds_file_header_t));
		pos += 17;                 
		memcpy(fn,file_header.name,8);
		for(k=0;fileidlist[k] != 0xFF && k < 20;k++) {
			if(file_header.fileid == 0xE0) {
				log_hle("loadfiles: file '%s', id $%X, size $%X, load addr $%04X (data @ %d, side %d)\n",fn,file_header.fileid,file_header.filesize,file_header.loadaddr,pos % 65500,diskside);
			}
			if(file_header.fileid == fileidlist[k]) {
				log_hle("loadfiles: loading file '%s', id $%X, size $%X, load addr $%04X\n",fn,file_header.fileid,file_header.filesize,file_header.loadaddr);
				loadedfiles++;       
				wasloaded = 1;
				//load into cpu
				if(file_header.loadarea == 0) {
					for(j=0;j<file_header.filesize;j++)
						cpu_write(j + file_header.loadaddr,nes.cart->disk.data[pos + j + 1]);
				}

				//load into ppu
				else {
					for(j=0;j<file_header.filesize;j++)
						ppu_memwrite(j + file_header.loadaddr,nes.cart->disk.data[pos + j + 1]);
				}
			}
		}
		if(wasloaded == 0)
			log_hle("loadfiles: skipping file '%s', id $%X, size $%X, load addr $%04X\n",fn,file_header.fileid,file_header.filesize,file_header.loadaddr);
		pos += file_header.filesize;
	}

	log_hle("loadfiles: loaded %d files\n",loadedfiles);

	//put loaded files into y register
	nes.cpu.y = loadedfiles;

	//put error code in accumulator
	nes.cpu.a = 0;
}

HLECALL(writefile)
{
	fds_disk_header_t *disk_header;
	fds_file_header_t *file_header;
	fds_file_header2_t file_header2;
	u32 addr1,addr2,pos;
	char fn[10] = "        \0";
	int i;

	addr1 = hle_getparam(0);
	addr2 = hle_getparam(1);
	hle_fixretaddr(2);

	log_hle("hle writefile: addr1 = %04X, addr2 = %04X\n",addr1,addr2);

	//position of disk data start
	pos = diskside * 65500;

	//pointer to disk header
	disk_header = (fds_disk_header_t*)(nes.cart->disk.data + pos);

	log_hle("hle writefile: %d files on disk (A = %d)\n",disk_header->numfiles,nes.cpu.a);

	//seek past the disk header
	pos += 57;

	log_hle("file header: ");
	for(i=0;i<17;i++) {
		log_hle(" $%02X",cpu_read(addr2 + i));
	}
	log_hle("\n");

	//fill the file header2 struct
	file_header2.fileid = cpu_read(addr2 + 0);
	for(i=0;i<8;i++)
		file_header2.name[i] = cpu_read(addr2 + 1);
	file_header2.loadaddr = cpu_read(addr2 + 9) | (cpu_read(addr2 + 10) << 8);
	file_header2.filesize = cpu_read(addr2 + 11) | (cpu_read(addr2 + 12) << 8);
	file_header2.loadarea = cpu_read(addr2 + 13);
	file_header2.srcaddr = cpu_read(addr2 + 14) | (cpu_read(addr2 + 15) << 8);
	file_header2.srcarea = cpu_read(addr2 + 16);

	//show some debug info
	memcpy(fn,&file_header2.name,8);
	log_hle("writing file, header:\n");
	log_hle("  fileid: %02X\n",file_header2.fileid);
	log_hle("  name:  '%8s'\n",fn);
	log_hle("  loadaddr: %04X\n",file_header2.loadaddr);
	log_hle("  filesize: %04X\n",file_header2.filesize);
	log_hle("  loadarea: %02X\n",file_header2.loadarea);
	log_hle("  srcaddr: %04X\n",file_header2.srcaddr);
	log_hle("  srcarea: %02X\n",file_header2.srcarea);

	//loop thru all the files
	for(i=0;i<disk_header->numfiles;i++) {
		file_header = (fds_file_header_t*)(nes.cart->disk.data + pos);
		pos += 17;
		if(file_header->fileid == file_header2.fileid) {
			log_hle("hle writefile: match! '%s', id $%X, size $%X, load addr $%04X\n",fn,file_header->fileid,file_header->filesize,file_header->loadaddr);
			log_hle("hle writefile: fileid match, writing at %d bytes @ %d (side %d)\n",file_header2.filesize,pos % 65500,diskside);

			memcpy(file_header->name,file_header2.name,8);
			file_header->loadaddr = file_header2.loadaddr;
			file_header->filesize = file_header2.filesize;
			file_header->loadarea = file_header2.loadarea;
			for(i=0;i<file_header2.filesize;i++) {
				if(file_header2.srcarea == 0)
					nes.cart->disk.data[pos + i + 1] = cpu_read(file_header2.srcaddr + i);
				else
					nes.cart->disk.data[pos + i + 1] = ppu_memread(file_header2.srcaddr + i);
			}
			nes.cart->disk.data[pos + i + 1] = 0xFF;
			break;
		}
		pos += file_header->filesize;
		memcpy(fn,file_header->name,8);
		log_hle("hle writefile: seeking over '%s', id $%X, size $%X, load addr $%04X\n",fn,file_header->fileid,file_header->filesize,file_header->loadaddr);
	}

	if(nes.cpu.a != 0xFF)
		disk_header->numfiles = nes.cpu.a + 1;
	else
		log_printf("appendfile!\n");

	cpu_write(6,disk_header->numfiles);

	nes.cpu.a = 0;
	nes.cpu.x = 0;
	nes.cpu.flags.n = 0;
	nes.cpu.flags.z = 1;
}

HLECALL(appendfile)
{
	log_printf("appendfile!\n");
	nes.cpu.a = 0xFF;
	hle_writefile();
}

HLECALL(getdiskinfo)
{
	log_printf("getdiskinfo not implemented\n");
}

HLECALL(checkfilecount)
{
	log_printf("checkfilecount not implemented\n");
}

HLECALL(adjustfilecount)
{
	log_printf("adjustfilecount not implemented\n");
}

HLECALL(setfilecount1)
{
	log_printf("setfilecount1 not implemented\n");
}

HLECALL(setfilecount)
{
	log_printf("setfilecount not implemented\n");
}
