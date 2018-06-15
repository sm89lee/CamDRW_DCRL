// DCRL_MNIST.cpp : 응용 프로그램에 대한 진입점을 정의합니다.

#include "stdafx.h"
#include "DCRL_MNIST_app.h"
#include "DCRL_MNIST_util.h"

//using namespace std;

// 전역 변수:
HINSTANCE hInst;								// 현재 인스턴스입니다.
HWND nowDlg;
HDC hDC, hDlgMemDC, hConMemDC;
HBITMAP hDlgBitmap, hConBitmap;
PAINTSTRUCT ps;
RECT rect;

WCHAR szTitle[MAX_LOADSTRING];				 // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];			// 기본 창 클래스 이름입니다.

// Total data
LocInt mImgNum, mImgTrainNum, mImgTestNum;

ImgT mImgTotal[mImgCap]; // 60000 x (784 + 1) = 47,100,000 Bytes
LocInt mImgTotalLoc[mImgCap];
LocInt mImgLabelLoc[mLabelNum][mLabelImgCap];
LocInt mImgLabelNum[mLabelNum] = { 0, };

// Kernel data
LocInt mKerNum[mLabelNum];
KerT mKerData[mLabelNum][mKerCap]; // (784 x 8 + 6000 x 4 + 4) x 100 x 10 = 30,276,000 Bytes
DataDouble mKerVar[mLabelNum][mKerCap]; // for analysis

// Image set for training and test
LocInt featDim; // M (M = N, SetNum)
LocInt maxSetCount; // max N_i

LocInt setTotalNum; // N
SetT *setTotal; // image sets
pSetT *setTrainPtr;
pSetT *setTestPtr;

pKerT *kerTotalPtr; // kernel pointer
pKerT *kerTrainPtr; 

LocInt setTrainNum;
LocInt setTestNum;
//LocInt *setTrainLoc;
//LocInt *setTestLoc;

// LSLF matrices
DataMat mat_A, mat_A_test;
DataMat mat_B;
DataMat mat_A_pinv;
DataMat mat_AW, mat_AW_test;
DataMat mat_W;
DataMat mat_AT_A, mat_AT_A_inv;
/*
mat_A, mat_A_test				: 60000 x 784 x 8		= 376,320,000 Bytes		(376.32 MB)
mat_B							: ~60000 x 100 x 8		= 48,000,000 Bytes		(48 MB)
mat_A_pinv						: 784 x 60000 x 8		= 376,320,000 Bytes		(376.32 MB)
mat_AW, mat_AW_test				: 60000 x 100 x 8		= 48,000,000 Bytes		(48 MB)
mat_W							: 784 x 100 x 8			= 627,200 Bytes			(0.6272 MB)
mat_AT_A, mat_AT_A_inv			: 784 x 784 x 8 x 2		= 9,834,496 Bytes		(9.84 MB)

Total : 856,101,696 Bytes (856 MB)
*/

DataDouble eMoveRate = 0.9;
LocInt eValuePrec = 35, eDiffPrec = 5;
LocInt eIterLimit = 250000;

DataMat *mat_H, *mat_H_test; // N x N_i x M, feature vector
DataMat *mat_C, *mat_C_test; // N x M x M, covariance matrices
DataMat *mat_log_C, *mat_log_C_test; // N x M x M, log covariance matrices
DataVec *mat_D, *mat_D_test; // N x M
DataVec *mat_log_D, *mat_log_D_test; // N x M 
DataMat *mat_UT, *mat_UT_test; // N x M x M
/*
mat_H[], mat_H_test[]			: 60000 x 100 x 8		= 48,000,000 Bytes	(48 MB)
mat_C[], mat_C_test[]			: 150 x 100 x 100 x 8	= 12,000,000 Bytes	(12 MB)
mat_log_C[], mat_log_C_test[]	: 150 x 100 x 100 x 8	= 12,000,000 Bytes	(12 MB)

Total : 72,000,000 Bytes (72 MB)
*/

DataMat GE_LEM, GE_A, GE_J;

// App data and flags
clock_t appTime, procTime, msgTime = 0;

FlagT csvLoad = 0;
FlagT kmeansLoad = 0;
FlagT kmeansSave = 0;
FlagT DCRLLoad = DCRL_LOAD_NONE;
FlagT DCRLSave_A_pinv = 0;
FlagT DCRLSave_W = 0;

FlagT newDC = 1;
FlagT funcProc = 0;

uint32_t dlgImgOut[dlgImgNumY * mHeight * dlgImgNumX * mWidth] = { 0, }; // 4 * 560^2 = 1,254,400 bytes
uint32_t confImgOut[confImgWidth * confImgHeight];

FILE *csvInput;
FILE *kmeansInput, *kmeansOutput;
FILE *LSLFInput, *LSLFOutput, *LSLFReport;
FILE *featOutput;

int32_t resMsgNum = 0;

// Data types, variables end
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 여기에 코드를 입력합니다.
	srand((unsigned int)time(NULL));
	InitCommonControls();

	// 전역 문자열을 초기화합니다.
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
#ifndef NO_BASE_WINDOW
	LoadStringW(hInstance, IDC_DCRL_MNIST, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
#endif

	// 응용 프로그램 초기화를 수행합니다.
	if (!InitInstance (hInstance, nCmdShow)) {
		return FALSE;
	}

#ifndef NO_BASE_WINDOW
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DCRL_MNIST));
#else
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDD_MAIN));
#endif
	MSG msg;

	// 기본 메시지 루프입니다.
	while (GetMessage(&msg, nullptr, 0, 0)) {
#ifndef NO_MSG_MONITOR
		if (msgReqNum < MAX_MSG_SIZE) {
			msgReqData[msgReqNum].cmd = msg.message;
			msgReqData[msgReqNum].lparam = msg.lParam;
			msgReqData[msgReqNum].wparam = msg.wParam;
			msgReqNum++;
		}
#endif
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}			
	}
	return (int) msg.wParam;
}


// 함수: MyRegisterClass()
// 목적: 창 클래스를 등록합니다.

#ifndef NO_BASE_WINDOW
ATOM MyRegisterClass(HINSTANCE hInstance) {
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = MainProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DCRL_MNIST));
	wcex.hCursor = LoadCursor(nullptr, IDC_CROSS); 
	wcex.hbrBackground = (HBRUSH)COLOR_HIGHLIGHTTEXT;
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_DCRL_MNIST);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}
#endif

// 함수: InitInstance(HINSTANCE, int)
// 목적: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
// 설명: 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//		주 프로그램 창을 만든 다음 표시합니다.

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

#ifndef NO_BASE_WINDOW
	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, // class label type
		300, 300, 300, 300, // x y w h
		nullptr, nullptr, hInstance, nullptr); // window menu instance lparam
	if (!hWnd) {
		return FALSE;
	}
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
#endif

#ifdef USE_MODAL_DIALOG
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), nullptr, MainProc);
#else
	CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN), nullptr, MainProc);
#endif
	
	return TRUE;
}

#ifndef NO_BASE_WINDOW

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE:
		break;
	case WM_COMMAND:
		// 메뉴 선택을 구문 분석합니다.
		switch (LOWORD(wParam)) {
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		} break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

#endif


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
		resMsgNum++;
	}
	msgTime = clock();
	//rnd_title(nowDlg);

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

clock_t init_set(HWND hDlg) {
	appTime = clock();
	nowDlg = hDlg;

	SetDlgItemText(nowDlg, IDC_INPUT_CSVNAME, csvFileDefW.c_str());
	SetDlgItemText(nowDlg, IDC_INPUT_CSVNUM, csvNumDefW.c_str());
	SetDlgItemText(nowDlg, IDC_INPUT_KMNUM1, kmeansNum1DefW.c_str());
	SetDlgItemText(nowDlg, IDC_INPUT_KMNUM2, kmeansNum2DefW.c_str());

	EnableWindow(GetDlgItem(nowDlg, IDC_RADIO_KMLOAD), false);
	EnableWindow(GetDlgItem(nowDlg, IDC_RADIO_KMSAVE), false);
	EnableWindow(GetDlgItem(nowDlg, IDC_RADIO_PINVLOAD), false);
	EnableWindow(GetDlgItem(nowDlg, IDC_RADIO_WLOAD), false);
	EnableWindow(GetDlgItem(nowDlg, IDC_CHECK_PINVSAVE), false);
	EnableWindow(GetDlgItem(nowDlg, IDC_CHECK_WSAVE), false);

	for (LocInt yLoc = 0; yLoc < confImgHeight; yLoc++) {
		for (LocInt xLoc = 0; xLoc < confImgWidth; xLoc++) {
			confImgOut[yLoc * confImgWidth + xLoc] = RGB(100, 100, 100);
		}
	}

	rnd_title();

	HFONT hFixedFont = CreateFont(
		13, 0, 0, 0, // h, w, esc, ori
		FW_DONTCARE, false, false, false, // weight, italic, underline, strkieout
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY,// charset, outprec, clipprec, quality
		FIXED_PITCH, L"Tahoma"); // pitch, facename
		//FIXED_PITCH, L"Fixedsys Excelsior 3.01"); // pitch, facename
	
	SendMessage(GetDlgItem(nowDlg, IDC_BTN_CSVLOAD), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_BTN_CSVSHOW), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_INPUT_CSVNAME), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_INPUT_CSVNUM), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_FRAME_CSV), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_STAT_CSV), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_STATIC_CSV1), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_STATIC_CSV2), WM_SETFONT, (WPARAM)hFixedFont, true);

	SendMessage(GetDlgItem(nowDlg, IDC_BTN_KMEVAL), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_INPUT_KMNUM1), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_INPUT_KMNUM2), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_RADIO_KMLOAD), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_RADIO_KMSAVE), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_FRAME_KM), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_STATIC_KM), WM_SETFONT, (WPARAM)hFixedFont, true);

	SendMessage(GetDlgItem(nowDlg, IDC_RADIO_PINVLOAD), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_RADIO_WLOAD), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_CHECK_PINVSAVE), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_CHECK_WSAVE), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_FRAME_LSLF), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_STAT_LSLF), WM_SETFONT, (WPARAM)hFixedFont, true);

	SendMessage(GetDlgItem(nowDlg, IDC_BTN_DCRLEVAL), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_BTN_DCRLEXP), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_FRAME_DCRL), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_STAT_DCRL), WM_SETFONT, (WPARAM)hFixedFont, true);

	SendMessage(GetDlgItem(nowDlg, IDC_FRAME_OUT), WM_SETFONT, (WPARAM)hFixedFont, true);
	SendMessage(GetDlgItem(nowDlg, IDC_TXT_OUT), WM_SETFONT, (WPARAM)hFixedFont, true);

	resolve_msg();

	return clock() - appTime;
}

