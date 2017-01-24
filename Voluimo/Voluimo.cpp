// Voluimo.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Voluimo.h"
#include "Controller.h"
#include <Shellapi.h>
#include <Strsafe.h>
#include <thread>

#include "LEDMatrix.h"
#include "NuimoTicker.h"

// Global Variables:
HINSTANCE hInst;
HWND hWnd;
std::unique_ptr<Controller> MController;

HICON Voluimo::Icon(int icon)
{
	return LoadIcon(
		hInst,
		MAKEINTRESOURCE(icon)
	);
}

HICON Voluimo::Icon(std::wstring path)
{
	WORD iconIdx = 0;
	LPWSTR iconStr = &path[0];
	return ExtractAssociatedIcon(
		GetModuleHandle(NULL),
		iconStr,
		&iconIdx
	);
}

void Voluimo::ChangeIcon(HICON icon)
{
	NOTIFYICONDATA nfidata = { };
	nfidata.cbSize = sizeof(nfidata);
	nfidata.hWnd = hWnd;
	nfidata.uFlags = NIF_ICON;
	nfidata.hIcon = icon;
	Shell_NotifyIcon(NIM_MODIFY, &nfidata);
}

INT_PTR CALLBACK AboutDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_APP + 100:
		switch (lParam)
		{
		case WM_RBUTTONUP: PostQuitMessage(0); break;
		case WM_LBUTTONUP: Voluimo::InitNuimoController(); break;
		}
		break;
	}

  //printf("Action %i %i %i\n", Message, wParam, lParam);
	return TRUE;
}

BOOL ConsoleHandler(DWORD event)
{
	if (event == CTRL_CLOSE_EVENT) {
		Voluimo::Cleanup();
		PostQuitMessage(0);
		return FALSE;
	}
	return FALSE;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	using namespace Voluimo;
	hInst = hInstance;
	InitializeCOM();
	InitConsole(lpCmdLine);
	InitShellIcon();
	InitNuimoController();

	MessageLoop();
	Cleanup();
	return 0;
}

void Voluimo::InitConsole(LPWSTR lpCmdLine)
{
	if (StrCmpW(L"console", lpCmdLine) == 0)
	{
		AllocConsole();
		SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE);
		FILE *stream;
		freopen_s(&stream, "CONOUT$", "w", stdout);
	}
}

void Voluimo::InitShellIcon()
{
	hWnd = CreateDialog(
		hInst,
		MAKEINTRESOURCE(IDD_HIDDEN),
		NULL,
		(DLGPROC)AboutDlgProc);

	NOTIFYICONDATA data{};
	data.cbSize = sizeof(NOTIFYICONDATA);
	data.uID = 0;
	data.uFlags = NIF_TIP | NIF_MESSAGE;
	StringCchCopy(data.szTip, ARRAYSIZE(data.szTip), L"Voluimo");
	data.hWnd = hWnd;
	data.uCallbackMessage = WM_APP + 100;
	Shell_NotifyIcon(NIM_ADD, &data);
	ChangeIcon(Icon(IDI_DISCONNECTED));
}

void Voluimo::InitNuimoController()
{
	if (MController)
	{
		return;
	}

	MController = std::make_unique<Controller>();
	MController->OnConnected([](bool connected) {
		ChangeIcon(Icon(connected ? IDI_CONNECTED : IDI_DISCONNECTED));
	});
	MController->OnIconChanged([](std::wstring icon) {
		if (icon.empty())
		{
			ChangeIcon(Icon(IDI_CONNECTED));
		}
		else
		{
			// Initially we showed the icon of the application, but that's a bit confusing.
			//ChangeIcon(Icon(icon));
		}
	});

	ChangeIcon(Icon(IDI_CONNECTING));
	if (MController->Connect())
	{
		printf("Connected\n");
		new std::thread([]() {
			Sleep(2000);
			MController->ShowBattery();
		});
	}
	else
	{
		printf("Unable to connect\n");
		MController.reset();
	}
}

void Voluimo::MessageLoop()
{
	BOOL bRet;
	MSG msg;
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			// handle the error and possibly exit
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

void Voluimo::Cleanup()
{
	NOTIFYICONDATA data{};
	data.cbSize = sizeof(NOTIFYICONDATA);
	data.uID = 0;
	data.hWnd = hWnd;
	Shell_NotifyIcon(NIM_DELETE, &data);
}