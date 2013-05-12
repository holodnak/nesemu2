/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2008 QMT Productions
 *
 * $URL: file:///c:/svnroot/Mappers/src/Hardware/Sound/s_MMC5.h $
 * $Id: s_MMC5.h 376 2008-06-29 20:58:13Z Quietust $
 */

#ifndef	S_MMC5_H
#define	S_MMC5_H

void		MMC5sound_Load		(void);
void		MMC5sound_Reset		(int);
void		MMC5sound_Unload	(void);
void		MMC5sound_Write		(int,int);
int		MMC5sound_Read		(int);
int		MMC5sound_Get		(int);
int		MMC5sound_SaveLoad	(int,int,unsigned char *);

#endif	/* S_MMC5_H */
