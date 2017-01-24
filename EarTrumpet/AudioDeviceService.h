#pragma once
#include <windows.h>
#include <Mmdeviceapi.h>
#include "PolicyConfig.h"
#include <vector>

#define EARTRUMPET_API __declspec(dllexport)

#pragma warning( push )
#pragma warning( disable : 4251 )  

namespace EarTrumpet
{
	namespace Interop
	{
		struct EARTRUMPET_API EarTrumpetAudioDevice
		{
			LPWSTR Id;
			LPWSTR DisplayName;
			bool IsDefault;
			bool IsMuted;
		};
	}
}

namespace EarTrumpet
{
	namespace Interop
	{
		EARTRUMPET_API void InitializeCOM();
		EARTRUMPET_API void DeInitializeCOM();

		class EARTRUMPET_API AudioDeviceService
		{
		private:
			static AudioDeviceService* __instance;
			std::vector<EarTrumpetAudioDevice> _devices;

			void CleanUpAudioDevices();
			HRESULT GetDeviceByDeviceId(PWSTR deviceId, IMMDevice** device);
			HRESULT SetMuteBoolForDevice(LPWSTR deviceId, BOOL value);
			HRESULT GetPolicyConfigClient(IPolicyConfig** client);

		public:
			static AudioDeviceService& instance();

			HRESULT GetAudioDevices(std::vector<EarTrumpetAudioDevice>& out);
			HRESULT GetAudioDevices(void** audioDevices);
			HRESULT GetAudioDeviceVolume(LPWSTR deviceId, float* volume);
			HRESULT SetAudioDeviceVolume(LPWSTR deviceId, float volume);
			HRESULT SetDefaultAudioDevice(LPWSTR deviceId);
			HRESULT MuteAudioDevice(LPWSTR deviceId);
			HRESULT UnmuteAudioDevice(LPWSTR deviceId);
			HRESULT RefreshAudioDevices();
			int GetAudioDeviceCount();
		};
	}
}

#pragma warning( pop )