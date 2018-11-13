#pragma once

static int TaskNameW = 330;
static int TaskStatusW = 120;
typedef struct {
	//remember string in c cannot be modifiedable by just using = (this is to remind myself)
	TCHAR name[256];
	int status;
} Task;

bool InitColumn(HWND);

//toggling the title 
void ToggleUnsavedTitle(HWND hwnd, bool saved) {
	TCHAR title[] = L"MyTodo (unsaved)";
	if (!saved) SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)title);
	else SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)WindowTitle);

	isSaved = saved;
}

HWND CreateListView(HWND parent, int id) {
	INITCOMMONCONTROLSEX icex;
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	RECT rc;
	GetClientRect(parent, &rc);

	HWND lview = CreateWindow(WC_LISTVIEW, L"",
		WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_VISIBLE ,
		0, editH, rc.right - rc.left,
		rc.bottom - (btnH + editH), parent, (HMENU)id, NULL, NULL);

	ListView_SetExtendedListViewStyle(lview, LVS_EX_INFOTIP|LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES);

	InitColumn(lview);

	return lview;
}

bool InitColumn(HWND lview) {
	LVCOLUMN lvc;

	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 330;
	lvc.pszText = L"Task Name";

	ListView_InsertColumn(lview, 1, &lvc);

	lvc.cx = 120;
	lvc.pszText = L"Status";
	ListView_InsertColumn(lview, 1, &lvc);

	return true;
}

//import task from file and insert it to the vector
bool LoadTaskFromFile(std::vector<Task> *TaskList, HWND lview) {

	FILE *file;
	fopen_s(&file,"data/data.csv", "r");
	TCHAR input[1024], *str, *buffer;

//check if file does not exist
	if (GetFileAttributes(_T("data/data.csv")) == INVALID_FILE_ATTRIBUTES) {
		CreateDirectory(_T(".//data"), NULL); //create a new folder

	//create a new file name data.csv in data folder
		HANDLE h = CreateFile(_T(".//data//data.csv"), GENERIC_READ, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		CloseHandle(h);
		return false;

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
	return true;
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

		(it->status) ? status = _T("Finished") : status = _T("Unfinished");

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

	for (std::vector<Task>::iterator it = TaskList.end()-1; it != TaskList.begin()-1; --it) {
		fwprintf(File, L"%s;%d;\n", it->name, it->status);
	}
 	fclose(File);
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

//toggle wether task finished or not
void ToggleTaskStatus(std::vector<Task> &TaskList, HWND lview, int iItem, int status) {
	LVITEM lvi;

	lvi.mask = LVIF_TEXT;
	lvi.iItem = iItem;
	lvi.iSubItem = 1;

	TaskList[iItem].status = status;
	(status) ? lvi.pszText = _T("Finished") : lvi.pszText = _T("Unfinished");

	ListView_SetItem(lview, &lvi);
}

bool AddTask(std::vector<Task> &TaskList, HWND lview, TCHAR pszText[256]) {
	LVITEM lvi;

	//create a new Task class and insert it to the vector
	Task newTask;
	wcscpy_s(newTask.name, pszText);
	newTask.status = 0; //status is absolutely unfinished 
	TaskList.insert(TaskList.begin(), newTask);

	int iItem = SendMessage(lview, LVM_GETITEMCOUNT, 0, 0); //get number of listview item
	lvi.mask = LVIF_TEXT;
	lvi.iItem = 0;
	lvi.iSubItem = 0;
	lvi.pszText = pszText;
	ListView_InsertItem(lview, &lvi);

	lvi.iSubItem = 1;
	lvi.pszText = _T("Unfinished");
	ListView_SetItem(lview, &lvi);

		return true;
}

bool ProcessLabelEdit(std::vector<Task> &TaskList, int iIndex, HWND lview) {
	LVITEM lvi;
	TCHAR text[256];
	HWND hEdit = ListView_GetEditControl(lview); //get the item
	GetWindowText(hEdit, text, sizeof(text)); //get item text
	if (wcslen(text) == 0) return 0;

	lvi.pszText = text;
	lvi.iSubItem = 0;
	SendMessage(lview, LVM_SETITEMTEXT, (WPARAM)iIndex, (LPARAM)&lvi); //set the item text

	if (wcscmp(TaskList[iIndex].name, text) != 0) { //if there is changes
		wcscpy_s(TaskList[iIndex].name, text); //copy the changes to the task name
		return 1;
	}
	return 0;
}

bool DeleteTaskBasedOnStatus(std::vector<Task> &TaskList,HWND lview, int status) {
	int i = TaskList.size() - 1;
	//use reverse loop to avoid error when deleting later items	
	for (std::vector<Task>::iterator it = TaskList.end() - 1; it != TaskList.begin() - 1; it--) {
		if (it->status == status) {
		//TaskList.erase(TaskList.begin() + i);
			TaskList.erase(it);
			SendMessage(lview, LVM_DELETEITEM, i, 0);
		}
		i--;
	}

	//other loop method
		/*for (int i = TaskList.size()-1; i >= 0; i--) {
			if (TaskList[i].status == status) {
				TaskList.erase(TaskList.begin() + i);
				SendMessage(lview, LVM_DELETEITEM, i, 0);
			}
		}*/
	return status + 1;
};
