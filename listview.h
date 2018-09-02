#pragma once

//toggling the title 
void ToggleUnsavedTitle(HWND hwnd, bool saved) {
	wchar_t title[] = L"MyTodo (unsaved)";
	if (!saved) SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)title);
	else SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)WindowTitle);

	isSaved = saved;
}

typedef struct {
	//remember string inside struct cannot be modifiedable by just using = (this is to remind myself)
	wchar_t name[256];
	int status;
} Task;

BOOL InitColumn(HWND);

HWND CreateListView(HWND parent, int id) {
	INITCOMMONCONTROLSEX icex;
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	RECT rc;
	GetClientRect(parent, &rc);

	HWND lview = CreateWindow(WC_LISTVIEW, L"",
		WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE | WS_BORDER,
		0, editH, rc.right - rc.left,
		rc.bottom - (btnH + editH), parent, (HMENU)id, NULL, NULL);

	ListView_SetExtendedListViewStyle(lview, LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES);

	InitColumn(lview);

	return lview;
}

BOOL InitColumn(HWND lview) {
	LVCOLUMN lvc;

	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 380;
	lvc.pszText = L"Task Name";

	ListView_InsertColumn(lview, 1, &lvc);

	lvc.cx = 120;
	lvc.pszText = L"Status";
	ListView_InsertColumn(lview, 1, &lvc);

	return TRUE;
}

//import task from file and insert it to the vector
BOOL LoadTaskFromFile(std::vector<Task> *TaskList, HWND lview) {

	FILE *file;
	fopen_s(&file,"data/data.csv", "r");
	wchar_t input[1024], *str, *buffer;

//check if file does not exist
	if (GetFileAttributes(L"data/data.csv") == INVALID_FILE_ATTRIBUTES) {
		CreateDirectory(L".//data", NULL); //create a new folder

	//create a new file name data.csv in data folder
		HANDLE h = CreateFile(L".//data//data.csv", GENERIC_READ, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		CloseHandle(h);
		return 0;

		/*
		- this one is to write the data in the appdata/roaming directory
		- SHGetFolderPath & SHCreateDirectory require shlobj.h
		- PathAppend require shlwapi.h
		- all require shlwapi.lib

		TCHAR szPath[MAX_PATH];

		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath))) {
		PathAppend(szPath, L"\\ListviewApp");
		SHCreateDirectory(NULL, szPath); //create an empty folder
		PathAppend(szPath, L"\\data.csv"); //append the file
		//you could straightly append to the file dir
		}
		HANDLE ha = CreateFile(szPath, GENERIC_READ, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		CloseHandle(ha);
		*/
	}
	
	/*
	// c way to check if file does not exist
	struct stat st;
	if (stat("data/data.csv", &st) == 1 ) {}
	
	*/

	while (fgetws(input, 1024, (FILE*)file)) {
		Task temp;

	//task name
		str = wcstok_s(input, L";", &buffer);
		wcscpy_s(temp.name, str);

	//task status
		str = wcstok_s(NULL, L";", &buffer);
		temp.status = _wtoi(str);
	 
		TaskList->insert(TaskList->begin(), temp); //insert the task to the first item of the vector
	}

	fclose(file);
	return 0;
}

//render task from the vector to the listview
void renderTask(std::vector<Task> TaskList, HWND lview) {
	LVITEM lvi;
	lvi.mask = LVIF_TEXT;
	LPWSTR status;
	int i = 0;

	for(std::vector<Task>::iterator it = TaskList.begin(); it != TaskList.end(); ++it) {
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.pszText = it->name;

		ListView_InsertItem(lview, &lvi);

		ListView_SetCheckState(lview, i, it->status);

		(it->status) ? status = L"Finished" : status = L"Unfinished";

		lvi.iItem = i;
		lvi.iSubItem = 1;
		lvi.pszText = status;
		ListView_SetItem(lview, &lvi);
		++i;
	}
	HWND hwnd = GetParent(lview);
	ToggleUnsavedTitle(hwnd, 1);
}

//write the task from the listview to the vector
void WriteTaskToFile(std::vector<Task> TaskList) {
	FILE *File;
	fopen_s(&File, "data/data.csv", "w+");

	/*for (std::vector<Task>::iterator it = TaskList.begin(); it != TaskList.end(); ++it) {
		fwprintf(File, L"%s;%d;\n", it->name, it->status);
	}*/

	for (std::vector<Task>::iterator it = TaskList.end()-1; it != TaskList.begin()-1; --it) {
		fwprintf(File, L"%s;%d;\n", it->name, it->status);
	}
 	fclose(File);
}

//toggle wether task finished or not
void ToggleTaskStatus(HWND lview, int iItem, int status) {
	LVITEM lvi;

	lvi.mask = LVIF_TEXT;
	lvi.iItem = iItem;
	lvi.iSubItem = 1;

	(status) ? lvi.pszText = L"Finished" : lvi.pszText = L"Unfinished";

	ListView_SetItem(lview, &lvi);
	HWND hwnd = GetParent(lview);
	ToggleUnsavedTitle(hwnd, 0);

}

//processing the custom draw message
LRESULT ProcessCustomDraw(LPARAM lparam, std::vector<Task> TaskList) {
	LPNMLVCUSTOMDRAW lpl = (LPNMLVCUSTOMDRAW)lparam;
	switch (lpl->nmcd.dwDrawStage) {
		case CDDS_PREPAINT:	{
			return CDRF_NOTIFYITEMDRAW;
		}
		break;
	
		case CDDS_ITEMPREPAINT: {
			return CDRF_NOTIFYSUBITEMDRAW;
		}
		break;

		case CDDS_SUBITEM | CDDS_ITEMPREPAINT: {
		//if the task not finished yet
			if (TaskList[lpl->nmcd.dwItemSpec].status == 0) lpl->clrText = RGB(0, 0, 0);
			else {
				lpl->clrText = RGB(80, 80, 80);
				lpl->clrTextBk = RGB(205, 205, 220);
			}

			return CDRF_NEWFONT;
		}
		break;

		default: return CDRF_DODEFAULT;
	}
}