// read csv data
clock_t load_csv() {
	clock_t procTime = clock();
	WCHAR csvFileW[MAX_LOADSTRING], csvNumW[MAX_LOADSTRING];
	char csvFileC[MAX_LOADSTRING];
	GetDlgItemText(nowDlg, IDC_INPUT_CSVNAME, csvFileW, MAX_LOADSTRING);
	GetDlgItemText(nowDlg, IDC_INPUT_CSVNUM, csvNumW, MAX_LOADSTRING);
	_WC(csvFileW, csvFileC, MAX_LOADSTRING);
	mImgNum = _WI32(csvNumW, MAX_LOADSTRING);
	if (csvFileC[0] == 0) {
		MessageBox(nowDlg, L"Please input MNIST csv path", DEF_TITLE, MB_OK | MB_ICONERROR);
		return -1;
	}
	if (mImgNum == 0) {
		MessageBox(nowDlg, L"Please input valid number of MNIST images", DEF_TITLE, MB_OK | MB_ICONERROR);
		return -1;
	}
	fopen_s(&csvInput, csvFileC, "r");
	if (csvInput == NULL) {
		MessageBox(nowDlg, L"MNIST data does not exist", DEF_TITLE, MB_OK | MB_ICONERROR);
		return -1;
	}
	char csvBuf[CSV_LINE_SIZE];
	ImgDataT csvDataBuf[mDim + 1];
	LabelT mLabelBuf;
	
	for (LocInt labelLoc = 0; labelLoc < mLabelNum; labelLoc++) {
		mImgLabelNum[labelLoc] = 0;
	}
	for (LocInt imgLoc = 0; imgLoc < mImgNum; imgLoc++) { // train data load
		fscanf_s(csvInput, "%s", &csvBuf, sizeof(csvBuf));
		parse_img(csvBuf, csvDataBuf, mDim + 1);
		mLabelBuf = csvDataBuf[0];
		mImgTotal[imgLoc].label = mLabelBuf;
		for (LocInt dimLoc = 0; dimLoc < mDim; dimLoc++) {
			mImgTotal[imgLoc].data[dimLoc] = csvDataBuf[dimLoc + 1];
		}
		mImgTotalLoc[imgLoc] = imgLoc;
		mImgLabelLoc[mLabelBuf][mImgLabelNum[mLabelBuf]] = imgLoc;
		mImgLabelNum[mLabelBuf]++;
	
		if (imgLoc * 100 / mImgNum < (imgLoc + 1) * 100 / mImgNum) {
			_W csvTextW = L"Train Image : " + _TW(imgLoc + 1) + L" / " + _TW(mImgNum) + L" (" + _TW((imgLoc + 1) * 100 / mImgNum) + L"%)";
			SetDlgItemText(nowDlg, IDC_STAT_CSV, csvTextW.c_str());
			resolve_msg();
		}
	}
	fclose(csvInput);

	_W outTextW = L"Total time (load CSV) : " + _TW(clock() - procTime) + L"\n";
	outTextW += L"Load report : \n";
	for (LocInt labelLoc = 0; labelLoc < mLabelNum; labelLoc++) {
		outTextW += _TW(labelLoc) + L" : " + _TW(mImgLabelNum[labelLoc]) + L"\n";
	}
	SetDlgItemText(nowDlg, IDC_TXT_OUT, outTextW.c_str());

	return clock() - procTime;
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

clock_t kmeans_init_ker(const ImgT srcData[], LocIntC srcList[], LocIntC srcNum, KerT kerData[], LocIntC kerNum, LocIntC dimNum) {
	clock_t locTime = clock();
	LocInt srcLoc = 0;
	for (LocInt kerLoc = 0; kerLoc < kerNum; kerLoc++) {
		srcLoc = (srcNum / kerNum) * kerLoc + rand() % (srcNum / kerNum);
		kerData[kerLoc].label = srcData[srcList[srcLoc]].label; // under assumption that all images in one kernel shares same label
		for (LocInt dimLoc = 0; dimLoc < dimNum; dimLoc++) {
			kerData[kerLoc].data[dimLoc] = (KerDataT)(srcData[srcList[srcLoc]].data[dimLoc]);
		}
	}

	return clock() - locTime;
}

clock_t kmeans_eval_dist(const ImgT srcData[], LocIntC srcList[], LocIntC srcNum, KerT kerData[], LocIntC kerNum, DataDouble kerVar[], LocIntC dimNum) {
	clock_t locTime = clock();
	DataValK nowDist, minDist;
	LocInt minLoc;
	for (LocInt kerLoc = 0; kerLoc < kerNum; kerLoc++) {
		kerData[kerLoc].count = 0;
		kerVar[kerLoc] = 0;
	}
	for (LocInt srcLoc = 0; srcLoc < srcNum; srcLoc++) {
		minDist = dimNum * 0xFF * 0xFF + 1;
		minLoc = 0;
		for (LocInt kerLoc = 0; kerLoc < kerNum; kerLoc++) {
			nowDist = 0;
			for (LocInt dimLoc = 0; dimLoc < dimNum; dimLoc++) {
				nowDist += _EX2((DataValK)srcData[srcList[srcLoc]].data[dimLoc] - (DataValK)kerData[kerLoc].data[dimLoc]);
			}
			if (nowDist < minDist) {
				minDist = nowDist;
				minLoc = kerLoc;
			}
		}
		kerData[minLoc].list[kerData[minLoc].count] = srcList[srcLoc];
		kerData[minLoc].count++;
		kerVar[minLoc] += minDist;

		resolve_msg();
	}

	return clock() - locTime;
}

clock_t kmeans_update_ker(const ImgT srcData[], KerT kerData[], LocIntC kerNum, LocIntC dimNum) {
	clock_t locTime = clock();
	DataInt kerSum;
	for (LocInt kerLoc = 0; kerLoc < kerNum; kerLoc++) {
		for (LocInt dimLoc = 0; dimLoc < dimNum; dimLoc++) {
			kerSum = 0;
			for (LocInt kerListLoc = 0; kerListLoc < kerData[kerLoc].count; kerListLoc++) {
				kerSum += srcData[kerData[kerLoc].list[kerListLoc]].data[dimLoc];
			}
			if (kerData[kerLoc].count > 0) {
				kerData[kerLoc].data[dimLoc] = (KerDataT)_RK((DataDouble)kerSum / kerData[kerLoc].count);
			}
		}
	}

	return clock() - locTime;
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

clock_t kmeans_def(LocInt kerNum, LocInt minCount = 0) {
	clock_t procTime = clock();

	if (kerNum <= 1 || kerNum > mKerCap) {
		MessageBox(nowDlg, (_W(L"Please input valid kernel range : 2 ~ ") + _TW(mKerCap)).c_str(), DEF_TITLE, MB_OK | MB_ICONERROR);
		return -1;
	}
	for (LocInt labelLoc = 0; labelLoc < mLabelNum; labelLoc++) {
		mKerNum[labelLoc] = kerNum;
		for (LocInt kerLoc = 0; kerLoc < mKerNum[labelLoc]; kerLoc++) {
			mKerData[labelLoc][kerLoc].label = labelLoc;
			mKerData[labelLoc][kerLoc].count = mImgLabelNum[labelLoc] * (kerLoc + 1) / mKerNum[labelLoc] - 
				mImgLabelNum[labelLoc] * kerLoc / mKerNum[labelLoc];
			for (LocInt imgLoc = 0; imgLoc < mKerData[labelLoc][kerLoc].count; imgLoc++) {
				mKerData[labelLoc][kerLoc].list[imgLoc] = mImgLabelLoc[labelLoc][mImgLabelNum[labelLoc] * kerLoc / mKerNum[labelLoc] + imgLoc];
			}
		}
	}

	return clock() - procTime;
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

clock_t kmeans_set(LocInt kerNum, LocInt minCount = 0) {
	clock_t procTime = clock();
	
	_S kmeansOutFileS;
	kmeansOutFileS = _S(kmeansOutFileDefC) + "_" + _TS(kerNum) + ".txt";

	if (kmeansLoad > 0) {
		if (fopen_s(&kmeansInput, kmeansOutFileS.c_str(), "r")) {
			MessageBox(nowDlg, L"Failed to open kmeans out file for load", DEF_TITLE, MB_OK | MB_ICONERROR);
			return -1;
		}
		LocInt nowLabel, nowKer, nowKerCount;
		for (LocInt labelLoc = 0; labelLoc < mLabelNum; labelLoc++) {
			mKerNum[labelLoc] = kerNum;
		}
		while (1) {
			if (fscanf_s(kmeansInput, "%d", &nowLabel) == EOF) {
				break;
			}
			if (fscanf_s(kmeansInput, "%d", &nowKer) == EOF) {
				break;
			}
			/*if (mKerNum[nowLabel] < nowKer + 1) {
				mKerNum[nowLabel] = nowKer + 1;
			}*/
			if (fscanf_s(kmeansInput, "%d", &nowKerCount) == EOF) {
				break;
			}
			mKerData[nowLabel][nowKer].label = nowLabel;
			mKerData[nowLabel][nowKer].count = nowKerCount;
			for (LocInt kerListLoc = 0; kerListLoc < mKerData[nowLabel][nowKer].count; kerListLoc++) {
				if (fscanf_s(kmeansInput, "%d", &(mKerData[nowLabel][nowKer].list[kerListLoc])) == EOF) {
					break;
				}
			}
		}
		fclose(kmeansInput);
		for (LocInt labelLoc = 0; labelLoc < mLabelNum; labelLoc++) {
			kmeans_update_ker(mImgTotal, mKerData[labelLoc], mKerNum[labelLoc], mDim);
		}
#if 0
		nowLabel = rand() % mLabelNum;
		put_kerSet_mono(mKerData[nowLabel], mKerNum[nowLabel], dlgImgNumX, dlgImgNumY, mWidth, mHeight, dlgImgOut);
		
		GetClientRect(nowDlg, &rect);
		InvalidateRect(nowDlg, &rect, false);
#endif
		return clock() - procTime;
	}

	LocInt iterNum;
	if (kerNum <= 1 || kerNum > mKerCap) {
		MessageBox(nowDlg, (_W(L"Please input valid kernel range : 2 ~ ") + _TW(mKerCap)).c_str(), DEF_TITLE, MB_OK | MB_ICONERROR);
		return -1;
	}
	DataDouble kerDev[mKerCap], kerDevSum, kerStdNow, kerStdPrev;
	for (LabelT nowLabel = 0; nowLabel < mLabelNum && funcProc > 0;) {
		iterNum = 0;
		mKerNum[nowLabel] = kerNum;
		kerStdPrev = 0;
		kmeans_init_ker(mImgTotal, mImgLabelLoc[nowLabel], mImgLabelNum[nowLabel], mKerData[nowLabel], mKerNum[nowLabel], mDim);

		while (iterNum < kmeansIterLimit) {
			if (iterNum % 10 == 0) {
				put_kerSet_mono(mKerData[nowLabel], mKerNum[nowLabel], dlgImgNumX, dlgImgNumY, mWidth, mHeight, dlgImgOut);
				GetClientRect(nowDlg, &rect);
				InvalidateRect(nowDlg, &rect, false);
			}

			kmeans_eval_dist(mImgTotal, mImgLabelLoc[nowLabel], mImgLabelNum[nowLabel], mKerData[nowLabel], mKerNum[nowLabel], mKerVar[nowLabel], mDim);
			kmeans_update_ker(mImgTotal, mKerData[nowLabel], mKerNum[nowLabel], mDim);
			
			kerDevSum = 0;
			for (LocInt kerLoc = 0; kerLoc < mKerNum[nowLabel]; kerLoc++) {
				kerDev[kerLoc] = (mKerData[nowLabel][kerLoc].count > 0) ? (mKerVar[nowLabel][kerLoc] / mKerData[nowLabel][kerLoc].count) : 0;
				kerDevSum += kerDev[kerLoc];
			}
			kerStdNow = sqrt(kerDevSum / mKerNum[nowLabel]);

			_W outTextW = L"Total time (K-means) : " + _TW(clock() - procTime) + L"\n";
			outTextW += L"Ker : " + _TW(mKerNum[nowLabel]) + L", Label : " + _TW(nowLabel) + L", Iter : " + _TW(iterNum) + L"\n";
			outTextW += L"Mean Dev : " + _DW(kerStdNow) + L"\n";
			for (LocInt kerLoc = 0; kerLoc < mKerNum[nowLabel]; kerLoc++) {
				outTextW += _TW(kerLoc) + L" : " + _TW(mKerData[nowLabel][kerLoc].count) + L", " + _TW(int(kerDev[kerLoc])) + L"\n";
			}
			SetDlgItemText(nowDlg, IDC_TXT_OUT, outTextW.c_str());

			if (kerStdNow - kerStdPrev == 0) {
				FlagT kerSuc = 1;
				for (LocInt kerLoc = 0; kerLoc < mKerNum[nowLabel]; kerLoc++) {
					if (mKerData[nowLabel][kerLoc].count <= minCount) {
						kerSuc = -1;
						break;
					}
				}
				if (kerSuc > 0) {
					nowLabel++;
				}
				break;
			} else {
				kerStdPrev = kerStdNow;
				iterNum++;
			}
		}
	}
	if (kmeansSave > 0) {
		if (fopen_s(&kmeansOutput, kmeansOutFileS.c_str(), "w")) {
			MessageBox(nowDlg, L"Failed to open kmeans out file for save", DEF_TITLE, MB_OK | MB_ICONERROR);
			return -1;
		}
		for (LocInt labelLoc = 0; labelLoc < mLabelNum; labelLoc++) {
			for (LocInt kerLoc = 0; kerLoc < mKerNum[labelLoc]; kerLoc++) {
				fprintf(kmeansOutput, "%d %d %d", labelLoc, kerLoc, mKerData[labelLoc][kerLoc].count);
				for (LocInt kerListLoc = 0; kerListLoc < mKerData[labelLoc][kerLoc].count; kerListLoc++) {
					fprintf(kmeansOutput, " %d", mKerData[labelLoc][kerLoc].list[kerListLoc]);
				}
				fprintf(kmeansOutput, "\n");
			}
		}
		fclose(kmeansOutput);
	}

	return clock() - procTime;
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

FlagT mat_init(DataMat *des, LocIntC mNum, LocIntC nNum) {
	*des = new DataDouble*[mNum];
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		(*des)[mLoc] = new DataDouble[nNum];
	}
	if (*des != nullptr) {
		return 1;
	} else {
		return -1;
	}
}

FlagT mat_delete(DataMat des, LocIntC mNum) {
	clock_t locTime = clock();
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		delete[] des[mLoc];
	}
	delete[] des;

	return 1;
}

FlagT mat_zero(DataMat des, LocIntC mNum, LocIntC nNum) {
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			des[mLoc][nLoc] = 0;
		}
	}

	return 1;
}

FlagT mat_iden(DataMat des, LocIntC mNum, DataDoubleC iVal = 1) {
	for (LocInt rLoc = 0; rLoc < mNum; rLoc++) {
		for (LocInt cLoc = 0; cLoc < mNum; cLoc++) {
			des[rLoc][cLoc] = (rLoc == cLoc) * iVal;
		}
	}

	return 1;
}

FlagT mat_copy(DataMat des, DataMat src, LocIntC mNum, LocIntC nNum, DataDoubleC cVal = 1) {
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			des[mLoc][nLoc] = cVal * src[mLoc][nLoc];
		}
	}

	return 1;
}

FlagT mat_copyT(DataMat des, DataMat src, LocIntC mNum, LocIntC nNum, DataDoubleC cVal = 1) {
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			des[mLoc][nLoc] = cVal * src[nLoc][mLoc];
		}
	}

	return 1;
}

FlagT mat_scale(DataMat des, LocIntC mNum, LocIntC nNum, DataDoubleC sVal) {
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			des[mLoc][nLoc] *= sVal;
		}
	}

	return 1;
}

FlagT mat_add(DataMat des, DataMat src1, DataMat src2, LocIntC mNum, LocIntC nNum,
	DataDoubleC val1 = 1, DataDoubleC val2 = 1) {
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			des[mLoc][nLoc] = val1 * src1[mLoc][nLoc] + val2 * src2[mLoc][nLoc];
		}
	}

	return 1;
}

FlagT mat_addI(DataMat des, LocIntC mNum, DataDoubleC eps) {
	for (LocInt rLoc = 0; rLoc < mNum; rLoc++) {
		des[rLoc][rLoc] += eps;
	}

	return 1;
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

clock_t mat_mul(DataMat des, DataMat src1, DataMat src2, LocIntC mNum, LocIntC pNum, LocIntC nNum) { // (m,p) x (p,n) = (m,n)
	clock_t locTime = clock();
	DataDouble sum;
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			sum = 0;
			for (LocInt pLoc = 0; pLoc < pNum; pLoc++) {
				sum += src1[mLoc][pLoc] * src2[pLoc][nLoc];
			}
			des[mLoc][nLoc] = sum;
		}
		if (resolve_msg() > -1) {
#ifdef MAT_OUT
			_W outTextW = L"Total time : " + _TW(clock() - procTime) + L"\n";
			outTextW += L"Mul time : " + _TW(clock() - locTime) + L"\n";
			outTextW += L"Row : " + _TW(mLoc) + L" / " + _TW(mNum) + L"\n";
			SetDlgItemText(nowDlg, DEF_MAT_OUT, outTextW.c_str());
#endif
		}
	}

	return clock() - locTime;
}

clock_t mat_mulATB(DataMat des, DataMat srcT, DataMat src, LocIntC pNum, LocIntC mNum, LocIntC nNum) { // (p,m)T x (p,n) = (m,n)
	clock_t locTime = clock();
	DataDouble sum;
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			sum = 0;
			for (LocInt pLoc = 0; pLoc < pNum; pLoc++) {
				sum += srcT[pLoc][mLoc] * src[pLoc][nLoc];
			}
			des[mLoc][nLoc] = sum;
		}
		if (resolve_msg() > -1) {
#ifdef MAT_OUT
			_W outTextW = L"Total time : " + _TW(clock() - procTime) + L"\n";
			outTextW += L"Mul ATB time : " + _TW(clock() - locTime) + L"\n";
			outTextW += L"Row : " + _TW(mLoc) + L" / " + _TW(mNum) + L"\n";
			SetDlgItemText(nowDlg, DEF_MAT_OUT, outTextW.c_str());
#endif
		}

	}

	return clock() - locTime;
}

clock_t mat_mulABT(DataMat des, DataMat src, DataMat srcT, LocIntC mNum, LocIntC nNum, LocIntC pNum) { // (m,p) x (n,p)T = (m,n)
	clock_t locTime = clock();
	DataDouble sum;
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			sum = 0;
			for (LocInt pLoc = 0; pLoc < pNum; pLoc++) {
				sum += src[mLoc][pLoc] * srcT[nLoc][pLoc];
			}
			des[mLoc][nLoc] = sum;
		}
		if (resolve_msg() > -1) {
#ifdef MAT_OUT
			_W outTextW = L"Total time : " + _TW(clock() - procTime) + L"\n";
			outTextW += L"Mul ABT time : " + _TW(clock() - locTime) + L"\n";
			outTextW += L"Row : " + _TW(mLoc) + L" / " + _TW(mNum) + L"\n";
			SetDlgItemText(nowDlg, DEF_MAT_OUT, outTextW.c_str());
#endif
		}
	}

	return clock() - locTime;
}

clock_t mat_mulATA(DataMat des, DataMat src, LocIntC mNum, LocIntC nNum) { // (m,n)T x (m,n) = (n,n)
	clock_t locTime = clock();
	LocInt locCount = 0;
	DataDouble sum;
	for (LocInt rLoc = 0; rLoc < nNum; rLoc++) {
		for (LocInt cLoc = rLoc; cLoc < nNum; cLoc++) {
			sum = 0;
			for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
				sum += src[mLoc][rLoc] * src[mLoc][cLoc];
			}
			des[rLoc][cLoc] = sum;
			des[cLoc][rLoc] = sum;

			locCount++;
			if (resolve_msg() > -1) {
#ifdef MAT_OUT
				_W outTextW = L"Total time : " + _TW(clock() - procTime) + L"\n";
				outTextW += L"Mul ATA time : " + _TW(clock() - locTime) + L"\n";
				outTextW += L"Count : " + _TW(locCount) + L" / " + _TW(nNum * (nNum + 1) / 2) + L"\n";
				SetDlgItemText(nowDlg, DEF_MAT_OUT, outTextW.c_str());
#endif
			}
		}
	}

	return clock() - locTime;
}

