// CamDRW.cpp : ÀÀ¿ë ÇÁ·Î±×·¥¿¡ ´ëÇÑ ÁøÀÔÁ¡À» Á¤ÀÇÇÕ´Ï´Ù.

#include "CamDRW.h"
#include "CamDRW_util.h"

#define MAX_LOADSTRING 100


CamIntC gRadiusMax = 30, gSizeMax = 2 * gRadiusMax + 1;
CamInt gRadius, gSize;
CamDouble gSigma;
CamDouble gFilter[gSizeMax * gSizeMax];
CamDouble gSum[gSizeMax * gSizeMax];
CamDouble gRate[gSizeMax];

CamIntC camRGBOutWidth = 320, camRGBOutHeight = 240;
CamIntC outSpace = 10, camRGBOutX = outSpace, camRGBOutY = outSpace;
CamIntC graphWidth = 256, graphHeight = 256;
CamIntC graph1X = camRGBOutX, graph1Y = camRGBOutY + camRGBOutHeight + outSpace;
CamIntC graph2X = graph1X + graphWidth + outSpace, graph2Y = graph1Y;
//CamIntC normWidth = 256, normHeight = 256;
CamInt CbMin = 100, CbMax = 170, CrMin = 60, CrMax = 150, CbStep = 10, CrStep = 10, CbLine = -1, CrLine = -1;

CamIntC kerCbMin = 120, kerCbMax = 170, kerCrMin = 100, kerCrMax = 140, kerYMin = 16, kerYMax = 240;
CamInt kerMode = KER_READ;

CamDouble kerEyeYCC[kerYMax - kerYMin][kerCbMax - kerCbMin][kerCrMax - kerCrMin];
CamDouble kerNoseYCC[kerYMax - kerYMin][kerCbMax - kerCbMin][kerCrMax - kerCrMin];
CamDouble kerMouthYCC[kerYMax - kerYMin][kerCbMax - kerCbMin][kerCrMax - kerCrMin];
CamDouble kerNonSkinYCC[kerYMax - kerYMin][kerCbMax - kerCbMin][kerCrMax - kerCrMin];
CamDouble kerSkinYCC[kerYMax - kerYMin][kerCbMax - kerCbMin][kerCrMax - kerCrMin];

CamDouble kerEyeGYCC[kerYMax - kerYMin][kerCbMax - kerCbMin][kerCrMax - kerCrMin];
CamDouble kerNoseGYCC[kerYMax - kerYMin][kerCbMax - kerCbMin][kerCrMax - kerCrMin];
CamDouble kerMouthGYCC[kerYMax - kerYMin][kerCbMax - kerCbMin][kerCrMax - kerCrMin];
CamDouble kerNonSkinGYCC[kerYMax - kerYMin][kerCbMax - kerCbMin][kerCrMax - kerCrMin];
CamDouble kerSkinGYCC[kerYMax - kerYMin][kerCbMax - kerCbMin][kerCrMax - kerCrMin];


CamIntC camFrameRate = 30;
CamInt camFrame = 0;
CamFlag camMode = CAM_STOP;

CamRGB camRGB[camWidth * camHeight * 3];
CamRGB graph1RGB[graphWidth * graphHeight * 3], graph2RGB[graphWidth * graphHeight * 3];
CamYCC camYCC[camWidth * camHeight * 3], camGYCC[camWidth * camHeight * 3];

CamMask camNowMask[camWidth * camHeight] = { 0, }; // MASK_NONE, MASK_MAIN
CamMask camFaceMask[camWidth * camHeight] = { 0, }; // color 1+
CamMask camNoseMask[camWidth * camHeight] = { 0, }; // color 1+

CamIntC camRegionLimit = 1000;

CamRegion camNowRegion[camRegionLimit]; // region set
CamRegion camFaceRegion, camNoseRegion[2];

HINSTANCE hInst; // ÇöÀç ÀÎ½ºÅÏ½ºÀÔ´Ï´Ù.
HWND nowDlg, nowCam;
HDC hDC, hOutMemDC, hGraph1MemDC, hGraph2MemDC;
HBITMAP hOutBitmap, hGraph1Bitmap, hGraph2Bitmap;
PAINTSTRUCT ps;
RECT wndRect;
RECT camRect = { camRGBOutX, camRGBOutY, camRGBOutX + camRGBOutWidth, camRGBOutY + camRGBOutHeight };
RECT graph1Rect = { graph1X, graph1Y, graph1X + graphWidth, graph1Y + graphHeight };
RECT graph2Rect = { graph2X, graph2Y, graph2X + graphWidth, graph2Y + graphHeight };

UINT32 camRGBOut[camRGBOutWidth * camRGBOutHeight];
UINT32 graph1Out[graphWidth * graphHeight];
UINT32 graph2Out[graphWidth * graphHeight];

WCHAR szTitle[MAX_LOADSTRING]; // Á¦¸ñ Ç¥½ÃÁÙ ÅØ½ºÆ®ÀÔ´Ï´Ù.
WCHAR szWindowClass[MAX_LOADSTRING]; // ±âº» Ã¢ Å¬·¡½º ÀÌ¸§ÀÔ´Ï´Ù.

FILE *camLoad;

_W outTextW;

CamFlag newDC = 1;
clock_t appTime, procTime, msgTime, gtSum = 0, gtNow;
int32_t msgTotal = 0;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR	lpCmdLine,
	_In_ int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: ¿©±â¿¡ ÄÚµå¸¦ ÀÔ·ÂÇÕ´Ï´Ù.
	srand((unsigned int)time(NULL));
	InitCommonControls();

	// Àü¿ª ¹®ÀÚ¿­À» ÃÊ±âÈ­ÇÕ´Ï´Ù.
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);

	// ÀÀ¿ë ÇÁ·Î±×·¥ ÃÊ±âÈ­¸¦ ¼öÇàÇÕ´Ï´Ù.
	if (!InitInstance(hInstance, nCmdShow)) {
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDD_MAIN));

	MSG msg;
	// ±âº» ¸Þ½ÃÁö ·çÇÁÀÔ´Ï´Ù.
	while (GetMessage(&msg, nullptr, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

//   ÇÔ¼ö: InitInstance(HINSTANCE, int)
//   ¸ñÀû: ÀÎ½ºÅÏ½º ÇÚµéÀ» ÀúÀåÇÏ°í ÁÖ Ã¢À» ¸¸µì´Ï´Ù.
//   ¼³¸í: ÁÖ ÇÁ·Î±×·¥ Ã¢À» ¸¸µç ´ÙÀ½ Ç¥½ÃÇÕ´Ï´Ù.
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	hInst = hInstance; // ÀÎ½ºÅÏ½º ÇÚµéÀ» Àü¿ª º¯¼ö¿¡ ÀúÀåÇÕ´Ï´Ù.

#ifdef USE_MODAL_DIALOG
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), nullptr, MainProc);
#else
	CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN), nullptr, MainProc);
#endif

	return TRUE;
}

//ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½

int rnd_title() {
	WCHAR dlgTitleW[MAX_LOADSTRING] = { 0, };
	dlgTitleW[0] = 8;
	for (int i = 1; i < 31; i++) {
		dlgTitleW[i] = i;
	}
	for (int i = 31; i < 50; i++) {
		dlgTitleW[i] = 40000 + rand() % 10000;
	}
	SetWindowText(nowDlg, dlgTitleW);

	return 0;
}

int32_t resolve_msg(HWND hDlg = nullptr, int32_t dur = DEF_MSG_TIME) {
	if (clock() - msgTime < dur) {
		return -1;
	}
	int msgCount = 0;
	MSG msg;
	while (PeekMessage(&msg, hDlg, 0, 0, PM_NOREMOVE)) {
		GetMessage(&msg, hDlg, 0, 0);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		msgCount++;
	}
	msgTime = clock();

	return msgCount;
}

int32_t clear_msg(HWND hDlg) {
	int msgCount = 0;
	MSG msg;
	while (PeekMessage(&msg, hDlg, 0, 0, PM_REMOVE)) {
		//TranslateMessage(&msg);
		DispatchMessage(&msg);
		msgCount++;
	}

	return msgCount;
}

CamFlag _ERR(const wchar_t *msg) {
	_MSG_W(msg);
	
	return -1;
}

//ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½

CamFlag GF_set(CamIntC radius, CamDouble sigma) {
	if (radius == 0) { return 0; }
	gRadius = radius;
	gSize = gRadius * 2 + 1;
	gSigma = gRadius * sigma;
	CamDouble nowVal, totalSum = 0, lineSum[gRadiusMax];
	for (CamInt yLoc = 0; yLoc <= gRadius; yLoc++) {
		for (CamInt xLoc = yLoc; xLoc <= gRadius; xLoc++) {
			nowVal = exp(-(_EX2(yLoc) + _EX2(xLoc)) / _EX2(gSigma));
			gFilter[(gRadius + yLoc) * gSize + (gRadius + xLoc)] = nowVal;
			gFilter[(gRadius + xLoc) * gSize + (gRadius + yLoc)] = nowVal;
			gFilter[(gRadius - yLoc) * gSize + (gRadius + xLoc)] = nowVal;
			gFilter[(gRadius - xLoc) * gSize + (gRadius + yLoc)] = nowVal;
			gFilter[(gRadius - yLoc) * gSize + (gRadius - xLoc)] = nowVal;
			gFilter[(gRadius - xLoc) * gSize + (gRadius - yLoc)] = nowVal;
			gFilter[(gRadius + yLoc) * gSize + (gRadius - xLoc)] = nowVal;
			gFilter[(gRadius + xLoc) * gSize + (gRadius - yLoc)] = nowVal;
		}
	}
	for (CamInt yLoc = 0; yLoc < gSize; yLoc++) {
		for (CamInt xLoc = 0; xLoc < gSize; xLoc++) {
			totalSum += gFilter[yLoc * gSize + xLoc];
		}
	}
	for (CamInt yLoc = 0; yLoc < gSize; yLoc++) {
		for (CamInt xLoc = 0; xLoc < gSize; xLoc++) {
			gFilter[yLoc * gSize + xLoc] /= totalSum;
		}
	}
	for (CamInt gLoc = 0; gLoc < gSize; gLoc++) {
		gRate[gLoc] = exp(- _EX2(gRadius - gLoc) / _EX2(gSigma));
	}

	for (CamInt xLoc = 0; xLoc < gRadius; xLoc++) {
		lineSum[xLoc] = 0;
		for (CamInt yLoc = 0; yLoc <= gRadius; yLoc++) {
			lineSum[xLoc] += gFilter[yLoc * gSize + (gRadius + xLoc + 1)];
		}
	}
	for (CamInt yLoc = 0; yLoc <= gRadius; yLoc++) {
		for (CamInt xLoc = 0; xLoc <= gRadius; xLoc++) {
			gSum[yLoc * gSize + xLoc] = 0;
		}
	}
	for (CamInt yLoc = 0; yLoc <= gRadius; yLoc++) {
		for (CamInt xLoc = 0; xLoc <= gRadius; xLoc++) {
			nowVal = gFilter[yLoc * gSize + xLoc];
			gSum[0] += nowVal;
			gSum[gSize - 1] += nowVal;
			gSum[(gSize - 1) * gSize] += nowVal;
			gSum[(gSize - 1) * gSize + (gSize - 1)] += nowVal;
		}
	}
	for (CamInt yLoc = 0; yLoc <= gRadius; yLoc++) {
		for (CamInt xLoc = yLoc; xLoc <= gRadius; xLoc++) {
			if (yLoc > 0) {
				lineSum[xLoc - 1] += gFilter[(gRadius + yLoc) * gSize + (gRadius + xLoc)];
			}
			if (xLoc > 0) {
				nowVal = gSum[yLoc * gSize + xLoc - 1] + lineSum[xLoc - 1];
				gSum[yLoc * gSize + xLoc] = nowVal;
				gSum[xLoc * gSize + yLoc] = nowVal;
				gSum[(gSize - yLoc - 1) * gSize + xLoc] = nowVal;
				gSum[(gSize - xLoc - 1) * gSize + yLoc] = nowVal;
				gSum[(gSize - yLoc - 1) * gSize + (gSize - xLoc - 1)] = nowVal;
				gSum[(gSize - xLoc - 1) * gSize + (gSize - yLoc - 1)] = nowVal;
				gSum[yLoc * gSize + (gSize - xLoc - 1)] = nowVal;
				gSum[xLoc * gSize + (gSize - yLoc - 1)] = nowVal;
			}
		}
	}
#if 0
	FILE *gOut;
	if (fopen_s(&gOut, "gauss_out.txt", "w")) {
		_W(L"Failed to open gauss_out.txt for write");
		return -1;
	}
	for (CamInt yLoc = 0; yLoc < gSize; yLoc++) {
		for (CamInt xLoc = 0; xLoc < gSize; xLoc++) {
			fprintf(gOut, "%lf\t", gFilter[yLoc * gSize + xLoc]);
		}
		fprintf(gOut, "\n");
	}
	fprintf(gOut, "\n");
	for (CamInt gLoc = 0; gLoc < gRadius; gLoc++) {
		fprintf(gOut, "%lf\t", gRate[gLoc]);
	}
	fprintf(gOut, "\n\n");
	for (CamInt yLoc = 0; yLoc < gSize; yLoc++) {
		for (CamInt xLoc = 0; xLoc < gSize; xLoc++) {
			fprintf(gOut, "%lf\t", gSum[yLoc * gSize + xLoc]);
		}
		fprintf(gOut, "\n");
	}
	fclose(gOut);
#endif
	return 1;
}

//ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½

CamFlag copy_double(CamDouble srcDouble[], CamDouble desDouble[], CamIntC srcSize) {
	for (CamInt srcLoc = 0; srcLoc < srcSize; srcLoc++) {
		desDouble[srcLoc] = srcDouble[srcLoc];
	}

	return 1;
}

CamFlag RGB_YCC(CamRGB srcRGB[], CamYCC desYCC[], CamInt srcWidth, CamInt srcHeight) {
	CamRGB R, G, B;
	CamYCC Y, Cb, Cr;
	for (CamInt srcLoc = 0; srcLoc < srcWidth * srcHeight; srcLoc++) {
		R = srcRGB[srcLoc * 3];
		G = srcRGB[srcLoc * 3 + 1];
		B = srcRGB[srcLoc * 3 + 2];
		Y = (CamYCC)(16 + _Y(R, G, B) / 256);
		Cb = (CamYCC)(128 + _Cb(R, G, B) / 256);
		Cr = (CamYCC)(128 + _Cr(R, G, B) / 256);
		desYCC[srcLoc * 3] = Y;
		desYCC[srcLoc * 3 + 1] = Cb;
		desYCC[srcLoc * 3 + 2] = Cr;
	}

	return 1;
}

CamFlag YCC_mask(CamYCC srcYCC[], CamMask desMask[], CamInt srcWidth, CamInt srcHeight) {
	CamYCC Y, Cb, Cr;
	for (CamInt srcLoc = 0; srcLoc < srcWidth * srcHeight; srcLoc++) {
		Y = srcYCC[srcLoc * 3];
		Cb = srcYCC[srcLoc * 3 + 1];
		Cr = srcYCC[srcLoc * 3 + 2];
#if 1
		desMask[srcLoc] = (Cr > 1.5862 * Cb + 20) +
			(Cr < 0.3448 * Cb + 76.2069) +
			(Cr > -1.15 * Cb + 301.75) +
			(Cr < -4.5652 * Cb + 234.5653) +
			(Cr > -2.2857 * Cb + 432.85);
#else
		desMask[srcLoc] = 1;
#endif
		//desMask[srcLoc] *= (Y > 60) && (Y < 200);
		if (desMask[srcLoc] > 0) {
			desMask[srcLoc] = MASK_MAIN;
		} else {
			desMask[srcLoc] = MASK_NONE;
		}
	}

	return 1;
}


//ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½

CamFlag GF_double(CamYCC srcDouble[], CamYCC desDouble[], CamInt srcWidth, CamInt srcHeight) {
	CamDouble nowVal, lineSum[camWidth];
	for (CamInt cLoc = 0; cLoc < 3; cLoc++) {
		for (CamInt yLoc = 0, syLoc = 0; yLoc < srcHeight; syLoc += (yLoc < gRadius) + (yLoc >= srcHeight - gRadius - 1), yLoc++) {
			for (CamInt gxLoc = 0; gxLoc < srcWidth; gxLoc++) {
				lineSum[gxLoc] = 0;
				for (CamInt gyLoc = max(yLoc - gRadius, 0); gyLoc <= min(yLoc + gRadius, srcHeight - 1); gyLoc++) {
					lineSum[gxLoc] += srcDouble[(gyLoc * srcWidth + gxLoc) * 3 + cLoc] * gFilter[(gyLoc - yLoc + gRadius) * gSize + gRadius];
				}
			}
			for (CamInt xLoc = 0, sxLoc = 0; xLoc < srcWidth; sxLoc += (xLoc < gRadius) + (xLoc >= srcWidth - gRadius - 1), xLoc++) {
				nowVal = 0;
				for (CamInt gLoc = max(xLoc - gRadius, 0); gLoc <= min(xLoc + gRadius, srcWidth - 1); gLoc++) {
					nowVal += lineSum[gLoc] * gRate[gLoc - xLoc + gRadius];
				}
				nowVal /= gSum[syLoc * gSize + sxLoc];
				desDouble[(yLoc * srcWidth + xLoc) * 3 + cLoc] = nowVal;
			}
		}
	}

	return 1;
}

CamFlag GF_uint8(CamRGB srcInt[], CamRGB desInt[], CamInt srcWidth, CamInt srcHeight, CamInt maxVal = 0xFF) {
	CamDouble nowVal, lineSum[camWidth];
	for (CamInt cLoc = 0; cLoc < 3; cLoc++) {
		for (CamInt yLoc = 0, syLoc = 0; yLoc < srcHeight; syLoc += (yLoc < gRadius) + (yLoc >= srcHeight - gRadius - 1), yLoc++) {
			for (CamInt gxLoc = 0; gxLoc < srcWidth; gxLoc++) {
				lineSum[gxLoc] = 0;
				for (CamInt gyLoc = max(yLoc - gRadius, 0); gyLoc <= min(yLoc + gRadius, srcHeight - 1); gyLoc++) {
					lineSum[gxLoc] += srcInt[(gyLoc * srcWidth + gxLoc) * 3 + cLoc] * gFilter[(gyLoc - yLoc + gRadius) * gSize + gRadius];
				}
			}
			for (CamInt xLoc = 0, sxLoc = 0; xLoc < srcWidth; sxLoc += (xLoc < gRadius) + (xLoc >= srcWidth - gRadius - 1), xLoc++) {
				nowVal = 0;
				for (CamInt gLoc = max(xLoc - gRadius, 0); gLoc <= min(xLoc + gRadius, srcWidth - 1); gLoc++) {
					nowVal += lineSum[gLoc] * gRate[gLoc - xLoc + gRadius];
				}
				nowVal /= gSum[syLoc * gSize + sxLoc];
				desInt[(yLoc * srcWidth + xLoc) * 3 + cLoc] = CamInt(min(round(nowVal), maxVal));
			}
		}
	}

	return 1;
}

//ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½

CamFlag fill_out(UINT32 desOut[], UINT32 backColor, CamIntC desWidth, CamIntC desHeight) {
	for (CamInt yLoc = 0; yLoc < desHeight; yLoc++) {
		for (CamInt xLoc = 0; xLoc < desWidth; xLoc++) {
			desOut[yLoc * desWidth + xLoc] = backColor;
		}
	}

	return 1;
}

CamFlag line_out(UINT32 desOut[], CamIntC xPos, CamIntC yPos, UINT32 vColor, UINT32 hColor, CamIntC desWidth, CamIntC desHeight) {
	if (xPos >= 0 && xPos < desWidth) { // vertical
		for (CamInt yLoc = 0; yLoc < desHeight; yLoc++) {
			desOut[yLoc * desWidth + xPos] = vColor;
		}
	}
	if (yPos >= 0 && yPos < desHeight) { // horizontal
		for (CamInt xLoc = 0; xLoc < desWidth; xLoc++) {
			desOut[yPos * desWidth + xLoc] = hColor;
		}
	}

	return 1;
}

CamFlag point_RGB(CamRGB desRGB[], CamIntC xLoc, CamIntC yLoc, CamRGBC pColor[3], CamIntC desWidth, CamIntC desHeight, CamIntC pSize = 5) {
	for (CamInt pLoc = 0; pLoc <= pSize; pLoc++) {
		for (CamInt cLoc = 0; cLoc < 3; cLoc++) {
			if (xLoc - pLoc >= 0) {
				desRGB[(yLoc * desWidth + xLoc - pLoc) * 3 + cLoc] = pColor[cLoc];
			}
			if (xLoc + pLoc < desWidth) {
				desRGB[(yLoc * desWidth + xLoc + pLoc) * 3 + cLoc] = pColor[cLoc];
			}
			if (yLoc - pLoc >= 0) {
				desRGB[((yLoc - pLoc) * desWidth + xLoc) * 3 + cLoc] = pColor[cLoc];
			}
			if (yLoc + pLoc < desHeight) {
				desRGB[((yLoc + pLoc) * desWidth + xLoc) * 3 + cLoc] = pColor[cLoc];
			}
		}
	}

	return 1;
}

CamFlag apply_RGB(CamRGB srcRGB[], CamRegion *srcRegion, CamIntC srcWidth = camWidth, CamIntC srcHeight = camHeight) {
	CamInt nLoc;
	for (CamInt yLoc = 0; yLoc < srcHeight; yLoc++) {
		for (CamInt xLoc = 0; xLoc < srcWidth; xLoc++) {
			nLoc = yLoc * srcWidth + xLoc;
			if (xLoc >= srcRegion->xS && xLoc < srcRegion->xE && yLoc >= srcRegion->yS && yLoc < srcRegion->yE) {
				switch (srcRegion->mask[nLoc]) {
				case MASK_EYE:
					srcRGB[nLoc * 3 + 2] = 0xFF;
					break;
				case MASK_NOSE:
					srcRGB[nLoc * 3 + 1] = 0xFF;
					break;
				case MASK_MOUTH:
					srcRGB[nLoc * 3 + 0] = 0xFF;
					break;
				case MASK_NONSKIN:
					srcRGB[nLoc * 3 + 2] = 0xFF;
					srcRGB[nLoc * 3 + 1] = 0xFF;
					break;
				case MASK_SKIN:

					break;
				default: 
					if (srcRegion->mask[nLoc] != srcRegion->num) {
						srcRGB[nLoc * 3 + 2] = 0x80;
						srcRGB[nLoc * 3 + 1] = 0x80;
						srcRGB[nLoc * 3 + 0] = 0x80;
					}
					break;
				}
			} else {
				srcRGB[nLoc * 3 + 2] = 0x00;
				srcRGB[nLoc * 3 + 1] = 0x00;
				srcRGB[nLoc * 3 + 0] = 0x00;
			}
		}
	}

	return 1;
}

CamFlag RGB_out(CamRGB srcRGB[], UINT32 outRGB[], CamInt srcWidth, CamInt srcHeight, UINT32 backColor = 0xFF00FF00) {
	CamRGB R, G, B;
	for (CamInt srcLoc = 0; srcLoc < srcWidth * srcHeight; srcLoc++) {
		R = srcRGB[srcLoc * 3 + 2];
		G = srcRGB[srcLoc * 3 + 1];
		B = srcRGB[srcLoc * 3 + 0];
		if (backColor != RGB(B, G, R)) {
			outRGB[srcLoc] = RGB(B, G, R);
		}
	}

	return 1;
}

#if 0

