#pragma once

#include "stdafx.h"

#define USE_1x_RES
//#define USE_2x_RES

typedef INT16 CamFlag;
typedef INT32 CamInt;
typedef DOUBLE CamDouble;
typedef INT16 CamMask;
typedef UINT8 CamRGB;
typedef double CamYCC;
//typedef UINT16 CamYCC;

typedef const CamFlag CamFlagC;
typedef const CamInt CamIntC;
typedef const CamDouble CamDoubleC;
typedef const CamRGB CamRGBC;
typedef const CamYCC CamYCCC;

typedef CamInt *CamIntP;

#define MASK_EXTRA		(camRegionLimit + 0x3F)
#define MASK_MAIN		(camRegionLimit + 0x2F)
#define MASK_SUB		(camRegionLimit + 0x1F)
#define MASK_NONE		0

#define MASK_EYE		(camRegionLimit + 0x50)
#define MASK_NOSE		(camRegionLimit + 0x40)
#define MASK_MOUTH		(camRegionLimit + 0x30)
#define MASK_NONSKIN	(camRegionLimit + 0x20)
#define MASK_SKIN		(camRegionLimit + 0x10)

#define PI 3.14159265358979323846

#ifdef USE_1x_RES 
CamIntC camWidth = 320, camHeight = 240;
#endif
#ifdef USE_2x_RES
CamIntC camWidth = 640, camHeight = 480;
#endif

struct CamRegion {
	CamInt area = 0;
	CamInt num = 0;
	CamInt xS = 0, xE = 0, yS = 0, yE = 0;
	CamDouble xC = 0, yC = 0;
	CamMask *mask = nullptr;
};

CamRGBC _C_RED[] =		{ 0x00, 0x00, 0xFF };
CamRGBC _C_GREEN[] =	{ 0x00, 0xFF, 0x00 };
CamRGBC _C_BLUE[] =		{ 0xFF, 0x00, 0x00 };
CamRGBC _C_YELLOW[] =	{ 0x00, 0xFF, 0xFF };
CamRGBC _C_AQUA[] =		{ 0xFF, 0xFF, 0x00 };
CamRGBC _C_MAGENTA[] =	{ 0xFF, 0x00, 0xFF };
CamRGBC _C_WHITE[] =	{ 0xFF, 0xFF, 0xFF };
CamRGBC _C_BLACK[] =	{ 0x00, 0x00, 0x00 };

CamIntC camFPS = 30;