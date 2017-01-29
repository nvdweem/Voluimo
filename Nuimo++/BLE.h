#pragma once

#include <memory>
#include <unordered_map>
#include <functional>

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
	typedef std::vector<BLUETOOTH_GATT_EVENT_HANDLE> RegisteredCallbacks;
	typedef std::unique_ptr<BTH_LE_GATT_CHARACTERISTIC_VALUE[]> BTResult;
	typedef std::function<void()> DisconnectCallback;

	struct DeviceHandle
	{

		DeviceHandle(void* in, DisconnectCallback cb) : hHandle(in), MCallbacks(), MDisconnectCallback(cb) {}
		DeviceHandle(const DeviceHandle&) = delete;

		void Disconnected()
		{
			UnregisterCallbacks();
			if (MDisconnectCallback)
			{
				MDisconnectCallback();
			}
		}

	// Member variables
		void*               hHandle;
	private:
		friend struct HandleCloser;
		void UnregisterCallbacks();

		// Member variables
		RegisteredCallbacks MCallbacks;
		DisconnectCallback  MDisconnectCallback;
	};

	struct HandleCloser
	{
		void operator()(DeviceHandle* handle);
	};
	typedef std::unique_ptr<DeviceHandle, HandleCloser> BLEClosingHandle;

	struct DeviceCharacteristics
	{
		BLEClosingHandle device;
		Characteristics charactaristics;
	};

	typedef std::unordered_map<GUID, DeviceCharacteristics> UID2Device;


	bool ConnectService(DisconnectCallback disconnectCallback, GUID& guid, UID2Device& target);
	BTResult Read(DeviceHandle& handle, PBTH_LE_GATT_CHARACTERISTIC currGattChar);
	bool Write(DeviceHandle& handle, PBTH_LE_GATT_CHARACTERISTIC currGattChar, std::string str);
	bool AddCallback(DeviceHandle& handle, PBTH_LE_GATT_CHARACTERISTIC currGattChar, PFNBLUETOOTH_GATT_EVENT_CALLBACK callback, void* context);
	GUID MkGuid(std::wstring in);
}