clock_t mat_mulADAT(DataMat des, DataMat srcAT, DataVec srcD, LocIntC mNum) { // A(m,m) x D(m,m) x AT(m,m) = (m,m)
	clock_t locTime = clock();
	DataDouble sum;
	for (LocInt rLoc = 0; rLoc < mNum; rLoc++) {
		for (LocInt cLoc = rLoc; cLoc < mNum; cLoc++) {
			sum = 0;
			for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
				sum += srcAT[mLoc][rLoc] * srcAT[mLoc][cLoc] * srcD[mLoc];
			}
			des[rLoc][cLoc] = sum;
			des[cLoc][rLoc] = sum;
			resolve_msg();
		}
	}

	return clock() - locTime;
}

clock_t mat_cov(DataMat des, DataMat src, LocIntC mNum, LocIntC nNum) { // C = HT x J x H
	clock_t locTime = clock();
	DataDouble sum, mean;
	for (LocInt cLoc = 0; cLoc < nNum; cLoc++) { // column loc of H
		mean = 0;
		for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
			mean += src[mLoc][cLoc];
		}
		mean /= mNum;
		for (LocInt rLoc = cLoc; rLoc < nNum; rLoc++) { // row loc of HT
			sum = 0;
			for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
				sum += src[mLoc][rLoc] * (src[mLoc][cLoc] - mean);
			}
			des[rLoc][cLoc] = sum / (mNum - 1);
			des[cLoc][rLoc] = des[rLoc][cLoc];
		}
		resolve_msg();
	}

	return clock() - locTime;
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

FlagT row_init(DataVec *des, LocIntC nNum) {
	*des = new DataDouble[nNum];
	if (*des == nullptr) {
		return -1;
	} else {
		return 1;
	}
}

FlagT row_delete(DataVec des) {
	delete[] des;

	return 1;
}

FlagT row_init_zero(DataVec des, LocIntC nNum) {
	for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
		des[nLoc] = 0;
	}

	return 1;
}

FlagT row_init_one(DataVec des, LocIntC nNum, DataDoubleC sVal = 1) {
	for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
		des[nLoc] = sVal;
	}

	return 1;
}

FlagT row_init_unit(DataVec des, LocIntC nNum, LocIntC nPos) {
	for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
		des[nLoc] = (nLoc == nPos);
	}

	return 1;
}

FlagT row_init_rnd(DataVec des, LocIntC nNum, LocIntC mod = 0x8000) {
	for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
		des[nLoc] = -mod / 2 + rand() % mod;
	}

	return 1;
}

FlagT row_copy(DataVec des, DataVec src, LocIntC nNum, DataDoubleC sVal = 1) {
	for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
		des[nLoc] = src[nLoc] * sVal;
	}

	return 1;
}

FlagT row_add(DataVec des, DataVec src1, DataVec src2, LocIntC nNum, DataDoubleC sVal1 = 1, DataDoubleC sVal2 = 1) {
	for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
		des[nLoc] = src1[nLoc] * sVal1 + src2[nLoc] * sVal2;
	}

	return 1;
}

DataDouble row_norm(DataVec src, LocIntC nNum) {
	DataDouble norm = 0;
	for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
		norm += _EX2(src[nLoc]);
	}

	return sqrt(norm);
}

DataDouble row_norm2(DataVec src, LocIntC nNum) {
	DataDouble norm = 0;
	for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
		norm += _EX2(src[nLoc]);
	}

	return norm;
}

FlagT row_std_norm(DataVec des, LocIntC nNum) {
	DataDouble norm = row_norm(des, nNum);
	if (norm > 0) {
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			des[nLoc] /= norm;
		}
	}

	return 1;
}

FlagT row_swap(DataVec *des1, DataVec *des2) {
	DataVec tRow;
	tRow = *des1;
	*des1 = *des2;
	*des2 = tRow;

	return 1;
}

clock_t mat_shuffle(DataMat des, LocIntC sPos, LocIntC ePos, LocIntC iterNum) {
	clock_t locTime = clock();
	if (ePos <= sPos || ePos - sPos >= RAND_MAX) {
#ifdef MAT_OUT
		MessageBox(nowDlg, L"Invalid shuffle range", DEF_TITLE, MB_OK | MB_ICONERROR);
#endif
		return -1;
	}
#if 1
	LocInt *rndPos = new LocInt[ePos - sPos], nowLoc, tmpPos;
	DataVec *srcRow = new DataVec[ePos - sPos];
	for (LocInt pLoc = 0; pLoc < ePos - sPos; pLoc++) {
		rndPos[pLoc] = pLoc;
		srcRow[pLoc] = des[sPos + pLoc];
	}
	for (LocInt pLoc = 0; pLoc < ePos - sPos - 1; pLoc++) {
		nowLoc = pLoc + rand() % (ePos - sPos - pLoc);
		tmpPos = rndPos[pLoc];
		rndPos[pLoc] = rndPos[nowLoc];
		rndPos[nowLoc] = tmpPos;
	}
	for (LocInt pLoc = 0; pLoc < ePos - sPos; pLoc++) {
		des[sPos + pLoc] = srcRow[rndPos[pLoc]];
	}
	delete[] srcRow;
	delete[] rndPos;
#else
	LocInt rLoc1, rLoc2;
	for (LocInt iterLoc = 0; iterLoc < iterNum; iterLoc++) {
		rLoc1 = sPos + rand() % (ePos - sPos);
		rLoc2 = sPos + rand() % (ePos - sPos);
		row_swap(&(des[rLoc1]), &(des[rLoc2]));
	}
#endif

	return clock() - locTime;
}

FlagT row_scale(DataVec des, DataDoubleC sVal, LocIntC nNum) {
	for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
		des[nLoc] *= sVal;
	}

	return 1;
}

FlagT row_scale_inv(DataVec des, DataDoubleC sVal, LocIntC nNum) {
	for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
		des[nLoc] /= sVal;
	}

	return 1;
}

FlagT row_addto(DataVec des, DataVec src, DataDoubleC sVal, LocIntC nNum) {
	for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
		des[nLoc] += src[nLoc] * sVal;
	}

	return 1;
}

clock_t mat_inv(DataMat des, DataMat src, LocIntC mNum, LocIntC prec = 23) {
	clock_t locTime = clock();
	DataDouble eps = 1.0;
	for (LocInt pLoc = 0; pLoc < prec; pLoc++) {
		eps /= 2;
	}
	for (LocInt rLoc = 0; rLoc < mNum; rLoc++) {
		for (LocInt cLoc = 0; cLoc < mNum; cLoc++) {
			des[rLoc][cLoc] = (rLoc == cLoc);
		}
	}
	LocInt fLoc = 0;
	DataDouble nowScale;
	for (LocInt cLoc = 0; cLoc < mNum && fLoc < mNum; cLoc++) {
		for (LocInt rLoc = fLoc; rLoc < mNum; rLoc++) {
			if (_NZ(src[rLoc][cLoc], eps)) {
				row_swap(&(src[rLoc]), &(src[fLoc]));
				row_swap(&(des[rLoc]), &(des[fLoc]));

				nowScale = src[fLoc][cLoc];
				row_scale_inv(src[fLoc], nowScale, mNum);
				row_scale_inv(des[fLoc], nowScale, mNum);

				for (LocInt sLoc = 0; sLoc < mNum; sLoc++) {
					if (sLoc != fLoc && _NZ(src[rLoc][cLoc], eps)) {
						nowScale = -src[sLoc][cLoc];
						row_addto(src[sLoc], src[fLoc], nowScale, mNum);
						row_addto(des[sLoc], des[fLoc], nowScale, mNum);
					}
				}
				fLoc++;
				if (resolve_msg() > -1) {
#ifdef MAT_OUT
					_W outTextW = L"Total time : " + _TW(clock() - procTime) + L"\n";
					outTextW += L"Inv time : " + _TW(clock() - locTime) + L"\n";
					outTextW += L"Rank : " + _TW(fLoc) + L"\n";
					SetDlgItemText(nowDlg, DEF_MAT_OUT, outTextW.c_str());
#endif
				}

				break;
			}
		}
	}

	return 0;
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

DataDouble row_product(DataVec src1, DataVec src2, LocIntC nNum) { // inner product
	DataDouble sum = 0;
	for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
		sum += src1[nLoc] * src2[nLoc];
	}

	return sum;
}

DataDouble row_proj(DataVec des, DataVec src, LocIntC nNum) { // remove src part from des
	DataDouble inner = row_product(des, src, nNum);
	for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
		des[nLoc] -= src[nLoc] * inner;
	}

	return inner;
}

clock_t mat_mulV(DataVec dVec, DataMat sMat, DataVec sVec, LocIntC mNum, LocIntC nNum) {
	clock_t locTime = clock();
	DataDouble sum;
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		sum = 0;
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			sum += sMat[mLoc][nLoc] * sVec[nLoc];
		}
		dVec[mLoc] = sum;
	}

	return clock() - locTime;
}

DataDouble mat_eigen_test(DataMat A, DataMat UT, DataVec D, LocIntC mNum, FlagT outMode = 0) {
	DataVec AV_DV, AV_DV_val;
	DataDouble AV_DV_sum = 0;
	row_init(&AV_DV, mNum);
	row_init(&AV_DV_val, mNum);
	for (LocInt vLoc = 0; vLoc < mNum; vLoc++) {
		mat_mulV(AV_DV, A, UT[vLoc], mNum, mNum);
		row_add(AV_DV, AV_DV, UT[vLoc], mNum, 1, -D[vLoc]);
		AV_DV_val[vLoc] = row_norm2(AV_DV, mNum);
		if (AV_DV_val[vLoc] > 0) {
			AV_DV_val[vLoc] = -2 / log(min(AV_DV_val[vLoc], 0.5));
		}
		AV_DV_sum += AV_DV_val[vLoc];
	}
	if (outMode > 1) {
		FILE *eigenOut = nullptr;
		if (fopen_s(&eigenOut, "eigen_test.txt", "w")) {
			_MSG_W(L"Error to open eigen_test.txt for writing");
		} else {
			fprintf(eigenOut, "AV_DV_sum\t%.17f\n\n", AV_DV_sum);
			for (LocInt vLoc = 0; vLoc < mNum; vLoc++) {
				fprintf(eigenOut, "-1 / log(AV_DV_val[%d])\t%.17f\n", vLoc, AV_DV_val[vLoc]);
			}
			fclose(eigenOut);
		}
	}
	row_delete(AV_DV);
	row_delete(AV_DV_val);

	return AV_DV_sum;
}

LocInt mat_eigen_eval(DataMat A, DataVec E, DataDouble *l, DataVec V, DataVec E_list[], LocIntC mNum, LocIntC eSize, FlagT outMode = 0) {
	LocInt suc = -1;
	LocInt iterLoc = 0;
	DataDouble epsValue = exp(-eValuePrec), epsDiff = exp(-eDiffPrec), preL = -1, newL, dvInner, dvNormPre, dvNormNew;
	DataVec dvPre, dvNew;
	row_init(&dvNew, mNum);
	row_init(&dvPre, mNum);
	row_init_rnd(dvPre, mNum);
	dvNormPre = row_norm(dvPre, mNum);
	if (outMode > 0) {
		FILE *eigenOut;
		if (fopen_s(&eigenOut, "eigen_error.txt", "a")) {
			_MSG_W(L"Error to open eigen_error.txt for writing");
			return -1;
		}
		fprintf(eigenOut, "Time : %d, eSize : %d\n", clock() - procTime, eSize);
		while (iterLoc < eIterLimit) {
			mat_mulV(E, A, V, mNum, mNum); // E = A * v
			newL = row_norm(E, mNum); // l = norm(E)
			fprintf(eigenOut, "%d\t%.17f", iterLoc, newL);
			for (LocInt eLoc = 0; eLoc < eSize; eLoc++) { // projection elimination
				row_proj(E, E_list[eLoc], mNum);
			}
			row_std_norm(E, mNum); // E = E / norm(V)
			row_add(dvNew, E, V, mNum, 1, -1);
			dvNormNew = row_norm(dvNew, mNum);
			dvInner = abs(row_product(dvNew, dvPre, mNum) / dvNormPre / dvNormNew - 1);
			if (dvInner < epsDiff) {
				row_add(V, E, dvNew, mNum, 1, eMoveRate); // V = E + (E - V);
			} else {
				row_copy(V, E, mNum); // V = E;
			}
			for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
				fprintf(eigenOut, "\t%lf", E[mLoc]);
			}
			fprintf(eigenOut, "\n");
			if (_Z(newL, epsValue)) { // l < eps, eigenvalue is too small
				fprintf(eigenOut, "fail : l < eps\n");
				break;
			} else if (_Z(newL - preL, epsValue)) { // exit condition
				fprintf(eigenOut, "suc\n");
				suc = 1;
				break;
			} else if (iterLoc == eIterLimit - 1) { // full count
				fprintf(eigenOut, "fail : overcount");
				break;
			} else { // update
				preL = newL;
				dvNormPre = dvNormNew;
				row_copy(dvPre, dvNew, mNum);
			}
			iterLoc++;
			if (iterLoc % 1000 == 1) {
				resolve_msg();
			}
		}
		fprintf(eigenOut, "\n");
		fclose(eigenOut);
	} else {
		while (iterLoc < eIterLimit) {
			mat_mulV(E, A, V, mNum, mNum); // E = A * v
			newL = row_norm(E, mNum); // l = norm(E)
			for (LocInt eLoc = 0; eLoc < eSize; eLoc++) { // projection elimination
				row_proj(E, E_list[eLoc], mNum);
			}
			row_std_norm(E, mNum); // E = E / norm(V)
			row_add(dvNew, E, V, mNum, 1, -1);
			dvNormNew = row_norm(dvNew, mNum);
			dvInner = abs(row_product(dvNew, dvPre, mNum) / dvNormPre / dvNormNew - 1);
			if (dvInner < epsDiff) {
				row_add(V, E, dvNew, mNum, 1, eMoveRate); // V = E + (E - V);
			} else {
				row_copy(V, E, mNum); // V = E;
			}
			if (_Z(newL, epsValue)) { // l < eps, eigenvalue is too small
				break;
			} else if (_Z(newL - preL, epsValue)) { // exit condition
				suc = 1;
				break;
			} else { // update
				preL = newL;
				dvNormPre = dvNormNew;
				row_copy(dvPre, dvNew, mNum);
			}
			iterLoc++;
		}
	}
	*l = newL;
	row_delete(dvNew);
	row_delete(dvPre);
#if 1
	if (suc < 0) {
		FILE *eigenOut;
		if (fopen_s(&eigenOut, "eigen_error.txt", "a")) {
			_MSG_W(L"Error to open eigen_error.txt for writing");
		}
		fprintf(eigenOut, "Time : %d\n", clock() - procTime);
		fprintf(eigenOut, "Iter : %d\n", iterLoc);
		fprintf(eigenOut, "eValue : %.17f\n", newL);
		fprintf(eigenOut, "eVec : ");
		for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
			fprintf(eigenOut, "%.17f\t", E[mLoc]);
		}
		fprintf(eigenOut, "\n");
		fclose(eigenOut);
	}
