/* Nintendulator Mapper DLLs
 * Copyright (C) 2002-2008 QMT Productions
 *
 * $URL: file:///c:/svnroot/Mappers/src/Hardware/Sound/s_FDS.h $
 * $Id: s_FDS.h 376 2008-06-29 20:58:13Z Quietust $
 */

#ifndef	S_FDS_H
#define	S_FDS_H

void		FDSsound_Load		(void);
void		FDSsound_Reset		(void);
void		FDSsound_Unload		(void);
int		FDSsound_Read		(int);
void		FDSsound_Write		(int,int);
int	FDSsound_Get		(int);
int	FDSsound_SaveLoad	(int,int,unsigned char *);

#endif	/* S_FDS_H */
