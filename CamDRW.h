#pragma once

#include "stdafx.h"
#include <commctrl.h>
#include "resource.h"
#include "CamDRW_def.h"

//#include "opencv2/opencv.hpp"

#define MAX_LOADSTRING		100
#define DEF_MSG_TIME		500
#define DEF_TITLE L""

#define ID_CAMTIMER			0x100

#define USE_MODAL_DIALOG

#define KER_READ		0
#define KER_WRITE		1

#define CAM_START		1
#define CAM_STOP		0
#define CAM_FINISH		2

_W camFileDef = L"cam_video_eyestable.txt";
_W kerFileDef = L"ker_YCC.txt";
CamIntC gSizeDef = 5;

#pragma comment(linker, \
  "\"/manifestdependency:type='Win32' "\
  "name='Microsoft.Windows.Common-Controls' "\
  "version='6.0.0.0' "\
  "processorArchitecture='*' "\
  "publicKeyToken='6595b64144ccf1df' "\
  "language='*'\"")
#pragma comment(lib, "ComCtl32.lib")

//ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
INT_PTR CALLBACK	MainProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
