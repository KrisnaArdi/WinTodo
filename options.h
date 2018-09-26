#pragma once

/*typedef struct {
	TCHAR name[256];
	BOOL value;
} Options;

Options OptList[4];*/

extern int AddBtnW;

bool parseOptionFile(HWND hwnd);
bool writeOptionToFile(HWND hwnd);
bool SetDefaultOption(HWND hwnd);
bool isMenuChecked(HMENU hmenu, UINT uId);

bool parseOptionFile(HWND hwnd) {
	FILE *file;
	fopen_s(&file, "data/Options.csv", "r");
	wchar_t input[1024], *str, *buffer;
	HMENU hmenu = GetMenu(hwnd);
	
	if (GetFileAttributes(_T("data/Options.csv")) == INVALID_FILE_ATTRIBUTES) {
		CreateDirectory(L".//data", NULL); //create a new folder

		HANDLE h = CreateFile(_T(".//data//Options.csv"), GENERIC_READ, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		CloseHandle(h);
		SetDefaultOption(hwnd);
		
		return false;
	}

	while (fgetws(input, 1024, (FILE*)file)) {
		str = wcstok_s(input, _T("="), &buffer);

	//confirm on delete
		if (wcscmp(str, _T("ConfirmOnDelete")) == 0) {
			str = wcstok_s(NULL, _T("\n"), &buffer);
			int value = _wtoi(str);

			if(value) CheckMenuItem(hmenu, ID_CONFIRMONDELETE, MF_CHECKED);
			else CheckMenuItem(hmenu, ID_CONFIRMONDELETE, MF_UNCHECKED);
		}
	
	//window height
		if (wcscmp(str, _T("WindowHeight")) == 0) {
			str = wcstok_s(NULL, _T("\n"), &buffer);
			int value = _wtoi(str); //the height value

			RECT rc;
			GetWindowRect(hwnd, &rc);
			int width = rc.right - rc.left;
			SetWindowPos(hwnd, 0, 0, 0, width, value, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	
	//window width
		if (wcscmp(str, _T("WindowWidth")) == 0) {
			str = wcstok_s(NULL, _T("\n"), &buffer);
			int value = _wtoi(str);

			RECT rc;
			GetWindowRect(hwnd, &rc);
			int height = rc.bottom - rc.top;
			SetWindowPos(hwnd, 0, 0, 0, value, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}

	//the task name column in the listview width 
		if (wcscmp(str, _T("TaskNameWidth")) == 0) {
			HWND lview = GetDlgItem(hwnd, LVIEW);
			str = wcstok_s(NULL, _T("\n"), &buffer);
			int value = _wtoi(str);

			ListView_SetColumnWidth(lview, 0, value);
		}

	//the status column in the listview width 
		if (wcscmp(str, _T("StatusWidth")) == 0) {
			HWND lview = GetDlgItem(hwnd, LVIEW);
			str = wcstok_s(NULL, _T("\n"), &buffer);
			int value = _wtoi(str);

			ListView_SetColumnWidth(lview, 1, value);
		}
	
	//show/hide add button
		if (wcscmp(str, _T("ShowAddButton")) == 0) {
			str = wcstok_s(NULL, _T("\n"), &buffer);
			int value = _wtoi(str);

			if (value) CheckMenuItem(hmenu, ID_SHOWADDBUTTON, MF_CHECKED);
			else { 
				HWND AddBtn = GetDlgItem(hwnd, ID_ADD);
				HWND edit = GetDlgItem(hwnd, ID_INPUT);
				CheckMenuItem(hmenu, ID_SHOWADDBUTTON, MF_UNCHECKED);
				AddBtnW = 0; //defining the Button Width as 0 for resizing the input.
				
				ShowWindow(AddBtn, SW_HIDE);
				
			}
		}
	}

	fclose(file);
	return true;
};

bool writeOptionToFile(HWND hwnd) {
	HMENU hmenu = GetMenu(hwnd);
	HWND lview = GetDlgItem(hwnd, LVIEW);
	FILE *file;
	fopen_s(&file, "data/Options.csv", "w+");
	RECT rc;
	GetWindowRect(hwnd, &rc);
	int hwndW = rc.right - rc.left;
	int hwndH = rc.bottom - rc.top;
	int TaskNameWidth = ListView_GetColumnWidth(lview, 0);

	fwprintf_s(file, _T("%s=%d\n"), _T("ConfirmOnDelete"), isMenuChecked(hmenu, ID_CONFIRMONDELETE));
	fwprintf_s(file, _T("%s=%d\n"), _T("WindowHeight"), hwndH);
	fwprintf_s(file, _T("%s=%d\n"), _T("WindowWidth"), hwndW);
	fwprintf_s(file, _T("%s=%d\n"), _T("TaskNameWidth"), ListView_GetColumnWidth(lview, 0));
	fwprintf_s(file, _T("%s=%d\n"), _T("StatusWidth"), ListView_GetColumnWidth(lview, 1));
	fwprintf_s(file, _T("%s=%d\n"), _T("ShowAddButton"), isMenuChecked(hmenu, ID_SHOWADDBUTTON));

	fclose(file);
	DestroyMenu(hmenu);
	return true;
}

bool SetDefaultOption(HWND hwnd) {
	return 1;
}

bool isMenuChecked(HMENU hmenu, UINT uId) {
	UINT state = GetMenuState(hmenu, uId, MF_BYCOMMAND);
	return (state & MF_CHECKED);
}
