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

#ifndef __cart_unif_h__
#define __cart_unif_h__

#include "nes/cart/cart.h"
#include "nes/state/block.h"

#define ID_MAPR	MAKEID('M','A','P','R')
#define ID_NAME	MAKEID('N','A','M','E')
#define ID_MIRR	MAKEID('M','I','R','R')
#define ID_WRTR	MAKEID('W','R','T','R')
#define ID_BATR	MAKEID('B','A','T','R')
#define ID_TVCI	MAKEID('T','V','C','I')
#define ID_PRG0	MAKEID('P','R','G','0')
#define ID_PRG1	MAKEID('P','R','G','1')
#define ID_PRG2	MAKEID('P','R','G','2')
#define ID_PRG3	MAKEID('P','R','G','3')
#define ID_PRG4	MAKEID('P','R','G','4')
#define ID_PRG5	MAKEID('P','R','G','5')
#define ID_PRG6	MAKEID('P','R','G','6')
#define ID_PRG7	MAKEID('P','R','G','7')
#define ID_PRG8	MAKEID('P','R','G','8')
#define ID_PRG9	MAKEID('P','R','G','9')
#define ID_PRGA	MAKEID('P','R','G','A')
#define ID_PRGB	MAKEID('P','R','G','B')
#define ID_PRGC	MAKEID('P','R','G','C')
#define ID_PRGD	MAKEID('P','R','G','D')
#define ID_PRGE	MAKEID('P','R','G','E')
#define ID_PRGF	MAKEID('P','R','G','F')
#define ID_CHR0	MAKEID('C','H','R','0')
#define ID_CHR1	MAKEID('C','H','R','1')
#define ID_CHR2	MAKEID('C','H','R','2')
#define ID_CHR3	MAKEID('C','H','R','3')
#define ID_CHR4	MAKEID('C','H','R','4')
#define ID_CHR5	MAKEID('C','H','R','5')
#define ID_CHR6	MAKEID('C','H','R','6')
#define ID_CHR7	MAKEID('C','H','R','7')
#define ID_CHR8	MAKEID('C','H','R','8')
#define ID_CHR9	MAKEID('C','H','R','9')
#define ID_CHRA	MAKEID('C','H','R','A')
#define ID_CHRB	MAKEID('C','H','R','B')
#define ID_CHRC	MAKEID('C','H','R','C')
#define ID_CHRD	MAKEID('C','H','R','D')
#define ID_CHRE	MAKEID('C','H','R','E')
#define ID_CHRF	MAKEID('C','H','R','F')
#define ID_PCK0	MAKEID('P','C','K','0')
#define ID_PCK1	MAKEID('P','C','K','1')
#define ID_PCK2	MAKEID('P','C','K','2')
#define ID_PCK3	MAKEID('P','C','K','3')
#define ID_PCK4	MAKEID('P','C','K','4')
#define ID_PCK5	MAKEID('P','C','K','5')
#define ID_PCK6	MAKEID('P','C','K','6')
#define ID_PCK7	MAKEID('P','C','K','7')
#define ID_PCK8	MAKEID('P','C','K','8')
#define ID_PCK9	MAKEID('P','C','K','9')
#define ID_PCKA	MAKEID('P','C','K','A')
#define ID_PCKB	MAKEID('P','C','K','B')
#define ID_PCKC	MAKEID('P','C','K','C')
#define ID_PCKD	MAKEID('P','C','K','D')
#define ID_PCKE	MAKEID('P','C','K','E')
#define ID_PCKF	MAKEID('P','C','K','F')
#define ID_CCK0	MAKEID('C','C','K','0')
#define ID_CCK1	MAKEID('C','C','K','1')
#define ID_CCK2	MAKEID('C','C','K','2')
#define ID_CCK3	MAKEID('C','C','K','3')
#define ID_CCK4	MAKEID('C','C','K','4')
#define ID_CCK5	MAKEID('C','C','K','5')
#define ID_CCK6	MAKEID('C','C','K','6')
#define ID_CCK7	MAKEID('C','C','K','7')
#define ID_CCK8	MAKEID('C','C','K','8')
#define ID_CCK9	MAKEID('C','C','K','9')
#define ID_CCKA	MAKEID('C','C','K','A')
#define ID_CCKB	MAKEID('C','C','K','B')
#define ID_CCKC	MAKEID('C','C','K','C')
#define ID_CCKD	MAKEID('C','C','K','D')
#define ID_CCKE	MAKEID('C','C','K','E')
#define ID_CCKF	MAKEID('C','C','K','F')

int cart_load_unif(cart_t *ret,const char *filename);

#endif
