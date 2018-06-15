#include "stdafx.h"
#include "DCRL_MNIST_util.h"

int _CW(const char *src, wchar_t *des, LocIntC len) {
	int i = 0;
	while (i < len) {
		des[i] = src[i];
		if (src[i] == 0) { break; }
		i++;
	}

	return i;
}

int _UCW(unsigned const char *src, wchar_t *des, LocIntC len) {
	int i = 0;
	while (i < len) {
		des[i] = src[i];
		if (src[i] == 0) { break; }
		i++;
	}

	return i;
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

int _WC(const wchar_t *src, char *des, LocIntC len) {
	int i = 0, j = 0;
	while (i < len) {
		des[j] = src[i] & 0xFF;
		if (src[i] > 0xFF) {
			des[++j] = src[i] >> 8;
		} else if (src[i] == 0) {
			break;
		}
		i++;
		j++;
	}

	return j;
}

int _WCU(const wchar_t *src, unsigned char *des, LocIntC len) {
	int i = 0, j = 0;
	while (i < len) {
		des[j] = src[i] & 0xFF;
		if (src[i] > 0xFF) {
			des[++j] = src[i] >> 8;
		} else if (src[i] == 0) {
			break;
		}
		i++;
		j++;
	}

	return j;
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

int32_t _WI32(const wchar_t *src, LocIntC len) { // wchar to int32
	LocInt i = 0;
	int32_t r = 0;
	while (i < len) {
		if (src[i] >= '0' && src[i] <= '9') {
			r = r * 10 + src[i] - '0';
		} else if (src[i] == 0) {
			break;
		}
		i++;
	}

	return r;
}

int parse_img(char* src, ImgDataT *des, LocIntC len) {
	LocInt srcLoc = 0, desLoc = 0, desVal = 0;
	while (desLoc != len) {
		if (src[srcLoc] >= '0' && src[srcLoc] <= '9') {
			desVal = desVal * 10 + src[srcLoc] - '0';
		} else {
			des[desLoc] = desVal;
			desVal = 0;
			desLoc++;
			if (src[srcLoc] == 0) {
				break;
			}
		}
		srcLoc++;
	};

	return desLoc;
}

_W _DW(const double src, LocIntC pNum) {
	_W rtw = ((src < 0) ? L"-" : L"") +_TW(INT64(src)) + L".";
	double rNum = abs(src - INT64(src));
	for (LocInt pLoc = 0; pLoc < pNum; pLoc++) {
		rNum *= 10;
		rtw += _TW(INT64(rNum));
		rNum = rNum - INT64(rNum);
	}
	return rtw;
}

_S _DS(const double src, LocIntC pNum) {
	_S rts = ((src < 0) ? "-" : "") + _TS(INT64(src)) + ".";
	double rNum = abs(src - INT64(src));
	for (LocInt pLoc = 0; pLoc < pNum; pLoc++) {
		rNum *= 10;
		rts += _TS(INT64(rNum));
		rNum = rNum - INT64(rNum);
	}
	return rts;
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

int put_img_mono(const ImgT srcImg, LocIntC srcWidth, LocIntC srcHeight, UINT32 outImg[], LocIntC outWidth, LocIntC outBaseX, LocIntC outBaseY) {
	ImgDataT nowColor;
	for (LocInt srcY = 0; srcY < srcHeight; srcY++) {
		for (LocInt srcX = 0; srcX < srcWidth; srcX++) {
			nowColor = srcImg.data[srcY * srcWidth + srcX];
			outImg[(outBaseY + srcY) * outWidth + outBaseX + srcX] = RGB(srcX * 7, nowColor, srcY * 6);
		}
	}
	return 0;
}

int put_ker_mono(const KerT srcKer, LocIntC srcWidth, LocIntC srcHeight,
	UINT32 outImg[], LocIntC outWidth, LocIntC outBaseX, LocIntC outBaseY, LocIntC baseCount) {
	ImgDataT nowColor, nowBack[2];
	nowBack[0] = (srcKer.count < baseCount) ? int(srcKer.count * 0xFF / baseCount) : 0xFF;
	nowBack[1] = (srcKer.count < baseCount) ? 0 : ((srcKer.count < baseCount * 2) ? int((srcKer.count - baseCount) * 0xFF / baseCount) : 0xFF);
	for (LocInt srcY = 0; srcY < srcHeight; srcY++) {
		for (LocInt srcX = 0; srcX < srcWidth; srcX++) {
			nowColor = (ImgDataT)round(srcKer.data[srcY * srcWidth + srcX]);
			outImg[(outBaseY + srcY) * outWidth + outBaseX + srcX] = RGB(nowBack[0] * srcX / srcWidth, nowColor, nowBack[1] * srcY / srcHeight);
			//outImg[(outBaseY + srcY) * outWidth + outBaseX + srcX] = RGB(nowBack[0], nowColor, nowBack[1]);
			//outImg[(outBaseY + srcY) * outWidth + outBaseX + srcX] = RGB(srcX * 7, nowColor, srcY * 6);
		}
	}
	return 0;
}

int put_color(UINT32 outImg[], LocIntC desWidth, LocIntC desHeight, LocIntC outWidth, LocIntC outBaseX, LocIntC outBaseY, const UINT32 iColor) {
	switch (iColor) {
	case IMG_FILL_EMPTY:
		break;
	case IMG_FILL_RND:
		for (LocInt desY = 0; desY < desHeight; desY++) {
			for (LocInt desX = 0; desX < desWidth; desX++) {
				outImg[(outBaseY + desY) * outWidth + outBaseX + desX] = RGB(rand() % 0xFF, rand() % 0xFF, rand() % 0xFF);
			}
		}
		break;
	case IMG_FILL_RND_DARK:
		for (LocInt desY = 0; desY < desHeight; desY++) {
			for (LocInt desX = 0; desX < desWidth; desX++) {
				outImg[(outBaseY + desY) * outWidth + outBaseX + desX] = RGB(rand() % 0x80, rand() % 0x80, rand() % 0x80);
			}
		}
		break;
	case IMG_FILL_RND_LIGHT:
		for (LocInt desY = 0; desY < desHeight; desY++) {
			for (LocInt desX = 0; desX < desWidth; desX++) {
				outImg[(outBaseY + desY) * outWidth + outBaseX + desX] = ~RGB(rand() % 0x80, rand() % 0x80, rand() % 0x80);
			}
		}
		break;
	default:
		for (LocInt desY = 0; desY < desHeight; desY++) {
			for (LocInt desX = 0; desX < desWidth; desX++) {
				outImg[(outBaseY + desY) * outWidth + outBaseX + desX] = iColor;
			}
		}
		break;
	}

	return 0;
}

int put_imgList_mono(const ImgT srcImgData[], LocIntC srcImgList[], LocIntC srcNum, LocIntC srcNumX, LocIntC srcNumY,
	LocIntC srcWidth, LocIntC srcHeight, UINT32 outImg[], const UINT32 iColor) {
	LocInt srcLocX, srcLocY;
	for (LocInt srcLoc = 0; srcLoc < srcNum; srcLoc++) {
		srcLocY = srcLoc / srcNumX;
		srcLocX = srcLoc % srcNumX;
		put_img_mono(srcImgData[srcImgList[srcLoc]], srcWidth, srcHeight, outImg, srcNumX * srcWidth, srcWidth * srcLocX, srcHeight * srcLocY);
	}
	if (iColor == IMG_FILL_RND_ALTER) {
		for (LocInt srcLoc = srcNum; srcLoc < srcNumX * srcNumY; srcLoc++) {
			srcLocY = srcLoc / srcNumX;
			srcLocX = srcLoc % srcNumX;
			put_color(outImg, srcWidth, srcHeight, srcNumX * srcWidth, srcWidth * srcLocX, srcHeight * srcLocY,
				((srcLoc / srcNumX + srcLoc % srcNumX) % 2) ? IMG_FILL_RND_DARK : IMG_FILL_RND_LIGHT);
		}
	} else {
		for (LocInt srcLoc = srcNum; srcLoc < srcNumX * srcNumY; srcLoc++) {
			srcLocY = srcLoc / srcNumX;
			srcLocX = srcLoc % srcNumX;
			put_color(outImg, srcWidth, srcHeight, srcNumX * srcWidth, srcWidth * srcLocX, srcHeight * srcLocY, iColor);
		}
	}

	return 0;
}

/*
int put_setPtr_mono(const pImgT srcImgSet[], LocIntC srcNum, LocIntC srcNumX, LocIntC srcNumY,
	LocIntC srcWidth, LocIntC srcHeight, UINT32 outImg[], const UINT32 iColor) {
	LocInt srcLocX, srcLocY;
	for (LocInt srcLoc = 0; srcLoc < srcNum; srcLoc++) {
		srcLocY = srcLoc / srcNumX;
		srcLocX = srcLoc % srcNumX;
		put_img_mono(*(srcImgSet[srcLoc]), srcWidth, srcHeight, outImg, srcNumX * srcWidth, srcWidth * srcLocX, srcHeight * srcLocY);
	}
	if (iColor == IMG_FILL_RND_ALTER) {
		for (LocInt srcLoc = srcNum; srcLoc < srcNumX * srcNumY; srcLoc++) {
			srcLocY = srcLoc / srcNumX;
			srcLocX = srcLoc % srcNumX;
			put_color(outImg, srcWidth, srcHeight, srcNumX * srcWidth, srcWidth * srcLocX, srcHeight * srcLocY,
				((srcLoc / srcNumX + srcLoc % srcNumX) % 2) ? IMG_FILL_RND_DARK : IMG_FILL_RND_LIGHT);
		}
	} else {
		for (LocInt srcLoc = srcNum; srcLoc < srcNumX * srcNumY; srcLoc++) {
			srcLocY = srcLoc / srcNumX;
			srcLocX = srcLoc % srcNumX;
			put_color(outImg, srcWidth, srcHeight, srcNumX * srcWidth, srcWidth * srcLocX, srcHeight * srcLocY, iColor);
		}
	}

	return 0;
}
*/

int put_kerSet_mono(const KerT srcKerSet[], LocIntC srcNum, LocIntC srcNumX, LocIntC srcNumY,
	LocIntC srcWidth, LocIntC srcHeight, UINT32 outImg[], const UINT32 iColor) {
	LocInt srcLocX, srcLocY, totalCount = 0;
	for (LocInt srcLoc = 0; srcLoc < srcNum; srcLoc++) {
		totalCount += srcKerSet[srcLoc].count;
	}
	for (LocInt srcLoc = 0; srcLoc < srcNum; srcLoc++) {
		srcLocY = srcLoc / srcNumX;
		srcLocX = srcLoc % srcNumX;
		put_ker_mono(srcKerSet[srcLoc], srcWidth, srcHeight, outImg, srcNumX * srcWidth, srcWidth * srcLocX, srcHeight * srcLocY, 
			totalCount * 3 / 2 / srcNum);
	}
	if (iColor == IMG_FILL_RND_ALTER) {
		for (LocInt srcLoc = srcNum; srcLoc < srcNumX * srcNumY; srcLoc++) {
			srcLocY = srcLoc / srcNumX;
			srcLocX = srcLoc % srcNumX;
			put_color(outImg, srcWidth, srcHeight, srcNumX * srcWidth, srcWidth * srcLocX, srcHeight * srcLocY,
				((srcLoc / srcNumX + srcLoc % srcNumX) % 2) ? IMG_FILL_RND_DARK : IMG_FILL_RND_LIGHT);
		}
	} else {
		for (LocInt srcLoc = srcNum; srcLoc < srcNumX * srcNumY; srcLoc++) {
			srcLocY = srcLoc / srcNumX;
			srcLocX = srcLoc % srcNumX;
			put_color(outImg, srcWidth, srcHeight, srcNumX * srcWidth, srcWidth * srcLocX, srcHeight * srcLocY, iColor);
		}
	}

	return 0;
}

int put_conf(ConT *con, LocInt mNum, UINT32 outImg[], LocInt bWidth, LocInt bHeight, LocInt bSpace) {
	LocInt outWidth = mNum * bWidth + (mNum - 1) * bSpace, nowColor;
	LocInt cBase = 0;
	for (LocInt rLoc = 0; rLoc < mNum; rLoc++) {
		for (LocInt cLoc = 0; cLoc < mNum; cLoc++) {
			nowColor = cBase + con->count[rLoc][cLoc] * (0xFF - cBase) / con->rcount[rLoc];
			for (LocInt yLoc = rLoc * (bHeight + bSpace); yLoc < rLoc * (bHeight + bSpace) + bHeight; yLoc++) {
				for (LocInt xLoc = cLoc * (bWidth + bSpace); xLoc < cLoc * (bWidth + bSpace) + bWidth; xLoc++) {
					outImg[yLoc * outWidth + xLoc] = RGB(0, nowColor * (nowColor > cBase), 0xFF * (nowColor > cBase));
				}
			}
		}
	}

	return 0;
}