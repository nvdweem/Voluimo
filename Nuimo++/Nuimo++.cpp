//
// Nuimo++.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"

#include "Nuimo++.h"
#include "BLE.h"

#include <unordered_map>

GUID SBatteryStatus = BLE::MkGuid(L"{0000180F-0000-1000-8000-00805F9B34FB}");
GUID SDeviceInfo    = BLE::MkGuid(L"{0000180A-0000-1000-8000-00805F9B34FB}");
GUID SLedService    = BLE::MkGuid(L"{F29B1523-CB19-40F3-BE5C-7241ECB82FD1}");
GUID SNuimoService  = BLE::MkGuid(L"{F29B1525-CB19-40F3-BE5C-7241ECB82FD2}");

GUID SBattery    = BLE::MkGuid(L"{00002A19-0000-1000-8000-00805F9B34FB}");
GUID SRotation   = BLE::MkGuid(L"{F29B1528-CB19-40F3-BE5C-7241ECB82FD2}");
GUID SClick      = BLE::MkGuid(L"{F29B1529-CB19-40F3-BE5C-7241ECB82FD2}");
GUID SSwipetouch = BLE::MkGuid(L"{F29B1527-CB19-40F3-BE5C-7241ECB82FD2}");

struct CallbackParams
{
	BTH_LE_GATT_EVENT_TYPE eventType;
	PVOID eventOutParameter;
	PBTH_LE_GATT_CHARACTERISTIC context;
};

void Nuimo::Device::Callback(void* _params)
{
	CallbackParams& params = *(CallbackParams*)_params;

	PBLUETOOTH_GATT_VALUE_CHANGED_EVENT ValueChangedEventParameters = (PBLUETOOTH_GATT_VALUE_CHANGED_EVENT)params.eventOutParameter;

	if (params.context->CharacteristicUuid.Value.LongUuid == SRotation)
	{
		_ASSERT(ValueChangedEventParameters->CharacteristicValue->DataSize == 2);
		short int value = ValueChangedEventParameters->CharacteristicValue->Data[0] | ValueChangedEventParameters->CharacteristicValue->Data[1] << 8;
		if (MRotateCallback)
		{
			MRotateCallback(value);
		}
	}
	else if (params.context->CharacteristicUuid.Value.LongUuid == SClick)
	{
		_ASSERT(ValueChangedEventParameters->CharacteristicValue->DataSize == 1);
		if (MClickCallback)
		{
			MClickCallback((Nuimo::ClickType) ValueChangedEventParameters->CharacteristicValue->Data[0]);
		}
	}
	else if (params.context->CharacteristicUuid.Value.LongUuid == SSwipetouch)
	{
		_ASSERT(ValueChangedEventParameters->CharacteristicValue->DataSize == 1);
		if (MTouchCallback)
		{
			MTouchCallback((Nuimo::TouchType) ValueChangedEventParameters->CharacteristicValue->Data[0]);
		}
	}
	else if (params.context->CharacteristicUuid.IsShortUuid && params.context->CharacteristicUuid.Value.ShortUuid == SBattery.Data1)
	{
		_ASSERT(ValueChangedEventParameters->CharacteristicValue->DataSize == 1);
		printf("Battery status changed to: %i\n", ValueChangedEventParameters->CharacteristicValue->Data[0]);
	}
	else
	{
		// What else are we listening to?
		_ASSERT(false);
	}
}

BLE::UID2Device& Map(void* pMap)
{
	BLE::UID2Device& result = *((BLE::UID2Device*)pMap);
	return result;
}

// Get/set callback
void Nuimo::Device::ClickCallback(Nuimo::ClickCallback cb)
{
	MClickCallback = cb;
}
void Nuimo::Device::TouchCallback(Nuimo::TouchCallback cb)
{
	MTouchCallback = cb;
}
void Nuimo::Device::RotateCallback(Nuimo::RotateCallback cb)
{
	MRotateCallback = cb;
}

int Nuimo::Device::BatteryLevel()
{
	Info();

	auto itt = Map(MPMap).find(SBatteryStatus);
	if (itt == Map(MPMap).end())
	{
		return -1;
	}

	auto result = Read(*itt->second.device, &itt->second.charactaristics[0]);
	_ASSERT(result.get()->DataSize == 1);
	return (unsigned int) result.get()->Data[0];
}

Nuimo::NuimoInfo Nuimo::Device::Info()
{
	NuimoInfo info{};
	auto itt = Map(MPMap).find(SDeviceInfo);
	if (itt == Map(MPMap).end())
	{
		return info;
	}

	info.success = true;
	for (auto& c : itt->second.charactaristics)
	{
		auto result = Read(*itt->second.device, &c);
		
		switch (c.CharacteristicUuid.Value.ShortUuid)
		{
		case 10793: info.name.assign((char*) &result.get()->Data[0], (size_t)result.get()->DataSize); break; // Name
		case 10788: info.color.assign((char*) &result.get()->Data[0], (size_t)result.get()->DataSize); break; // Color
		case 10791: info.hardwareVersion.assign((char*) &result.get()->Data[0], (size_t)result.get()->DataSize); break; // Hardware
		case 10790: info.firmwareVersion.assign((char*) &result.get()->Data[0], (size_t)result.get()->DataSize); break; // Firmware
		}
	}
	return info;
}

