#pragma once

#include "stdafx.h"

#define USE_TRAIN
//#define USE_INT_KERNEL
#define USE_TEST_MAX // if not distance to 1 would be used
#define USE_FEAT_LSLF

#define DataIntMax INT_MAX
#define ImgDataTMax UCHAR_MAX

typedef short FlagT; // Flag, system variable type

typedef int LocInt; // location, count, numbering integer type
typedef const LocInt LocIntC;

typedef unsigned char LabelT; // Label range type
typedef int32_t DataInt; // Data int value type

typedef double DataDouble; // Data double value type
typedef const DataDouble DataDoubleC;
typedef DataDouble *DataVec; // Data vector type
typedef DataDouble **DataMat; // Data matrix type

#ifdef USE_INT_KERNEL
	typedef unsigned char KerDataT; // Kernel storing type
	typedef DataDouble DataValK;
	#define _RK(x) (round(x)) 
#else
	typedef double KerDataT; // Kernel storing type
	typedef DataInt DataValK;
	#define _RK(x) (x) 
#endif

LocIntC mWidth = 28, mHeight = 28;
LocIntC mDim = mWidth * mHeight;
#ifdef USE_TRAIN
LocIntC mImgCap = 60000;
#else
LocIntC mImgTrainCap = 8000;
LocIntC mImgTestCap = 10000 - mImgTrainCap;
#endif
LocIntC mLabelNum = 10;
LocIntC mLabelImgCap = int(mImgCap / mLabelNum * 1.2);

typedef unsigned char ImgDataT; // Data storing type
typedef struct ImgT { // Input image
	LabelT label;
	ImgDataT data[mDim];
} *pImgT;

LocIntC mKerCap = 110;
LocIntC mKerMemCap = mLabelImgCap;
LocIntC kmeansIterLimit = 500;
typedef struct KerT { // K-means kernel
	LabelT label;
	LocInt count;
	LocInt list[mKerMemCap];
	KerDataT data[mDim];
} *pKerT;

typedef struct SetT { // DCRL image set
	LabelT label;
	LocInt count;
	LocInt *list;
} *pSetT;

typedef struct ConT { // confusion matrix
	LocInt count[mLabelNum][mLabelNum];
	LocInt rcount[mLabelNum];
	LocInt ccount[mLabelNum];
	LocInt tcount, scount;
} *pConT;


