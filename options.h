#pragma once

/*typedef struct {
	TCHAR name[256];
	BOOL value;
} Options;

Options OptList[4];*/
BOOL SetDefaultOption(HWND hwnd);

BOOL parseOptionFile(HWND hwnd) {
	FILE *file;
	fopen_s(&file, "data/Options.csv", "r");
	wchar_t input[1024], *str, *buffer;
	HMENU hmenu = GetMenu(hwnd);
	
	if (GetFileAttributes(L"data/Options.csv") == INVALID_FILE_ATTRIBUTES) {
		CreateDirectory(L".//data", NULL); //create a new folder

		HANDLE h = CreateFile(L".//data//Options.csv", GENERIC_READ, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		CloseHandle(h);
		SetDefaultOption(hwnd);
		
		return 0;
	}

	while (fgetws(input, 1024, (FILE*)file)) {
		str = wcstok_s(input, L"=", &buffer);

	//confirm on delete
		if (wcscmp(str, L"ConfirmOnDelete") == 0) {
			str = wcstok_s(NULL, L"\n", &buffer);
			int value = _wtoi(str);

			if(value) CheckMenuItem(hmenu, ID_CONFIRMONDELETE, MF_CHECKED);
			else CheckMenuItem(hmenu, ID_CONFIRMONDELETE, MF_UNCHECKED);
		}
	}

	fclose(file);
};

BOOL SetDefaultOption(HWND hwnd) {
	HMENU hmenu = GetMenu(hwnd);
	CheckMenuItem(hmenu, ID_CONFIRMONDELETE, MF_CHECKED);
	DestroyMenu(hmenu);
	return 0;
}

bool isMenuChecked(HMENU hmenu, UINT uId) {
	UINT state = GetMenuState(hmenu, uId, MF_BYCOMMAND);
	return (state & MF_CHECKED);
}

BOOL writeOptionToFile(HWND hwnd) {
	HMENU hmenu = GetMenu(hwnd);
	FILE *file;
	fopen_s(&file, "data/Options.csv", "w+");

	fwprintf_s(file, L"%s=%d", L"ConfirmOnDelete", (int*)isMenuChecked(hmenu, ID_CONFIRMONDELETE));

	fclose(file);
	DestroyMenu(hmenu);
	return 0;
}