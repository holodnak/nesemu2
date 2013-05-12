/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2008 QMT Productions
 *
 * $URL: file:///c:/svnroot/Mappers/src/Hardware/Sound/s_VRC7.h $
 * $Id: s_VRC7.h 376 2008-06-29 20:58:13Z Quietust $
 */

#ifndef	S_VRC7_H
#define	S_VRC7_H

void		VRC7sound_Load		(void);
void		VRC7sound_Reset		(void);
void		VRC7sound_Unload	(void);
void		VRC7sound_Write		(int,int);
int	VRC7sound_Get		(int);
int	VRC7sound_SaveLoad	(int,int,unsigned char *);

#endif	/* S_VRC7_H */
