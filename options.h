#pragma once

LPCWSTR filePath = L".//data//config.ini";

extern int AddBtnW;

bool parseOptionFile(HWND hwnd);
bool writeOptionToFile(HWND hwnd);
bool isMenuChecked(HMENU hmenu, UINT uId);

bool parseOptionFile(HWND hwnd) {
	HMENU hmenu = GetMenu(hwnd);
	HWND lview = GetDlgItem(hwnd, LVIEW);
	WINDOWPLACEMENT wp;
	int temp;

	//confirm on delete
	temp = GetPrivateProfileInt(_T("Menu"), _T("ConfirmOnDelete"), 1, filePath);
	if(temp)  CheckMenuItem(hmenu, ID_CONFIRMONDELETE, MF_CHECKED);
	else CheckMenuItem(hmenu, ID_CONFIRMONDELETE, MF_UNCHECKED);

	//Show add button
	temp = GetPrivateProfileInt(_T("Menu"), _T("ShowAddButton"), 1, filePath);
	if (temp)  CheckMenuItem(hmenu, ID_SHOWADDBUTTON, MF_CHECKED);
	else {
		HWND AddBtn = GetDlgItem(hwnd, ID_ADD);
		HWND edit = GetDlgItem(hwnd, ID_INPUT);
		CheckMenuItem(hmenu, ID_SHOWADDBUTTON, MF_UNCHECKED);
		AddBtnW = 0; //defining the Button Width as 0 for resizing the input.

		ShowWindow(AddBtn, SW_HIDE);

	}

	//Task Name Column Width
	temp = GetPrivateProfileInt(_T("Column"), _T("TaskNameWidth"), 330, filePath);
	ListView_SetColumnWidth(lview, 0, temp);

	//Date Column Width
	temp = GetPrivateProfileInt(_T("Column"), _T("DateWidth"), 120, filePath);
	ListView_SetColumnWidth(lview, 1, temp);

	//Status Column Width
	temp = GetPrivateProfileInt(_T("Column"), _T("StatusWidth"), 120, filePath);
	ListView_SetColumnWidth(lview, 2, temp);

	//Widnow Placement
	GetPrivateProfileStruct(_T("WindowSize"), _T("Placement"), LPBYTE(&wp), sizeof(wp), filePath);
	SetWindowPlacement(hwnd, &wp);

	return 0;
}

bool writeOptionToFile(HWND hwnd) {
	HMENU hmenu = GetMenu(hwnd);
	HWND lview = GetDlgItem(hwnd, LVIEW);
	TCHAR temp[256];
	WINDOWPLACEMENT wp;

	//confirm on delete
	_itow_s(isMenuChecked(hmenu, ID_CONFIRMONDELETE), temp, 10);
	WritePrivateProfileString(_T("Menu"), _T("ConfirmOnDelete"), temp, filePath);
	
	//show add button
	_itow_s(isMenuChecked(hmenu, ID_SHOWADDBUTTON), temp, 10);
	WritePrivateProfileString(_T("Menu"), _T("ShowAddButton"), temp, filePath);
	
	//Task Name Column Width
	_itow_s(ListView_GetColumnWidth(lview, 0), temp, 10);
	WritePrivateProfileString(_T("Column"), _T("TaskNameWidth"), temp, filePath);
	
	//Date Column Width
	_itow_s(ListView_GetColumnWidth(lview, 1), temp, 10);
	WritePrivateProfileString(_T("Column"), _T("DateWidth"), temp, filePath);

	//Status Column Width
	_itow_s(ListView_GetColumnWidth(lview, 2), temp, 10);
	WritePrivateProfileString(_T("Column"), _T("StatusWidth"), temp, filePath);

	//Window Placement
	GetWindowPlacement(hwnd, &wp);
	WritePrivateProfileStruct(_T("WindowSize"), _T("Placement"), LPBYTE(&wp), sizeof(wp), filePath);
	return 0;
}

bool isMenuChecked(HMENU hmenu, UINT uId) {
	UINT state = GetMenuState(hmenu, uId, MF_BYCOMMAND);
	return (state & MF_CHECKED);
}
