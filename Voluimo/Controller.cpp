#include "stdafx.h"
#include "Controller.h"

Controller::Controller()
	: MNuimo(),
	  MDisconnectCallback(),
	  MDeviceService(AudioDeviceService::instance()),
    MSessionService(AudioSessionService::instance()),
	  MDefaultDevice(),
	  MSelectedSession(),
	  MControllingGlobal(true),
	  MCurrentVolume(.5),
	  MCurrentMuted(false),
	  MConnected(),
	  MIconChanged(),
	  MPTicker()
{
	MNuimo.RotateCallback([this](int volume) {
		this->ChangeVolume(volume);
	});
	MNuimo.ClickCallback([this](Nuimo::ClickType type) {
		if (type == Nuimo::ClickType::EDown)
		{
			// Would like to use mute/unmute here, but the rotation callbacks stop working after
			// a while, and clicking the button enables them again. For now it's better if the button
			// doesn't actually do anything.
			//MuteUnmute();
			StopText();
			ShowVolume();
		}
	});
	MNuimo.TouchCallback([this](Nuimo::TouchType type) {
		switch (type)
		{
		case Nuimo::TouchType::ESwipeLeft:
		case Nuimo::TouchType::ETouchLeft:
		case Nuimo::TouchType::ESwipeRight: 
		case Nuimo::TouchType::ETouchRight: SelectSession(type == Nuimo::TouchType::ESwipeRight || type == Nuimo::TouchType::ETouchRight); break;
		case Nuimo::TouchType::ETouchDown:
		case Nuimo::TouchType::ESwipeDown: SelectFocussedSession(); break;
		case Nuimo::TouchType::ESwipeUp: SelectGlobal(true); break;
		}
	});

	Init();
}

Controller::~Controller()
{
}

// Operations

bool Controller::Connect()
{
	if (MNuimo.Connect(MDisconnectCallback))
	{
		if (MConnected)
		{
			MConnected(true);
		}
		MNuimo.LEDMatrix(LEDMatrix::SLogo, false, 255, 50);
		return true;
	}

	if (MConnected)
	{
		MConnected(false);
	}
	return false;
}

void Controller::ShowBattery()
{
	ShowNumber(MNuimo.BatteryLevel());
}

void Controller::ShowText(std::wstring text)
{
	MPTicker = std::make_unique<NuimoTicker>(*this, LEDMatrix::CreateText(text));
	MPTicker->Start(50, 1000);
	MPTicker->Callback([this]() {
		ShowVolume();
	});
}

void Controller::StopText()
{
	MPTicker.reset();
}

void Controller::SendMatrix(const LEDMatrix::Matrix& matrix, unsigned char time)
{
	MNuimo.LEDMatrix(matrix, true, 255, time);
}

void Controller::OnConnected(Connected fnc)
{
	MConnected = fnc;
}

void Controller::OnIconChanged(IconChanged fnc)
{
	MIconChanged = fnc;
}

void Controller::Init() 
{
	SelectGlobal(false);
}

// Control current device

void Controller::SetCurrentVolume()
{
	if (MControllingGlobal) 
	{
		MDeviceService.SetAudioDeviceVolume(MDefaultDevice.Id, MCurrentVolume);
	}
	else
	{
		MSessionService.SetAudioSessionVolume(MSelectedSession.SessionId, MCurrentVolume);
	}
	StopText();
	ShowVolume();
}

void Controller::MuteUnmute()
{
	MCurrentMuted = !MCurrentMuted;
	if (MControllingGlobal)
	{
		if (MCurrentMuted)
		{
			MDeviceService.MuteAudioDevice(MDefaultDevice.Id);
		}
		else
		{
			MDeviceService.UnmuteAudioDevice(MDefaultDevice.Id);
		}
	}
	else
	{
		MSessionService.SetAudioSessionMute(MSelectedSession.SessionId, MCurrentMuted);
	}
	StopText();
	ShowVolume();
}

