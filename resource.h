#pragma once
#pragma comment(lib, "comctl32.lib")
#define LVIEW			100
#define ID_INPUT		102
#define ID_LVSTATIC		103

#define ID_ADD			201
#define ID_DEL			202
#define ID_DELALL		203
#define ID_DELFIN		204
#define ID_DELUNFIN		205
#define ID_SETFAVORITE	206

#define ID_STATUSBAR	1001
#define ID_TBAR			1002
#define ID_ADDTBAR		1003
#define IDB_TOOLBITMAP	301

#define ID_TRAY_APP_ICON 2002
#define ID_TRAY_MENU_EXIT 201
#define WM_TRAYICON (WM_USER+1)

int iSelect = 0;
int btnW = 32;
int btnH = 32;
int editH = 32;
int TBarH = 32;
static int AddBtnW = 32;
static bool SingleColumn = 0;

TCHAR WindowTitle[] = L"WinTodo";
static bool isSaved = true;

HFONT EditFont = CreateFont(24, 0, 0, 0, 
	FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
	OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	DEFAULT_PITCH | FF_DONTCARE, TEXT("Segoe UI"));

HFONT LviewFont = CreateFont(20, 0, 0, 0,
	FW_DONTCARE, 0, 0, 0, ANSI_CHARSET,
	OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	DEFAULT_PITCH | FF_DONTCARE, TEXT("Calibri"));

TCHAR HelpText[] = _T("In this todo program you could do this following action :\r\n-to add a todo list\r\n- deleting a todo list\r\n- check wether a todo list is done or not\r\n- edit the already existing todo list\r\n\r\n# Adding a todo list\r\nTo add a todo list item, you need to fill the textbox on the top and then press the addItem button or straightly press enter.You could hide the addItem button in options\r\n\r\n#deleting a todo list\r\nTo deleting a todolist item, you need to select an item first, then to delete using the delete button in the bottom or press delete key in the numpad.You could delete multiple item if multiple item is selected, you could also delete all item with the delete all button, all finished item with delete checked button, or delete all unfinished item with delete unchecked item.\r\n\r\n#Set item as finished\r\nyou could set your todo item as finished with the checkbox in the left side of your todo item.\r\n\r\n#Edit the existing item\r\nyou could edit the name of your item by double - clicking your todo item.To cancel the change press ESC before quiting the editing session by either clicking outside the label, or by pressing enter.");