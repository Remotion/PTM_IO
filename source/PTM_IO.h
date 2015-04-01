#ifndef _PTM_IO_RW_H_
#define _PTM_IO_RW_H_

#include "PTM_Types.h"

LONG ReadGrayRAW(BaseFile *file, TMap2d<UCHAR> &bmp,
				 LONG width,LONG height, LONG format, LONG part);

LONG ReadGrayRAW(BaseFile *file, TMap2d<TryByte> &bmp,
				 LONG width,LONG height, LONG format, LONG part);

LONG ReadA6RAW(BaseFile *file, TMap2d<TryByte> &a123, 
			   TMap2d<TryByte> &a456,LONG width,LONG height);

LONG ReadRGBRAW(BaseFile *file, TMap2d<TryByte> &rgb, 
				LONG width,LONG height);
/*
LONG ReadTryJPG(BaseFile *file, TMap2d<TryByte> &rgb, 
				LONG fileRpos, LONG fileGpos,LONG fileBpos);//, LONG frame)
*/
//TODO: ->>>>
LONG WriteLRGB(	Filename name, 
				TMap2d<Vector> &t_rgb, 
				TMap2d<Vector> &t_a012, 
				TMap2d<Vector> &t_a345,
				Vector t_scale012,
				Vector t_scale345,
				Vector t_bias012, 
				Vector t_bias345
			   );




#endif