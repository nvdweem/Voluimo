#pragma once

#include "../Nuimo++/Nuimo++.h"
#include "../EarTrumpet/AudioDeviceService.h"
#include "../EarTrumpet/AudioSessionService.h"
#include "LEDMatrix.h"
#include "NuimoTicker.h"

using namespace EarTrumpet::Interop;

class Controller
{
public:
	typedef std::function<void(bool)>         Connected;
	typedef std::function<void(std::wstring)> IconChanged;
	
	Controller();
	~Controller();

// Operations
	bool Connect();
	void ShowBattery();
	void ShowText(std::wstring text);
	void StopText();
	bool SendMatrix(const LEDMatrix::Matrix& matrix, unsigned char time);
	void OnConnected(Connected);
	void OnIconChanged(IconChanged);

	void DisconnectCallback(Nuimo::Device::DisconnectCallback cb)
	{
		MDisconnectCallback = cb;
	}

private:
	// Operations
	  void Init();
		void SelectSession(EarTrumpetAudioSession& session);
		void SelectFocussedSession();

  // Callbacks
	  void ChangeVolumeRelative(int value);
		void ChangeVolume(unsigned char value);
		void SelectSession(bool next);
		void SelectGlobal(bool show);

	// Control device
		void SetCurrentVolume();
		void MuteUnmute();
		void ShowVolume();
		void ShowNumber(int nr);

	// Member variables
	  Nuimo::Device                     MNuimo;
		Nuimo::Device::DisconnectCallback MDisconnectCallback;
		AudioDeviceService&               MDeviceService;
		AudioSessionService&              MSessionService;

		EarTrumpetAudioDevice MDefaultDevice;
		EarTrumpetAudioSession MSelectedSession;
		bool MControllingGlobal;
		float MCurrentVolume;
		bool MCurrentMuted;

		Connected MConnected;
		IconChanged MIconChanged;
		std::unique_ptr<NuimoTicker> MPTicker;
};

