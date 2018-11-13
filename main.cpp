#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include <shellapi.h>
#include <vector>
#include "resource1.h"
#include "resource.h"
#include "listview.h"
#include "options.h"

//using visual styles
//visual styles has been added through  manifest.xml
//#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version = '6.0.0.0' processorArchitecture = '*' publicKeyToken = '6595b64144ccf1df' language = '*'\"")

TCHAR ClassName[] = _T("WindowClass");
std::vector<Task> TaskList;
 MSG mesg; //window message
 NOTIFYICONDATA iconData;
 HWND hwnd;
 HMENU hmenu;

 void minimize() { //minimize window
	 Shell_NotifyIcon(NIM_ADD, &iconData);
	 ShowWindow(hwnd, SW_HIDE);
 }

 void restore() { //restore window
	 Shell_NotifyIcon(NIM_DELETE, &iconData);
	 ShowWindow(hwnd, SW_SHOW);
 }

 void initNotifyIconData() { //initializing the shell notification
	 memset(&iconData, 0, sizeof(iconData));

	 iconData.cbSize = sizeof(NOTIFYICONDATA);
	 iconData.hWnd = hwnd;
	 iconData.uID = ID_TRAY_APP_ICON;
	 iconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	 iconData.uCallbackMessage = WM_TRAYICON;
	 iconData.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));

	 //setting the tooltip text
	 wcscpy_s(iconData.szTip, TEXT("WinTodo"));
 }

//the editbox procedure
LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, UINT_PTR Subclass, DWORD_PTR refdata) {
	HWND parent = GetParent(hwnd); //get the main hwnd
	HWND AddBtn = GetDlgItem(parent, ID_ADD); //get the add button

	//if enter is pressed
	if (msg == WM_CHAR && wparam == VK_RETURN) {
		 SendMessage(AddBtn, BM_CLICK, 0, 0); //trigger the add button click event
		return 0; 
	}

	LRESULT lres = DefSubclassProc(hwnd, msg, wparam, lparam);
	if (msg == WM_DESTROY) RemoveWindowSubclass(hwnd, EditProc, 0);

	return lres;
}

//list view procedure
LRESULT CALLBACK LViewProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, UINT_PTR Subclass, DWORD_PTR refdata) {
	HWND parent = GetParent(hwnd); //get the main window

	if (msg == WM_CHAR && wparam == VK_DELETE) {
		SendMessage(parent, WM_COMMAND, ID_DEL, 0);
		return 0;
	}

	LRESULT lres = DefSubclassProc(hwnd, msg, wparam, lparam);
	if (msg == WM_DESTROY) RemoveWindowSubclass(hwnd, EditProc, 0);
	return lres;
}

//about dialog procedure, also used for help check the dialog in the resource file
LRESULT CALLBACK AboutProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_INITDIALOG: {
		HWND HelpEdit = GetDlgItem(hwnd, IDC_EDIT1);
		SetWindowText(HelpEdit, HelpText);
		return TRUE;
	}

	case WM_COMMAND :
		switch (LOWORD(wparam)) {
			case IDOK : EndDialog(hwnd, IDOK);
		} break;

	case WM_CLOSE : EndDialog(hwnd, 0);

	default: return FALSE;
	}
	return TRUE;
}

