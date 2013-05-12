/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2008 QMT Productions
 *
 * $URL: file:///c:/svnroot/Mappers/src/Hardware/Sound/s_FME7.h $
 * $Id: s_FME7.h 376 2008-06-29 20:58:13Z Quietust $
 */

#ifndef	S_FME7_H
#define	S_FME7_H

void		FME7sound_Load		(void);
void		FME7sound_Reset	(void);
void		FME7sound_Unload	(void);
void		FME7sound_Write		(int,int);
int	FME7sound_Get		(int);
int	FME7sound_SaveLoad	(int statetype,int,unsigned char *);

#endif	/* S_FME7_H */