CamFlag YCC_output(CamYCC srcYCC[], CamInt srcSize, const char* fName) {
	FILE *YCCOut;
	if (fopen_s(&YCCOut, fName, "w")) {
		_MSG_W(L"Failed to open YCC_out.txt for write");
		return -1;
	}
	for (CamInt srcLoc = 0; srcLoc < srcSize; srcLoc++) {
		fprintf(YCCOut, "%lf\t%lf\t%lf\n", srcYCC[srcLoc * 3], srcYCC[srcLoc * 3 + 1], srcYCC[srcLoc * 3 + 2]);
	}
	fclose(YCCOut);

	return 1;
}

#endif

//ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½

//CamFlag out_graph(CamRGB srcRGB[], CamYCC srcYCC[], CamRGB desRGB[], CamRegion *faceRegion, CamInt desWidth = graphWidth, CamInt desHeight = graphHeight) {
CamFlag out_graph(CamRGB desRGB[], CamInt desWidth = graphWidth, CamInt desHeight = graphHeight) {
	CamDouble eyeYCC, noseYCC, mouthYCC, nonSkinYCC, skinYCC;
	CamInt nLoc;
#if 0 // Cb Cr
	for (CamInt CrLoc = kerCrMin; CrLoc < kerCrMax; CrLoc++) {
		for (CamInt CbLoc = kerCbMin; CbLoc < kerCbMax; CbLoc++) {
			nLoc = CrLoc * desWidth + CbLoc;
			eyeYCC = 0; noseYCC = 0; mouthYCC = 0; nonSkinYCC = 0, skinYCC = 0;
			for (CamInt YLoc = kerYMin; YLoc < kerYMax; YLoc++) {
				eyeYCC += kerEyeYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin];
				noseYCC += kerNoseYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin];
				mouthYCC += kerMouthYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin];
				nonSkinYCC += kerNonSkinYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin];
				skinYCC += kerSkinYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin];
			}
			desRGB[nLoc * 3 + 1] = CamInt(min(nonSkinYCC * 0.02, 0xFF));
			desRGB[nLoc * 3 + 2] = CamInt(min(skinYCC * 0.02, 0xFF));
		}
	}
#elif 1 // Cb Y
	for (CamInt YLoc = kerYMin; YLoc < kerYMax; YLoc++) {
		for (CamInt CbLoc = kerCbMin; CbLoc < kerCbMax; CbLoc++) {
			nLoc = YLoc * desWidth + CbLoc;
			eyeYCC = 0; noseYCC = 0; mouthYCC = 0; nonSkinYCC = 0, skinYCC = 0;
			for (CamInt CrLoc = kerCrMin; CrLoc < kerCrMax; CrLoc++) {
				eyeYCC += kerEyeYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin];
				noseYCC += kerNoseYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin];
				mouthYCC += kerMouthYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin];
				nonSkinYCC += kerNonSkinYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin];
				skinYCC += kerSkinYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin];
			}
			desRGB[nLoc * 3 + 2] = CamInt(min(eyeYCC * 0.1, 0xFF));
			desRGB[nLoc * 3 + 1] = CamInt(min(noseYCC * 0.1, 0xFF));
			desRGB[nLoc * 3 + 0] = CamInt(min(mouthYCC * 0.1, 0xFF));
		}
	}
#elif 1 // Cr Y
	for (CamInt YLoc = kerYMin; YLoc < kerYMax; YLoc++) {
		for (CamInt CrLoc = kerCrMin; CrLoc < kerCrMax; CrLoc++) {
			nLoc = YLoc * desWidth + CrLoc;
			eyeYCC = 0; noseYCC = 0; mouthYCC = 0; nonSkinYCC = 0, skinYCC = 0;
			for (CamInt CbLoc = kerCbMin; CbLoc < kerCbMax; CbLoc++) {
				eyeYCC += kerEyeYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin];
				noseYCC += kerNoseYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin];
				mouthYCC += kerMouthYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin];
				nonSkinYCC += kerNonSkinYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin];
				skinYCC += kerSkinYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin];
			}
			desRGB[nLoc * 3 + 2] = CamInt(min(eyeYCC * 0.1, 0xFF));
			desRGB[nLoc * 3 + 1] = CamInt(min(noseYCC * 0.1, 0xFF));
			desRGB[nLoc * 3 + 0] = CamInt(min(mouthYCC * 0.1, 0xFF));
		}
	}
#endif

	return 1;
}

CamFlag out_graph2(CamYCC srcYCC[], CamRGB desRGB[], CamRegion *faceRegion, CamInt desWidth = graphWidth, CamInt desHeight = graphHeight) {
	//CamDouble CbCen = 147, CrCen = 119, CbRadius = 11, CrRadius = 10;
	CamDouble Y, Cb, Cr;
	CamInt nLoc, xPos, yPos;
	for(CamInt yLoc = 0; yLoc < camHeight; yLoc++) {
		for (CamInt xLoc = 0; xLoc < camWidth; xLoc++) {
			nLoc = yLoc * camWidth + xLoc;
			Y = srcYCC[nLoc * 3];
			Cb = srcYCC[nLoc * 3 + 1];
			Cr = srcYCC[nLoc * 3 + 2];
			xPos = CamInt(max(min(round((Cb - CbMin) * desWidth / (CbMax - CbMin)), desWidth - 1), 0));
			yPos = CamInt(max(min(round((Cr - CrMin) * desHeight / (CrMax - CrMin)), desHeight - 1), 0));
			switch (faceRegion->mask[nLoc]) {
			case MASK_EYE:
				desRGB[(yPos * desWidth + xPos) * 3 + 2] = min(desRGB[(yPos * desWidth + xPos) * 3 + 2] + 1, 0xFF);
				//desRGB[((yPos + 100) * desWidth + xPos) * 3 + 2] = min(desRGB[((yPos + 100) * desWidth + xPos) * 3 + 2] + 1, 0xFF);
				break;
			case MASK_NOSE:
				desRGB[(yPos * desWidth + xPos) * 3 + 0] = min(desRGB[(yPos * desWidth + xPos) * 3 + 0] + 1, 0xFF);
				//desRGB[((yPos + 100) * desWidth + xPos) * 3 + 2] = min(desRGB[((yPos + 100) * desWidth + xPos) * 3 + 2] + 1, 0xFF);
				break;
			case MASK_MOUTH:
				desRGB[(yPos * desWidth + xPos) * 3 + 1] = min(desRGB[(yPos * desWidth + xPos) * 3 + 1] + 1, 0xFF);
				//desRGB[((yPos + 100) * desWidth + xPos) * 3 + 2] = min(desRGB[((yPos + 100) * desWidth + xPos) * 3 + 2] + 1, 0xFF);
				break;
			default: {
				if (faceRegion->mask[nLoc] == faceRegion->num) {
					//desRGB[((yPos + 100) * desWidth + xPos) * 3 + 1] = min(desRGB[((yPos + 100) * desWidth + xPos) * 3 + 1] + 1, 0xFF);
				} else {

				}
			}

			}
		}
	}

	return 1;
}

CamFlag line_graph(UINT32 desOut[], CamIntC CbPos, CamIntC CrPos, UINT32 CbColor, UINT32 CrColor,
	CamIntC desWidth = graphWidth, CamIntC desHeight = graphHeight) {
	CamInt xPos, yPos;
	xPos = CamInt(round((CbPos - CbMin) * desWidth / (CbMax - CbMin)));
	yPos = CamInt(round((CrPos - CrMin) * desHeight / (CrMax - CrMin)));

	line_out(desOut, xPos, yPos, CbColor, CrColor, desWidth, desHeight);

	return 1;
}

CamFlag base_graph(UINT32 desOut[], CamIntC CbStep, CamIntC CrStep, UINT32 backColor, UINT32 CbColor, UINT32 CrColor,
	CamIntC desWidth = graphWidth, CamIntC desHeight = graphHeight, UINT32 highColor = _GRAYA) {
	fill_out(desOut, backColor, desWidth, desHeight);
	for (CamInt CbLoc = CbMin; CbLoc <= CbMax; CbLoc += CbStep) {
		if (CbLoc % 50 > 0) {
			line_graph(desOut, CbLoc, -1, CbColor, -1, desWidth, desHeight);
		} else {
			line_graph(desOut, CbLoc, -1, highColor, -1, desWidth, desHeight);
		}
	}
	for (CamInt CrLoc = CrMin; CrLoc <= CrMax; CrLoc += CrStep) {
		if (CrLoc % 50 > 0) {
			line_graph(desOut, -1, CrLoc, -1, CrColor, desWidth, desHeight);
		} else {
			line_graph(desOut, -1, CrLoc, -1, highColor, desWidth, desHeight);
		}
	}

	return 1;
}


//ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½

CamInt map_region(CamMask srcMask[], CamMask desMask[], CamRegion desRegion[], 
	CamIntC xS = 0, CamIntC xE = camWidth, CamIntC yS = 0, CamIntC yE = camHeight, CamIntC regSize = camRegionLimit, CamIntC aThr = 5) {
	CamInt pList[camWidth * camHeight]; // queue
	CamInt nLoc, pLocNow = 0, pLocNext = 0, pNum = 0, pArea, mNum = 0;
	CamInt pXS, pXE, pYS, pYE, pXC, pYC;
	
	for (CamInt yLoc = yS; yLoc < yE; yLoc++) {
		for (CamInt xLoc = xS; xLoc < xE; xLoc++) {
			desMask[yLoc * camWidth + xLoc] = 0;
		}
	}
	for (CamInt yLoc = yS; yLoc < yE; yLoc++) {
		for (CamInt xLoc = xS; xLoc < xE; xLoc++) {
			nLoc = yLoc * camWidth + xLoc;
			pArea = 0;
			if (srcMask[nLoc] == MASK_MAIN && desMask[nLoc] == 0) {
				pNum++;
				pList[pLocNext] = nLoc;
				desMask[pList[pLocNext]] = pNum;
				pLocNext++;
				pXS = xLoc; pXE = xLoc + 1;
				pYS = yLoc; pYE = yLoc + 1;
				pXC = 0; pYC = 0;
			}
			while (pLocNow < pLocNext) {
				pXC += pList[pLocNow] % camWidth;
				pYC += pList[pLocNow] / camWidth;
				pXS = min(pXS, pList[pLocNow] % camWidth);
				pXE = max(pXE, pList[pLocNow] % camWidth + 1);
				pYS = min(pYS, pList[pLocNow] / camWidth);
				pYE = max(pYE, pList[pLocNow] / camWidth + 1);
				if (pList[pLocNow] % camWidth > xS) { // left
					if (srcMask[pList[pLocNow] - 1] == MASK_MAIN && desMask[pList[pLocNow] - 1] == 0) {
						pList[pLocNext] = pList[pLocNow] - 1;
						desMask[pList[pLocNext]] = pNum;
						pLocNext++;
					}
				}
				if (pList[pLocNow] % camWidth < xE - 1) { // right
					if (srcMask[pList[pLocNow] + 1] == MASK_MAIN && desMask[pList[pLocNow] + 1] == 0) {
						pList[pLocNext] = pList[pLocNow] + 1;
						desMask[pList[pLocNext]] = pNum;
						pLocNext++;
					}
				}
				if (pList[pLocNow] / camWidth > yS) { // up
					if (srcMask[pList[pLocNow] - camWidth] == MASK_MAIN && desMask[pList[pLocNow] - camWidth] == 0) {
						pList[pLocNext] = pList[pLocNow] - camWidth;
						desMask[pList[pLocNext]] = pNum;
						pLocNext++;
					}
				}
				if (pList[pLocNow] / camWidth < yE - 1) { // down
					if (srcMask[pList[pLocNow] + camWidth] == MASK_MAIN && desMask[pList[pLocNow] + camWidth] == 0) {
						pList[pLocNext] = pList[pLocNow] + camWidth;
						desMask[pList[pLocNext]] = pNum;
						pLocNext++;
					}
				}
				pLocNow++;
				pArea++;
			}
			if (pArea > aThr && mNum < regSize) {
				desRegion[mNum].xS = pXS; desRegion[mNum].xE = pXE;
				desRegion[mNum].yS = pYS; desRegion[mNum].yE = pYE;
				desRegion[mNum].xC = (CamDouble)pXC / pArea;
				desRegion[mNum].yC = (CamDouble)pYC / pArea;
				desRegion[mNum].area = pArea;
				desRegion[mNum].num = pNum;
				desRegion[mNum].mask = desMask;
				mNum++;
			}
		}
	}

	return mNum;
}

