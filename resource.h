#pragma once
#pragma comment(lib, "comctl32.lib")
#define LVIEW 101
#define ID_INPUT		102
#define ID_LVSTATIC		103

#define ID_ADD			201
#define ID_DEL			202
#define ID_DELALL		203
#define ID_DELFIN		204
#define ID_DELUNFIN		205

#define ID_STATUSBAR	1001

int btnW = 100;
int btnH = 24;
int editH = 30;
int iSelect = 0;

wchar_t WindowTitle[] = L"WinTodo";
static bool isSaved = true;

HFONT BtnFont = CreateFont(14, 0, 0, 0,
	FW_BLACK, 0, 0, 0, ANSI_CHARSET, 
	OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	DEFAULT_PITCH | FF_DONTCARE, TEXT("Calibri"));

HFONT AddBtnFont = CreateFont(18, 0, 0, 0,
	FW_BLACK, 0, 0, 0, ANSI_CHARSET,
	OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	DEFAULT_PITCH | FF_DONTCARE, TEXT("Calibri"));

HFONT EditFont = CreateFont(20, 0, 0, 0, 
	FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
	OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	DEFAULT_PITCH | FF_DONTCARE, TEXT("Tahoma"));

HFONT LviewFont = CreateFont(20, 0, 0, 0,
	FW_DONTCARE, 0, 0, 0, ANSI_CHARSET,
	OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	DEFAULT_PITCH | FF_DONTCARE, TEXT("Franklin Gothic Book"));