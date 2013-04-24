/* based on implementation by Finn Yannick Jacobs */

#include "crc32.h"

/* crc_tab[] -- this crcTable is being build by chksum_crc32GenTab().
 *		so make sure, you call it before using the other
 *		functions!
 */
static u32 crc_tab[256];

/* crc32() -- to a given block, this one calculates the
 *            crc32-checksum until the length is
 *            reached. the crc32-checksum will be
 *            the result.
 */
u32 crc32(unsigned char *block,unsigned int length)
{
	register unsigned long crc;
	unsigned long i;

	crc = 0xFFFFFFFF;
	for(i=0;i<length;i++) {
		crc = ((crc >> 8) & 0x00FFFFFF) ^ crc_tab[(crc ^ *block++) & 0xFF];
	}
	return(crc ^ 0xFFFFFFFF);
}

/* crc32gentab() -- to a global crc_tab[256], this one will
 *                  calculate the crcTable for crc32-checksums.
 *                  it is generated to the polynom [..]
 */

void crc32_gentab()
{
	unsigned long crc,poly;
	int i,j;

	poly = 0xEDB88320L;
	for(i=0;i<256;i++) {
		crc = i;
		for(j=8;j>0;j--) {
			if(crc & 1)
				crc = (crc >> 1) ^ poly;
			else
				crc >>= 1;
		}
      crc_tab[i] = crc;
	}
}
