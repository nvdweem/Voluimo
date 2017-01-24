#pragma once

#include <memory>
#include <unordered_map>

#pragma warning(push)
#pragma warning(disable: 4068) // Prevent Unknown pragma
#include <Bluetoothleapis.h>
#pragma warning(pop)

namespace std
{
	template <>
	struct hash<GUID>
	{
		size_t operator()(GUID const & x) const noexcept
		{
			std::string d4(8, x.Data4[0]);

			return std::hash<unsigned long>{}(x.Data1)
				   + std::hash<unsigned short>{}(x.Data2)
				   + std::hash<unsigned short>{}(x.Data3)
				   + std::hash<std::string>{}(d4);
		}
	};
}

namespace BLE
{
	typedef std::vector<BTH_LE_GATT_CHARACTERISTIC> Characteristics;
	typedef std::unique_ptr<BTH_LE_GATT_CHARACTERISTIC_VALUE[]> BTResult;

	struct BLEDeviceHandle
	{
		BLEDeviceHandle(void* in) : hHandle(in) {}
		BLEDeviceHandle(const BLEDeviceHandle&) = delete;
		void* hHandle;
	};
	struct HandleCloser
	{
		void operator()(BLEDeviceHandle* handle)
		{
			CloseHandle(handle->hHandle);
		}
	};
	typedef std::unique_ptr<BLEDeviceHandle, HandleCloser> BLEClosingHandle;

	struct DeviceCharacteristics
	{
		BLEClosingHandle device;
		Characteristics charactaristics;
	};

	typedef std::unordered_map<GUID, DeviceCharacteristics> UID2Device;


	bool ConnectService(GUID& guid, UID2Device& target);
	BTResult Read(BLEDeviceHandle& handle, PBTH_LE_GATT_CHARACTERISTIC currGattChar);
	bool Write(BLEDeviceHandle& handle, PBTH_LE_GATT_CHARACTERISTIC currGattChar, std::string str);
	bool AddCallback(BLEDeviceHandle& handle, PBTH_LE_GATT_CHARACTERISTIC currGattChar, PFNBLUETOOTH_GATT_EVENT_CALLBACK callback, void* context);
	GUID MkGuid(std::wstring in);
}