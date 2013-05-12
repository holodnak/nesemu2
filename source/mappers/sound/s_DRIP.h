#ifndef __s_DRIP_h__
#define __s_DRIP_h__

void	DRIPsound_Load		(void);
void	DRIPsound_Reset	(void);
void	DRIPsound_Unload	(void);
int	DRIPsound_Read		(int);
void	DRIPsound_Write	(int,int);
int	DRIPsound_Get		(int);
int	DRIPsound_SaveLoad(int statetype,int,unsigned char *);

#endif
