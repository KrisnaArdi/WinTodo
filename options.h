#pragma once

/*typedef struct {
	TCHAR name[256];
	BOOL value;
} Options;

Options OptList[4];*/

bool parseOptionFile(HWND hwnd);
bool SetDefaultOption(HWND hwnd);
bool isMenuChecked(HMENU hmenu, UINT uId);
bool writeOptionToFile(HWND hwnd);

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
	}

	fclose(file);
	return true;
};

bool SetDefaultOption(HWND hwnd) {
	HMENU hmenu = GetMenu(hwnd);
	CheckMenuItem(hmenu, ID_CONFIRMONDELETE, MF_CHECKED);
	DestroyMenu(hmenu);
	return true;
}

bool isMenuChecked(HMENU hmenu, UINT uId) {
	UINT state = GetMenuState(hmenu, uId, MF_BYCOMMAND);
	return (state & MF_CHECKED);
}

bool writeOptionToFile(HWND hwnd) {
	HMENU hmenu = GetMenu(hwnd);
	FILE *file;
	fopen_s(&file, "data/Options.csv", "w+");

	fwprintf_s(file, L"%s=%d", L"ConfirmOnDelete", isMenuChecked(hmenu, ID_CONFIRMONDELETE));

	fclose(file);
	DestroyMenu(hmenu);
	return true;
}