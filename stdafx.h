// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
// Windows 헤더 파일:
#include <windows.h>

// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <time.h>

#include <vfw.h>
#include <string>

#ifdef WM_USER
#define A 0
#else
#define A 1
#endif
// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.

#define _W std::wstring
#define _TW std::to_wstring
#define _TWC(x) (std::to_wstring(x).c_str())

#define _S std::string
#define _TS std::to_string
#define _TSC(x) (std::to_string(x).c_str())

#define _Y(r,g,b) (65.738 * r + 129.057 * g + 25.064 * b)
#define _Cb(r,g,b) (-37.945 * r - 74.494 * g + 112.439 * b)
#define _Cr(r,g,b) (112.439 * r - 94.154 * g - 18.285 * b)

#define _C(x) ((x).c_str())

#define _RED		RGB(0x00, 0x00, 0xFF)
#define _GREEN		RGB(0x00, 0xFF, 0x00)
#define _BLUE		RGB(0xFF, 0x00, 0x00)
#define _YELLOW		RGB(0x00, 0xFF, 0xFF)
#define _AQUA		RGB(0xFF, 0xFF, 0x00)
#define _MAGENTA	RGB(0xFF, 0x00, 0xFF)
#define _WHITE		RGB(0xFF, 0xFF, 0xFF)
#define _BLACK		RGB(0x00, 0x00, 0x00)

#define _GRAY0		RGB(0x00, 0x00, 0x00)
#define _GRAY1		RGB(0x10, 0x10, 0x10)
#define _GRAY2		RGB(0x20, 0x20, 0x20)
#define _GRAY3		RGB(0x30, 0x30, 0x30)
#define _GRAY4		RGB(0x40, 0x40, 0x40)
#define _GRAY5		RGB(0x50, 0x50, 0x50)
#define _GRAY6		RGB(0x60, 0x60, 0x60)
#define _GRAY7		RGB(0x70, 0x70, 0x70)
#define _GRAY8		RGB(0x80, 0x80, 0x80)
#define _GRAY9		RGB(0x90, 0x90, 0x90)
#define _GRAYA		RGB(0xA0, 0xA0, 0xA0)
#define _GRAYB		RGB(0xB0, 0xB0, 0xB0)
#define _GRAYC		RGB(0xC0, 0xC0, 0xC0)
#define _GRAYD		RGB(0xD0, 0xD0, 0xD0)
#define _GRAYE		RGB(0xE0, 0xE0, 0xE0)
#define _GRAYF		RGB(0xF0, 0xF0, 0xF0)

#define _Z1 0.1
#define _Z2 0.01
#define _Z4 0.0001
#define _Z8 0.00000001

#define _EX2(x) ((x)*(x))
#define _Z(x,e) ((x)<(e)&&(x)>-(e))
#define _NZ(x,e) ((x)>(e)||(x)<-(e))

#define _MSG_W(x) MessageBox(nowDlg, x, DEF_TITLE, MB_OK)
#define _MSG_WN(x,y) MessageBox(nowDlg, (_W(x) + _TW(y)).c_str(), DEF_TITLE, MB_OK)

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