void Controller::ShowVolume()
{
	if (MCurrentMuted)
	{
		MNuimo.LEDMatrix(LEDMatrix::SMuted, false, 255, 255);
		return;
	}

	int volume = (int) (MCurrentVolume * 100);
	ShowNumber(volume);
}

void Controller::ShowNumber(int nr)
{
	if (nr < 0 || nr > 100)
	{
		printf("Unable to show %i since its not 0 >= x > 100\n", nr);
		return;
	}

	std::string volumeStr = std::to_string(nr);

	auto matrix = LEDMatrix::CreateMatrix();
	int x = 0;
	int y = 0;
	for (size_t i = 0; i < volumeStr.length(); i++)
	{
		LEDMatrix::AddNumber(matrix, x, y, volumeStr[i], i == 0 && volumeStr.length() == 3);
	}
	MNuimo.LEDMatrix(matrix, true, 255, 30);
}

// Callbacks
void Controller::ChangeVolume(int value)
{
	MCurrentVolume += (float)(value / 5000.0);
	if (MCurrentVolume < 0)
	{
		MCurrentVolume = 0;
	}
	else if (MCurrentVolume > 1)
	{
		MCurrentVolume = 1;
	}

	SetCurrentVolume();
}

void Controller::SelectGlobal(bool show)
{
	MDeviceService.RefreshAudioDevices();
	std::vector<EarTrumpetAudioDevice> devices;
	MDeviceService.GetAudioDevices(devices);

	for (auto& device : devices)
	{
		if (device.IsDefault)
		{
			MDefaultDevice = device;
		}
	}
	MControllingGlobal = true;
	MDeviceService.GetAudioDeviceVolume(MDefaultDevice.Id, &MCurrentVolume);
	MCurrentMuted = MDefaultDevice.IsMuted;

	if (MIconChanged)
	{
		MIconChanged(L"");
	}

	if (show)
	{
		ShowText(L"Global");
	}
}

void Controller::SelectSession(bool next)
{
	MSessionService.RefreshAudioSessions();
	std::vector<EarTrumpetAudioSession> services;
	MSessionService.GetAudioSessions(services);

	if (services.size() == 0)
	{
		return;
	}

	for (size_t idx = 0; idx < services.size(); idx++)
	{
		if (services[idx].SessionId == MSelectedSession.SessionId)
		{
			size_t toChose = 0;
			if (next)
			{
				toChose = idx + 1 == services.size() ? 0 : idx + 1;
			}
			else
			{
				toChose = idx == 0 ? services.size() - 1 : idx - 1;
			}

			SelectSession(services[toChose]);
			return;
		}
	}

	if (next)
	{
		SelectSession(services[0]);
	}
	else
	{
		SelectSession(services[services.size() - 1]);
	}
}

void Controller::SelectSession(EarTrumpetAudioSession& session)
{
	std::wstring toShow(session.DisplayName);
	toShow = toShow.substr(0, toShow.find_last_of('.'));
	ShowText(toShow);
	printf("Selected: %ws\n", session.DisplayName);

	MControllingGlobal = false;
	MSelectedSession = session;
	MCurrentMuted = session.IsMuted;
	MCurrentVolume = session.Volume;

	if (MIconChanged)
	{
		MIconChanged(session.IconPath);
	}
}

void Controller::SelectFocussedSession()
{
	HWND hWnd = GetForegroundWindow();
	DWORD dwPID;
	GetWindowThreadProcessId(hWnd, &dwPID);

	MSessionService.RefreshAudioSessions();
	std::vector<EarTrumpetAudioSession> sessions;
	MSessionService.GetAudioSessions(sessions);
	for (auto& session : sessions)
	{
		if (session.ProcessId == dwPID)
		{
			printf("Found %ws to be the foreground window\n", session.DisplayName);
			SelectSession(session);
			return;
		}
	}
	printf("Could not find audio session for foreground window\n");

	auto matrix = LEDMatrix::CreateMatrix();
	int x = 2;
	LEDMatrix::AddToMatrix(matrix, x, 0, LEDMatrix::CreateText(L"?"));
	SendMatrix(matrix, 30);
}
