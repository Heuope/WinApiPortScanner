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

#define ID_LIST 1
#define BUFFERSIZE 5000
#define MEMSIZE 90024 

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

typedef std::vector<std::string>(*ALLPORTS)();
typedef std::vector<std::string>(*CERTAINPORT)(int);

ALLPORTS allPorts;
CERTAINPORT certainPort;

TCHAR dllName[] = TEXT("PortScannerLib");

HINSTANCE hMyDll;
HWND hListBox;
HWND FindPort;

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

void CreateUI(HWND hEdit, HWND hWnd)
{
	HWND hwnd_st_u = CreateWindow(L"static", NULL,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP,
		20, 50, 500, 20,
		hWnd, reinterpret_cast<HMENU>(3),
		nullptr, nullptr);

	SetWindowText(hwnd_st_u, L"Process name        | Process pid        | Process port");

	hListBox = CreateWindow(L"listbox", NULL,
		WS_CHILD | WS_VISIBLE | LBS_STANDARD |
		LBS_WANTKEYBOARDINPUT,
		20, 70, 500, 300,
		hWnd, (HMENU)ID_LIST, nullptr, nullptr);

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
					std::vector<std::string> data = allPorts();
					SendMessage(hListBox, LB_RESETCONTENT, 0, 0);

					for (int i = 0; i < data.size(); i++)
					{
						TCHAR lpStr[256];
						TransformToLptstr(lpStr, data.at(i));

						SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)(LPSTR)lpStr);
					}
				}
				break;
			case 1:
				if (allPorts != NULL)
				{
					std::vector<std::string> data = certainPort(GetPort());
					SendMessage(hListBox, LB_RESETCONTENT, 0, 0);

					for (int i = 0; i < data.size(); i++)
					{
						TCHAR lpStr[256];
						TransformToLptstr(lpStr, data.at(i));

						SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)(LPSTR)lpStr);
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