CamFlag max_region(CamRegion srcRegion[], CamRegion *desRegion, CamIntC srcSize = camRegionLimit) {
	CamInt maxArea = 0, maxLoc = -1;
	if (srcSize > 0) {
		for (CamInt srcLoc = 0; srcLoc < srcSize; srcLoc++) {
			if (srcRegion[srcLoc].area > maxArea) {
				maxArea = srcRegion[srcLoc].area;
				maxLoc = srcLoc;
			}
		}
		if (maxLoc >= 0) {
			*desRegion = srcRegion[maxLoc];
		} else {
			return _ERR(L"max_region: maxLoc = -1");
		}
	} else {
		return _ERR(L"max_region: srcSize = 0");
	}
	
	return 1;
}

CamDouble eval_circular(CamRegion *srcRegion) {
	if (srcRegion->area > 0) {
		CamDouble srcR = sqrt(srcRegion->area / PI);
		CamInt nLoc, inArea = 0;
		for (CamInt yLoc = srcRegion->yS; yLoc < srcRegion->yE; yLoc++) {
			for (CamInt xLoc = srcRegion->xS; xLoc < srcRegion->xE; xLoc++) {
				nLoc = yLoc * camWidth + xLoc;
				if (srcRegion->mask[nLoc] == srcRegion->num && _EX2(xLoc - srcRegion->xC) + _EX2(yLoc - srcRegion->yC) <= _EX2(srcR)) {
					inArea++;
				}
			}
		}
		return CamDouble(inArea) / srcRegion->area;
	} else {
		return _ERR(L"eval_circular: srcRegion->area = 0");
	}
}

CamFlag near_circle_region2(CamRegion srcRegion[], CamRegion desRegion[], CamDoubleC cX, CamDoubleC cY, 
	CamIntC srcSize = camRegionLimit, CamDoubleC sThr = 0.5) {
	CamDouble nearDist[2] = { -1, -1 }, nowDist, nowCir;
	CamInt nearLoc[2] = { -1, -1 };
	for (CamInt srcLoc = 0; srcLoc < srcSize; srcLoc++) {
		nowCir = eval_circular(&(srcRegion[srcLoc]));
		if (nowCir > sThr) {
			nowDist = _EX2(srcRegion[srcLoc].xC - cX) + _EX2(srcRegion[srcLoc].yC - cY);
			if (nearDist[0] < 0 || nowDist < nearDist[0]) {
				nearDist[1] = nearDist[0];
				nearLoc[1] = nearLoc[0];
				nearDist[0] = nowDist;
				nearLoc[0] = srcLoc;
			} else if (nearDist[1] < 0 || nowDist < nearDist[1]) {
				nearDist[1] = nowDist;
				nearLoc[1] = srcLoc;
			}
		}
	}
	if (nearLoc[0] >= 0 && nearLoc[1] >= 0) {
		desRegion[0] = srcRegion[nearLoc[0]];
		desRegion[1] = srcRegion[nearLoc[1]];
	} else {
		return _ERR(L"near_circle_region2: nearLoc[] = -1");
	}

	return 1;
}

CamFlag fill_regionH(CamRegion *desRegion, CamMask desNum) {
	CamInt xS, xE;
	for (CamInt yLoc = desRegion->yS; yLoc < desRegion->yE; yLoc++) {
		xS = 0;
		xE = 0;
		for (CamInt xLoc = desRegion->xS; xLoc < desRegion->xE; xLoc++) {
			if (desRegion->mask[yLoc * camWidth + xLoc] == desRegion->num) {
				xS = xLoc;
				break;
			}
		}
		for (CamInt xLoc = desRegion->xE - 1; xLoc >= desRegion->xS; xLoc--) {
			if (desRegion->mask[yLoc * camWidth + xLoc] == desRegion->num) {
				xE = xLoc;
				break;
			}
		}
		for (CamInt xLoc = xS + 1; xLoc < xE; xLoc++) {
			if (desRegion->mask[yLoc * camWidth + xLoc] != desRegion->num) {
				desRegion->mask[yLoc * camWidth + xLoc] = desNum;
				desRegion->area++;
			}
		}
	}

	return 1;
}

CamFlag set_region(CamRegion *faceRegion) {
	CamInt xBias = camFrame / 250;
	CamInt eye1X = 138 + xBias, eye1Y = 88; CamDouble eye1W = 27, eye1H = 14;
	CamInt eye2X = 199 + xBias, eye2Y = 90; CamDouble eye2W = 28, eye2H = 14;
	CamInt nose1X = 159 + xBias, nose1Y = 123; CamDouble nose1W = 9, nose1H = 7;
	CamInt nose2X = 178 + xBias, nose2Y = 124; CamDouble nose2W = 9, nose2H = 7;
	CamInt mouthX = 167 + xBias, mouthY = 151; CamDouble mouthW = 51, mouthH = 24;
	for (CamInt yLoc = CamInt(eye1Y - eye1H / 2); yLoc <= CamInt(eye1Y + eye1H / 2 + 1); yLoc++) {
		for (CamInt xLoc = CamInt(eye1X - eye1W / 2); xLoc <= CamInt(eye1X + eye1W / 2 + 1); xLoc++) {
			if (_EX2(xLoc - eye1X) / _EX2(eye1W / 2) + _EX2(yLoc - eye1Y) / _EX2(eye1H / 2) <= 1) {
				faceRegion->mask[yLoc * camWidth + xLoc] = MASK_EYE;
			}
		}
	}
	for (CamInt yLoc = CamInt(eye2Y - eye2H / 2); yLoc <= CamInt(eye2Y + eye2H / 2 + 1); yLoc++) {
		for (CamInt xLoc = CamInt(eye2X - eye2W / 2); xLoc <= CamInt(eye2X + eye2W / 2 + 1); xLoc++) {
			if (_EX2(xLoc - eye2X) / _EX2(eye2W / 2) + _EX2(yLoc - eye2Y) / _EX2(eye2H / 2) <= 1) {
				faceRegion->mask[yLoc * camWidth + xLoc] = MASK_EYE;
			}
		}
	}
	for (CamInt yLoc = CamInt(nose1Y - nose1H / 2); yLoc <= CamInt(nose1Y + nose1H / 2 + 1); yLoc++) {
		for (CamInt xLoc = CamInt(nose1X - nose1W / 2); xLoc <= CamInt(nose1X + nose1W / 2 + 1); xLoc++) {
			if (_EX2(xLoc - nose1X) / _EX2(nose1W / 2) + _EX2(yLoc - nose1Y) / _EX2(nose1H / 2) <= 1) {
				faceRegion->mask[yLoc * camWidth + xLoc] = MASK_NOSE;
			}
		}
	}
	for (CamInt yLoc = CamInt(nose2Y - nose2H / 2); yLoc <= CamInt(nose2Y + nose2H / 2 + 1); yLoc++) {
		for (CamInt xLoc = CamInt(nose2X - nose2W / 2); xLoc <= CamInt(nose2X + nose2W / 2 + 1); xLoc++) {
			if (_EX2(xLoc - nose2X) / _EX2(nose2W / 2) + _EX2(yLoc - nose2Y) / _EX2(nose2H / 2) <= 1) {
				faceRegion->mask[yLoc * camWidth + xLoc] = MASK_NOSE;
			}
		}
	}
	for (CamInt yLoc = CamInt(mouthY - mouthH / 2); yLoc <= CamInt(mouthY + mouthH / 2 + 1); yLoc++) {
		for (CamInt xLoc = CamInt(mouthX - mouthW / 2); xLoc <= CamInt(mouthX + mouthW / 2 + 1); xLoc++) {
			if (_EX2(xLoc - mouthX) / _EX2(mouthW / 2) + _EX2(yLoc - mouthY) / _EX2(mouthH / 2) <= 1) {
				faceRegion->mask[yLoc * camWidth + xLoc] = MASK_MOUTH;
			}
		}
	}

	return 1;
}

//ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½

CamFlag ker_init() {
	for (CamInt YLoc = kerYMin; YLoc < kerYMax; YLoc++) {
		for (CamInt CbLoc = kerCbMin; CbLoc < kerCbMax; CbLoc++) {
			for (CamInt CrLoc = kerCrMin; CrLoc < kerCrMax; CrLoc++) {
				kerEyeYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin] = 0;
				kerNoseYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin] = 0;
				kerMouthYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin] = 0;
				kerNonSkinYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin] = 0;
				kerSkinYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin] = 0;
			}
		}
	}

	return 1;
}

