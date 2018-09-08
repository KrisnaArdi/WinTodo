#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <vector>
#include "resource1.h"
#include "resource.h"
#include "listview.h"

//using visual styles
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version = '6.0.0.0' processorArchitecture = '*' publicKeyToken = '6595b64144ccf1df' language = '*'\"")

wchar_t ClassName[] = L"WindowClass";
std::vector<Task> TaskList;
MSG mesg; //window message

//the editbox procedure
LRESULT CALLBACK EditProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, UINT_PTR Subclass, DWORD_PTR refdata) {
	HWND parent = GetParent(hwnd); //get the main hwnd
	HWND AddBtn = GetDlgItem(parent, ID_ADD); //get the add button

	//if enter is pressed
	if (msg == WM_CHAR && wparam == VK_RETURN) {
		SendMessage(AddBtn, BM_CLICK, 0, 0); //trigger the add button click event
		return 0; //this will prevent annoying beeping sound when enter is pressed
	}

	LRESULT lres = DefSubclassProc(hwnd, msg, wparam, lparam);
	if (msg == WM_DESTROY) RemoveWindowSubclass(hwnd, EditProc, 0);

	return lres;
}

LRESULT CALLBACK LViewProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, UINT_PTR Subclass, DWORD_PTR refdata) {
	HWND parent = GetParent(hwnd); //get the main hwnd
	if (msg == WM_CHAR && wparam == VK_DELETE) {
		SendMessage(parent, WM_COMMAND, ID_DEL, 0);
		return 0;
	}

	LRESULT lres = DefSubclassProc(hwnd, msg, wparam, lparam);
	if (msg == WM_DESTROY) RemoveWindowSubclass(hwnd, EditProc, 0);
	return lres;
}