/////  Main Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	HWND  edit, AddBtn, lview, TBar;
	RECT rc;
	
	switch (msg) {
		case WM_CLOSE: {
			writeOptionToFile(hwnd);
			if (isSaved)  PostQuitMessage(0);

			else {
				int confirm = MessageBox(NULL, _T("Your list is still unsaved, save changes to the list?"), _T("Save your list?"), MB_ICONQUESTION | MB_YESNOCANCEL);
				if (confirm == IDYES) {
					WriteTaskToFile(TaskList); //saving changes before closing files
					PostQuitMessage(0);
				}
				else if (confirm == IDNO)  PostQuitMessage(0);
				else if (confirm == IDCANCEL)  break;
			}
		}
		break;

		case WM_CREATE: {
			GetClientRect(hwnd, &rc);
			int btnY = rc.bottom - btnH;

			hmenu = CreatePopupMenu(); //create a popup menu
			AppendMenu(hmenu, MF_STRING, ID_TRAY_MENU_EXIT, TEXT("Exit")); //append menu item

			lview = CreateListView(hwnd, LVIEW);
			
			edit = CreateWindow(_T("EDIT"), _T(""),
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
				0, 0, rc.right - AddBtnW, editH,
				hwnd, (HMENU)ID_INPUT, NULL, NULL);

			AddBtn = CreateWindow(_T("BUTTON"), _T(""),
				WS_VISIBLE |WS_BORDER| WS_CHILD |BS_OWNERDRAW,
				rc.right - AddBtnW, 0, AddBtnW+1, editH,
				hwnd, (HMENU)ID_ADD, NULL, NULL);
			
		/* ------ Creating Toolbar -------------------------------- */
			TBBUTTON tbb[7];
			TBar = CreateWindow(TOOLBARCLASSNAME, NULL, WS_VISIBLE | WS_CHILD | TBSTYLE_LIST | TBSTYLE_TOOLTIPS,
				0, 0, 0, 0,
				hwnd, (HMENU)ID_TBAR, GetModuleHandle(NULL), NULL);
			SendMessage(TBar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

			HIMAGELIST imgList = ImageList_Create(20, 20, ILC_COLOR32, 1, 5);
			ImageList_AddIcon(imgList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DELETE)));
			ImageList_AddIcon(imgList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DELUNCHECK)));
			ImageList_AddIcon(imgList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DELFIN)));
			ImageList_AddIcon(imgList, LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_DELALL)));
			SendMessage(TBar, TB_SETIMAGELIST, 0, (LPARAM)imgList);		
			ZeroMemory(tbb, sizeof(tbb));

			tbb[0].iBitmap = 0;
			tbb[0].idCommand = ID_DEL;
			tbb[0].fsState = TBSTATE_ENABLED;
			tbb[0].fsStyle = TBSTYLE_BUTTON | TBSTYLE_FLAT;

			tbb[1].fsStyle = BTNS_SEP; //separator between 2 button
			tbb[1].iBitmap = 8;

			tbb[2].iBitmap = 1;
			tbb[2].idCommand = ID_DELUNFIN;
			tbb[2].fsState = TBSTATE_ENABLED;
			tbb[2].fsStyle = TBSTYLE_BUTTON | TBSTYLE_FLAT;

			tbb[3].fsStyle = BTNS_SEP;
			tbb[3].iBitmap = 8;

			tbb[4].iBitmap = 2;
			tbb[4].idCommand = ID_DELFIN;
			tbb[4].fsState = TBSTATE_ENABLED;
			tbb[4].fsStyle = TBSTYLE_BUTTON | TBSTYLE_FLAT;

			tbb[5].fsStyle = BTNS_SEP;
			tbb[5].iBitmap = 8;

			tbb[6].iBitmap = 3;
			tbb[6].idCommand = ID_DELALL;
			tbb[6].fsState = TBSTATE_ENABLED;
			tbb[6].fsStyle = TBSTYLE_BUTTON | TBSTYLE_FLAT;
			
			SendMessage(TBar, TB_SETMAXTEXTROWS, 0, 0);
			SendMessage(TBar, TB_ADDBUTTONS, 7, (LPARAM)&tbb);
			SendMessage(TBar, TB_SETEXTENDEDSTYLE, 0, (LPARAM)TBSTYLE_EX_MIXEDBUTTONS);
			SendMessage(TBar, TB_AUTOSIZE, 0, 0);
			ShowWindow(TBar, SW_SHOW);

			parseOptionFile(hwnd);

		/*---- Setting Font ---------------------------*/
			SendMessage(edit, WM_SETFONT, (WPARAM)EditFont, TRUE); //edit font
			SendMessage(lview, WM_SETFONT, (WPARAM)LviewFont, TRUE); //listview font

			SendMessage(edit, EM_SETCUEBANNER, 0, LPARAM(_T("Add a new item here"))); //setting placeholder for edit 
			LoadTaskFromFile(&TaskList, lview); //loading task from file
			renderTask(TaskList, lview); //render the task
		
		//subclassing dialog to its procedure
			SetWindowSubclass(lview, LViewProc, 0, 0);
			SetWindowSubclass(edit, EditProc, 0, 0); 
		}
		break;

		case WM_NOTIFY: {
			switch (LOWORD(wparam)) {
				case LVIEW: {
					lview = GetDlgItem(hwnd, LVIEW);
					LPNMLISTVIEW pnm = (LPNMLISTVIEW)lparam;

				//processing custom draw
					if (pnm->hdr.hwndFrom == lview && pnm->hdr.code == NM_CUSTOMDRAW) {
					return ProcessCustomDraw(lparam, TaskList); //return LRESULT of custom draw function
				}
			
				//processing doubleclick
					if (((LPNMHDR)lparam)->code == NM_DBLCLK) {
						int iIndex = SendMessage(lview, LVM_GETNEXTITEM, -1, LVNI_FOCUSED);
						if (iIndex == -1) break;
						SendMessage(lview, LVM_EDITLABEL, (WPARAM)iIndex, NULL); //call edit label			
					}

				//processing the end of label edit
					if (((LPNMHDR)lparam)->code == LVN_ENDLABELEDIT) {
						int iIndex = SendMessage(lview, LVM_GETNEXTITEM, -1, LVNI_FOCUSED);
						if (SendMessage(lview, LVM_GETNEXTITEM, -1, LVNI_FOCUSED) == -1) break;

						if ((TCHAR)mesg.wParam == 0x1b) return 0; //if user press esc then cancel change
						int process = ProcessLabelEdit(TaskList, iIndex, lview);
						if(process)ToggleUnsavedTitle(hwnd, 0);
				}

				//processing checkbox message
					if(pnm->uChanged & LVIF_STATE) {
						switch (pnm->uNewState) {
							case INDEXTOSTATEIMAGEMASK(2): {// item was checked
								ToggleTaskStatus(TaskList, lview, pnm->iItem, 1); //changing the text in status column
								ToggleUnsavedTitle(hwnd, 0);
							}
							break;

							case INDEXTOSTATEIMAGEMASK(1): { //item was unchecked
								ToggleTaskStatus(TaskList, lview, pnm->iItem, 0);	
								ToggleUnsavedTitle(hwnd, 0);
							}
							break;
						}
						
					}
				}
			break;
			}

		//tooltip
			if (((LPNMHDR)lparam)->code == TTN_GETDISPINFO) {
				LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT)lparam;
				lpttt->hinst = GetModuleHandle(NULL);
				UINT_PTR idButton = lpttt->hdr.idFrom;
				switch (idButton) {
				case ID_DEL: {
					lpttt->lpszText = _TEXT("Delete task");
					break;
				}
				case ID_DELUNFIN: {
					lpttt->lpszText = _TEXT("Delete unchecked tasks");
					break;
				}
				case ID_DELFIN: {
					lpttt->lpszText = _TEXT("Delete checked tasks");
					break;
				}
				case ID_DELALL: {
					lpttt->lpszText = _TEXT("Delete all tasks");
					break;
				}
				}
			}
		}
		break;

		case WM_COMMAND: {
			switch (LOWORD(wparam)) {
				case ID_ADD: {
					edit = GetDlgItem(hwnd, ID_INPUT);
					lview = GetDlgItem(hwnd, LVIEW);
					TCHAR pszText[256];
			
					GetDlgItemText(hwnd, ID_INPUT, pszText, 256); //get the text from input		

				//return an error if the input is blank
					if (pszText[0] == 0) {
					MessageBox(NULL, _T("You cannot create an empty task!"), _T("Information"), MB_ICONERROR | MB_OK);
					break;
				}

				//create a new task
					AddTask(TaskList, lview, pszText);

					ToggleUnsavedTitle(hwnd, 0); //toggling the title 
					SetDlgItemText(hwnd, ID_INPUT, _T(""));
					SetFocus(edit); //focus to edit control
				}
				break;	

				case ID_DEL: {
					lview = GetDlgItem(hwnd, LVIEW);
					int iSelect = SendMessage(lview, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
					HMENU hmenu = GetMenu(hwnd);

					if (iSelect == -1) MessageBox(NULL, _T("Select item first before delete it by clicking the item!"), _T("Information"), MB_ICONINFORMATION | MB_OK);
				//if an item is not selected	
					else {
						if (isMenuChecked(hmenu, ID_CONFIRMONDELETE)) { //confirmation box
							int confirm = MessageBox(NULL, _T("Delete this task?"), _T("Confirmation"), MB_ICONQUESTION | MB_YESNO);
							if (confirm == IDNO) break;
						}
						while (iSelect != -1) {
							SendMessage(lview, LVM_DELETEITEM, iSelect, 0); //delete the task from the listview
							TaskList.erase(TaskList.begin() + iSelect); //delete the task from the vector
							iSelect = SendMessage(lview, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
						}
						ToggleUnsavedTitle(hwnd, 0); //toggling the title
					}
				}
				break;

				case ID_DELALL: {
					lview = GetDlgItem(hwnd, LVIEW);
					int iItem = SendMessage(lview, LVM_GETITEMCOUNT, 0, 0);
	
					if (iItem == 0) {
						MessageBox(NULL, _T("There is no item!"), _T("ERROR!"), MB_ICONERROR | MB_OK);
						break;
					}
			
				//make a confirmation box
					int confirm = MessageBox(NULL, _T("You are about to delete all task items, are you sure?"), _T("Confirmation"), MB_ICONQUESTION | MB_YESNO);
				//user press yes
					if (confirm == IDYES) {
						SendMessage(lview, LVM_DELETEALLITEMS, 0, 0);
						TaskList.clear();
						MessageBox(NULL, _T("All items has been deleted"), _T("Information"), MB_ICONINFORMATION | MB_OK);
						ToggleUnsavedTitle(hwnd, 0); 
					}
				//user press no
					if (confirm == IDNO) break;
				}
				break;

				case ID_DELFIN: {
					lview = GetDlgItem(hwnd, LVIEW);
			
					int confirm = MessageBox(NULL, _T("All checked items is about to be deleted"), _T("Confirmation"), MB_ICONQUESTION |MB_OKCANCEL);
					
					if (confirm == IDOK) {
						DeleteTaskBasedOnStatus(TaskList, lview, true);
						ToggleUnsavedTitle(hwnd, false);
						MessageBox(NULL, _T("All checked items has been deleted"), _T("Information"), MB_ICONINFORMATION | MB_OK);
					}
				}
				break;
	
				case ID_DELUNFIN: {
					lview = GetDlgItem(hwnd, LVIEW);

					int confirm = MessageBox(NULL, _T("All unchecked items is about to be deleted"), _T("Confirmation"), MB_ICONQUESTION | MB_OKCANCEL);
					if (confirm == IDOK) {
						DeleteTaskBasedOnStatus(TaskList, lview, false);
						ToggleUnsavedTitle(hwnd, 0);
						MessageBox(NULL, _T("All unchecked items has been deleted"), _T("Information"), MB_ICONINFORMATION | MB_OK);
					} 
			}
				break;

				case ID_FILE_SAVECHANGES: {
				WriteTaskToFile(TaskList);

				ToggleUnsavedTitle(hwnd, 1);
				MessageBox(NULL, _T("Changes saved!"), _T("Information"), MB_ICONINFORMATION | MB_OK);
				}
				break;
				
				case ID_FILE_MINIMIZETOTRAY: {
					minimize();
				}
				break;

				case ID_FILE_EXIT: {
				PostMessage(hwnd, WM_CLOSE, 0, 0);
				}
				break;

				case ID_HELP_ABOUT: {
					//calling a new dialog
					DialogBox(GetModuleHandle(NULL),
						MAKEINTRESOURCE(IDD_ABOUT), hwnd, (DLGPROC)AboutProc);
				}
				break;

				case ID_HELP_HELP: {
					DialogBox(GetModuleHandle(NULL),
						MAKEINTRESOURCE(IDD_HELP), hwnd, (DLGPROC)AboutProc);
				}

				case ID_CONFIRMONDELETE : {
					HMENU hmenu = GetMenu(hwnd);
					if(isMenuChecked(hmenu, ID_CONFIRMONDELETE)) CheckMenuItem(hmenu, ID_CONFIRMONDELETE, MF_UNCHECKED);
					else  CheckMenuItem(hmenu, ID_CONFIRMONDELETE, MF_CHECKED);
				}break;

				case ID_SHOWADDBUTTON: {
					HWND AddBtn = GetDlgItem(hwnd, ID_ADD);
					HWND edit = GetDlgItem(hwnd, ID_INPUT);
					HMENU hmenu = GetMenu(hwnd);
 
					RECT rc;
					GetClientRect(hwnd, &rc);
					int width = rc.right;

					if (isMenuChecked(hmenu, ID_SHOWADDBUTTON)) { 
						CheckMenuItem(hmenu, ID_SHOWADDBUTTON, MF_UNCHECKED);
						AddBtnW = 0;
						ShowWindow(AddBtn, SW_HIDE);
					}
					else { 
						CheckMenuItem(hmenu, ID_SHOWADDBUTTON, MF_CHECKED);
						AddBtnW = btnW;
						ShowWindow(AddBtn, SW_RESTORE);
					}

					SetWindowPos(edit, 0, 0, 0, width - AddBtnW, editH, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
				}break;
				
		}
	}
		break;

		case WM_DRAWITEM: {
			switch ((UINT)wparam) {
			case ID_ADD: {
				LPDRAWITEMSTRUCT lpdis = (DRAWITEMSTRUCT*)lparam;
				int IconW = 24;
				int IconH = 24;
				HICON hIcon = (HICON)LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICONADD));
				RECT rc = lpdis->rcItem;
				SetBkColor(lpdis->hDC, COLOR_WINDOW);
				
				if (lpdis->itemState & ODS_SELECTED) {
					FillRect(lpdis->hDC, &lpdis->rcItem, (HBRUSH)COLOR_WINDOW);
					DrawIconEx(lpdis->hDC,
						((AddBtnW - IconW) / 2)+1,
						((AddBtnW - IconH) / 2)+1,
						(HICON)hIcon,
						24, 24, 0, NULL, DI_NORMAL);
				}
				else {
					FillRect(lpdis->hDC, &lpdis->rcItem, (HBRUSH)COLOR_WINDOW);
					DrawIconEx(lpdis->hDC,
						(AddBtnW - IconW) / 2,
						(AddBtnW - IconH) / 2,
						(HICON)hIcon,
						24, 24, 0, NULL, DI_NORMAL);
				}
				DrawEdge(lpdis->hDC, &lpdis->rcItem,
					(lpdis->itemState & ODS_SELECTED ? EDGE_SUNKEN : EDGE_RAISED), //chek the item state of the button and set the edge according to the state
					BF_RECT);
				return TRUE;
			}		
			}
		}break;

		case WM_SIZE: {
			edit = GetDlgItem(hwnd, ID_INPUT);
			AddBtn = GetDlgItem(hwnd, ID_ADD);
			lview = GetDlgItem(hwnd, LVIEW);
			TBar = GetDlgItem(hwnd, ID_TBAR);
			
			SendMessage(TBar, TB_AUTOSIZE, 0, 0);
			GetClientRect(hwnd, &rc); //getting the client of the hwnd
			int btnY = rc.bottom - 36;

			SetWindowPos(AddBtn, 0, rc.right - AddBtnW, TBarH, AddBtnW, editH, SWP_NOZORDER | SWP_NOSIZE); //repos the button
			SetWindowPos(edit, 0, 0, TBarH, rc.right - AddBtnW, editH, SWP_NOZORDER); //repos the input
			SetWindowPos(TBar, 0, 4, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			SetWindowPos(lview, 0, 0, TBarH+editH, rc.right, rc.bottom - (35+editH), SWP_NOZORDER); //repos the listview
		}
		break;

		case WM_TRAYICON: {
			switch (lparam) {
			case WM_LBUTTONUP: restore(); //restore window on left click
				break;

			case WM_RBUTTONDOWN: {
				POINT curPoint;
				GetCursorPos(&curPoint);

				// should SetForegroundWindow according
				 // to original poster so the popup shows on top
				SetForegroundWindow(hwnd);

				UINT clicked = TrackPopupMenu(
					hmenu,
					TPM_RETURNCMD | TPM_NONOTIFY, // don't send me WM_COMMAND messages about this window, instead return the identifier of the clicked menu item
					curPoint.x,
					curPoint.y,
					0, hwnd, NULL);


				if (clicked == ID_TRAY_MENU_EXIT) { //click the exit menu in the popup
					PostQuitMessage(0);
				}
				break;
			}
			}
			break;
		}

		default: return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hins, HINSTANCE hprev, LPSTR lpcmd, int ncmd) {
	WNDCLASSEX wc;
	BOOL bRet;
	HACCEL haccel;
	haccel = LoadAccelerators(hins, MAKEINTRESOURCE(IDR_ACCELERATOR1)); //binding windows with an accelerator

	memset(&wc, 0, sizeof(wc));

	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hins;
	wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	wc.hIconSm = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wc.lpszClassName = ClassName;

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,
		ClassName, WindowTitle, WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 471, 620,
		NULL, NULL, hins, NULL);

	if (hwnd == NULL) {
		MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// Initialize the NOTIFYICONDATA structure once
	initNotifyIconData();

	while (bRet = GetMessage(&mesg, NULL, 0, 0) > 0) {

		if (bRet == -1) {
			PostQuitMessage(0);
		}

		else {
			if (!TranslateAccelerator(hwnd, haccel, &mesg)) {
				TranslateMessage(&mesg);
				DispatchMessage(&mesg);
			}
		}
	}

	// Once you get the quit message, before exiting the app,
// clean up and remove the tray icon
	if (!IsWindowVisible(hwnd)) {
		Shell_NotifyIcon(NIM_DELETE, &iconData);
	}

	return mesg.wParam;
}