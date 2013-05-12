/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2008 QMT Productions
 *
 * $URL: file:///c:/svnroot/Mappers/src/Hardware/Sound/s_VRC6.h $
 * $Id: s_VRC6.h 376 2008-06-29 20:58:13Z Quietust $
 */

#ifndef	S_VRC6_H
#define	S_VRC6_H

void		VRC6sound_Load		(void);
void		VRC6sound_Reset		(void);
void		VRC6sound_Unload	(void);
void		VRC6sound_Write		(int,int);
int	VRC6sound_Get		(int);
int	VRC6sound_SaveLoad	(int,int,unsigned char *);

#endif	/* S_VRC6_H */
