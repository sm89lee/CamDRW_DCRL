#pragma once

#include "stdafx.h"
#include "DCRL_MNIST_def.h"

#define IMG_FILL_EMPTY		0x01000000
#define IMG_FILL_RND		0x10000000
#define IMG_FILL_RND_ALTER	0x11000000
#define IMG_FILL_RND_DARK	0xF0000000
#define IMG_FILL_RND_LIGHT	0xF1000000

int _CW(const char *src, wchar_t *des, LocIntC len);

int _WC(const wchar_t *src, char *des, LocIntC len);

int _UCW(unsigned const char *src, wchar_t *des, LocIntC len);

int _WCU(const wchar_t *src, unsigned char *des, LocIntC len);

//

int32_t _WI32(const wchar_t *src, LocIntC len);

int parse_img(char* src, ImgDataT *des, LocIntC len);

_W _DW(const double src, LocIntC pNum = 3);

_S _DS(const double src, LocIntC pNum = 3);

//

int put_img_mono(const ImgT srcImg, LocIntC srcWidth, LocIntC srcHeight,
	UINT32 outImg[], LocIntC outWidth, LocIntC outBaseX = 0, LocIntC outBaseY = 0);

int put_ker_mono(const KerT srcKer, LocIntC srcWidth, LocIntC srcHeight,
	UINT32 outImg[], LocIntC outWidth, LocIntC outBaseX = 0, LocIntC outBaseY = 0, LocIntC baseCount = 0xFF);

int put_color(UINT32 outImg[], LocIntC desWidth, LocIntC desHeight, LocIntC outWidth, 
	LocIntC outBaseX = 0, LocIntC outBaseY = 0, const UINT32 iColor = 0);

int put_imgList_mono(const ImgT srcImgData[], LocIntC srcImgList[], LocIntC srcNum, LocIntC srcNumX, LocIntC srcNumY,
	LocIntC srcWidth, LocIntC srcHeight, UINT32 outImg[], const UINT32 iColor = IMG_FILL_RND_ALTER);

/*int put_setPtr_mono(const pImgT srcImgSet[], LocIntC srcNum, LocIntC srcNumX, LocIntC srcNumY,
	LocIntC srcWidth, LocIntC srcHeight, UINT32 outImg[], const UINT32 iColor = IMG_FILL_RND_ALTER);*/

int put_kerSet_mono(const KerT srcKerSet[], LocIntC srcNum, LocIntC srcNumX, LocIntC srcNumY,
	LocIntC srcWidth, LocIntC srcHeight, UINT32 outImg[], const UINT32 iColor = IMG_FILL_RND_ALTER);

int put_conf(ConT *con, LocInt mNum, UINT32 outImg[], LocInt bWidth, LocInt bHeight, LocInt bSpace);