CamFlag ker_GF(CamIntC radius, CamDouble sigma) {
	GF_set(radius, sigma);
	if (radius == 0) { return 0; }
	for (CamInt CrLoc = radius; CrLoc < kerCrMax - kerCrMin - radius; CrLoc++) { // Cr fixed
		for (CamInt YLoc = radius; YLoc < kerYMax - kerYMin - radius; YLoc++) {
			for (CamInt CbLoc = radius; CbLoc < kerCbMax - kerCbMin - radius; CbLoc++) {
				kerEyeGYCC[YLoc][CbLoc][CrLoc] = 0;
				kerNoseGYCC[YLoc][CbLoc][CrLoc] = 0;
				kerMouthGYCC[YLoc][CbLoc][CrLoc] = 0;
				kerNonSkinGYCC[YLoc][CbLoc][CrLoc] = 0;
				kerSkinGYCC[YLoc][CbLoc][CrLoc] = 0;
				for (CamInt dYLoc = YLoc - radius; dYLoc <= YLoc + radius; dYLoc++) {
					for (CamInt dCbLoc = CbLoc - radius; dCbLoc <= CbLoc + radius; dCbLoc++) {
						kerEyeGYCC[YLoc][CbLoc][CrLoc] += kerEyeYCC[dYLoc][dCbLoc][CrLoc] *
							gFilter[(dYLoc - YLoc + radius) * gSize + (dCbLoc - CbLoc + radius)];
						kerNoseGYCC[YLoc][CbLoc][CrLoc] += kerNoseYCC[dYLoc][dCbLoc][CrLoc] *
							gFilter[(dYLoc - YLoc + radius) * gSize + (dCbLoc - CbLoc + radius)];
						kerMouthGYCC[YLoc][CbLoc][CrLoc] += kerMouthYCC[dYLoc][dCbLoc][CrLoc] *
							gFilter[(dYLoc - YLoc + radius) * gSize + (dCbLoc - CbLoc + radius)];
						kerNonSkinGYCC[YLoc][CbLoc][CrLoc] += kerNonSkinYCC[dYLoc][dCbLoc][CrLoc] *
							gFilter[(dYLoc - YLoc + radius) * gSize + (dCbLoc - CbLoc + radius)];
						kerSkinGYCC[YLoc][CbLoc][CrLoc] += kerSkinYCC[dYLoc][dCbLoc][CrLoc] *
							gFilter[(dYLoc - YLoc + radius) * gSize + (dCbLoc - CbLoc + radius)];
					}
				}
			}
		}
	}
	for (CamInt YLoc = radius; YLoc < kerYMax - kerYMin - radius; YLoc++) { // update
		for (CamInt CbLoc = radius; CbLoc < kerCbMax - kerCbMin - radius; CbLoc++) {
			for (CamInt CrLoc = radius; CrLoc < kerCrMax - kerCrMin - radius; CrLoc++) {
				kerEyeYCC[YLoc][CbLoc][CrLoc] = kerEyeGYCC[YLoc][CbLoc][CrLoc];
				kerNoseYCC[YLoc][CbLoc][CrLoc] = kerNoseGYCC[YLoc][CbLoc][CrLoc];
				kerMouthYCC[YLoc][CbLoc][CrLoc] = kerMouthGYCC[YLoc][CbLoc][CrLoc];
				kerNonSkinYCC[YLoc][CbLoc][CrLoc] = kerNonSkinGYCC[YLoc][CbLoc][CrLoc];
				kerSkinYCC[YLoc][CbLoc][CrLoc] = kerSkinGYCC[YLoc][CbLoc][CrLoc];
			}
		}
	}
#if 1
	for (CamInt CbLoc = radius; CbLoc < kerCbMax - kerCbMin - radius; CbLoc++) { // Cb fixed
		for (CamInt YLoc = radius; YLoc < kerYMax - kerYMin - radius; YLoc++) {
			for (CamInt CrLoc = radius; CrLoc < kerCrMax - kerCrMin - radius; CrLoc++) {
				kerEyeGYCC[YLoc][CbLoc][CrLoc] = 0;
				kerNoseGYCC[YLoc][CbLoc][CrLoc] = 0;
				kerMouthGYCC[YLoc][CbLoc][CrLoc] = 0;
				kerNonSkinGYCC[YLoc][CbLoc][CrLoc] = 0;
				kerSkinGYCC[YLoc][CbLoc][CrLoc] = 0;
				for (CamInt dYLoc = YLoc - radius; dYLoc <= YLoc + radius; dYLoc++) {
					for (CamInt dCrLoc = CrLoc - radius; dCrLoc <= CrLoc + radius; dCrLoc++) {
						kerEyeGYCC[YLoc][CbLoc][CrLoc] += kerEyeYCC[dYLoc][CbLoc][dCrLoc] *
							gFilter[(dYLoc - YLoc + radius) * gSize + (dCrLoc - CrLoc + radius)];
						kerNoseGYCC[YLoc][CbLoc][CrLoc] += kerNoseYCC[dYLoc][CbLoc][dCrLoc] *
							gFilter[(dYLoc - YLoc + radius) * gSize + (dCrLoc - CrLoc + radius)];
						kerMouthGYCC[YLoc][CbLoc][CrLoc] += kerMouthYCC[dYLoc][CbLoc][dCrLoc] *
							gFilter[(dYLoc - YLoc + radius) * gSize + (dCrLoc - CrLoc + radius)];
						kerNonSkinGYCC[YLoc][CbLoc][CrLoc] += kerNonSkinYCC[dYLoc][CbLoc][dCrLoc] *
							gFilter[(dYLoc - YLoc + radius) * gSize + (dCrLoc - CrLoc + radius)];
						kerSkinGYCC[YLoc][CbLoc][CrLoc] += kerSkinYCC[dYLoc][CbLoc][dCrLoc] *
							gFilter[(dYLoc - YLoc + radius) * gSize + (dCrLoc - CrLoc + radius)];
					}
				}
			}
		}
	}
#endif
	for (CamInt YLoc = radius; YLoc < kerYMax - kerYMin - radius; YLoc++) { // update
		for (CamInt CbLoc = radius; CbLoc < kerCbMax - kerCbMin - radius; CbLoc++) {
			for (CamInt CrLoc = radius; CrLoc < kerCrMax - kerCrMin - radius; CrLoc++) {
				kerEyeYCC[YLoc][CbLoc][CrLoc] = kerEyeGYCC[YLoc][CbLoc][CrLoc];
				kerNoseYCC[YLoc][CbLoc][CrLoc] = kerNoseGYCC[YLoc][CbLoc][CrLoc];
				kerMouthYCC[YLoc][CbLoc][CrLoc] = kerMouthGYCC[YLoc][CbLoc][CrLoc];
				kerNonSkinYCC[YLoc][CbLoc][CrLoc] = kerNonSkinGYCC[YLoc][CbLoc][CrLoc];
				kerSkinYCC[YLoc][CbLoc][CrLoc] = kerSkinGYCC[YLoc][CbLoc][CrLoc];
			}
		}
	}

	return 1;
}

CamFlag ker_applyYCC(CamYCC srcYCC[], CamRegion *srcRegion, CamIntC srcWidth = camWidth, CamIntC srcHeight = camHeight) {
	CamInt nLoc, YLoc, CbLoc, CrLoc;
	CamDouble eyeYCC, noseYCC, mouthYCC, nonSkinYCC, skinYCC;
	for (CamInt yLoc = srcRegion->yS; yLoc < srcRegion->yE; yLoc++) {
		for (CamInt xLoc = srcRegion->xS; xLoc < srcRegion->xE; xLoc++) {
			nLoc = yLoc * srcWidth + xLoc;
			if (srcRegion->mask[nLoc] == srcRegion->num) {
				YLoc = CamInt(round(srcYCC[nLoc * 3 + 0]));
				CbLoc = CamInt(round(srcYCC[nLoc * 3 + 1]));
				CrLoc = CamInt(round(srcYCC[nLoc * 3 + 2]));
				if (YLoc < kerYMin || YLoc >= kerYMax) {
					srcRegion->mask[nLoc] = MASK_NONE;
					continue;
				}
				if (CbLoc < kerCbMin || CbLoc >= kerCbMax) {
					srcRegion->mask[nLoc] = MASK_NONE;
					continue;
				}
				if (CrLoc < kerCrMin || CrLoc >= kerCrMax) {
					srcRegion->mask[nLoc] = MASK_NONE;
					continue;
				}
				eyeYCC = kerEyeYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin];
				noseYCC = kerNoseYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin];
				mouthYCC = kerMouthYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin];
				nonSkinYCC = kerNonSkinYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin];
				skinYCC = kerSkinYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin];
#if 1
				if (skinYCC > 0 && nonSkinYCC > 0) {
					CamDouble logYCC = log(skinYCC / nonSkinYCC) * 30;
					camRGB[nLoc * 3 + 2] = 0xFF - min(max(CamInt(logYCC), 0), 0xFF);
					camRGB[nLoc * 3 + 1] = 0xFF + max(min(CamInt(logYCC), 0), -0xFF);
					camRGB[nLoc * 3 + 0] = 0 * min(CamInt(eyeYCC / 2), 0xFF);
				} else if (skinYCC > 0) {
					camRGB[nLoc * 3 + 2] = 0x00;
					camRGB[nLoc * 3 + 1] = 0x80;
					camRGB[nLoc * 3 + 0] = 0x00;
				} else if (nonSkinYCC > 0) {
					camRGB[nLoc * 3 + 2] = 0x80;
					camRGB[nLoc * 3 + 1] = 0x00;
					camRGB[nLoc * 3 + 0] = 0x00;
				} else {
					camRGB[nLoc * 3 + 2] = 0x00;
					camRGB[nLoc * 3 + 1] = 0x00;
					camRGB[nLoc * 3 + 0] = 0x00;
				}
#endif
			}
		}
	}

	return 1;
}

CamFlag ker_putYCC(CamYCC srcYCC[], CamRegion *srcRegion, CamIntC srcWidth = camWidth, CamIntC srcHeight = camHeight) {
	CamInt nLoc, YLoc, CbLoc, CrLoc;
	for (CamInt yLoc = srcRegion->yS; yLoc < srcRegion->yE; yLoc++) {
		for (CamInt xLoc = srcRegion->xS; xLoc < srcRegion->xE; xLoc++) {
			nLoc = yLoc * srcWidth + xLoc;
			YLoc = CamInt(round(srcYCC[nLoc * 3 + 0]));
			CbLoc = CamInt(round(srcYCC[nLoc * 3 + 1]));
			CrLoc = CamInt(round(srcYCC[nLoc * 3 + 2]));
			if (YLoc < kerYMin || YLoc >= kerYMax) {
				continue;
			}
			if (CbLoc < kerCbMin || CbLoc >= kerCbMax) {
				continue;
			}
			if (CrLoc < kerCrMin || CrLoc >= kerCrMax) {
				continue;
			}
			switch (srcRegion->mask[nLoc]) {
			case MASK_EYE:
				kerEyeYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin]++;
				kerNonSkinYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin]++;
				break;
			case MASK_NOSE:
				kerNoseYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin]++;
				kerNonSkinYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin]++;
				break;
			case MASK_MOUTH:
				kerMouthYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin]++;
				kerNonSkinYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin]++;
				break;
			default: {
				if (srcRegion->mask[nLoc] == srcRegion->num) {
					kerSkinYCC[YLoc - kerYMin][CbLoc - kerCbMin][CrLoc - kerCrMin]++;
				}
			}
			}
		}
	}

	return 1;
}

CamFlag ker_saveYCC(const char* fName) {
	FILE *kerSave;
	if(fopen_s(&kerSave, fName, "wb")) {
		//_MSG_W(L"ker_saveYCC : Unable to open the file to write");
		return -1;
	}
	for (CamInt YLoc = 0; YLoc < kerYMax - kerYMin; YLoc++) {
		for (CamInt CbLoc = 0; CbLoc < kerCbMax - kerCbMin; CbLoc++) {
			fwrite(kerEyeYCC[YLoc][CbLoc], sizeof(CamDouble), kerCrMax - kerCrMin, kerSave);
			fwrite(kerNoseYCC[YLoc][CbLoc], sizeof(CamDouble), kerCrMax - kerCrMin, kerSave);
			fwrite(kerMouthYCC[YLoc][CbLoc], sizeof(CamDouble), kerCrMax - kerCrMin, kerSave);
			fwrite(kerNonSkinYCC[YLoc][CbLoc], sizeof(CamDouble), kerCrMax - kerCrMin, kerSave);
			fwrite(kerSkinYCC[YLoc][CbLoc], sizeof(CamDouble), kerCrMax - kerCrMin, kerSave);
		}
	}
	fclose(kerSave);

	return 1;
}

CamFlag ker_loadYCC(const char* fName) {
	FILE *kerLoad;
	if (fopen_s(&kerLoad, fName, "rb")) {
		return -1;
	}
	for (CamInt YLoc = 0; YLoc < kerYMax - kerYMin; YLoc++) {
		for (CamInt CbLoc = 0; CbLoc < kerCbMax - kerCbMin; CbLoc++) {
			fread_s(kerEyeYCC[YLoc][CbLoc], sizeof(CamDouble) * (kerCrMax - kerCrMin), sizeof(CamDouble), kerCrMax - kerCrMin, kerLoad);
			fread_s(kerNoseYCC[YLoc][CbLoc], sizeof(CamDouble) * (kerCrMax - kerCrMin), sizeof(CamDouble), kerCrMax - kerCrMin, kerLoad);
			fread_s(kerMouthYCC[YLoc][CbLoc], sizeof(CamDouble) * (kerCrMax - kerCrMin), sizeof(CamDouble), kerCrMax - kerCrMin, kerLoad);
			fread_s(kerNonSkinYCC[YLoc][CbLoc], sizeof(CamDouble) * (kerCrMax - kerCrMin), sizeof(CamDouble), kerCrMax - kerCrMin, kerLoad);
			fread_s(kerSkinYCC[YLoc][CbLoc], sizeof(CamDouble) * (kerCrMax - kerCrMin), sizeof(CamDouble), kerCrMax - kerCrMin, kerLoad);
		}
	}
	fclose(kerLoad);

	return 1;
}

CamFlag ker_showYCC() {
	base_graph(graph1Out, CrStep, CbStep, _GRAY4, _GRAY6, _GRAY6); // background fill and grid line plot
	out_graph(graph1RGB); // graphOut plot
	RGB_out(graph1RGB, graph1Out, graphWidth, graphHeight, _BLACK); // graphOut to screen
	line_graph(graph1Out, CrLine, CbLine, _AQUA, _AQUA); // screen line plot
	
	GetClientRect(nowDlg, &wndRect);
	InvalidateRect(nowDlg, &wndRect, false);

	return 1;
}

//ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½

CamIntC fMax = 1000;

CamFlag out_region(CamRegion *srcRegion) {
	outTextW += L"Region Info\n(xS, yS) = (" + _TW(srcRegion->xS) + L", " + _TW(srcRegion->yS) +
		L")\n(xE, yE) = (" + _TW(srcRegion->xE) + L", " + _TW(srcRegion->yE) + L")\n";

	return 1;
}