//about dialog procedure, check the dialog in the resource file
LRESULT CALLBACK AboutProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_INITDIALOG :
		return TRUE;

	case WM_COMMAND :
		switch (LOWORD(wparam)) {
			case IDOK : EndDialog(hwnd, IDOK);
		} break;

	case WM_CLOSE : EndDialog(hwnd, 0);

	default: return FALSE;
	}
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

	HWND  edit, AddBtn, lview, delBtn, delAllBtn, delFin, delUnFin;
	LVITEM lvi;
	RECT rc;

	switch (msg) {
		case WM_CLOSE: {
			if (isSaved)  PostQuitMessage(0);

			else {
				int confirm = MessageBox(NULL, L"Your list is still unsaved, save changes to the list?", L"Save your list?", MB_ICONQUESTION | MB_YESNOCANCEL);
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

			lview = CreateListView(hwnd, LVIEW);

			edit = CreateWindow(L"EDIT", L"",
				WS_VISIBLE | WS_CHILD | WS_BORDER,
				0, 0, rc.right - 102, editH,
				hwnd, (HMENU)ID_INPUT, NULL, NULL);

			AddBtn = CreateWindow(L"BUTTON", L"Add item",
				WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_USERBUTTON,
				rc.right - btnW, 0, btnW, editH,
				hwnd, (HMENU)ID_ADD, NULL, NULL);

			delBtn = CreateWindow(L"Button", L"Delete",
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				0, btnY, btnW, btnH, hwnd, (HMENU)ID_DEL, NULL, NULL);

			delAllBtn = CreateWindow(L"Button", L"Delete all",
				WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
				btnW, btnY, btnW, btnH, hwnd, (HMENU)ID_DELALL, NULL, NULL);

			delFin = CreateWindow(L"Button", L"Delete checked",
				WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
				(2 * btnW), btnY, btnW, btnH, hwnd, (HMENU)ID_DELFIN, NULL, NULL);

			delUnFin = CreateWindow(L"Button", L"Delete unchecked",
				WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
				(3 * btnW), btnY, btnW, btnH, hwnd, (HMENU)ID_DELUNFIN, NULL, NULL);

		/*Setting Font */
			//note : i don't know where i should put this code.
			HWND buttons[4] = {delBtn, delAllBtn, delFin, delUnFin }; //all button control
			for (int i = 0; i < 4; ++i) { //set font for all button control
				SendMessage(buttons[i], WM_SETFONT, (WPARAM)BtnFont, TRUE);
			}

			SendMessage(AddBtn, WM_SETFONT, (WPARAM)AddBtnFont, TRUE);
			SendMessage(edit, WM_SETFONT, (WPARAM)EditFont, TRUE); //edit font
			SendMessage(lview, WM_SETFONT, (WPARAM)LviewFont, TRUE); //listview font

			SendMessage(edit, EM_SETCUEBANNER, 0, LPARAM(L"Add a new item here")); //setting placeholder for edit 
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
						
						int iIndex;
						TCHAR text[256];
						HWND hEdit = ListView_GetEditControl(lview); //get the item
						iIndex = SendMessage(lview, LVM_GETNEXTITEM, -1, LVNI_FOCUSED);
						if (iIndex == -1) break;

						if ((TCHAR)mesg.wParam == 0x1b) return 0; //if user press esc then cancel change
				
						GetWindowText(hEdit, text, sizeof(text)); //get item text
						if (wcslen(text) == 0) return 0; //if the string empty then cancel change wcslen is to check wstring length
						
						lvi.pszText = text;
						lvi.iSubItem = 0;
						SendMessage(lview, LVM_SETITEMTEXT, (WPARAM)iIndex, (LPARAM)&lvi); //set the item text
						
						if (wcscmp(TaskList[iIndex].name, text) != 0) {
							ToggleUnsavedTitle(hwnd, 0);
						}
						wcscpy_s(TaskList[iIndex].name, text);	
				}

				//this is to process the checkbox message
					if(pnm->uChanged & LVIF_STATE) {
						switch (pnm->uNewState) {
							case INDEXTOSTATEIMAGEMASK(2): {// item was checked

								TaskList[pnm->iItem].status = 1; //change status in the vector

								ToggleTaskStatus(lview, pnm->iItem, 1); //changing the text in status column
							}
							break;
							case INDEXTOSTATEIMAGEMASK(1): { //item was unchecked
								TaskList[pnm->iItem].status = 0;
								ToggleTaskStatus(lview, pnm->iItem, 0);	
							}
							break;
						}
						
					}
				}
			break;
			}
		}
		break;

		case WM_COMMAND: {
			switch (LOWORD(wparam)) {
				case ID_ADD: {
					edit = GetDlgItem(hwnd, ID_INPUT);
					lview = GetDlgItem(hwnd, LVIEW);
					wchar_t pszText[256];
					int iItem;
			
					GetDlgItemText(hwnd, ID_INPUT, pszText, 256); //get the text from input
			
				//return an error if the input is blank
					if (pszText[0] == 0) {
					MessageBox(NULL, L"You cannot create an empty task!", L"Information", MB_ICONERROR | MB_OK);
					break;
				}

				//create a new Task class and insert it to the vector
					Task newTask;
					wcscpy_s(newTask.name, pszText);
					newTask.status = 0; //status is absolutely unfinished 
					TaskList.insert(TaskList.begin(), newTask);

					iItem = SendMessage(lview, LVM_GETITEMCOUNT, 0, 0); //get number of listview item
					lvi.mask = LVIF_TEXT;
					lvi.iItem = 0;
					lvi.iSubItem = 0;
					lvi.pszText = pszText;
					ListView_InsertItem(lview, &lvi);

					lvi.iSubItem = 1;
					lvi.pszText = L"Unfinished";
					ListView_SetItem(lview, &lvi);
	
					SetDlgItemText(hwnd, ID_INPUT, L"");
					SetFocus(edit); //focus to edit control

					ToggleUnsavedTitle(hwnd, 0); //toggling the title 
				}
				break;	

				case ID_DEL: {
					lview = GetDlgItem(hwnd, LVIEW);
					int iSelect = SendMessage(lview, LVM_GETNEXTITEM, -1, LVNI_SELECTED);

					if (iSelect == -1) MessageBox(NULL, L"Select item first before delete it by clicking the item!", L"Information", MB_ICONINFORMATION | MB_OK);
				//if an item is not selected	
					else {
						SendMessage(lview, LVM_DELETEITEM, iSelect, 0); //delete the task from the listview
						TaskList.erase(TaskList.begin() + iSelect); //delete the task from the vector
						ToggleUnsavedTitle(hwnd, 0); //toggling the title
					}
				}
				break;

				case ID_DELALL: {
					lview = GetDlgItem(hwnd, LVIEW);
					int iItem = SendMessage(lview, LVM_GETITEMCOUNT, 0, 0);
	
					if (iItem == 0) {
						MessageBox(NULL, L"There is no item!", L"ERROR!", MB_ICONERROR | MB_OK);
						break;
					}
			
				//make a convirmation box
					int confirm = MessageBox(NULL, L"You are about to delete all task items, are you sure?", L"Confirmation", MB_ICONQUESTION | MB_YESNO);
				//user press yes
					if (confirm == IDYES) {
						SendMessage(lview, LVM_DELETEALLITEMS, 0, 0);
						TaskList.clear();
						MessageBox(NULL, L"All items has been deleted", L"Information", MB_ICONINFORMATION | MB_OK);\

						ToggleUnsavedTitle(hwnd, 0); 
					}
				//user press no
					if (confirm == IDNO) break;
				}
				break;

				case ID_DELFIN: {
					lview = GetDlgItem(hwnd, LVIEW);
					std::vector<int> tmp; //this is a vector of finished item index number
			
					int confirm = MessageBox(NULL, L"All checked items is about to be deleted", L"Confirmation", MB_ICONQUESTION |MB_OKCANCEL);
					
					if (confirm == IDOK) {
						int i = TaskList.size() - 1;
					//use reverse loop to avoid error when deleting later items	
						for (std::vector<Task>::iterator it = TaskList.end()-1; it != TaskList.begin()-1; it--) { 
							if (it->status) { //if task finished
							//TaskList.erase(TaskList.begin() + i);
								TaskList.erase(it);
								SendMessage(lview, LVM_DELETEITEM, i, 0);
							}
							i--;
						} 

					//other loop method
						/*for (int i = TaskList.size()-1; i >= 0; i--) {
							if (TaskList[i].status) {
								TaskList.erase(TaskList.begin() + i);
								SendMessage(lview, LVM_DELETEITEM, i, 0);
							}
						}*/

						ToggleUnsavedTitle(hwnd, false);
						MessageBox(NULL, L"All checked items has been deleted", L"Information", MB_ICONINFORMATION | MB_OK);
					}
				}
				break;
	
				case ID_DELUNFIN: {
				//this one has only few change compared to delfin
					lview = GetDlgItem(hwnd, LVIEW);
					std::vector<int> tmp; //this is a vector of finished item index number

					int confirm = MessageBox(NULL, L"All unchecked items is about to be deleted", L"Confirmation", MB_ICONQUESTION | MB_OKCANCEL);
					
					
					if (confirm == IDOK) {
						int i = TaskList.size() - 1;

						for (std::vector<Task>::iterator it = TaskList.end()-1; it != TaskList.begin()-1; --it) {
							if(it->status == 0) { //if task unfinished
								TaskList.erase(it);
								SendMessage(lview, LVM_DELETEITEM, i, 0);
							}
							--i;
						}

						ToggleUnsavedTitle(hwnd, 0);
						MessageBox(NULL, L"All unchecked items has been deleted", L"Information", MB_ICONINFORMATION | MB_OK);
					} 
			}
				break;

				case ID_FILE_SAVECHANGES: {
				WriteTaskToFile(TaskList);

				ToggleUnsavedTitle(hwnd, 1);
				MessageBox(NULL, L"Changes saved!", L"Information", MB_ICONINFORMATION | MB_OK);
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
				
		}
	}
		break;

		case WM_SIZE: {
			edit = GetDlgItem(hwnd, ID_INPUT);
			AddBtn = GetDlgItem(hwnd, ID_ADD);
			lview = GetDlgItem(hwnd, LVIEW);
			delBtn = GetDlgItem(hwnd, ID_DEL);
			delAllBtn = GetDlgItem(hwnd, ID_DELALL);
			delFin = GetDlgItem(hwnd, ID_DELFIN);
			delUnFin = GetDlgItem(hwnd, ID_DELUNFIN);

			GetClientRect(hwnd, &rc); //getting the client of the hwnd
			int btnY = rc.bottom - btnH;

			SetWindowPos(edit, 0, 0, 0, rc.right - btnW, editH, SWP_NOZORDER); //resize the input
			SetWindowPos(AddBtn, 0, rc.right - btnW, 0, btnW, editH, SWP_NOZORDER | SWP_NOSIZE); //resize the button
			SetWindowPos(lview, 0, 0, editH, rc.right, rc.bottom - (btnH+editH), SWP_NOZORDER); //resize the listview
			SetWindowPos(delBtn, 0, 0, btnY, btnW, btnH, SWP_NOZORDER | SWP_NOSIZE); //resize the delete button
			SetWindowPos(delAllBtn, 0, btnW, btnY, btnW, btnH, SWP_NOZORDER | SWP_NOSIZE); //resize the delete all button
			SetWindowPos(delFin, 0, (2*btnW), btnY, btnW, btnH, SWP_NOZORDER | SWP_NOSIZE); //delete finished button
			SetWindowPos(delUnFin, 0, (3 * btnW), btnY, btnW, btnH, SWP_NOZORDER | SWP_NOSIZE); //delete unfinished button	
		}
		break;

		default: return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hins, HINSTANCE hprev, LPSTR lpcmd, int ncmd) {
	WNDCLASSEX wc;
	HWND hwnd;
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
		CW_USEDEFAULT, CW_USEDEFAULT, 521, 620,
		NULL, NULL, hins, NULL);

	if (hwnd == NULL) {
		MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

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

	return mesg.wParam;
}