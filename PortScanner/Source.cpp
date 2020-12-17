#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include <Windows.h>
#include <windowsx.h>
#include <string>
#include <vector>
#include <mbstring.h>
#include <stdlib.h>
#include "commctrl.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

typedef std::vector<std::vector<std::string>>(*ALLPORTS)();
typedef std::vector<std::vector<std::string>>(*CERTAINPORT)(int);

ALLPORTS allPorts;
CERTAINPORT certainPort;

TCHAR dllName[] = TEXT("PortScannerLib");

HINSTANCE hMyDll;
HWND hListView;
HWND FindPort;

void TransformToLptstr(TCHAR* dest, std::string source);
int GetLength(const LPTSTR a);
int GetPort();
void ShowPortInfos(std::vector<std::string> data);

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR szCmdLine, int nCmdShow)
{
	MSG msg{};
	HWND hwnd{};
	WNDCLASSEX wc{ sizeof(WNDCLASSEX) };
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = L"MyAppClass";
	wc.lpszMenuName = nullptr;
	wc.style = CS_VREDRAW | CS_HREDRAW;

	if (!RegisterClassEx(&wc))
	{
		return EXIT_FAILURE;
	}

	if (hwnd = CreateWindow(wc.lpszClassName, L"PortScanner", WS_OVERLAPPEDWINDOW, 0, 0, 720, 480, nullptr, nullptr, wc.hInstance, nullptr);
		hwnd == INVALID_HANDLE_VALUE)
	{
		return EXIT_FAILURE;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return static_cast<int> (msg.wParam);
}

int CreateItem(HWND hwndList, wchar_t* text)
{
	LVITEMW lvi = { 0 };
	lvi.mask = LVIF_TEXT;
	lvi.pszText = text;
	return ListView_InsertItem(hwndList, &lvi);
}

void AddColumn(HWND hwnd, int iCol, std::string Text)
{
	int widthCol = 200;

	TCHAR lpStr[256];
	TransformToLptstr(lpStr, Text);

	LV_COLUMN p;
	p.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	p.fmt = LVCFMT_LEFT;
	p.cx = widthCol;
	p.pszText = lpStr;
	p.cchTextMax = 0;
	 
	ListView_InsertColumn(hwnd, iCol, &p);
}

void InsertItems(std::vector<std::string> data, int iRow)
{
	LVITEM lvi;

	memset(&lvi, 0, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_TEXT;

	lvi.iItem = iRow;

	lvi.iSubItem = 0;
	ListView_InsertItem(hListView, &lvi);
	lvi.iSubItem = 1;
	ListView_InsertItem(hListView, &lvi);
	lvi.iSubItem = 2;
	ListView_InsertItem(hListView, &lvi);

	TCHAR lpStrFirst[256];
	TCHAR lpStrSecond[256];
	TCHAR lpStrThird[256];

	TransformToLptstr(lpStrFirst, data.at(0));
	TransformToLptstr(lpStrSecond, data.at(1));
	TransformToLptstr(lpStrThird, data.at(2));

	ListView_SetItemText(hListView, iRow, 0, lpStrFirst);
	ListView_SetItemText(hListView, iRow, 1, lpStrSecond);
	ListView_SetItemText(hListView, iRow, 2, lpStrThird);
}

void CreateUI(HWND hEdit, HWND hWnd)
{
	hListView = CreateWindow(WC_LISTVIEW, L"",
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | LVS_REPORT | LVS_EDITLABELS,
		20, 50, 600, 400, hWnd, reinterpret_cast<HMENU>(3), nullptr, nullptr);

	HWND hAllPortsButton = CreateWindow(
		L"BUTTON",
		L"All ports",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		20, 20, 80, 20, hWnd, reinterpret_cast<HMENU>(0), nullptr, nullptr
	);

	HWND hCertainPortButton = CreateWindow(
		L"BUTTON",
		L"Certain port",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		105, 20, 80, 20, hWnd, reinterpret_cast<HMENU>(1), nullptr, nullptr
	);

	FindPort = CreateWindow(
		L"EDIT",
		L"",
		WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
		190, 20, 330, 20, hWnd, reinterpret_cast<HMENU>(2), nullptr, nullptr
	);

	if ((hMyDll = LoadLibrary(dllName)) != NULL)
	{
		allPorts = (ALLPORTS)GetProcAddress(hMyDll, "GetPortsInfo");
		certainPort = (CERTAINPORT)GetProcAddress(hMyDll, "CheckSpecificPort");
	}

	AddColumn(hListView, 0, "Process name");
	AddColumn(hListView, 0, "Process pid");
	AddColumn(hListView, 0, "Process port");
}

int CastChar(LPTSTR str)
{
	char* a = new char[50];

	for (int i = 0; i < 50; i++)
	{
		a[i] = str[i];
	}

	return atoi(a);
}

int GetLength(const LPTSTR a)
{
	for (int i = 0; i < 1024; i++)
	{
		if (a[i] == '\0')
			return i;
	}
}

void TransformToLptstr(TCHAR* dest, std::string source)
{
	for (int i = 0; i < source.length(); i++)
	{
		dest[i] = source[i];
	}
	dest[source.length()] = '\0';
}

int GetPort()
{
	LPTSTR pBuffer = new TCHAR[128];
	GetWindowText(FindPort, pBuffer, 50);

	if (GetLength(pBuffer) == 0)
		return -1;

	return CastChar(pBuffer);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND hEdit;

	if (message == WM_CREATE)
	{
		CreateUI(hEdit, hWnd);
	}

	switch (message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case 0:
				if (allPorts != NULL)
				{
					std::vector<std::vector<std::string>> data = allPorts();
					ListView_DeleteAllItems(hListView);
					for (int i = 0; i < data.size(); i++)
					{
						InsertItems(data.at(i), i);
					}
				}
				break;
			case 1:
				if (allPorts != NULL)
				{
					ListView_DeleteAllItems(hListView);
					std::vector<std::vector<std::string>> data = certainPort(GetPort());
					for (int i = 0; i < data.size(); i++)
					{
						InsertItems(data.at(i), i);
					}
				}
				break;
			default:
				break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(EXIT_SUCCESS);
		break;
	default:
		return(DefWindowProc(hWnd, message, wParam, lParam));
	}
	return 0;
}