#endif
	return suc * iterLoc;
}

DataDouble mat_log(DataMat log_A, DataMat A, DataMat UT, DataVec D, DataVec log_D, LocIntC mNum, FlagT outMode = 0) { // eigen decomposition for symmetric square matrix
	clock_t locTime = clock();
	LocInt vLoc = 0, fCount = 0, fLimit = 5, fSum = 0, *iterNum;
	DataVec V = nullptr;
	row_init(&V, mNum);
	iterNum = new LocInt[mNum];
	do {
		if (fCount > 0) {
			row_init_rnd(V, mNum);
		} else {
			row_init_rnd(V, mNum);
		}
		iterNum[vLoc] = mat_eigen_eval(A, UT[vLoc], &(D[vLoc]), V, UT, mNum, vLoc, fCount);
		resolve_msg();
		if (iterNum[vLoc] > 0) { // done
			fCount = 0;
			vLoc++;
		} else { // retry
			fCount++;
			fSum++;
		}
	} while (vLoc < mNum && fCount < fLimit);
	if (outMode > 0) {
		FILE *logOut = nullptr;
		if (fopen_s(&logOut, "log_result.txt", "w")) {
			_MSG_W(L"Error to open log_result.txt for writing");
		} else {
			for (LocInt eLoc = 0; eLoc < vLoc; eLoc++) {
				fprintf(logOut, "%d\t%d\t%.17f", eLoc, iterNum[eLoc], D[eLoc]);
				for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
					fprintf(logOut, "\t%.10f", UT[eLoc][mLoc]);
				}
				fprintf(logOut, "\n");
			}
			fclose(logOut);
		}
	}
	row_delete(V);
	delete[] iterNum;
	if (vLoc == mNum) {
		for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
			log_D[mLoc] = log(D[mLoc]);
		}
		mat_mulADAT(log_A, UT, log_D, mNum); // A_log = U * D_log * UT
		return mat_eigen_test(A, UT, D, mNum);
	} else { // fail
		return -1;
	}
}

#if 1

clock_t mat_eigen_output(DataMat UT, DataVec D, LocIntC mNum, const char *fNameC) {
	clock_t locTime = clock();
	FILE *eigOut;
	WCHAR fNameW[MAX_LOADSTRING];
	_CW(fNameC, fNameW, MAX_LOADSTRING);
	if (fopen_s(&eigOut, fNameC, "w")) {
#ifdef MAT_OUT
		MessageBox(nowDlg, (L"Failed to open " + _W(fNameW) + L" for output").c_str(), DEF_TITLE, MB_OK | MB_ICONERROR);
#endif
		return -1;
	}
	for (LocInt rLoc = 0; rLoc < mNum; rLoc++) {
		fprintf(eigOut, "Vec[%d] = %.17f\n", rLoc, D[rLoc]);
		for (LocInt cLoc = 0; cLoc < mNum; cLoc++) {
			fprintf(eigOut, "%lf\t", UT[rLoc][cLoc]);
		}
		fprintf(eigOut, "\n\n");
	}
	resolve_msg();
	fclose(eigOut);

	return clock() - locTime;
}

#endif

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

DataDouble mat_mean(DataMat src, LocIntC mNum, LocIntC nNum) {
	DataDouble mean = 0;
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			mean += src[mLoc][nLoc];
		}
	}

	return mean / mNum / nNum;
}

DataDouble mat_var(DataMat src, LocIntC mNum, LocIntC nNum) {
	DataDouble mean = mat_mean(src, mNum, nNum), var = 0;
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			var += _EX2(src[mLoc][nLoc] - mean);
		}
	}

	return var / mNum / nNum;
}

DataDouble mat_norm(DataMat src, LocIntC mNum, LocIntC nNum) {
	DataDouble norm = 0;
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			norm += _EX2(src[mLoc][nLoc]);
		}
	}

	return sqrt(norm);
}

DataDouble mat_norm_sym(DataMat src, LocIntC mNum) {
	DataDouble norm = 0;
	for (LocInt rLoc = 0; rLoc < mNum; rLoc++) {
		norm += _EX2(src[rLoc][rLoc]);
		for (LocInt cLoc = rLoc + 1; cLoc < mNum; cLoc++) {
			norm += 2 * _EX2(src[rLoc][cLoc]);
		}
	}

	return sqrt(norm);
}

DataDouble mat_diff_var(DataMat src1, DataMat src2, LocIntC mNum, LocIntC nNum) {
	DataDouble norm = 0;
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			norm += _EX2(src1[mLoc][nLoc] - src2[mLoc][nLoc]);
		}
	}

	return norm;
}

DataDouble mat_diff_varND(DataMat src1, DataMat src2, LocIntC mNum, LocIntC nNum) {
	DataDouble norm = 0;
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			norm += (mLoc != nLoc) * _EX2(src1[mLoc][nLoc] - src2[mLoc][nLoc]);
		}
	}

	return norm;
}

DataDouble mat_diff_varD(DataMat src1, DataMat src2, LocIntC mNum, LocIntC nNum) {
	DataDouble norm = 0;
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			norm += (mLoc == nLoc) * _EX2(src1[mLoc][nLoc] - src2[mLoc][nLoc]);
		}
	}

	return norm;
}

DataDouble mat_diff_norm(DataMat src1, DataMat src2, LocIntC mNum, LocIntC nNum) {
	DataDouble norm = 0;
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			norm += _EX2(src1[mLoc][nLoc] - src2[mLoc][nLoc]);
		}
	}

	return sqrt(norm);
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

FlagT mat_std_mean(DataMat des, LocIntC mNum, LocIntC nNum) {
	DataDouble mean = mat_mean(des, mNum, nNum);
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			des[mLoc][nLoc] -= mean;
		}
	}

	return 1;
}

FlagT mat_std_var(DataMat des, LocIntC mNum, LocIntC nNum) {
	DataDouble mean = mat_mean(des, mNum, nNum), sdev = sqrt(mat_var(des, mNum, nNum));
	if (sdev > 0) {
		for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
			for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
				des[mLoc][nLoc] = (des[mLoc][nLoc] - mean) / sdev + mean;
			}
		}
	}

	return 1;
}

FlagT mat_std_total(DataMat des, LocIntC mNum, LocIntC nNum) {
	DataDouble mean = 0, sdev = 0;
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			mean += des[mLoc][nLoc];
		}
	}
	mean /= mNum * nNum;
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			sdev += _EX2(des[mLoc][nLoc] - mean);
		}
	}
	sdev = sqrt(sdev / mNum / nNum);
	if (sdev > 0) {
		for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
			for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
				des[mLoc][nLoc] = (des[mLoc][nLoc] - mean) / sdev;
			}
		}
	}

	return 1;
}

FlagT mat_std_norm(DataMat des, LocIntC mNum, LocIntC nNum) {
	DataDouble norm = sqrt(mat_norm(des, mNum, nNum));
	if (norm > 0) {
		for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
			for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
				des[mLoc][nLoc] /= norm;
			}
		}
	}

	return 1;
}

FlagT mat_std_diag(DataMat des, LocIntC mNum) {
	DataDouble diag = 0;
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		diag += des[mLoc][mLoc];
	}
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		des[mLoc][mLoc] += 0 - diag / mNum;
	}

	return 1;
}

FlagT mat_std(DataMat des, LocIntC mNum, LocIntC nNum, const FlagT mode) {
	switch (mode) {
	case NORM_DIAG:
		mat_std_diag(des, mNum);
		break;
	case NORM_UNIT:
		mat_std_norm(des, mNum, nNum);
		break;
	case NORM_STD:
		mat_std_total(des, mNum, nNum);
		break;
	case NORM_NONE:
	default:
		break;
	}

	return 1;
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

clock_t mat_save(DataMat src, LocIntC mNum, LocIntC nNum, const char *fNameC) {
	clock_t locTime = clock();
	FILE *matSave;
	WCHAR fNameW[MAX_LOADSTRING];
	_CW(fNameC, fNameW, MAX_LOADSTRING);
	if (fopen_s(&matSave, fNameC, "wb")) {
#ifdef MAT_OUT
		MessageBox(nowDlg, (L"Failed to open " + _W(fNameW) + L" for save").c_str(), DEF_TITLE, MB_OK | MB_ICONERROR);
#endif
		return -1;
	}
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		fwrite(src[mLoc], sizeof(DataDouble), nNum, matSave);
	}
	fclose(matSave);

	return clock() - locTime;
}

clock_t mat_load(DataMat src, LocIntC mNum, LocIntC nNum, const char *fNameC) {
	clock_t locTime = clock();
	FILE *matLoad;
	WCHAR fNameW[MAX_LOADSTRING];
	_CW(fNameC, fNameW, MAX_LOADSTRING);
	if (fopen_s(&matLoad, fNameC, "rb")) {
#ifdef MAT_OUT
		MessageBox(nowDlg, (L"Failed to open " + _W(fNameW) + L" for load").c_str(), DEF_TITLE, MB_OK | MB_ICONERROR);
#endif
		return -1;
	}
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		fread_s(src[mLoc], nNum * sizeof(DataDouble), sizeof(DataDouble), nNum, matLoad);
	}
	fclose(matLoad);

	return clock() - locTime;
}

