#include "mappers/mapperinc.h"

static u8 reg;
static u8 irqenable,irqmode;
static readfunc_t read4;
static writefunc_t write4;

static void sync()
{
	mem_setprg32(8,reg >> 4);
	mem_setchr8(0,reg & 0xF);
}

static u8 read_reg(u32 addr)
{
	if(addr < 0x4020)
		return(read4(addr));
	log_printf("txc-strikewolf.c:  read_reg:  $%04X\n",addr);
	return(0);
}

static void write_reg(u32 addr,u8 data)
{
	if(addr < 0x4020) {
		write4(addr,data);
		return;
	}
	switch(addr) {
		case 0x4100:
			break;
		case 0x4101:
			break;
		case 0x4102:
			break;
		case 0x4103:
			irqenable = 0;
			break;
		case 0x4104:
			irqenable = 1;
			break;
	}
	log_printf("txc-strikewolf.c:  write_reg:  $%04X = $%02X\n",addr,data);
}

static void write(u32 addr,u8 data)
{
	if(addr >= 0x8400 && addr < 0xFFFF)
		reg = data;
	else
		log_printf("txc-strikewolf.c:  write:  $%04X = $%02X\n",addr,data);
	sync();
}

static void reset(int hard)
{
	int i;

	read4 = mem_getreadfunc(4);
	write4 = mem_getwritefunc(4);
	mem_setreadfunc(4,read_reg);
	mem_setwritefunc(4,write_reg);
	for(i=8;i<0x10;i++)
		mem_setwritefunc(i,write);
	reg = 0;
	irqenable = 0;
	irqmode = 0;
	sync();
}

static void ppucycle()
{

}

static void state(int mode,u8 *data)
{
	STATE_U8(reg);
	sync();
}

MAPPER(B_TXC_STRIKEWOLF,reset,0,ppucycle,0,state);
