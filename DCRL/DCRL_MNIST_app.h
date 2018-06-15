#pragma once

//#include "stdafx.h"
#include <commctrl.h>
#include "resource.h"
#include "DCRL_MNIST_def.h"

#define MAX_LOADSTRING		100
#define CSV_LINE_SIZE		2500
#define DEF_MSG_TIME		500
#define DEF_TITLE L""

#define DCRL_LOAD_NONE		0x00
#define DCRL_LOAD_PINV		0x01
#define DCRL_LOAD_W			0x11

#define DCRL_TEST_FEAT		0x00
#define DCRL_TEST_COV		0x01
#define DCRL_TEST_LOGCOV	0x11

#define DCRL_NORM_NONE		0x00
#define DCRL_NORM_STD		0x11
#define DCRL_NORM_COV		0x02
#define DCRL_NORM_LOG		0x20
#define DCRL_NORM_COVLOG	0x22

#define NORM_NONE		0x0
#define NORM_STD		0x1
#define NORM_UNIT		0x2
#define NORM_DIAG		0x3

#define DCRL_SEL_SETONE		0x00
#define DCRL_SEL_SETSUM		0x01
#define DCRL_SEL_IMGONE		0x10

#define MAT_OUT
#define DEF_MAT_OUT IDC_TXT_OUT

#define NO_BASE_WINDOW
#define NO_LOCAL_DC
#define NO_MSG_MONITOR
#define USE_MODAL_DIALOG

#pragma comment(linker, \
  "\"/manifestdependency:type='Win32' "\
  "name='Microsoft.Windows.Common-Controls' "\
  "version='6.0.0.0' "\
  "processorArchitecture='*' "\
  "publicKeyToken='6595b64144ccf1df' "\
  "language='*'\"")
#pragma comment(lib, "ComCtl32.lib")
												// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
#ifndef NO_BASE_WINDOW
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
#endif
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	MainProc(HWND, UINT, WPARAM, LPARAM);

// Data types, variables start

LocIntC dlgImgX = 10, dlgImgY = 10;
LocIntC dlgImgNumX = 20, dlgImgNumY = 15;

LocIntC confBlockWidth = 16, confBlockHeight = 16, confBlockSpace = 2;
LocIntC confImgX = 828, confImgY = 29;
LocIntC confImgWidth = mLabelNum * confBlockWidth + (mLabelNum - 1) * confBlockSpace;
LocIntC confImgHeight = mLabelNum * confBlockHeight + (mLabelNum - 1) * confBlockSpace;

#define _MSG_W(x) MessageBox(nowDlg, x, DEF_TITLE, MB_OK)
#define _MSG_WN(x,y) MessageBox(nowDlg, (_W(x) + _TW(y)).c_str(), DEF_TITLE, MB_OK)

#ifdef USE_TRAIN
_W csvFileDefW = L"mnist_train.csv";
_W csvNumDefW = L"60000";
#else
_W csvFileDefW = L"mnist_test.csv";
_W csvNumDefW = L"10000";
#endif

_W kmeansNum1DefW = L"3";
_W kmeansNum2DefW = L"57";

const char kmeansOutFileDefC[MAX_LOADSTRING] = "kmeans_out";
//const char kmeansResultFileDefC[MAX_LOADSTRING] = "kmeans_result";

const char DCRL_A_pinv_FileDefC[MAX_LOADSTRING] = "mat_A_pinv";
const char DCRL_W_FileDefC[MAX_LOADSTRING] = "mat_W";


