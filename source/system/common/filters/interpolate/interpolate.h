#ifndef __interpolate_h__
#define __interpolate_h__

#include "types.h"

void interpolate2x(void *dst,u32 dst_slice,void *src,u32 src_slice,u32 width,u32 height);
void interpolate3x(void *dst,u32 dst_slice,void *src,u32 src_slice,u32 width,u32 height);
void interpolate4x(void *dst,u32 dst_slice,void *src,u32 src_slice,u32 width,u32 height);

#endif
