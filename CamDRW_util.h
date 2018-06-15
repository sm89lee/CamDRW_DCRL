#pragma once

#include "stdafx.h"
#include "CamDRW_def.h"

#define IMG_FILL_EMPTY		0x01000000
#define IMG_FILL_RND		0x10000000
#define IMG_FILL_RND_ALTER	0x11000000
#define IMG_FILL_RND_DARK	0xF0000000
#define IMG_FILL_RND_LIGHT	0xF1000000

int _CW(const char *src, wchar_t *des, const int len);

int _WC(const wchar_t *src, char *des, const int len);

int32_t _WI32(const wchar_t *src, const int len);

int32_t _CI32(const char *src, const int len);

int _IW32(int32_t src, wchar_t *des);

_W _DW(const double src, const int pNum = 3);

_S _DS(const double src, const int pNum = 3);

//