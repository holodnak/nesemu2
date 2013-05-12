/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2008 QMT Productions
 *
 * $URL: file:///c:/svnroot/Mappers/src/Hardware/Sound/s_N106.h $
 * $Id: s_N106.h 376 2008-06-29 20:58:13Z Quietust $
 */

#ifndef	S_N106_H
#define	S_N106_H

void		N106sound_Load		(void);
void		N106sound_Reset		(void);
void		N106sound_Unload	(void);
int		N106sound_Read		(int);
void		N106sound_Write		(int,int);
int	N106sound_Get		(int);
int	N106sound_SaveLoad	(int statetype,int,unsigned char *);

#endif	/* S_N106_H */