CamFlag eye_region(CamRGB srcRGB[], CamYCC srcYCC[], CamRegion *faceRegion) {
	CamInt nLoc;
	CamDouble srcR, srcG, srcB, desH, desS, desB;
	for (CamInt yLoc = faceRegion->yS; yLoc < faceRegion->yE; yLoc++) {
		for (CamInt xLoc = faceRegion->xS; xLoc < faceRegion->xE; xLoc++) {
			nLoc = yLoc * camWidth + xLoc;
#if 1
			srcR = srcRGB[nLoc * 3 + 2] / 255.0;
			srcG = srcRGB[nLoc * 3 + 1] / 255.0;
			srcB = srcRGB[nLoc * 3 + 0] / 255.0;
			desH = 0; desS = 0; desB = 0;
			while (1) { // H = -2 ~ 2, S = 0 ~ 1, B = 2 ~ 0
				if ((1 - srcR) * srcR < 0 || (1 - srcG) * srcG < 0 || (1 - srcB) * srcB < 0) { // not valid color																					 //desH = srcR; desS = srcG; desB = srcB;
					break;
				}
				if (srcR == srcG && srcG == srcB) { // grayscale
					desB = 2 * (1 - srcR);
				} else if (srcR >= srcG && srcR >= srcB) {
					desB = max(2 - srcR - srcG, 2 - srcR - srcB);
					desS = (2 * srcR + desB - 2) / min(desB, 2 - desB);
					srcG = (srcG + min(0, desB - 1)) / min(desB, 2 - desB);
					srcB = (srcB + min(0, desB - 1)) / min(desB, 2 - desB);
					desH = (srcG - srcB) / desS;
				} else if (srcG >= srcB && srcG >= srcR) {
					desB = max(2 - srcG - srcB, 2 - srcG - srcR);
					desS = (2 * srcG + desB - 2) / min(desB, 2 - desB);
					srcB = (srcB + min(0, desB - 1)) / min(desB, 2 - desB);
					srcR = (srcR + min(0, desB - 1)) / min(desB, 2 - desB);
					desH = 2 + (srcB - srcR) / desS;
				} else {
					desB = max(2 - srcB - srcR, 2 - srcB - srcG);
					desS = (2 * srcB + desB - 2) / min(desB, 2 - desB);
					srcR = (srcR + min(0, desB - 1)) / min(desB, 2 - desB);
					srcG = (srcG + min(0, desB - 1)) / min(desB, 2 - desB);
					desH = -2 + (srcR - srcG) / desS;
				}
				break;
			}
			/*if (desH > 35.0 / 255 * 6 || desH < -25.0 / 255 * 6) {
				faceRegion->mask[nLoc] = MASK_NONE;
			}*/
			if (srcYCC[nLoc * 3 + 0] < 60) {
				faceRegion->mask[nLoc] = MASK_NONE;
			}
#else
			if (faceRegion->mask[nLoc] == faceRegion->num) {
				Y = _Y(srcRGB[nLoc * 3 + 2], srcRGB[nLoc * 3 + 1], srcRGB[nLoc * 3 + 0]) / 256;
				if (Y < 50) {
					faceRegion->mask[nLoc] = MASK_NONE;
				}
			}
#endif
		}
	}

	return 1;
}

CamFlag nose_region(CamRGB srcRGB[], CamRegion *faceRegion, CamRegion *noseRegion) {
	CamInt nLoc, cNum = 0;
	CamDouble Y, mR = 0, mG = 0, mB = 0, mY = 0, mDist = 0, cX = faceRegion->xC, cY = faceRegion->yC;
	for (CamInt yLoc = faceRegion->yS; yLoc < faceRegion->yE; yLoc++) {
		for (CamInt xLoc = faceRegion->xS; xLoc < faceRegion->xE; xLoc++) {
			nLoc = yLoc * camWidth + xLoc;
			if (faceRegion->mask[nLoc] == faceRegion->num) {
				mDist += _EX2(xLoc - cX) + _EX2(yLoc - cY);
				mR += srcRGB[nLoc * 3 + 2];
				mG += srcRGB[nLoc * 3 + 1];
				mB += srcRGB[nLoc * 3 + 0];
				mY += _Y(srcRGB[nLoc * 3 + 2], srcRGB[nLoc * 3 + 1], srcRGB[nLoc * 3 + 0]);
				cNum++;
			}
		}
	}
	if (cNum != faceRegion->area) {
		MessageBox(nowDlg, _C(L"area : " + _TW(faceRegion->area) + L", cNum : " + _TW(cNum)), DEF_TITLE, MB_OK);
	}
	mR /= cNum; mG /= cNum; mB /= cNum;
	mY /= cNum;
	mDist = sqrt(mDist / cNum);
	CamDouble rX, rY;
	rX = mDist * 0.7;
	rY = mDist * 0.4;
	CamInt nXS, nXE, nYS, nYE;
	nXS = CamInt(cX) - CamInt(rX);
	nXE = CamInt(cX) + CamInt(rX) + 1;
	nYS = CamInt(cY) - CamInt(rY);
	nYE = CamInt(cY) + CamInt(rY) + 1;
	for (CamInt yLoc = nYS ; yLoc < nYE; yLoc++) {
		for (CamInt xLoc = nXS; xLoc < nXE; xLoc++) {
			nLoc = yLoc * camWidth + xLoc;
			Y = _Y(srcRGB[nLoc * 3 + 2], srcRGB[nLoc * 3 + 1], srcRGB[nLoc * 3 + 0]);
			if (faceRegion->mask[nLoc] == faceRegion->num) {
				if (Y < mY * 0.5) {
					camNowMask[nLoc] = MASK_MAIN;
					//srcRGB[nLoc * 3 + 0] = 0xFF;
				} else {
					camNowMask[nLoc] = MASK_NONE;
					//srcRGB[nLoc * 3 + 2] = 0xCC;
				}
			}
		}
	}
	CamRegion noseTempRegion;
	if (near_circle_region2(camNowRegion, noseRegion, cX, cY, map_region(camNowMask, camNoseMask, camNowRegion, nXS, nXE, nYS, nYE)) > 0) {
		if (noseRegion[0].xC > noseRegion[1].xC) {
			noseTempRegion = noseRegion[0];
			noseRegion[0] = noseRegion[1];
			noseRegion[1] = noseTempRegion;
		}
	} else {
		return _ERR(L"nose_region: near_circle_Region2 failed");
	}

	return 1;
}

CamFlag face_norm(CamRGB srcRGB[], CamRGB desRGB[], CamRegion *faceRegion, CamRegion noseRegion[2], 
	CamIntC nWidth = graphWidth, CamIntC nHeight = graphHeight) {
	CamDouble noseDistX, noseDistY, faceCX, faceCY, faceAngle, sinVal, cosVal;
	noseDistX = noseRegion[1].xC - noseRegion[0].xC;
	noseDistY = noseRegion[1].yC - noseRegion[0].yC;
	faceCX = (noseRegion[0].xC + noseRegion[1].xC) / 2;
	faceCY = (noseRegion[0].yC + noseRegion[1].yC) / 2;
	if (noseDistX < 1) {
		return _ERR(L"noseDistX is too small");
	}
	faceAngle = atan(noseDistY / noseDistX);
	sinVal = sin(faceAngle);
	cosVal = cos(faceAngle);
	CamDouble nxPos, nyPos, rxPos, ryPos, sxPos, syPos;
	CamInt sxLow, syLow, dnLoc, fNum = faceRegion->num;
	CamDouble xy[2][2], xyS, desR, desG, desB;
	for (CamInt dyLoc = 0; dyLoc < nHeight; dyLoc++) {
		for (CamInt dxLoc = 0; dxLoc < nWidth; dxLoc++) {
			dnLoc = dyLoc * nWidth + dxLoc;
			nxPos = (dxLoc - nWidth / 2) * 150.0 / (nWidth / 2);
			nyPos = (dyLoc - nHeight / 2) * 150.0 / (nHeight / 2);
			rxPos = nxPos * cosVal - nyPos * sinVal;
			ryPos = nxPos * sinVal + nyPos * cosVal;
#if 1
			sxPos = rxPos + faceCX; sxLow = CamInt(floor(sxPos));
			syPos = ryPos + faceCY; syLow = CamInt(floor(syPos));
			xy[0][0] = (sxLow + 1 - sxPos) * (syLow + 1 - syPos);
			xy[1][0] = (sxPos - sxLow) * (syLow + 1 - syPos);
			xy[0][1] = (sxLow + 1 - sxPos) * (syPos - syLow);
			xy[1][1] = (sxPos - sxLow) * (syPos - syLow);
			for (CamInt xLoc = 0; xLoc < 2; xLoc++) {
				if (sxLow + xLoc < faceRegion->xS || sxLow + xLoc >= faceRegion->xE) {
					xy[xLoc][0] = 0; xy[xLoc][1] = 0;
				}
			}
			for (CamInt yLoc = 0; yLoc < 2; yLoc++) {
				if (syLow + yLoc < faceRegion->yS || syLow + yLoc >= faceRegion->yE) {
					xy[0][yLoc] = 0; xy[1][yLoc] = 0;
				}
			}
			xyS = 0;
			for (CamInt xLoc = 0; xLoc < 2; xLoc++) {
				for (CamInt yLoc = 0; yLoc < 2; yLoc++) {
					if (xy[xLoc][yLoc] > 0) {
						xy[xLoc][yLoc] *= (faceRegion->mask[(syLow + yLoc) * camWidth + sxLow + xLoc] == fNum);
						xyS += xy[xLoc][yLoc];
					}
				}
			}
			desR = 0; desG = 0; desB = 0;
			if (xyS > _Z4) {
				for (CamInt xLoc = 0; xLoc < 2; xLoc++) {
					for (CamInt yLoc = 0; yLoc < 2; yLoc++) {
						if (xy[xLoc][yLoc] > 0) {
							desR += srcRGB[((syLow + yLoc) * camWidth + sxLow + xLoc) * 3 + 2] * xy[xLoc][yLoc];
							desG += srcRGB[((syLow + yLoc) * camWidth + sxLow + xLoc) * 3 + 1] * xy[xLoc][yLoc];
							desB += srcRGB[((syLow + yLoc) * camWidth + sxLow + xLoc) * 3 + 0] * xy[xLoc][yLoc];
						}
					}
				}
				desRGB[dnLoc * 3 + 2] = CamInt(round(desR / xyS));
				desRGB[dnLoc * 3 + 1] = CamInt(round(desG / xyS));
				desRGB[dnLoc * 3 + 0] = CamInt(round(desB / xyS));
			} else {
				desRGB[dnLoc * 3 + 2] = 0x00;
				desRGB[dnLoc * 3 + 1] = 0x90;
				desRGB[dnLoc * 3 + 0] = 0x00;
			}
#else
			sxLow = CamInt(round(rxPos + faceCX));
			syLow = CamInt(round(ryPos + faceCY));
			if (sxLow >= faceRegion->xS && sxLow < faceRegion->xE && syLow >= faceRegion->yS && syLow < faceRegion->yE) {
				if (faceRegion->mask[syLow * camWidth + sxLow] == faceRegion->num) {
					desRGB[dnLoc * 3 + 2] = srcRGB[(syLow * camWidth + sxLow) * 3 + 2];
					desRGB[dnLoc * 3 + 1] = srcRGB[(syLow * camWidth + sxLow) * 3 + 1];
					desRGB[dnLoc * 3 + 0] = srcRGB[(syLow * camWidth + sxLow) * 3 + 0];
				} else {
					desRGB[dnLoc * 3 + 2] = 0x80;
					desRGB[dnLoc * 3 + 1] = 0x00;
					desRGB[dnLoc * 3 + 0] = 0x00;
				}
			} else {
				desRGB[dnLoc * 3 + 2] = 0x00;
				desRGB[dnLoc * 3 + 1] = 0x80;
				desRGB[dnLoc * 3 + 0] = 0x00;
			}
#endif
		}
	}

	outTextW += L"faceAngle : " + _TW(faceAngle * 180 / PI) + L"\n";

	point_RGB(srcRGB, CamInt(noseRegion[0].xC), CamInt(noseRegion[0].yC), _C_WHITE, camWidth, camHeight, 2);
	point_RGB(srcRGB, CamInt(noseRegion[1].xC), CamInt(noseRegion[1].yC), _C_WHITE, camWidth, camHeight, 2);

	point_RGB(srcRGB, CamInt(faceRegion->xC), CamInt(faceRegion->yC), _C_GREEN, camWidth, camHeight, 4);

	return 1;
}

//ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½

CamFlag init_cam(CamIntC radius, CamDoubleC sigma = 0.6) {
	if (radius >= 0 && radius <= gRadiusMax) {
		GF_set(radius, sigma);
	} else {
		_MSG_W(L"Please input valid GF size number");
	}

	return 1;
}

void cam_load(/*HWND hDlg, UINT message, UINT_PTR idEvent, DWORD dwTime*/) {
	if (camFrame == 0) {
		procTime = clock();
	}
	camFrame++;
	if (!feof(camLoad) && camFrame < fMax) {
		gtNow = clock();

		fread_s(camRGB, camWidth * camHeight * 3 * sizeof(UINT8) * 3, sizeof(UINT8), camWidth * camHeight * 3, camLoad);
		
		RGB_YCC(camRGB, camYCC, camWidth, camHeight); // RGB to YCC
		if (gSize > 0) {
			GF_double(camYCC, camGYCC, camWidth, camHeight); // GF(YCC)
		} else {
			copy_double(camYCC, camGYCC, camWidth * camHeight * 3); // GF(YCC)
		}
		YCC_mask(camGYCC, camNowMask, camWidth, camHeight); // YCC to mask

		max_region(camNowRegion, &camFaceRegion, map_region(camNowMask, camFaceMask, camNowRegion));
		fill_regionH(&camFaceRegion, camFaceRegion.num);

		//camRGB labeling EYE, NOSE, MOUTH, NONSKIN, SKIN
		if (kerMode == KER_READ) {
			ker_applyYCC(camGYCC, &camFaceRegion);
		} else if (kerMode == KER_WRITE) {
			set_region(&camFaceRegion);
			ker_putYCC(camGYCC, &camFaceRegion);
		}
		
		//eye_region(camRGB, camYCC, &camFaceRegion);
		//nose_region(camRGB, &camFaceRegion, camNoseRegion);
		//face_norm(camRGB, graphRGB, &camFaceRegion, camNoseRegion, normWidth, normHeight);

		gtSum += (clock() - gtNow) * (camFrame > 1);

		base_graph(graph1Out, CrStep, CbStep, _GRAY4, _GRAY6, _GRAY6);
		out_graph(graph1RGB);
		RGB_out(graph1RGB, graph1Out, graphWidth, graphHeight, _BLACK);
		//line_graph(graph1Out, CrLine, CbLine, _AQUA, _AQUA);

		base_graph(graph2Out, 5, 5, _GRAY4, _GRAY6, _GRAY6);
		out_graph2(camGYCC, graph2RGB, &camFaceRegion);
		RGB_out(graph2RGB, graph2Out, graphWidth, graphHeight, _BLACK);
		

		if (kerMode == KER_READ) {

		} else if (kerMode == KER_WRITE) {
			apply_RGB(camRGB, &camFaceRegion);
		}
		RGB_out(camRGB, camRGBOut, camWidth, camHeight);

		outTextW += L"Frame : " + _TW(camFrame) + L"\n";
		outTextW += L"Time : " + _TW(clock() - procTime) + L"\n";
		outTextW += L"Mtf : " + _DW((double)(clock() - procTime) / camFrame) + L" (" + _DW(camFrame * 1000.0 / (clock() - procTime)) + L" Fps)\n";
		SetDlgItemText(nowDlg, IDC_TXT_OUT, _C(outTextW));
		outTextW = L"";
		
		InvalidateRect(nowDlg, &camRect, false);
		InvalidateRect(nowDlg, &graph1Rect, false);
		InvalidateRect(nowDlg, &graph2Rect, false);
	} else {
		fclose(camLoad);
		KillTimer(nowDlg, ID_CAMTIMER);
		//_MSG_WN(L"gtSum = ", gtSum);
	}

}

//ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½

clock_t init_set(HWND hDlg) {
	appTime = clock();
	srand((unsigned int)time(nullptr));
	nowDlg = hDlg;
	rnd_title();
	SetDlgItemText(nowDlg, IDC_INPUT_CAMFILE, _C(camFileDef));
	SetDlgItemText(nowDlg, IDC_INPUT_KERFILE, _C(kerFileDef));
	SetDlgItemText(nowDlg, IDC_INPUT_GFSIZE, _C(_TW(gSizeDef)));
	SendMessage(GetDlgItem(nowDlg, IDC_RADIO_KERAPPLY), BM_SETCHECK, BST_CHECKED, true);

	HFONT hFixedFont = CreateFont(
		13, 0, 0, 0, // h, w, esc, ori
		FW_DONTCARE, false, false, false, // weight, italic, underline, strkieout
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY,// charset, outprec, clipprec, quality
		FIXED_PITCH, L"Tahoma"); // pitch, facename
								 //FIXED_PITCH, L"Fixedsys Excelsior 3.01"); // pitch, facename

	SendMessage(GetDlgItem(nowDlg, IDC_BTN_CAMLOAD), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_BTN_CAMSTOP), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_INPUT_CAMFILE), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_STATIC_CAMFILE), WM_SETFONT, (WPARAM)hFixedFont, true);

	SendMessage(GetDlgItem(nowDlg, IDC_BTN_KERLOAD), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_BTN_KERSAVE), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_BTN_KERGF), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_BTN_KERGFUP), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_BTN_KERGFDOWN), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_RADIO_KERAPPLY), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_RADIO_KERCAP), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_INPUT_KERFILE), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_FRAME_KER), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_STATIC_KERFILE), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_STATIC_KERMODE), WM_SETFONT, (WPARAM)hFixedFont, true);

	SendMessage(GetDlgItem(nowDlg, IDC_INPUT_GFSIZE), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_STATIC_GFSIZE), WM_SETFONT, (WPARAM)hFixedFont, true);

	SendMessage(GetDlgItem(nowDlg, IDC_BTN_GRAPHLINE), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_INPUT_GRAPHX), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_INPUT_GRAPHY), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_STATIC_GRAPHX), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_STATIC_GRAPHY), WM_SETFONT, (WPARAM)hFixedFont, true);

	SendMessage(GetDlgItem(nowDlg, IDC_TXT_OUT), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_BTN_CAP), WM_SETFONT, (WPARAM)hFixedFont, true);

	resolve_msg();

	//EnableWindow(GetDlgItem(nowDlg, IDC_RADIO_KMLOAD), false);
	//ShowWindow(nowDlg, SW_SHOW);
	//UpdateWindow(nowDlg);

	return clock() - appTime;
}

//ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½ï¿½Ì±ï¿½

INT_PTR CALLBACK MainProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	switch (message) {
	case WM_INITDIALOG:
		init_set(hDlg);
#ifdef USE_MODAL_DIALOG

#else
		ShowWindow(hDlg, SW_SHOW);
		UpdateWindow(hDlg);
#endif
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BTN_CAMLOAD: {
			WCHAR camFileWC[MAX_LOADSTRING], kerFileWC[MAX_LOADSTRING], gSizeWC[MAX_LOADSTRING];
			char camFileSC[MAX_LOADSTRING], kerFileSC[MAX_LOADSTRING];
			GetDlgItemText(nowDlg, IDC_INPUT_CAMFILE, camFileWC, MAX_LOADSTRING);
			GetDlgItemText(nowDlg, IDC_INPUT_KERFILE, kerFileWC, MAX_LOADSTRING);
			GetDlgItemText(nowDlg, IDC_INPUT_GFSIZE, gSizeWC, MAX_LOADSTRING);
			init_cam(_WI32(gSizeWC, MAX_LOADSTRING));
			_WC(camFileWC, camFileSC, MAX_LOADSTRING);
			_WC(kerFileWC, kerFileSC, MAX_LOADSTRING);
			if (kerMode == KER_READ) {
				if (ker_loadYCC(kerFileSC) > 0) {
					ker_showYCC();
				} else {
					_MSG_W(_C(L"Failed to open " + _W(kerFileWC)));
					break;
				}
			} else if (kerMode == KER_WRITE) {
				ker_init();
				ker_showYCC();
			}
			if (fopen_s(&camLoad, camFileSC, "rb")) {
				_MSG_W(_C(L"Failed to open " + _W(camFileWC)));
			} else {
				camFrame = 0;
				camMode = CAM_START;
				SetTimer(nowDlg, ID_CAMTIMER, camFrameRate, (TIMERPROC)cam_load);
			}
		} break;
		case IDC_RADIO_KERAPPLY:
			if (HIWORD(wParam) == BN_CLICKED) {
				kerMode = KER_READ;
			}
			break;
		case IDC_RADIO_KERCAP:
			if (HIWORD(wParam) == BN_CLICKED) {
				kerMode = KER_WRITE;
			}
			break;
		case IDC_BTN_KERGFUP: {
			WCHAR gSizeNowWC[MAX_LOADSTRING], gSizeNewWC[MAX_LOADSTRING];
			GetDlgItemText(nowDlg, IDC_INPUT_GFSIZE, gSizeNowWC, MAX_LOADSTRING);
			_IW32(_WI32(gSizeNowWC, MAX_LOADSTRING) + 1, gSizeNewWC);
			SetDlgItemText(nowDlg, IDC_INPUT_GFSIZE, gSizeNewWC);
		} break;
		case IDC_BTN_KERGFDOWN: {
			WCHAR gSizeNowWC[MAX_LOADSTRING], gSizeNewWC[MAX_LOADSTRING];
			GetDlgItemText(nowDlg, IDC_INPUT_GFSIZE, gSizeNowWC, MAX_LOADSTRING);
			_IW32(_WI32(gSizeNowWC, MAX_LOADSTRING) - 1, gSizeNewWC);
			SetDlgItemText(nowDlg, IDC_INPUT_GFSIZE, gSizeNewWC);
		} break;
		case IDC_BTN_GRAPHLINE: {
			WCHAR xLineWC[MAX_LOADSTRING], yLineWC[MAX_LOADSTRING];
			GetDlgItemText(nowDlg, IDC_INPUT_GRAPHX, xLineWC, MAX_LOADSTRING);
			GetDlgItemText(nowDlg, IDC_INPUT_GRAPHY, yLineWC, MAX_LOADSTRING);
			CbLine = _WI32(xLineWC, MAX_LOADSTRING);
			CrLine = _WI32(yLineWC, MAX_LOADSTRING);
		} break;
		case IDC_BTN_KERLOAD: {
			/*WCHAR kerFileWC[MAX_LOADSTRING];
			char kerFileSC[MAX_LOADSTRING];
			GetDlgItemText(nowDlg, IDC_INPUT_KERFILE, kerFileWC, MAX_LOADSTRING);
			_WC(kerFileWC, kerFileSC, MAX_LOADSTRING);
			if (ker_loadYCC(kerFileSC) > 0) {
				ker_showYCC();
			} else {
				_MSG_W(_C(L"Failed to open " + _W(kerFileWC)));
				break;
			}*/
		} break;
		case IDC_BTN_KERSAVE: {
			WCHAR kerFileWC[MAX_LOADSTRING];
			char kerFileSC[MAX_LOADSTRING];
			GetDlgItemText(nowDlg, IDC_INPUT_KERFILE, kerFileWC, MAX_LOADSTRING);
			_WC(kerFileWC, kerFileSC, MAX_LOADSTRING);
			if (ker_saveYCC(kerFileSC) > 0) {
				_MSG_W(_C(L"Succesfully saved " + _W(kerFileWC)));
			} else {
				_MSG_W(_C(L"Failed to save " + _W(kerFileWC)));
			}
		} break;
		case IDC_BTN_KERGF: {
			ker_GF(5, 0.6);
			ker_showYCC();
		} break;
		case IDC_BTN_CAP:
			break;
		case IDC_BTN_CAMSTOP: {
			switch (camMode) {
			case CAM_START:
				camMode = CAM_STOP;
				KillTimer(nowDlg, ID_CAMTIMER);
				break;
			case CAM_STOP:
			case CAM_FINISH:
				camMode = CAM_START;
				SetTimer(nowDlg, ID_CAMTIMER, camFrameRate, (TIMERPROC)cam_load);
				break;
			default:
				break;
			}
		} break;
		}
	case WM_SIZE:
		break;
	case WM_PAINT: {
		hDC = BeginPaint(hDlg, &ps);
		if (newDC > 0) {
			newDC = 0;
			hOutMemDC = CreateCompatibleDC(hDC);
			hOutBitmap = CreateCompatibleBitmap(hDC, camRGBOutWidth, camRGBOutHeight);

			hGraph1MemDC = CreateCompatibleDC(hDC);
			hGraph1Bitmap = CreateCompatibleBitmap(hDC, graphWidth, graphHeight);

			hGraph2MemDC = CreateCompatibleDC(hDC);
			hGraph2Bitmap = CreateCompatibleBitmap(hDC, graphWidth, graphHeight);
		}
		if (1) {
			SetBitmapBits(hOutBitmap, camRGBOutWidth * camRGBOutHeight * 4, camRGBOut);
			SelectObject(hOutMemDC, hOutBitmap);
			BitBlt(hDC, camRGBOutX, camRGBOutY, camRGBOutWidth, camRGBOutHeight, hOutMemDC, 0, 0, SRCCOPY);

			SetBitmapBits(hGraph1Bitmap, graphWidth * graphHeight * 4, graph1Out);
			SelectObject(hGraph1MemDC, hGraph1Bitmap);
			BitBlt(hDC, graph1X, graph1Y, graphWidth, graphHeight, hGraph1MemDC, 0, 0, SRCCOPY);

			SetBitmapBits(hGraph2Bitmap, graphWidth * graphHeight * 4, graph2Out);
			SelectObject(hGraph2MemDC, hGraph2Bitmap);
			BitBlt(hDC, graph2X, graph2Y, graphWidth, graphHeight, hGraph2MemDC, 0, 0, SRCCOPY);
		}
		EndPaint(hDlg, &ps);
	} break;

	case WM_DESTROY:
		DeleteDC(hOutMemDC);
		DeleteObject(hOutBitmap);
		DeleteDC(hGraph1MemDC);
		DeleteObject(hGraph1Bitmap);
		DeleteDC(hGraph2MemDC);
		DeleteObject(hGraph2Bitmap);
