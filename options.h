#pragma once

LPCWSTR filePath = L".//data//config.ini";

extern int AddBtnW;
extern bool SingleColumn;

bool parseOptionFile(HWND hwnd);
bool writeOptionToFile(HWND hwnd);
bool isMenuChecked(HMENU hmenu, UINT uId);

bool parseOptionFile(HWND hwnd) {
	HMENU hmenu = GetMenu(hwnd);
	HWND lview = GetDlgItem(hwnd, LVIEW);
	WINDOWPLACEMENT wp;
	int temp;

	//Widnow Placement
	GetPrivateProfileStruct(_T("WindowSize"), _T("Placement"), LPBYTE(&wp), sizeof(wp), filePath);
	SetWindowPlacement(hwnd, &wp);

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

	//track select
	temp = GetPrivateProfileInt(_T("Menu"), _T("TrackSelect"), 1, filePath);
	if (temp)  CheckMenuItem(hmenu, ID_TRACKSELECT, MF_CHECKED);
	else { 
		DWORD dwExStyle = ListView_GetExtendedListViewStyle(lview);
		dwExStyle &= ~LVS_EX_TRACKSELECT;
		ListView_SetExtendedListViewStyle(lview, dwExStyle);
		CheckMenuItem(hmenu, ID_TRACKSELECT, MF_UNCHECKED); }

	//Task Name Column Width
	temp = GetPrivateProfileInt(_T("Column"), _T("TaskNameWidth"), 330, filePath);
	ListView_SetColumnWidth(lview, 0, temp);

	//Date Column Width
	temp = GetPrivateProfileInt(_T("Column"), _T("DateWidth"), 120, filePath);
	ListView_SetColumnWidth(lview, 1, temp);

	//Status Column Width
	temp = GetPrivateProfileInt(_T("Column"), _T("StatusWidth"), 120, filePath);
	ListView_SetColumnWidth(lview, 2, temp);

	//Single Column
	//must be placed after set all the column width, otherwise cannot get the column width from file
	temp = GetPrivateProfileInt(_T("Menu"), _T("SingleColumn"), 1, filePath);
	if (temp) {
		RECT rc;
		GetClientRect(lview, &rc);
		CheckMenuItem(hmenu, ID_SINGLECOLUMN, MF_CHECKED);
		SingleColumn = true;

		TaskNameW = ListView_GetColumnWidth(lview, 0);
		TaskDateW = ListView_GetColumnWidth(lview, 1);
		TaskStatusW = ListView_GetColumnWidth(lview, 2);
		ListView_SetColumnWidth(lview, 0, (rc.right - 1));
		ListView_DeleteColumn(lview, 1);
		ListView_DeleteColumn(lview, 1);
	}
	else CheckMenuItem(hmenu, ID_SINGLECOLUMN, MF_UNCHECKED);

	return 0;
}

bool writeOptionToFile(HWND hwnd) {
	HMENU hmenu = GetMenu(hwnd);
	HWND lview = GetDlgItem(hwnd, LVIEW);
	TCHAR temp[256];
	WINDOWPLACEMENT wp;

	//Window Placement
	GetWindowPlacement(hwnd, &wp);
	WritePrivateProfileStruct(_T("WindowSize"), _T("Placement"), LPBYTE(&wp), sizeof(wp), filePath);

	//confirm on delete
	_itow_s(isMenuChecked(hmenu, ID_CONFIRMONDELETE), temp, 10);
	WritePrivateProfileString(_T("Menu"), _T("ConfirmOnDelete"), temp, filePath);
	
	//show add button
	_itow_s(isMenuChecked(hmenu, ID_SHOWADDBUTTON), temp, 10);
	WritePrivateProfileString(_T("Menu"), _T("ShowAddButton"), temp, filePath);

	//track select
	_itow_s(isMenuChecked(hmenu, ID_TRACKSELECT), temp, 10);
	WritePrivateProfileString(_T("Menu"), _T("TrackSelect"), temp, filePath);

	_itow_s(isMenuChecked(hmenu, ID_SINGLECOLUMN), temp, 10);
	WritePrivateProfileString(_T("Menu"), _T("SingleColumn"), temp, filePath);
	
	if (SingleColumn == true) return 0;

	//Task Name Column Width
	_itow_s(ListView_GetColumnWidth(lview, 0), temp, 10);
	WritePrivateProfileString(_T("Column"), _T("TaskNameWidth"), temp, filePath);
	
	//Date Column Width
	_itow_s(ListView_GetColumnWidth(lview, 1), temp, 10);
	WritePrivateProfileString(_T("Column"), _T("DateWidth"), temp, filePath);

	//Status Column Width
	_itow_s(ListView_GetColumnWidth(lview, 2), temp, 10);
	WritePrivateProfileString(_T("Column"), _T("StatusWidth"), temp, filePath);
	return 0;
}

bool isMenuChecked(HMENU hmenu, UINT uId) {
	UINT state = GetMenuState(hmenu, uId, MF_BYCOMMAND);
	return (state & MF_CHECKED);
}