clock_t mat_output(DataMat src, LocIntC mNum, LocIntC nNum, const char *fNameC, FlagT scrOut = 1) {
	clock_t locTime = clock();
	FILE *matOut;
	WCHAR fNameW[MAX_LOADSTRING];
	_CW(fNameC, fNameW, MAX_LOADSTRING);
	if (fopen_s(&matOut, fNameC, "w")) {
#ifdef MAT_OUT
		MessageBox(nowDlg, (L"Failed to open " + _W(fNameW) + L" for output").c_str(), DEF_TITLE, MB_OK | MB_ICONERROR);
#endif
		return -1;
	}
	for (LocInt mLoc = 0; mLoc < mNum; mLoc++) {
		for (LocInt nLoc = 0; nLoc < nNum; nLoc++) {
			fprintf(matOut, "%.17f\t", src[mLoc][nLoc]);
		}
		fprintf(matOut, "\n");
		if (resolve_msg() > -1 && scrOut > 0) {
#ifdef MAT_OUT
			_W outTextW = L"Total time : " + _TW(clock() - procTime) + L"\n";
			outTextW += L"Out time for " + _W(fNameW) + L" : " + _TW(clock() - locTime) + L"\n";
			outTextW += L"Rows : " + _TW(mLoc) + L" / " + _TW(mNum) + L"\n";
			SetDlgItemText(nowDlg, DEF_MAT_OUT, outTextW.c_str());
#endif
		}
	}
	fclose(matOut);

	return clock() - locTime;
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

FlagT conf_init(ConT *con, LocInt mNum = mLabelNum) {
	con->tcount = 0;
	con->scount = 0;
	for (LocInt rLoc = 0; rLoc < mNum; rLoc++) {
		con->rcount[rLoc] = 0;
	}
	for (LocInt cLoc = 0; cLoc < mNum; cLoc++) {
		con->ccount[cLoc] = 0;
	}
	for (LocInt rLoc = 0; rLoc < mNum; rLoc++) {
		for (LocInt cLoc = 0; cLoc < mNum; cLoc++) {
			con->count[rLoc][cLoc] = 0;
		}
	}

	return 1;
}

FlagT conf_push(ConT *con, LocInt testLoc, LocInt getLoc, LocInt num = 1) {
	con->count[testLoc][getLoc] += num;
	con->rcount[testLoc] += num;
	con->ccount[getLoc] += num;
	con->tcount += num;
	con->scount += num * (testLoc == getLoc);

	return 1;
}

_W conf_output(ConT *con, LocInt mNum = mLabelNum) {
	_W outTextW = L"Total : " + _TW(con->scount) + L" / " + _TW(con->tcount) + L"\t (" + _DW(con->scount * 100.0 / con->tcount) + L" %)\n";
	for (LocInt rLoc = 0; rLoc < mNum; rLoc++) {
		outTextW += L"Label " + _TW(rLoc) + L" : " + _TW(con->count[rLoc][rLoc]) + L" / " + _TW(con->rcount[rLoc]) +
			L"\t (" + _DW(con->count[rLoc][rLoc] * 100.0 / con->rcount[rLoc]) + L" %)\n";
	}
	
	return outTextW;
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

clock_t DCRL_init(LocInt trainNum, LocInt testNum, LocInt shuffle = 0) {
	clock_t locTime = clock();
	SetDlgItemText(nowDlg, IDC_STAT_DCRL, L"Stage 1 : Construct set data");
	WCHAR trainNumW[MAX_LOADSTRING], testNumW[MAX_LOADSTRING];
	GetDlgItemText(nowDlg, IDC_INPUT_KMNUM1, trainNumW, MAX_LOADSTRING);
	GetDlgItemText(nowDlg, IDC_INPUT_KMNUM2, testNumW, MAX_LOADSTRING);
	
	setTrainNum = trainNum * mLabelNum;
	setTestNum = testNum * mLabelNum;
	setTotalNum = setTrainNum + setTestNum;
	featDim = setTrainNum;
	maxSetCount = 0;

	setTotal = new SetT[setTotalNum];
	setTrainPtr = new pSetT[setTotalNum];
	setTestPtr = new pSetT[setTotalNum];
	kerTotalPtr = new pKerT[setTotalNum];
	kerTrainPtr = new pKerT[setTrainNum];

	for (LocInt labelLoc = 0, setLoc = 0; labelLoc < mLabelNum; labelLoc++) {
		for (LocInt kerLoc = 0; kerLoc < mKerNum[labelLoc]; kerLoc++, setLoc++) {
			kerTotalPtr[setLoc] = &(mKerData[labelLoc][kerLoc]);
			setTotal[setLoc].label = labelLoc;
			setTotal[setLoc].count = kerTotalPtr[setLoc]->count;
			if (setTotal[setLoc].count > maxSetCount) {
				maxSetCount = setTotal[setLoc].count;
			}
			if (setTotal[setLoc].count == 0) {
				MessageBox(nowDlg, L"Set with no image exists", DEF_TITLE, MB_OK | MB_ICONEXCLAMATION);
				return -1;
			}
			setTotal[setLoc].list = new LocInt[setTotal[setLoc].count];
			for (LocInt imgLoc = 0; imgLoc < setTotal[setLoc].count; imgLoc++) {
				setTotal[setLoc].list[imgLoc] = kerTotalPtr[setLoc]->list[imgLoc];
			}
		}
	}
	resolve_msg();

	if (shuffle > 0) { // shuffle sets

	}

	// train and test mapping
	mImgTrainNum = 0;
	mImgTestNum = 0;
	for (LocInt setLoc = 0, trainLoc = 0, testLoc = 0; setLoc < setTotalNum;) {
		for (LocInt nowLoc = 0; nowLoc < setTrainNum / mLabelNum; nowLoc++, trainLoc++, setLoc++) {
			setTrainPtr[trainLoc] = &(setTotal[setLoc]);
			kerTrainPtr[trainLoc] = kerTotalPtr[setLoc];
			mImgTrainNum += setTrainPtr[trainLoc]->count;
		}
		for (LocInt nowLoc = 0; nowLoc < setTestNum / mLabelNum; nowLoc++, testLoc++, setLoc++) {
			setTestPtr[testLoc] = &(setTotal[setLoc]);
			mImgTestNum += setTestPtr[testLoc]->count;
		}
	}
	resolve_msg();
	
	// mat initialize
	mat_init(&mat_A, mImgTrainNum, mDim);
	mat_init(&mat_A_test, mImgTestNum, mDim);

	mat_init(&mat_B, mImgTrainNum, featDim);
	//mat_init(&mat_B_test, mImgTestNum, featDim);

	mat_init(&mat_W, mDim, featDim);
	mat_init(&mat_AW, mImgTrainNum, featDim);
	mat_init(&mat_AW_test, mImgTestNum, featDim);

	mat_init(&mat_A_pinv, mDim, mImgTrainNum);

	mat_init(&mat_AT_A, mDim, mDim);
	mat_init(&mat_AT_A_inv, mDim, mDim);

	mat_H = new DataMat[setTrainNum];
	mat_C = new DataMat[setTrainNum];
	mat_log_C = new DataMat[setTrainNum];
	mat_D = new DataVec[setTrainNum];
	mat_log_D = new DataVec[setTrainNum];
	mat_UT = new DataMat[setTrainNum];
	for (LocInt setLoc = 0; setLoc < setTrainNum; setLoc++) {
		mat_init(&(mat_H[setLoc]), setTrainPtr[setLoc]->count, featDim); // mat_H : N_i x M
		mat_init(&(mat_C[setLoc]), featDim, featDim); // mat_C : M x M
		mat_init(&(mat_log_C[setLoc]), featDim, featDim); // mat_log_C : M x M
		row_init(&(mat_D[setLoc]), featDim);
		row_init(&(mat_log_D[setLoc]), featDim);
		mat_init(&(mat_UT[setLoc]), featDim, featDim);
	}
	mat_H_test = new DataMat[setTestNum];
	mat_C_test = new DataMat[setTestNum];
	mat_log_C_test = new DataMat[setTestNum];
	mat_D_test = new DataVec[setTestNum];
	mat_log_D_test = new DataVec[setTestNum];
	mat_UT_test = new DataMat[setTestNum];
	for (LocInt setLoc = 0; setLoc < setTestNum; setLoc++) {
		mat_init(&(mat_H_test[setLoc]), setTestPtr[setLoc]->count, featDim);
		mat_init(&(mat_C_test[setLoc]), featDim, featDim);
		mat_init(&(mat_log_C_test[setLoc]), featDim, featDim);
		row_init(&(mat_D_test[setLoc]), featDim);
		row_init(&(mat_log_D_test[setLoc]), featDim);
		mat_init(&(mat_UT_test[setLoc]), featDim, featDim);
	}
	mat_init(&GE_LEM, setTrainNum, setTrainNum);
	mat_init(&GE_A, setTrainNum, setTrainNum);
	mat_init(&GE_J, setTrainNum, setTrainNum);
	resolve_msg();

	// mat_A setting, list is redefined to row number
	for (LocInt setLoc = 0, rowLoc = 0; setLoc < setTrainNum; setLoc++) {
		for (LocInt imgLoc = 0; imgLoc < setTrainPtr[setLoc]->count; imgLoc++, rowLoc++) {
			for (LocInt dimLoc = 0; dimLoc < mDim; dimLoc++) {
				mat_A[rowLoc][dimLoc] = mImgTotal[setTrainPtr[setLoc]->list[imgLoc]].data[dimLoc] / 256.0;
			}
			setTrainPtr[setLoc]->list[imgLoc] = rowLoc;
		}
	}
	// mat_A_test setting, list is redefined to row number
	for (LocInt setLoc = 0, rowLoc = 0; setLoc < setTestNum; setLoc++) {
		for (LocInt imgLoc = 0; imgLoc < setTestPtr[setLoc]->count; imgLoc++, rowLoc++) {
			for (LocInt dimLoc = 0; dimLoc < mDim; dimLoc++) {
				mat_A_test[rowLoc][dimLoc] = mImgTotal[setTestPtr[setLoc]->list[imgLoc]].data[dimLoc] / 256.0;
			}
			setTestPtr[setLoc]->list[imgLoc] = rowLoc;
		}
	}

	return clock() - locTime;
}

clock_t DCRL_clear() {
	clock_t locTime = clock();
	for (LocInt setLoc = 0; setLoc < setTrainNum; setLoc++) {
		mat_delete(mat_H[setLoc], setTrainPtr[setLoc]->count);
		mat_delete(mat_C[setLoc], featDim);
		mat_delete(mat_log_C[setLoc], featDim);
		row_delete(mat_D[setLoc]);
		row_delete(mat_log_D[setLoc]);
		mat_delete(mat_UT[setLoc], featDim);
	}
	for (LocInt setLoc = 0; setLoc < setTestNum; setLoc++) {
		mat_delete(mat_H_test[setLoc], setTestPtr[setLoc]->count);
		mat_delete(mat_C_test[setLoc], featDim);
		mat_delete(mat_log_C_test[setLoc], featDim);
		row_delete(mat_D_test[setLoc]);
		row_delete(mat_log_D_test[setLoc]);
		mat_delete(mat_UT_test[setLoc], featDim);
	}
	delete[] mat_H;
	delete[] mat_C;
	delete[] mat_log_C;
	delete[] mat_D;
	delete[] mat_log_D;
	delete[] mat_UT;
	delete[] mat_H_test;
	delete[] mat_C_test;
	delete[] mat_log_C_test;
	delete[] mat_D_test;
	delete[] mat_log_D_test;
	delete[] mat_UT_test;

	mat_delete(mat_A, mImgTrainNum);
	mat_delete(mat_A_test, mImgTestNum);

	mat_delete(mat_B, mImgTrainNum);
	//mat_delete(mat_B_test, mImgTestNum, featDim);

	mat_delete(mat_W, mDim);
	mat_delete(mat_AW, mImgTrainNum);
	mat_delete(mat_AW_test, mImgTestNum);

	mat_delete(mat_A_pinv, mDim);

	mat_delete(mat_AT_A, mDim);
	mat_delete(mat_AT_A_inv, mDim);

	mat_delete(GE_LEM, setTrainNum);
	mat_delete(GE_A, setTrainNum);
	mat_delete(GE_J, setTrainNum);

	for (LocInt setLoc = 0; setLoc < setTotalNum; setLoc++) {
		delete[] setTotal[setLoc].list;
	}
	delete[] setTotal;
	delete[] setTrainPtr;
	delete[] setTestPtr;
	delete[] kerTotalPtr;
	delete[] kerTrainPtr;

	resolve_msg();

	return clock() - locTime;
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

clock_t eval_feat(DataMat desMat, const pSetT srcSet) { // evaluate target feature (not LSLF)
	clock_t locTime = clock();
	DataDouble nowVal;
	for (LocInt imgLoc = 0; imgLoc < srcSet->count; imgLoc++) {
		for (LocInt featLoc = 0; featLoc < featDim; featLoc++) {
			nowVal = 0;
			for (LocInt dimLoc = 0; dimLoc < mDim; dimLoc++) {
				nowVal += _EX2(mat_A[srcSet->list[imgLoc]][dimLoc] - kerTrainPtr[featLoc]->data[dimLoc] / 256.0);
			}
#if 0
			desMat[imgLoc][featLoc] = (nowVal / 0x100);
#else
			desMat[imgLoc][featLoc] = _EX2(nowVal / 0x100);
#endif
		}
	}

	return clock() - locTime;
}

clock_t get_feat(DataMat desMat, const pSetT srcSet, DataMat refMat) { // retrieve feature from reference matrix
	clock_t locTime = clock();
	for (LocInt imgLoc = 0; imgLoc < srcSet->count; imgLoc++) {
		for (LocInt featLoc = 0; featLoc < featDim; featLoc++) {
			desMat[imgLoc][featLoc] = refMat[srcSet->list[imgLoc]][featLoc];
		}
	}

	return clock() - locTime;
}

clock_t DCRL_feat_B(DataDouble rVal = 0, DataDouble sVal = 1) { // constructing target mat_B
	clock_t locTime = clock();
	SetDlgItemText(nowDlg, IDC_STAT_DCRL, L"Stage 2 : Eval target train feat");
	for (LocInt setLoc = 0; setLoc < setTrainNum; setLoc++) {
		for (LocInt imgLoc = 0; imgLoc < setTrainPtr[setLoc]->count; imgLoc++) {
			for (LocInt featLoc = 0; featLoc < featDim; featLoc++) {
				mat_B[setTrainPtr[setLoc]->list[imgLoc]][featLoc] =
					sVal * (setLoc == featLoc) + rVal * (setTrainPtr[setLoc]->label == setTrainPtr[featLoc]->label);
			}
		}
		resolve_msg();
	}
	//mat_output(mat_B, mImgTrainNum, featDim, "mat_B.txt");

	return clock() - locTime;
}

clock_t DCRL_feat_HB(DataDouble rVal = 1) { // constructing target mat_H, mat_B
	clock_t locTime = clock();
	SetDlgItemText(nowDlg, IDC_STAT_DCRL, L"Stage 2 : Eval target train feat");
	for (LocInt setLoc = 0; setLoc < setTrainNum; setLoc++) {
		eval_feat(mat_H[setLoc], setTrainPtr[setLoc]);
		if (resolve_msg() > -1 || setLoc + 1 == setTrainNum) {
			_W outTextW = L"Total time : " + _TW(clock() - procTime) + L"\n";
			outTextW += L"Target train feat time : " + _TW(clock() - locTime) + L"\n";
			outTextW += L"Set : " + _TW(setLoc + 1) + L" / " + _TW(setTrainNum);
			SetDlgItemText(nowDlg, IDC_TXT_OUT, outTextW.c_str());
		}
	}
	for (LocInt setLoc = 0; setLoc < setTrainNum; setLoc++) {
		for (LocInt imgLoc = 0; imgLoc < setTrainPtr[setLoc]->count; imgLoc++) {
			for (LocInt featLoc = 0; featLoc < featDim; featLoc++) {
				mat_B[setTrainPtr[setLoc]->list[imgLoc]][featLoc] = pow(mat_H[setLoc][imgLoc][featLoc], rVal);
			}
		}
		resolve_msg();
	}
	//mat_output(mat_B, mImgTrainNum, featDim, "mat_B.txt");

	return clock() - locTime;
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

clock_t DCRL_LSLF(FlagT mode_AW = 1, FlagT mode_AW_test = 1) {
	clock_t locTime = clock();
	SetDlgItemText(nowDlg, IDC_STAT_DCRL, L"Stage 3-0 : Eval LSLF pseudoinverse");
	_S DCRL_A_pinv_FileNameS = _S(DCRL_A_pinv_FileDefC) + "_" + _TS(mImgTrainNum) + ".txt";
	_S DCRL_W_FileNameS = _S(DCRL_W_FileDefC) + "_" + _TS(mImgTrainNum) + ".txt";
	
	switch (DCRLLoad) { 
	case DCRL_LOAD_W:
		SetDlgItemText(nowDlg, IDC_STAT_DCRL, L"Stage 3-3 : Load LSLF matrix W");
		mat_load(mat_W, mDim, featDim, DCRL_W_FileNameS.c_str());
		break;
	case DCRL_LOAD_PINV: // W = A+ * B
		SetDlgItemText(nowDlg, IDC_STAT_DCRL, L"Stage 3-2 : Load LSLF matrix A_pinv");
		mat_load(mat_A_pinv, mDim, mImgTrainNum, DCRL_A_pinv_FileNameS.c_str());

		SetDlgItemText(nowDlg, IDC_STAT_DCRL, L"Stage 3-3 : Eval LSLF matrix W");
		mat_mul(mat_W, mat_A_pinv, mat_B, mDim, mImgTrainNum, featDim);
		break;
	case DCRL_LOAD_NONE: // A+ = inv(AT*A)*AT, W = A+ * B
	default:
		SetDlgItemText(nowDlg, IDC_STAT_DCRL, L"Stage 3-1 : Eval LSLF matrix AT_A");
		DataDouble ieps = 0.01;
		mat_mulATA(mat_AT_A, mat_A, mImgTrainNum, mDim);
		mat_addI(mat_AT_A, mDim, ieps);
		mat_inv(mat_AT_A_inv, mat_AT_A, mDim);

		SetDlgItemText(nowDlg, IDC_STAT_DCRL, L"Stage 3-2 : Eval LSLF matrix A_pinv");
		mat_mulABT(mat_A_pinv, mat_AT_A_inv, mat_A, mDim, mImgTrainNum, mDim);

		SetDlgItemText(nowDlg, IDC_STAT_DCRL, L"Stage 3-3 : Eval LSLF matrix W");
		mat_mul(mat_W, mat_A_pinv, mat_B, mDim, mImgTrainNum, featDim);
		break;
	}
	if (DCRLSave_A_pinv > 0) {
		mat_save(mat_A_pinv, mDim, mImgTrainNum, DCRL_A_pinv_FileNameS.c_str());
	}
	if (DCRLSave_W > 0) {
		mat_save(mat_W, mDim, featDim, DCRL_W_FileNameS.c_str());
	}
	resolve_msg();

	if (mode_AW > 0) {
		SetDlgItemText(nowDlg, IDC_STAT_DCRL, L"Stage 4-0 : Eval LSLF test matrix AW");
		mat_mul(mat_AW, mat_A, mat_W, mImgTrainNum, mDim, featDim);
	}
	if (mode_AW_test > 0) {
		SetDlgItemText(nowDlg, IDC_STAT_DCRL, L"Stage 4-1 : Eval LSLF test matrix AW_test");
		mat_mul(mat_AW_test, mat_A_test, mat_W, mImgTestNum, mDim, featDim);
	}

	return clock() - locTime;
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

clock_t test_set_one(DataMat mTrain[], DataMat mTest[], ConT *con) {
	clock_t locTime = clock();
	LocInt minLoc;
	DataDouble minDiff, nowDiff;
	for (LocInt testLoc = 0; testLoc < setTestNum; testLoc++) {
		minLoc = -1;
		minDiff = 0;
		for (LocInt trainLoc = 0; trainLoc < setTrainNum; trainLoc++) {
			nowDiff = mat_diff_var(mTest[testLoc], mTrain[trainLoc], featDim, featDim);
			if (minLoc < 0 || nowDiff < minDiff) {
				minLoc = trainLoc;
				minDiff = nowDiff;
			}
			resolve_msg();
		}
		conf_push(con, setTestPtr[testLoc]->label, setTrainPtr[minLoc]->label, setTestPtr[testLoc]->count);
	}
	_W outTextW = L"Total time : " + _TW(clock() - procTime) + L"\n";
	outTextW += L"Test set time : " + _TW(clock() - locTime) + L"\n";
	outTextW += conf_output(con);
	SetDlgItemText(nowDlg, IDC_TXT_OUT, outTextW.c_str());
		
	return clock() - locTime;
}

clock_t test_set_sum(DataMat mTrain[], DataMat mTest[], ConT *con) {
	clock_t locTime = clock();
	LocInt minLabel;
	DataDouble minDiff, nowDiff[mLabelNum];
	for (LocInt testLoc = 0; testLoc < setTestNum; testLoc++) {
		minLabel = -1;
		minDiff = 0;
		for (LocInt labelLoc = 0; labelLoc < mLabelNum; labelLoc++) {
			nowDiff[labelLoc] = 0;
		}
		for (LocInt trainLoc = 0; trainLoc < setTrainNum; trainLoc++) {
			nowDiff[setTrainPtr[trainLoc]->label] += mat_diff_var(mTest[testLoc], mTrain[trainLoc], featDim, featDim);
			resolve_msg();
		}
		for (LocInt labelLoc = 0; labelLoc < mLabelNum; labelLoc++) {
			if (minLabel < 0 || nowDiff[labelLoc] < minDiff) {
				minLabel = labelLoc;
				minDiff = nowDiff[labelLoc];
			}
		}
		conf_push(con, setTestPtr[testLoc]->label, minLabel, setTestPtr[testLoc]->count);
	}
	_W outTextW = L"Total time : " + _TW(clock() - procTime) + L"\n";
	outTextW += L"Test set sum time : " + _TW(clock() - locTime) + L"\n";
	outTextW += conf_output(con);
	SetDlgItemText(nowDlg, IDC_TXT_OUT, outTextW.c_str());

	return clock() - locTime;
}

clock_t test_img_one(DataMat mTrain[], DataMat mFeat[], DataMat m_AW_test, ConT *con, FlagT covMode, FlagT normMode) { // new method
	clock_t locTime = clock();
	LocInt minLoc;
	DataDouble minDiff, nowDiff;
	DataMat testFeat, testCov, testCovLog, test_UT;
	DataVec test_D, test_log_D;
	mat_init(&testFeat, maxSetCount, featDim);
	mat_init(&testCov, featDim, featDim);
	mat_init(&testCovLog, featDim, featDim);
	mat_init(&test_UT, featDim, featDim);
	row_init(&test_D, featDim);
	row_init(&test_log_D, featDim);
	FILE *testOut;
	fopen_s(&testOut, "test_out.txt", "w");
	for (LocInt testLoc = 0; testLoc < setTestNum; testLoc++) {
		for (LocInt imgLoc = 0; imgLoc < 1; imgLoc++) {
			minLoc = -1;
			minDiff = 0;
			for (LocInt trainLoc = 0; trainLoc < setTrainNum; trainLoc += 1) {
				mat_copy(testFeat, mFeat[trainLoc], setTrainPtr[trainLoc]->count, featDim);
				row_copy(testFeat[0], m_AW_test[setTestPtr[testLoc]->list[imgLoc]], featDim);
				mat_cov(testCov, testFeat, setTrainPtr[trainLoc]->count, featDim);
				mat_std(testCov, featDim, featDim, normMode);
				if (covMode == DCRL_TEST_LOGCOV) {
					if (mat_log(testCovLog, testCov, test_UT, test_D, test_log_D, featDim) < 0) {
						_MSG_WN(L"Log error at ", trainLoc);
					} else {
						mat_std(testCovLog, featDim, featDim, normMode);
					}
					nowDiff = mat_diff_var(mTrain[trainLoc], testCovLog, featDim, featDim);
				} else {
					nowDiff = mat_diff_var(mTrain[trainLoc], testCov, featDim, featDim);
				}
				//nowDiff = mat_diff_var(mFeat[trainLoc], testFeat, setTrainPtr[trainLoc]->count, featDim);
				if (minLoc < 0 || nowDiff < minDiff) {
					minDiff = nowDiff;
					minLoc = trainLoc;
				}
				fprintf(testOut, "%lf\t", nowDiff);
			}
			fprintf(testOut, "\n");
			conf_push(con, setTestPtr[testLoc]->label, setTrainPtr[minLoc]->label, 1);
			resolve_msg();
			_W outTextW = L"Total time : " + _TW(clock() - procTime) + L"\n";
			outTextW += L"Test image time : " + _TW(clock() - locTime) + L"\n";
			outTextW += L"Set : " + _TW(testLoc) + L" / " + _TW(setTestNum) + L"\n";
			outTextW += L"Image : " + _TW(imgLoc) + L" / " + _TW(setTestPtr[testLoc]->count) + L"\n";
			SetDlgItemText(nowDlg, IDC_TXT_OUT, outTextW.c_str());
		}
	}
	fclose(testOut);
	mat_output(testCovLog, featDim, featDim, "test_cov_log.txt");

	mat_delete(testFeat, maxSetCount);
	mat_delete(testCov, featDim);
	mat_delete(testCovLog, featDim);
	mat_delete(test_UT, featDim);
	row_delete(test_D);
	row_delete(test_log_D);

	_W outTextW = L"Total time : " + _TW(clock() - procTime) + L"\n";
	outTextW += L"Test image time : " + _TW(clock() - locTime) + L"\n";
	outTextW += conf_output(con);
	SetDlgItemText(nowDlg, IDC_TXT_OUT, outTextW.c_str());

	return clock() - locTime;
}

clock_t test_set_feat(DataMat m_AW_test, DataMat m_AW, ConT *con) { // test m_AW with labels for each images (not set)
	clock_t locTime = clock();
	LocInt maxLoc, maxCon, maxConLoc, nowCon[mLabelNum];
	DataDouble maxVal, nowVal;
	for (LocInt testLoc = 0; testLoc < setTestNum; testLoc++) {
		for (LocInt labelLoc = 0; labelLoc < mLabelNum; labelLoc++) {
			nowCon[labelLoc] = 0;
		}
		for (LocInt imgLoc = 0; imgLoc < setTestPtr[testLoc]->count; imgLoc++) {
			maxLoc = -1;
			maxVal = 0;
			for (LocInt featLoc = 0; featLoc < featDim; featLoc++) {
				nowVal = m_AW_test[setTestPtr[testLoc]->list[imgLoc]][featLoc];
				if (maxLoc < 0 || nowVal > maxVal) {
					maxLoc = featLoc;
					maxVal = nowVal;
				}
			}
			nowCon[maxLoc * mLabelNum / setTrainNum]++;
		}
		maxConLoc = -1;
		maxCon = 0;
		for (LocInt labelLoc = 0; labelLoc < mLabelNum; labelLoc++) {
			if (maxConLoc < 0 || nowCon[labelLoc] > maxCon) {
				maxConLoc = labelLoc;
				maxCon = nowCon[labelLoc];
			}
		}
		conf_push(con, setTestPtr[testLoc]->label, maxConLoc, setTestPtr[testLoc]->count);
		if (resolve_msg() > -1) {
			_W outTextW = L"Total time : " + _TW(clock() - procTime) + L"\n";
			outTextW += L"Test feat time : " + _TW(clock() - locTime) + L"\n";
			outTextW += L"Sets : " + _TW(testLoc) + L" / " + _TW(setTestNum);
			SetDlgItemText(nowDlg, IDC_TXT_OUT, outTextW.c_str());
		}
	}
	_W outTextW = L"Total time : " + _TW(clock() - procTime) + L"\n";
	outTextW += L"Test feat time : " + _TW(clock() - locTime) + L"\n";
	outTextW += conf_output(con);
	SetDlgItemText(nowDlg, IDC_TXT_OUT, outTextW.c_str());

	return clock() - locTime;
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

DataDouble DCRL_test(FlagT testMode, FlagT shuf, FlagT normMode, FlagT selMode) {
	clock_t locTime = clock();
	SetDlgItemText(nowDlg, IDC_STAT_DCRL, L"Stage 5 : Eval mat_H, mat_C and test");
	ConT conf;
	conf_init(&conf);

	if (testMode == DCRL_TEST_FEAT) {
		test_set_feat(mat_AW_test, mat_AW, &conf);
	} else if (testMode == DCRL_TEST_COV || testMode == DCRL_TEST_LOGCOV) {
		pSetT shufStart, shufEnd;
		if (shuf > 0) {
			for (LocInt labelLoc = 0; labelLoc < mLabelNum; labelLoc++) {
				shufStart = setTrainPtr[labelLoc * (setTrainNum / mLabelNum)];
				shufEnd = setTrainPtr[(labelLoc + 1) * (setTrainNum / mLabelNum) - 1];
				mat_shuffle(mat_AW, shufStart->list[0], shufEnd->list[shufEnd->count - 1] + 1, mImgTrainNum * 10);
				resolve_msg();
			}
		}
		if (shuf > 0) {
			for (LocInt labelLoc = 0; labelLoc < mLabelNum; labelLoc++) {
				shufStart = setTestPtr[labelLoc * (setTestNum / mLabelNum)];
				shufEnd = setTestPtr[(labelLoc + 1) * (setTestNum / mLabelNum) - 1];
				mat_shuffle(mat_AW_test, shufStart->list[0], shufEnd->list[shufEnd->count - 1] + 1, mImgTestNum * 10);
				resolve_msg();
			}
		}
		for (LocInt setLoc = 0; setLoc < setTrainNum; setLoc++) {
			get_feat(mat_H[setLoc], setTrainPtr[setLoc], mat_AW);
		}
		resolve_msg();
		for (LocInt setLoc = 0; setLoc < setTestNum; setLoc++) {
			get_feat(mat_H_test[setLoc], setTestPtr[setLoc], mat_AW_test);
		}
		resolve_msg();
		for (LocInt setLoc = 0; setLoc < setTrainNum; setLoc++) {
			mat_cov(mat_C[setLoc], mat_H[setLoc], setTrainPtr[setLoc]->count, featDim);
			mat_std(mat_C[setLoc], featDim, featDim, normMode & 0x00F);
			//mat_addI(mat_C[setLoc], featDim, 0.5);
		}
		resolve_msg();
		for (LocInt setLoc = 0; setLoc < setTestNum; setLoc++) {
			mat_cov(mat_C_test[setLoc], mat_H_test[setLoc], setTestPtr[setLoc]->count, featDim);
			mat_std(mat_C_test[setLoc], featDim, featDim, normMode & 0x00F);
			//mat_addI(mat_C_test[setLoc], featDim, 0.5);
		}
		resolve_msg();
		if (testMode == DCRL_TEST_LOGCOV) { // log covariance
			DataDouble logDiffNow, logDiffSum = 0;
			clock_t logTime = clock();
			for (LocInt setLoc = 0; setLoc < setTrainNum; setLoc++) {
				if (setTrainPtr[setLoc]->count <= featDim) {
					_MSG_W(L"count < featDim");
					return -1;
				}
				logDiffNow = mat_log(mat_log_C[setLoc], mat_C[setLoc], mat_UT[setLoc], mat_D[setLoc], mat_log_D[setLoc], featDim);
				if (logDiffNow < 0) {
					return -2;
				}
				logDiffSum += logDiffNow;
				mat_std(mat_log_C[setLoc], featDim, featDim, normMode >> 4);
				if (setLoc % 10 < 1) {
					_W outTextW = L"Total time : " + _TW(clock() - procTime) + L"\n";
					outTextW += L"Log time : " + _TW(clock() - locTime) + L"\n";
					outTextW += L"-1 / LogErr : " + _DW(logDiffNow, 3) + L" (" + _DW(logDiffSum / (setLoc + 1), 3) + L")\n";
					outTextW += L"Set train : " + _TW(setLoc) + L" / " + _TW(setTrainNum) + L"\n";
					outTextW += L"N_i / M : " + _DW(setTrainPtr[setLoc]->count * 100.0 / featDim, 3) + L"\n";
					SetDlgItemText(nowDlg, IDC_TXT_OUT, outTextW.c_str());
				}
			}
			for (LocInt setLoc = 0; setLoc < setTestNum; setLoc++) {
				if (setTestPtr[setLoc]->count <= featDim) {
					_MSG_W(L"count < featDim");
					return -1;
				}
				logDiffNow = mat_log(mat_log_C_test[setLoc], mat_C_test[setLoc], mat_UT_test[setLoc], mat_D_test[setLoc], mat_log_D_test[setLoc], featDim);
				if (logDiffNow < 0) {
					return -2;
				}
				logDiffSum += logDiffNow;
				mat_std(mat_log_C_test[setLoc], featDim, featDim, normMode >> 4);
				if (setLoc % 10 < 1) {
					_W outTextW = L"Total time : " + _TW(clock() - procTime) + L"\n";
					outTextW += L"Log time : " + _TW(clock() - locTime) + L"\n";
					outTextW += L"-1 / LogErr : " + _DW(logDiffNow, 3) + L" (" + _DW(logDiffSum / (setLoc + 1 + setTrainNum), 3) + L")\n";
					outTextW += L"Set test : " + _TW(setLoc) + L" / " + _TW(setTestNum) + L"\n";
					outTextW += L"N_i / M : " + _DW(setTestPtr[setLoc]->count * 100.0 / featDim, 3) + L"\n";
					SetDlgItemText(nowDlg, IDC_TXT_OUT, outTextW.c_str());
				}
			}
			resolve_msg();
		}
		DataMat *trainMat = nullptr, *testMat = nullptr;
		switch (testMode) {
		case DCRL_TEST_COV:
			trainMat = mat_C;
			testMat = mat_C_test;
			break;
		case DCRL_TEST_LOGCOV:
			trainMat = mat_log_C;
			testMat = mat_log_C_test;
			break;
		default:
			break;
		}
		switch (selMode) {
		case DCRL_SEL_SETONE:
			test_set_one(trainMat, testMat, &conf);
			break;
		case DCRL_SEL_SETSUM:
			test_set_sum(trainMat, testMat, &conf);
			break;
		case DCRL_SEL_IMGONE:
			test_img_one(trainMat, mat_H, mat_AW_test, &conf, testMode, normMode);
			break;
		default:
			break;
		}
	}

	SetDlgItemText(nowDlg, IDC_FRAME_OUT, (L"Output : " + _DW(conf.scount * 100.0 / conf.tcount) + L" %").c_str());
	put_conf(&conf, mLabelNum, confImgOut, confBlockWidth, confBlockHeight, confBlockSpace);
	GetClientRect(nowDlg, &rect);
	InvalidateRect(nowDlg, &rect, false);

	return conf.scount * 100.0 / conf.tcount;
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

clock_t DCRL_eval_J(DataDouble *J, DataDouble *L, DataDouble S = 4, LocInt K_b = 0, LocInt K_W = 0, FlagT outMode = 0) {
	clock_t locTime = clock();
	*J = 0;
	*L = 0;
	for (LocInt setLoc1 = 0; setLoc1 < setTrainNum; setLoc1++) {
		GE_LEM[setLoc1][setLoc1] = 0;
		GE_A[setLoc1][setLoc1] = 0;
		GE_J[setLoc1][setLoc1] = 0;
		for (LocInt setLoc2 = setLoc1 + 1; setLoc2 < setTrainNum; setLoc2++) {
			GE_LEM[setLoc1][setLoc2] = mat_diff_var(mat_log_C[setLoc1], mat_log_C[setLoc2], featDim, featDim);
			GE_LEM[setLoc2][setLoc1] = GE_LEM[setLoc1][setLoc2];
			*L += GE_LEM[setLoc1][setLoc2];
			if (setTrainPtr[setLoc1]->label == setTrainPtr[setLoc2]->label) {
				GE_A[setLoc1][setLoc2] = exp(-GE_LEM[setLoc1][setLoc2] / (S * S));
			} else {
				GE_A[setLoc1][setLoc2] = -exp(-GE_LEM[setLoc1][setLoc2] / (S * S));
			}
			GE_A[setLoc2][setLoc1] = GE_A[setLoc1][setLoc2];
			GE_J[setLoc1][setLoc2] = -GE_A[setLoc1][setLoc2] * GE_LEM[setLoc1][setLoc2];
			GE_J[setLoc2][setLoc1] = GE_J[setLoc1][setLoc2];
			*J += GE_J[setLoc1][setLoc2];
		}
	}
	if (outMode > 0) {
		FILE *LEMOut;
		if (fopen_s(&LEMOut, "LEM_out.txt", "w")) {
			_MSG_W(L"Failed to open LEM_out.txt for write");
			return -1;
		}
		fprintf(LEMOut, "J_val = %.17f\n", *J);
		fprintf(LEMOut, "GE_LEM : \n");
		for (LocInt setLoc1 = 0; setLoc1 < setTrainNum; setLoc1++) {
			for (LocInt setLoc2 = 0; setLoc2 < setTrainNum; setLoc2++) {
				fprintf(LEMOut, "%lf\t", GE_LEM[setLoc1][setLoc2]);
			}
			fprintf(LEMOut, "\n");
		}
		fprintf(LEMOut, "\nGE_A : \n");
		for (LocInt setLoc1 = 0; setLoc1 < setTrainNum; setLoc1++) {
			for (LocInt setLoc2 = 0; setLoc2 < setTrainNum; setLoc2++) {
				fprintf(LEMOut, "%lf\t", GE_A[setLoc1][setLoc2]);
			}
			fprintf(LEMOut, "\n");
		}
		fprintf(LEMOut, "\nGE_J : \n");
		for (LocInt setLoc1 = 0; setLoc1 < setTrainNum; setLoc1++) {
			for (LocInt setLoc2 = 0; setLoc2 < setTrainNum; setLoc2++) {
				fprintf(LEMOut, "%lf\t", GE_J[setLoc1][setLoc2]);
			}
			fprintf(LEMOut, "\n");
		}
		fclose(LEMOut);
	}

	return clock() - locTime;
}


//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

clock_t DCRL_GE() {
	procTime = clock();

	WCHAR nowTrainSetNumW[MAX_LOADSTRING], nowTestSetNumW[MAX_LOADSTRING];
	GetDlgItemText(nowDlg, IDC_INPUT_KMNUM1, nowTrainSetNumW, MAX_LOADSTRING);
	GetDlgItemText(nowDlg, IDC_INPUT_KMNUM2, nowTestSetNumW, MAX_LOADSTRING);
	LocInt nowTrainNum = _WI32(nowTrainSetNumW, MAX_LOADSTRING);
	LocInt nowTestNum = _WI32(nowTestSetNumW, MAX_LOADSTRING);

	kmeans_set(nowTrainNum + nowTestNum, nowTrainNum * mLabelNum);

	/*UINT32 seed = rand();
	srand(seed);
	SetDlgItemText(nowDlg, IDC_FRAME_CSV, (L"MNIST Input : " + _TW(seed)).c_str());*/
	
	DataDouble nowJ = 0, nowL = 0;
	
	//srand(12345);

	DCRL_init(nowTrainNum, nowTestNum);
	DCRL_feat_B(1, 1);

	srand(741369);
	DCRL_LSLF();
	DCRL_test(DCRL_TEST_LOGCOV, 1, 0x30, DCRL_SEL_SETONE);
	DCRL_eval_J(&nowJ, &nowL, 16, 0, 0, 1);
	_MSG_WN(L"J_val = ", nowJ);

	srand(741369);
	DCRL_LSLF();
	DCRL_test(DCRL_TEST_LOGCOV, 1, 0x30, DCRL_SEL_SETONE);
	DCRL_eval_J(&nowJ, &nowL, 16, 0, 0, 1);
	_MSG_WN(L"J_val = ", nowJ);

	//mat_output(mat_C[0], featDim, featDim, "mat_C0_train.txt", 0);
	mat_output(mat_log_C[setTrainNum - 1], featDim, featDim, "mat_C0_log_train.txt", 0);
	mat_output(mat_log_C_test[setTestNum - 1], featDim, featDim, "mat_C0_log_test.txt", 0);
	//mat_output(mat_H[0], setTrainPtr[0]->count, featDim, "mat_H0_train.txt");
	//mat_output(mat_H_test[0], setTestPtr[0]->count, featDim, "mat_H0_test.txt");
	
	DCRL_clear();

	return clock() - procTime;
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

clock_t DCRL_GE_exp() {
	procTime = clock();

	FILE *expOut;
	if (fopen_s(&expOut, "DCRL_exp.txt", "w")) {
		return -1;
	}
	fclose(expOut);
	LocIntC iterNum = 200, expKer = 60;
	DataDouble nowAcc = 0, nowJ = 0, nowL = 0;

	for (LocInt kTrain = 3; kTrain <= 5; kTrain++) {
		for (LocInt iterLoc = 0; iterLoc < iterNum; iterLoc++) {
			SetDlgItemText(nowDlg, IDC_FRAME_DCRL, (L"K: " + _TW(kTrain) + L" / " + _TW(expKer) + L", I : " + _TW(iterLoc)).c_str());

			srand((unsigned int)time(nullptr));
			kmeans_set(expKer, kTrain * mLabelNum);
			DCRL_init(kTrain, expKer - kTrain);
			DCRL_feat_B(1, 1);

			if (fopen_s(&expOut, "DCRL_exp.txt", "a")) {
				return -1;
			}

			srand(741369);
			DCRL_LSLF();
			SetDlgItemText(nowDlg, IDC_FRAME_DCRL, (L"K: " + _TW(kTrain) + L" / " + _TW(expKer) + L", I : " + _TW(iterLoc) + L", SH, N, LN").c_str());
			nowAcc = DCRL_test(DCRL_TEST_LOGCOV, 1, 0x20, DCRL_SEL_SETONE);
			DCRL_eval_J(&nowJ, &nowL, 4, 0, 0, 1);
			fprintf(expOut, "%d\t%d\t%d\t%d\t%s\t%lf\t%lf\t%lf\n", clock() - procTime, expKer, kTrain, iterLoc, "SH, N, LN", nowAcc, nowL, nowJ);

			srand(741369);
			DCRL_LSLF();
			SetDlgItemText(nowDlg, IDC_FRAME_DCRL, (L"K: " + _TW(kTrain) + L" / " + _TW(expKer) + L", I : " + _TW(iterLoc) + L", SH, N, LD").c_str());
			nowAcc = DCRL_test(DCRL_TEST_LOGCOV, 1, 0x30, DCRL_SEL_SETONE);
			DCRL_eval_J(&nowJ, &nowL, 16, 0, 0, 1);
			fprintf(expOut, "%d\t%d\t%d\t%d\t%s\t%lf\t%lf\t%lf\n", clock() - procTime, expKer, kTrain, iterLoc, "SH, N, LD", nowAcc, nowL, nowJ);

			srand(741369);
			DCRL_LSLF();
			SetDlgItemText(nowDlg, IDC_FRAME_DCRL, (L"K: " + _TW(kTrain) + L" / " + _TW(expKer) + L", I : " + _TW(iterLoc) + L", SH, CN, LN").c_str());
			nowAcc = DCRL_test(DCRL_TEST_LOGCOV, 1, 0x22, DCRL_SEL_SETONE);
			DCRL_eval_J(&nowJ, &nowL, 4, 0, 0, 1);
			fprintf(expOut, "%d\t%d\t%d\t%d\t%s\t%lf\t%lf\t%lf\n", clock() - procTime, expKer, kTrain, iterLoc, "SH, CN, LN", nowAcc, nowL, nowJ);

			srand(741369);
			DCRL_LSLF();
			SetDlgItemText(nowDlg, IDC_FRAME_DCRL, (L"K: " + _TW(kTrain) + L" / " + _TW(expKer) + L", I : " + _TW(iterLoc) + L", SH, CN, LD").c_str());
			nowAcc = DCRL_test(DCRL_TEST_LOGCOV, 1, 0x32, DCRL_SEL_SETONE);
			DCRL_eval_J(&nowJ, &nowL, 16, 0, 0, 1);
			fprintf(expOut, "%d\t%d\t%d\t%d\t%s\t%lf\t%lf\t%lf\n", clock() - procTime, expKer, kTrain, iterLoc, "SH, CN, LD", nowAcc, nowL, nowJ);

			
			fclose(expOut);

			DCRL_clear();

			if (funcProc == 0) {
				fclose(expOut);
				return -1;
			}
		}
	}

	fclose(expOut);

	return clock() - procTime;
}

//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

int show_img() {
	int rndClass = rand() % 10;
	int rndImgNum = dlgImgNumX * dlgImgNumY - rand() % 30;

	put_imgList_mono(mImgTotal, mImgLabelLoc[rndClass], rndImgNum, dlgImgNumX, dlgImgNumY, mWidth, mHeight, dlgImgOut);

	GetClientRect(nowDlg, &rect);
	InvalidateRect(nowDlg, &rect, false);

	return 0;
}

INT_PTR CALLBACK MainProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
#ifndef NO_MSG_MONITOR
	if (msgProcNum < MAX_MSG_SIZE) {
		msgProcData[msgProcNum].cmd = message;
		msgProcData[msgProcNum].wparam = wParam;
		msgProcData[msgProcNum].lparam = lParam;
		msgProcNum++;
	}
#endif
	switch (message) {
	case WM_INITDIALOG:
		init_set(hDlg);
#ifndef USE_MODAL_DIALOG
		ShowWindow(nowDlg, SH_SHOW);
		UpdateWindow(nowDlg);
#endif	
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_MAINOK:
			funcProc = 0;
			break;
		case IDC_MAINCANCEL:
			funcProc = 0;
			break;
		case IDC_RADIO_KMLOAD:
			if (HIWORD(wParam) == BN_CLICKED) {
				kmeansLoad = 1;
				kmeansSave = 0;
			}
			break;
		case IDC_RADIO_KMSAVE:
			if (HIWORD(wParam) == BN_CLICKED) {
				kmeansLoad = 0;
				kmeansSave = 1;
			}
			break;
		case IDC_RADIO_PINVLOAD:
			if (HIWORD(wParam) == BN_CLICKED) {
				DCRLLoad = DCRL_LOAD_PINV;
			}
			break;
		case IDC_RADIO_WLOAD:
			if (HIWORD(wParam) == BN_CLICKED) {
				DCRLLoad = DCRL_LOAD_W;
			}
			break;
		case IDC_CHECK_PINVSAVE:
			if (HIWORD(wParam) == BN_CLICKED) {
				DCRLSave_A_pinv = !DCRLSave_A_pinv;
			}
			break;
		
		case IDC_CHECK_WSAVE:
			if (HIWORD(wParam) == BN_CLICKED) {
				DCRLSave_W = !DCRLSave_W;
			}
			break;
		case IDC_BTN_CSVLOAD:
			EnableWindow(GetDlgItem(nowDlg, IDC_BTN_CSVLOAD), false);
			csvLoad = (load_csv() > 0);
			if (csvLoad > 0) {
				EnableWindow(GetDlgItem(nowDlg, IDC_BTN_CSVLOAD), true);
				EnableWindow(GetDlgItem(nowDlg, IDC_BTN_CSVSHOW), true);

				EnableWindow(GetDlgItem(nowDlg, IDC_BTN_KMEVAL), true);
				EnableWindow(GetDlgItem(nowDlg, IDC_RADIO_KMLOAD), true);
				EnableWindow(GetDlgItem(nowDlg, IDC_RADIO_KMSAVE), true);

				EnableWindow(GetDlgItem(nowDlg, IDC_RADIO_PINVLOAD), true);
				EnableWindow(GetDlgItem(nowDlg, IDC_RADIO_WLOAD), true);
				EnableWindow(GetDlgItem(nowDlg, IDC_CHECK_PINVSAVE), true);
				EnableWindow(GetDlgItem(nowDlg, IDC_CHECK_WSAVE), true);

				EnableWindow(GetDlgItem(nowDlg, IDC_BTN_DCRLEVAL), true);
				EnableWindow(GetDlgItem(nowDlg, IDC_BTN_DCRLEXP), true);
			}
			break;
		case IDC_BTN_CSVSHOW:
			show_img();
			break;
		case IDC_BTN_KMEVAL:
			DCRL_clear();
			break;
		case IDC_BTN_DCRLEVAL:
			funcProc = 1;
			DCRL_GE();
			break;
		case IDC_BTN_DCRLEXP:
			funcProc = 1;
			DCRL_GE_exp();
			break;
		}
		break;
	case WM_SIZE:
		break;
	case WM_PAINT: {
#ifdef NO_LOCAL_DC
		hDC = BeginPaint(hDlg, &ps);
		if (newDC > 0) {
			newDC = 0;
			hDlgMemDC = CreateCompatibleDC(hDC);
			hDlgBitmap = CreateCompatibleBitmap(hDC, dlgImgNumX * mWidth, dlgImgNumY * mHeight);

			hConMemDC = CreateCompatibleDC(hDC);
			hConBitmap = CreateCompatibleBitmap(hDC, confImgWidth, confImgHeight);
		}
		if (csvLoad > 0) {
			SetBitmapBits(hDlgBitmap, dlgImgNumX * mWidth * dlgImgNumY * mHeight * 4, dlgImgOut);
			SelectObject(hDlgMemDC, hDlgBitmap);
			BitBlt(hDC, dlgImgX, dlgImgY, dlgImgNumX * mWidth, dlgImgNumY * mHeight, hDlgMemDC, 0, 0, SRCCOPY);

			SetBitmapBits(hConBitmap, confImgWidth * confImgHeight * 4, confImgOut);
			SelectObject(hConMemDC, hConBitmap);
			BitBlt(hDC, confImgX, confImgY, confImgWidth, confImgHeight, hConMemDC, 0, 0, SRCCOPY);
		}
#else
		PAINTSTRUCT ps;
		HDC hDC;
		hDC = BeginPaint(hDlg, &ps);
		if (csvLoad > 0) {
			HDC hDlgMemDC;
			HBITMAP hDlgBitmap;
			hDlgMemDC = CreateCompatibleDC(hDC);
			hDlgBitmap = CreateCompatibleBitmap(hDC, dlgImgNumX * mWidth, dlgImgNumY * mHeight);

			hConMemDC = CreateCompatibleDC(hDC);
			hConBitmap = CreateCompatibleBitmap(hDC, confImgWidth, confImgHeight);

			SetBitmapBits(hDlgBitmap, dlgImgNumX * mWidth * dlgImgNumY * mHeight * 4, dlgImgOut);
			SelectObject(hDlgMemDC, hDlgBitmap);
			BitBlt(hDC, dlgImgX, dlgImgY, dlgImgNumX * mWidth, dlgImgNumY * mHeight, hDlgMemDC, 0, 0, SRCCOPY);

			SetBitmapBits(hConBitmap, confImgWidth * confImgHeight * 4, confImgOut);
			SelectObject(hConMemDC, hConBitmap);
			BitBlt(hDC, confImgX, confImgY, confImgWidth, confImgHeight, hConMemDC, 0, 0, SRCCOPY);

			DeleteDC(hDlgMemDC);
			DeleteObject(hDlgBitmap);

			DelectDC(hConMemDC);
			DelectObject(hConBitmap);
		}
#endif
		EndPaint(hDlg, &ps);
	} break;

	case WM_DESTROY:
#ifdef NO_LOCAL_DC
		DeleteDC(hDlgMemDC);
		DeleteObject(hDlgBitmap);
		DeleteDC(hConMemDC);
		DeleteObject(hConBitmap);
#endif
#ifdef USE_MODAL_DIALOG
		EndDialog(hDlg, 0);
#endif
		PostQuitMessage(0);
		return (INT_PTR)TRUE;

	default:
		DefWindowProc(hDlg, message, wParam, lParam);
		break;
	}
	return (INT_PTR)FALSE;
}

// 정보 대화 상자의 메시지 처리기입니다.
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

int eval_rank(HWND hDlg) {
	FlagT zeroFlag;
	LocInt zeroNum = 0;
	for (LocInt dimLoc = 0; dimLoc < mDim; dimLoc++) {
		zeroFlag = 1;
		for (LocInt imgLoc = 0; imgLoc < mImgNum; imgLoc++) {
			if (mImgTotal[imgLoc].data[dimLoc] != 0) {
				zeroFlag = 0;
				break;
			}
}
		zeroNum += zeroFlag;
	}
	MessageBox(nowDlg, (_W(L"Zero num : ") + _TW(zeroNum)).c_str(), DEF_TITLE, MB_OK);

	return 0;
}

#endif

#ifndef USE_FEAT_LSLF

clock_t LSLF_init(HWND hDlg) {
	clock_t locTime = clock();
	if (LSLFInit == 0) {
		LSLFInit = 1;
		mat_init(&mat_A, mImgNum, mDim);
		mat_init(&mat_B, mImgNum, mLabelNum);

		mat_init(&mat_A_test, mImgTestNum, mDim);
		mat_init(&mat_B_test, mImgTestNum, mLabelNum);
		mat_init(&mat_W, mDim, mLabelNum);
		mat_init(&mat_AW_test, mImgTestNum, mLabelNum);

		mat_init(&mat_AT, mDim, mImgNum);
		mat_init(&mat_AT_A_AT, mDim, mImgNum);

		mat_init(&mat_AT_A, mDim, mDim);
		mat_init(&mat_AT_A_inv, mDim, mDim);
	}
	//mat_init(mat_M, mDim, mDim);

	resolve_msg();
	for (LocInt imgLoc = 0; imgLoc < mImgNum; imgLoc++) {
		for (LocInt dimLoc = 0; dimLoc < mDim; dimLoc++) {
			mat_A[imgLoc][dimLoc] = mImgTotal[imgLoc].data[dimLoc] / 256.0;
		}
		for (LocInt labelLoc = 0; labelLoc < mLabelNum; labelLoc++) {
			mat_B[imgLoc][labelLoc] = (mImgTotal[imgLoc].label == labelLoc);
		}
	}
	resolve_msg();
	for (LocInt imgLoc = 0; imgLoc < mImgTestNum; imgLoc++) {
		for (LocInt dimLoc = 0; dimLoc < mDim; dimLoc++) {
			mat_A_test[imgLoc][dimLoc] = mImgTestTotal[imgLoc].data[dimLoc] / 256.0;
		}
		for (LocInt labelLoc = 0; labelLoc < mLabelNum; labelLoc++) {
			mat_B_test[imgLoc][labelLoc] = (mImgTestTotal[imgLoc].label == labelLoc);
		}
	}
	resolve_msg();

	return clock() - locTime;
}

#if 0

LocInt mImgNum = 4, mDim = 5;
mat_A[0][0] = 0; mat_A[0][1] = 0; mat_A[0][2] = 0; mat_A[0][3] = 0; mat_A[0][4] = 7;
mat_A[1][0] = 0; mat_A[1][1] = 0; mat_A[1][2] = 0; mat_A[1][3] = 4; mat_A[1][4] = 8;
mat_A[2][0] = 0; mat_A[2][1] = 0; mat_A[2][2] = 2; mat_A[2][3] = 5; mat_A[2][4] = 9;
mat_A[3][0] = 0; mat_A[3][1] = 1; mat_A[3][2] = 3; mat_A[3][3] = 6; mat_A[3][4] = 10;

#endif

clock_t LSLF_test(HWND hDlg) {
	procTime = clock();
	LSLF_init(hDlg);

#if 0
	WCHAR mDimTempW[MAX_LOADSTRING], mImgNumTempW[MAX_LOADSTRING];
	GetDlgItemText(nowDlg, IDC_INPUT_KMNUM1, mImgNumTempW, MAX_LOADSTRING);
	GetDlgItemText(nowDlg, IDC_INPUT_KMNUM2, mDimTempW, MAX_LOADSTRING);

	LocInt mImgNum = _WI32(mImgNumTempW, MAX_LOADSTRING);
	LocInt mDim = _WI32(mDimTempW, MAX_LOADSTRING);

	for (LocInt imgLoc = 0; imgLoc < mImgNum; imgLoc++) {
		for (LocInt dimLoc = 0; dimLoc < mDim; dimLoc++) {
			mat_A[imgLoc][dimLoc] = (rand() % 10 < 3) * (rand() % 256 / 256.0);
		}
	}
#endif
	if (LSLF_A_pinv_Load > 0) {
		if (fopen_s(&LSLFInput, "LSLF_A_pinv.txt", "rb")) {
			MessageBox(nowDlg, L"Failed to open LSLF_A_pinv.txt", DEF_TITLE, MB_OK | MB_ICONERROR);
			return -1;
		}
		for (LocInt dimLoc = 0; dimLoc < mDim; dimLoc++) {
			fread_s(mat_AT_A[dimLoc], mDim * sizeof(DataDouble), sizeof(DataDouble), mDim, LSLFInput);
		}
		fclose(LSLFInput);
	} else {
		mat_mulATA(mat_AT_A, mat_A, mImgNum, mDim);
	}
	if (LSLF_A_pinv_Save > 0) {
		if (fopen_s(&LSLFOutput, "LSLF_A_pinv.txt", "wb")) {
			MessageBox(nowDlg, L"Failed to open LSLF_A_pinv.txt", DEF_TITLE, MB_OK | MB_ICONERROR);
			return -1;
		}
		for (LocInt dimLoc = 0; dimLoc < mDim; dimLoc++) {
			fwrite(mat_AT_A[dimLoc], sizeof(DataDouble), mDim, LSLFOutput);
		}
		fclose(LSLFOutput);
	}

#if 1
	DataDouble ieps = 0.01;
	mat_addI(mat_AT_A, mDim, ieps);
	mat_inv(mat_AT_A_inv, mat_AT_A, mDim);
	mat_mulABT(mat_AT, mat_AT_A_inv, mat_A, mDim, mImgNum, mDim);
	// A+ = inv(AT*A)*AT
	//mat_scale(mat_AT, mDim, mImgNum, 0.00001);
#else
	mat_copyT(mat_AT, mat_A, mDim, mImgNum, 0.0000001);
#endif

	if (fopen_s(&LSLFReport, "LSLF.txt", "w")) {
		MessageBox(nowDlg, L"Failed to open LSLF.txt", DEF_TITLE, MB_OK | MB_ICONERROR);
		return -1;
	}

	DataDouble dNormNow = 0, dNormPrev = -1;
	LocInt cNum;

	for (LocInt iterNum = 0; iterNum < 50; iterNum++) {
		mat_mul(mat_W, mat_AT, mat_B, mDim, mImgNum, mLabelNum);
		mat_mul(mat_AW_test, mat_A_test, mat_W, mImgTestNum, mDim, mLabelNum);
		cNum = mat_test(mat_AW_test, mat_B_test, mImgTestNum, mLabelNum);
		dNormNow = mat_diff_norm(mat_AW_test, mat_B_test, mImgTestNum, mLabelNum);

		_W outLSLFW = L"AC = " + _TW(cNum) + L", ";
		outLSLFW += L"log(B-AW) = " + _TW(log(dNormNow));
		_W frameLSLFW = L"Linear Feature Model (" + _TW(iterNum) + L")";
		SetDlgItemText(nowDlg, IDC_STAT_LSLF, outLSLFW.c_str());
		SetDlgItemText(nowDlg, IDC_FRAME_LSLF, frameLSLFW.c_str());
		fprintf(LSLFReport, "%d\t%d\t%d\t%lf\n", iterNum, clock() - procTime, cNum, log(dNormNow));
#if 1	
		if (funcProc == 0 || log10(abs(dNormNow - dNormPrev)) < -9) {
			break;
		}
		dNormPrev = dNormNow;

		mat_mul(mat_AT_A, mat_AT, mat_A, mDim, mImgNum, mDim);
		mat_mul(mat_AT_A_AT, mat_AT_A, mat_AT, mDim, mDim, mImgNum);
		mat_add(mat_AT, mat_AT, mat_AT_A_AT, mDim, mImgNum, 2, -1);
#else
		break;
#endif
	}
	fclose(LSLFReport);

	return clock() - procTime;
}



//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

LocInt test_kernel(HWND hDlg, ImgT srcData[], LocInt srcNum) {
	LocInt minLoc, solNum, totalNum;
	DataDouble nowVar, minVar;
	solNum = 0;
	totalNum = 0;
	for (LocInt imgLoc = 0; imgLoc < srcNum; imgLoc++) {
		minLoc = -1;
		minVar = 0x10000 * mDim + 1;
		for (LocInt kerLabelLoc = 0; kerLabelLoc < mLabelNum; kerLabelLoc++) {
			for (LocInt kerLoc = 0; kerLoc < mKerNum[kerLabelLoc]; kerLoc++) {
				nowVar = 0;
				for (LocInt dimLoc = 0; dimLoc < mDim; dimLoc++) {
					nowVar += _EX2((DataDouble)srcData[imgLoc].data[dimLoc] -
						(DataDouble)mKerData[kerLabelLoc][kerLoc].data[dimLoc]);
				}
				if (nowVar < minVar) {
					minVar = nowVar;
					minLoc = kerLabelLoc;
				}
			}
		}
		if (minLoc == srcData[imgLoc].label) {
			solNum++;
		}
		totalNum++;
		if (resolve_msg() > -1 || totalNum == srcNum) {
			_W outFeatW = L"Time : " + _TW(clock() - procTime) + L", Acc : " +
				_TW(solNum) + L" / " + _TW(totalNum) + L" (" + _TW(int(solNum * 100 / totalNum)) + L" %)";
			_W outFeatFrameW = L"Stage : " + _TW(imgLoc) + L" / " + _TW(srcNum);
			SetDlgItemText(nowDlg, IDC_STAT_DCRL, outFeatW.c_str());
			SetDlgItemText(nowDlg, IDC_STAT_DCRL, outFeatFrameW.c_str());
		}
	}

	return solNum;
}

clock_t test_feat(HWND hDlg) {
	clock_t locTime = clock();
	if (fopen_s(&featOutput, "feature.txt", "w")) {
		MessageBox(nowDlg, L"Failed to open feature.txt", DEF_TITLE, MB_OK | MB_ICONERROR);
		return -1;
	}
	LocInt iterNum = 1, kerMin, kerMax;
	WCHAR kerMinW[MAX_LOADSTRING], kerMaxW[MAX_LOADSTRING];
	GetDlgItemText(nowDlg, IDC_INPUT_KMNUM1, kerMinW, MAX_LOADSTRING);
	GetDlgItemText(nowDlg, IDC_INPUT_KMNUM2, kerMaxW, MAX_LOADSTRING);
	kerMin = _WI32(kerMinW, MAX_LOADSTRING);
	kerMax = _WI32(kerMaxW, MAX_LOADSTRING);

	DataDouble sol;
	for (LocInt kerNow = kerMin; kerNow <= kerMax; kerNow += int(kerNow / 10 + 1)) {
		for (LocInt iterNow = 0; iterNow < iterNum; iterNow++) {
			SetDlgItemText(nowDlg, IDC_INPUT_KMNUM1, _TW(kerNow).c_str());
			SetDlgItemText(nowDlg, IDC_INPUT_KMNUM2, _TW(iterNow).c_str());
			kmeansLoad = 0;
			kmeansSave = 0;
			kmeans_set(hDlg);

			sol = test_kernel(hDlg, mImgTotal, mImgNum) / (DataDouble)mImgNum;

			fprintf(featOutput, "%d\t%d\t%d\t%lf\n", clock() - procTime, kerNow, iterNow, sol);
		}
	}

	fclose(featOutput);

	return clock() - locTime;
}

#endif

#if 0
int save_msg(HWND hDlg) {
	FILE *msgReqOut, *msgProcOut;
	if (!fopen_s(&msgReqOut, "msgreq.txt", "w")) {
		for (int msgLoc = 0; msgLoc < msgReqNum; msgLoc++) {
			fprintf(msgReqOut, "#%d MSG: %d,\tWP: %d,\tLP: %d\n", msgLoc, msgReqData[msgLoc].cmd, msgReqData[msgLoc].wparam, msgReqData[msgLoc].lparam);
		}
		fclose(msgReqOut);
	}
	if (!fopen_s(&msgProcOut, "msgproc.txt", "w")) {
		for (int msgLoc = 0; msgLoc < msgProcNum; msgLoc++) {
			fprintf(msgProcOut, "#%d MSG: %d,\tWP: %d,\tLP: %d\n", msgLoc, msgProcData[msgLoc].cmd, msgProcData[msgLoc].wparam, msgProcData[msgLoc].lparam);
		}
		fclose(msgProcOut);
	}
	SetDlgItemText(nowDlg, IDC_TXT_OUT, _TW(msgProcNum).c_str());
	KillTimer(hDlg, IDT_BACK_MSG);
	
	return 0;
}
#endif

/*



TCHAR greeting[] = _T("Hello, World!");
TextOut(hdc, 10, 10, greeting, _tcslen(greeting));
MessageBox(nowDlg, msgText.c_str(), L"라마바", MB_OK);
mbstowcs(des, src, max);
//GetDeviceCaps(hdc)
//MoveWindow(hDlg, 0, 0, 800 + rand() % 100, 400 + rand() % 100, true);

/*HWND hwndButton = CreateWindowW(L"BUTTON", L"OK", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, // class label type
100, 100, 100, 100, // x y w h
hWnd, nullptr, hInstance, nullptr); // window menu instance lparam

ShowWindow(hwndButton, nCmdShow);
UpdateWindow(hwndButton);
//wcex.hbrBackground = CreateSolidBrush(RGB(rand() % 256, rand() % 256, rand() % 256));

//wchar_t csvBufW[CSV_LINE_SIZE];


//占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙占쏙옙

IDC_APPSTARTING(32650)
Standard arrow and small hourglass

IDC_ARROW(32512)
Standard arrow

IDC_CROSS(32515)
Crosshair

IDC_HAND(32649)
Hand

IDC_HELP(32651)
Arrow and question mark

IDC_IBEAM(32513)
I-beam

IDC_ICON(32641)
Obsolete for applications marked version 4.0 or later.

IDC_NO(32648)
Slashed circle

IDC_SIZE(32640)
Obsolete for applications marked version 4.0 or later. Use IDC_SIZEALL.

IDC_SIZEALL(32646)
Four-pointed arrow pointing north, south, east, and west

IDC_SIZENESW(32643)
Double-pointed arrow pointing northeast and southwest

IDC_SIZENS(32645)
Double-pointed arrow pointing north and south

IDC_SIZENWSE(32642)
Double-pointed arrow pointing northwest and southeast

IDC_SIZEWE(32644)
Double-pointed arrow pointing west and east

IDC_UPARROW(32516)
Vertical arrow

IDC_WAIT(32514)
Hourglass

*/

