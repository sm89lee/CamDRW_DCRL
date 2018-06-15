// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �Ǵ� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN			 // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
// Windows ��� ����:
#include <windows.h>

// C ��Ÿ�� ��� �����Դϴ�.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <time.h>

#include <string>

// TODO: ���α׷��� �ʿ��� �߰� ����� ���⿡�� �����մϴ�.
#define _TW std::to_wstring
#define _W std::wstring
#define _TS std::to_string
#define _S std::string
#define _EX2(x) ((x)*(x))

#define _Z(x,e) ((x)<(e)&&(x)>-(e))
#define _NZ(x,e) ((x)>(e)||(x)<-(e))

#define _Z3 0.001
#define _Z5 0.00001
#define _Z7 0.0000001
#define _Z9 0.000000001
#define _Z11 0.00000000001
#define _Z15 0.000000000000001
#define _Z19 0.0000000000000000001
#define _Z23 0.00000000000000000000001