#ifdef USE_MODAL_DIALOG
		if (EndDialog(hDlg, 0)) {
			PostQuitMessage(0);
		}
#else
		PostQuitMessage(0);
#endif
		return (INT_PTR)TRUE;
	default:
		DefWindowProc(hDlg, message, wParam, lParam);
		break;
	}
	return (INT_PTR)FALSE;
}

// Á¤º¸ ´ëÈ­ »óÀÚÀÇ ¸Þ½ÃÁö Ã³¸®±âÀÔ´Ï´Ù.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	switch (message) {
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

#if 0

DWORD WINAPI cap_func(PVOID pvParam) {
	nowCam = capCreateCaptureWindow(L"camera window", WS_CHILD | WS_VISIBLE, 10, 10, 480, 360, nowDlg, 0);

	while (!capDriverConnect(nowCam, 0));

	capPreviewRate(nowCam, 16);
	capPreview(nowCam, TRUE);
	capPreviewScale(nowCam, TRUE);

	capCount++;
	if (clock() - capTime > 1000) {
		capTime = clock();
		_W dlgTitle;
		dlgTitle += wchar_t(8);
		for (int i = 1; i < 31; i++) {
			dlgTitle += wchar_t(i);
		}
		dlgTitle += L" :: " + _TW(capCount) + L", " + _TW(msgCount) + L", " + _TW(paintCount);
		SetWindowText(nowDlg, dlgTitle.c_str());
	}

	return 0;
}

clock_t start_cam() {
	clock_t locTime = clock();
	hTest = CreateThread(NULL, 0, cap_func, nullptr, 0, &idTest);

	return clock() - locTime;
}

clock_t cap_cam() {
	clock_t locTime = clock();

	capSetCallbackOnFrame(nowCam, cap_func);
	capGrabFrameNoStop(nowCam);

	return clock() - locTime;
}

clock_t stop_cam() {
	clock_t locTime = clock();

	ShowWindow(nowCam, SW_HIDE);
	SendMessage(nowCam, WM_CAP_DRIVER_DISCONNECT, 0, 0);

	return clock() - locTime;
}

clock_t GF_RGB(CamRGB srcRGB[], CamRGB desRGB[], CamInt srcWidth, CamInt srcHeight) {
	clock_t locTime = clock();
	CamDouble nowVal, lineSum[gSize];
	for (CamInt yLoc = 0; yLoc < srcHeight; yLoc++) {
		for (CamInt xLoc = 0; xLoc < srcWidth; xLoc++) {
			for (CamInt cLoc = 0; cLoc < 3; cLoc++) {
				srcRGB[(yLoc * srcWidth + xLoc) * 3 + cLoc] = cLoc * (yLoc % 100);
			}
		}
	}
	for (CamInt cLoc = 0; cLoc < 3; cLoc++) {
		for (CamInt yLoc = 0; yLoc < srcHeight; yLoc++) {
			for (CamInt gxLoc = 0; gxLoc < gSize; gxLoc++) {
				lineSum[gxLoc] = 0;
			}
			for (CamInt gxLoc = gRadius; gxLoc < gSize; gxLoc++) {
				for (CamInt gyLoc = max(yLoc - gRadius, 0); gyLoc <= min(yLoc + gRadius, srcHeight - 1); gyLoc++) {
					lineSum[gxLoc] += srcRGB[(gyLoc * srcWidth + (gxLoc - gRadius)) * 3 + cLoc] * gFilter[(gyLoc - yLoc + gRadius) * gSize + gRadius];
				}
			}
			for (CamInt xLoc = 0; xLoc < srcWidth; xLoc++) {
				nowVal = 0;
				for (CamInt gLoc = 0; gLoc < gSize; gLoc++) {
					nowVal += lineSum[gLoc] * gRate[gLoc];
				}
				nowVal /= gSum[min(min(yLoc, srcHeight - yLoc - 1), gRadius) * gSize + min(min(xLoc, srcWidth - xLoc - 1), gRadius)];
				desRGB[(yLoc * srcWidth + xLoc) * 3 + cLoc] = (CamRGB)(round(min(nowVal, 0xFF)));
				for (CamInt gLoc = 0; gLoc < gSize - 1; gLoc++) {
					lineSum[gLoc] = lineSum[gLoc + 1];
				}
				lineSum[gSize - 1] = 0;
				if (xLoc + gRadius + 1 < srcWidth) {
					for (CamInt gLoc = max(yLoc - gRadius, 0); gLoc <= min(yLoc + gRadius, srcHeight - 1); gLoc++) {
						lineSum[gSize - 1] += srcRGB[(gLoc * srcWidth + (xLoc + gRadius + 1)) * 3 + cLoc] *
							gFilter[(gLoc - yLoc + gRadius) * gSize + gRadius];
					}
				}
			}
		}
	}

	return clock() - locTime;
}


{
	sxLoc = CamInt(floor(rxPos + faceCX));
	syLoc = CamInt(floor(ryPos + faceCY));
	if (sxLoc >= faceRegion->xS && sxLoc + 1 < faceRegion->xE) { // low high ok
		xLyL = sxLoc + 1 - (rxPos + faceCX); xHyL = rxPos + faceCX - sxLoc;
		xLyH = sxLoc + 1 - (rxPos + faceCX); xHyH = rxPos + faceCX - sxLoc;
	} else if (sxLoc >= faceRegion->xS && sxLoc < faceRegion->xE) { // low ok
		xLyL = 1; xLyH = 1; xHyL = 0; xHyH = 0;
	} else if (sxLoc + 1 >= faceRegion->xS && sxLoc + 1 < faceRegion->xE) { // high ok
		xLyL = 0; xLyH = 0; xHyL = 1; xHyH = 1;
	} else {
		xLyL = 0; xLyH = 0; xHyL = 0; xHyH = 0;
	}
	if (syLoc >= faceRegion->yS && syLoc + 1 < faceRegion->yE) { // low high ok
		xLyL *= syLoc + 1 - (ryPos + faceCY); xLyH *= ryPos + faceCY - syLoc;
		xHyL *= syLoc + 1 - (ryPos + faceCY); xHyH *= ryPos + faceCY - syLoc;
	} else if (syLoc >= faceRegion->yS && syLoc < faceRegion->yE) { // low ok
		xLyH = 0; xHyH = 0;
	} else if (syLoc + 1 >= faceRegion->yS && syLoc + 1 < faceRegion->yE) { // high ok
		xLyL = 0; xHyL = 0;
	} else {
		xLyL = 0; xLyH = 0; xHyL = 0; xHyH = 0;
	}
	if (xLyL > 0) {
		if (faceRegion->mask[syLoc * camWidth + sxLoc] != faceRegion->num) { xLyL = 0; }
	}
	if (xLyH > 0) {
		if (faceRegion->mask[(syLoc + 1) * camWidth + sxLoc] != faceRegion->num) { xLyH = 0; }
	}
	if (xHyL > 0) {
		if (faceRegion->mask[syLoc * camWidth + sxLoc + 1] != faceRegion->num) { xHyL = 0; }
	}
	if (xHyH > 0) {
		if (faceRegion->mask[(syLoc + 1) * camWidth + sxLoc + 1] != faceRegion->num) { xHyH = 0; }
	}
	xyLH = xLyL + xLyH + xHyL + xHyH;
	if (xyLH > _Z4) {
		desR = 0; desG = 0; desB = 0;
		if (xyLH < 1 - _Z4) {
			xLyL /= xyLH; xLyH /= xyLH; xHyL /= xyLH; xHyH /= xyLH;
		}
		if (xLyL > 0) {
			desR += srcRGB[(syLoc * camWidth + sxLoc) * 3 + 2] * xLyL;
			desG += srcRGB[(syLoc * camWidth + sxLoc) * 3 + 1] * xLyL;
			desB += srcRGB[(syLoc * camWidth + sxLoc) * 3 + 0] * xLyL;
		}
		if (xLyH > 0) {
			desR += srcRGB[((syLoc + 1) * camWidth + sxLoc) * 3 + 2] * xLyH;
			desG += srcRGB[((syLoc + 1) * camWidth + sxLoc) * 3 + 1] * xLyH;
			desB += srcRGB[((syLoc + 1) * camWidth + sxLoc) * 3 + 0] * xLyH;
		}
		if (xHyL > 0) {
			desR += srcRGB[(syLoc * camWidth + sxLoc + 1) * 3 + 2] * xHyL;
			desG += srcRGB[(syLoc * camWidth + sxLoc + 1) * 3 + 1] * xHyL;
			desB += srcRGB[(syLoc * camWidth + sxLoc + 1) * 3 + 0] * xHyL;
		}
		if (xHyH > 0) {
			desR += srcRGB[((syLoc + 1) * camWidth + sxLoc) * 3 + 2] * xHyH;
			desG += srcRGB[((syLoc + 1) * camWidth + sxLoc) * 3 + 1] * xHyH;
			desB += srcRGB[((syLoc + 1) * camWidth + sxLoc) * 3 + 0] * xHyH;
		}
		desRGB[dnLoc * 3 + 2] = CamInt(round(desR));
		desRGB[dnLoc * 3 + 1] = CamInt(round(desG));
		desRGB[dnLoc * 3 + 0] = CamInt(round(desB));
	} else { // zero
		desRGB[dnLoc * 3 + 2] = 0x00;
		desRGB[dnLoc * 3 + 1] = 0x80;
		desRGB[dnLoc * 3 + 0] = 0x00;
	}
}

#endif