bool Nuimo::Device::LEDMatrix(const LEDS& leds, bool fade, unsigned char brightness, unsigned char displayTime)
{
	std::string flat;
	if (leds.size() != 9)
	{
		return false;
	}

	for (auto& row : leds)
	{
		if (row.size() != 9)
		{
			return false;
		}
		for (const auto& cell : row)
		{
			flat.push_back(cell ? '*' : ' ');
		}
	}
	return LEDMatrix(flat, fade, brightness, displayTime);
}

bool Nuimo::Device::LEDMatrix(std::string& leds, bool fade, unsigned char brightness, unsigned char displayTime)
{
	if (leds.size() != 81)
	{
		return false;
	}

	auto itt = Map(MPMap).find(SLedService);
	if (itt == Map(MPMap).end())
	{
		return false;
	}
	_ASSERT(itt->second.charactaristics.size() == 1);

	std::vector<unsigned char> bytes;
	for (size_t i = 0; i < leds.size(); i++)
	{
		if (i % 8 == 0)
		{
			bytes.push_back(0);
		}
		bytes[bytes.size() - 1] |= (leds[i] == ' ' ? 0 : 1) << (i % 8);
	}
	_ASSERT(bytes.size() == 11);
	bytes[bytes.size() - 1] |= (fade ? 1 : 0) << 4;

	bytes.push_back(brightness);
	bytes.push_back(displayTime);

	std::string byteStr;
	for (unsigned char& c : bytes)
	{
		byteStr.push_back(c);
	}

	return Write(*itt->second.device, &itt->second.charactaristics[0], byteStr);
}

bool Nuimo::Device::AddCallbacksFor(GUID& guid)
{
	auto itt = Map(MPMap).find(guid);
	if (itt == Map(MPMap).end())
	{
		return false;
	}

	bool result = true;
	itt->second.device->UnregisterCallbacks();
	for (auto& c : itt->second.charactaristics)
	{
		auto pContext = std::make_unique<Nuimo::Device::UnknownContextPair>();
		pContext->first = &c;
		pContext->second = this;
		void* context = pContext.get();

		MCallbackContexts.push_back(std::move(pContext));
		auto func = [](BTH_LE_GATT_EVENT_TYPE EventType, PVOID EventOutParameter, PVOID Context) {
			auto pContextPair = (std::pair<PBTH_LE_GATT_CHARACTERISTIC, Nuimo::Device*>*)Context;

			auto pParams = std::make_unique<CallbackParams>();
			pParams->context = pContextPair->first;
			pParams->eventOutParameter = EventOutParameter;
			pParams->eventType = EventType;
			pContextPair->second->Callback(pParams.get());
		};

		result = AddCallback(*itt->second.device, &c, func, context) || result;
	}
	return result;
}

bool Nuimo::Device::AddCallbacks()
{
	MCallbackContexts.clear();
	bool result = AddCallbacksFor(SNuimoService);
	result = AddCallbacksFor(SBatteryStatus) && result;
	return result;
}

bool Nuimo::Device::Connect(DisconnectCallback callback)
{
	Map(MPMap).clear();
	if ( ConnectService(callback, SBatteryStatus, Map(MPMap))
		&& ConnectService(callback, SDeviceInfo, Map(MPMap))
		&& ConnectService(callback, SLedService, Map(MPMap))
		&& ConnectService(callback, SNuimoService, Map(MPMap))
		&& AddCallbacks())
	{
		return true;
	}

	Map(MPMap).clear();
	return false;
}

Nuimo::Device::Device()
	: MPMap(new BLE::UID2Device()),
	  MClickCallback(),
		MTouchCallback(),
		MRotateCallback(),
	  MCallbackContexts(),
	  MKeepAlive(),
	  MAlive(true)
{
	MKeepAlive = std::thread([&]() {
		KeepAlive();
	});
}

void Nuimo::Device::KeepAlive()
{
	// Not sure if this actually does something, but it seems to keep the connection alive :).
	while (MAlive)
	{
		for (auto& itt : Map(MPMap))
		{
			for (auto& c : itt.second.charactaristics)
			{
				if (c.IsReadable)
				{
					Read(*itt.second.device, &c);
				}
			}
		}
		for (int i = 0; MAlive && i < 60; i++)
		{
			Sleep(1000);
		}
	}
}

Nuimo::Device::~Device()
{
	MAlive = false;
	if (MKeepAlive.joinable())
	{
		MKeepAlive.join();
	}

	delete &Map(MPMap);
}