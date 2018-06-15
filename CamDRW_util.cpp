#include "CamDRW_util.h"

int _CW(const char *src, wchar_t *des, const int len) {
	int i = 0;
	while (i < len) {
		des[i] = src[i];
		if (src[i] == 0) { break; }
		i++;
	}

	return i;
}

int _WC(const wchar_t *src, char *des, const int len) {
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

int32_t _WI32(const wchar_t *src, const int len) { // wchar to int32
	int i = 0, s = 1;
	int32_t r = 0;
	if (src[i] == L'-') {
		s = -1;
		i++;
	}
	while (i < len) {
		if (src[i] >= L'0' && src[i] <= L'9') {
			r = r * 10 + src[i] - L'0';
		} else {
			break;
		}
		i++;
	}

	return r * s;
}

int32_t _CI32(const char *src, const int len) { // wchar to int32
	int i = 0, s = 1;
	int32_t r = 0;
	if (src[i] == '-') {
		s = -1;
		i++;
	}
	while (i < len) {
		if (src[i] >= '0' && src[i] <= '9') {
			r = r * 10 + src[i] - '0';
		} else {
			break;
		}
		i++;
	}

	return r * s;
}

int _IW32(int32_t src, wchar_t *des) {
	int i = 0, n;
	int32_t d = 9, dig[10] = { 1, };
	for (int e = 1; e < 10; e++) {
		dig[e] = dig[e - 1] * 10;
	}
	if (src < 0) {
		des[0] = L'-';
		src = -src;
		i++;
	}
	while (src < dig[d]) { d--; }
	if (d < 0) { d = 0; }
	for (int e = d; e >= 0; e--) {
		des[i] = L'0' + src / dig[e];
		src -= (src / dig[e]) * dig[e];
		i++;
	}
	des[i] = 0;

	return 1;
}

_W _DW(const double src, const int pNum) {
	_W rtw = ((src < 0) ? L"-" : L"") +_TW(INT64(src)) + L".";
	double rNum = abs(src - INT64(src));
	for (int pLoc = 0; pLoc < pNum; pLoc++) {
		rNum *= 10;
		rtw += _TW(INT64(rNum));
		rNum = rNum - INT64(rNum);
	}
	return rtw;
}

_S _DS(const double src, const int pNum) {
	_S rts = ((src < 0) ? "-" : "") + _TS(INT64(src)) + ".";
	double rNum = abs(src - INT64(src));
	for (int pLoc = 0; pLoc < pNum; pLoc++) {
		rNum *= 10;
		rts += _TS(INT64(rNum));
		rNum = rNum - INT64(rNum);
	}
	return rts;
}

//占싱깍옙占싱깍옙占싱깍옙占싱깍옙占싱깍옙占싱깍옙占싱깍옙占싱깍옙占싱깍옙占싱깍옙占싱깍옙占싱깍옙占싱깍옙占싱깍옙