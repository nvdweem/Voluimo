#pragma once

#include "resource.h"
#include <string>

namespace Voluimo
{
	void InitConsole(LPWSTR lpCmdLine);
	void InitShellIcon();
	void InitNuimoController();
	void MessageLoop();
	void Cleanup();

	HICON Icon(int icon);
	HICON Icon(std::wstring path);
	void ChangeIcon(HICON icon);
}