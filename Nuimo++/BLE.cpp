/*
Most of this code is from a comment of ihateble at https://social.msdn.microsoft.com/Forums/en-US/bad452cb-4fc2-4a86-9b60-070b43577cc9/is-there-a-simple-example-desktop-programming-c-for-bluetooth-low-energy-devices?forum=wdk
*/
#include "stdafx.h"

#include "BLE.h"
#include <setupapi.h>
#include <Objbase.h>

#pragma comment(lib, "SetupAPI")
#pragma comment(lib, "BluetoothApis.lib")

GUID BLE::MkGuid(std::wstring in)
{
	GUID AGuid;
	CLSIDFromString(in.c_str(), &AGuid);
	return AGuid;
}

void OkOrDisconnected(BLE::DeviceHandle& handle, HRESULT hr)
{
	if (hr == HRESULT_FROM_WIN32(ERROR_DEVICE_NOT_CONNECTED))
	{
		handle.Disconnected();
		return;
	}
	_ASSERT(hr == S_OK);
}

void BLE::HandleCloser::operator()(DeviceHandle* handle)
{
	handle->UnregisterCallbacks();
	CloseHandle(handle->hHandle);
}

void BLE::DeviceHandle::UnregisterCallbacks()
{
	for (BLUETOOTH_GATT_EVENT_HANDLE handle : MCallbacks)
	{
		HRESULT hr = BluetoothGATTUnregisterEvent(
			handle,
			BLUETOOTH_GATT_FLAG_NONE);
		_ASSERT(hr == S_OK);
	}
	MCallbacks.clear();
}

bool BLE::AddCallback(BLE::DeviceHandle& handle, PBTH_LE_GATT_CHARACTERISTIC currGattChar, PFNBLUETOOTH_GATT_EVENT_CALLBACK callback, void* context)
{
	BLUETOOTH_GATT_EVENT_HANDLE EventHandle;
	if (!currGattChar->IsNotifiable)
	{
		return false;
	}

	BTH_LE_GATT_EVENT_TYPE EventType = CharacteristicValueChangedEvent;

	BLUETOOTH_GATT_VALUE_CHANGED_EVENT_REGISTRATION EventParameterIn;
	EventParameterIn.Characteristics[0] = *currGattChar;
	EventParameterIn.NumCharacteristics = 1;

	HRESULT hr = BluetoothGATTRegisterEvent(
		handle.hHandle,
		EventType,
		&EventParameterIn,
		callback,
		context,
		&EventHandle,
		BLUETOOTH_GATT_FLAG_NONE);

	return hr == S_OK;
}

BLE::BLEClosingHandle GetBLEHandle(BLE::DisconnectCallback callback, __in GUID AGuid)
{
	HDEVINFO hDI;
	SP_DEVICE_INTERFACE_DATA did;
	SP_DEVINFO_DATA dd;
	GUID BluetoothInterfaceGUID = AGuid;
	BLE::BLEClosingHandle hComm = NULL;

	hDI = SetupDiGetClassDevs(&BluetoothInterfaceGUID, NULL, NULL, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

	if (hDI == INVALID_HANDLE_VALUE) return NULL;

	did.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	dd.cbSize = sizeof(SP_DEVINFO_DATA);

	for (DWORD i = 0; SetupDiEnumDeviceInterfaces(hDI, NULL, &BluetoothInterfaceGUID, i, &did); i++)
	{
		SP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData;
		DeviceInterfaceDetailData.cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
		DWORD size = 0;

		if (!SetupDiGetDeviceInterfaceDetail(hDI, &did, NULL, 0, &size, 0))
		{
			int err = GetLastError();
			if (err == ERROR_NO_MORE_ITEMS) break;

			auto pInterfaceDetailData = std::make_unique<SP_DEVICE_INTERFACE_DETAIL_DATA[]>(size);
			pInterfaceDetailData.get()->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			if (!SetupDiGetDeviceInterfaceDetail(hDI, &did, pInterfaceDetailData.get(), size, &size, &dd))
				break;

			hComm.reset(new BLE::DeviceHandle(
				CreateFile(
					pInterfaceDetailData.get()->DevicePath,
					GENERIC_WRITE | GENERIC_READ,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					0,
					NULL), callback
			));
		}
	}

	SetupDiDestroyDeviceInfoList(hDI);
	return hComm;
}

BLE::BTResult BLE::Read(BLE::DeviceHandle& handle, PBTH_LE_GATT_CHARACTERISTIC currGattChar)
{
	if (!currGattChar->IsReadable)
	{
		return NULL;
	}

	USHORT charValueDataSize;

	// Buffer size
	HRESULT hr = BluetoothGATTGetCharacteristicValue(
		handle.hHandle,
		currGattChar,
		0,
		NULL,
		&charValueDataSize,
		BLUETOOTH_GATT_FLAG_NONE);
	_ASSERT(HRESULT_FROM_WIN32(ERROR_MORE_DATA) == hr);

	// Retrieve the Characteristic Value
	auto pCharValueBuffer = std::make_unique<BTH_LE_GATT_CHARACTERISTIC_VALUE[]>(charValueDataSize);
	hr = BluetoothGATTGetCharacteristicValue(
		handle.hHandle,
		currGattChar,
		(ULONG)charValueDataSize,
		pCharValueBuffer.get(),
		NULL,
		BLUETOOTH_GATT_FLAG_NONE);
	OkOrDisconnected(handle, hr);

	return pCharValueBuffer;
}

bool BLE::Write(BLE::DeviceHandle& handle, PBTH_LE_GATT_CHARACTERISTIC currGattChar, std::string str)
{
	if (!currGattChar->IsWritable)
	{
		return false;
	}

	auto pCharValueBuffer = std::make_unique<BTH_LE_GATT_CHARACTERISTIC_VALUE[]>(str.length());
	pCharValueBuffer.get()->DataSize = (ULONG) str.length();
	for ( size_t i = 0; i < str.length(); i++ )
	{
		pCharValueBuffer.get()->Data[i] = str[i];
	}


	// Retrieve the Characteristic Value
	HRESULT hr = BluetoothGATTSetCharacteristicValue(
		handle.hHandle,
		currGattChar,
		pCharValueBuffer.get(),
		NULL,
		BLUETOOTH_GATT_FLAG_NONE
	);
	OkOrDisconnected(handle, hr);

	return true;
}

bool InitDescriptors(BLE::BLEClosingHandle& hLEDevice, PBTH_LE_GATT_CHARACTERISTIC currGattChar)
{
	// Descriptors
	USHORT descriptorBufferSize;
	HRESULT hr = BluetoothGATTGetDescriptors(
		hLEDevice->hHandle,
		currGattChar,
		0,
		NULL,
		&descriptorBufferSize,
		BLUETOOTH_GATT_FLAG_NONE);

	auto pDescriptorBuffer = std::make_unique<BTH_LE_GATT_DESCRIPTOR>();
	if (descriptorBufferSize > 0) {
		// Retrieve Descriptors
		USHORT numDescriptors;
		hr = BluetoothGATTGetDescriptors(
			hLEDevice->hHandle,
			currGattChar,
			descriptorBufferSize,
			pDescriptorBuffer.get(),
			&numDescriptors,
			BLUETOOTH_GATT_FLAG_NONE);

		OkOrDisconnected(*hLEDevice, hr);
		_ASSERT(numDescriptors == descriptorBufferSize);

		for (int kk = 0; kk<numDescriptors; kk++) {
			PBTH_LE_GATT_DESCRIPTOR  currGattDescriptor = &pDescriptorBuffer.get()[kk];

			// Determine Descriptor Value Buffer Size
			USHORT descValueDataSize;
			hr = BluetoothGATTGetDescriptorValue(
				hLEDevice->hHandle,
				currGattDescriptor,
				0,
				NULL,
				&descValueDataSize,
				BLUETOOTH_GATT_FLAG_NONE);
			_ASSERT(HRESULT_FROM_WIN32(ERROR_MORE_DATA) == hr);

			// Retrieve the Descriptor Value
			auto pDescValueBuffer = std::make_unique<BTH_LE_GATT_DESCRIPTOR_VALUE>();
			hr = BluetoothGATTGetDescriptorValue(
				hLEDevice->hHandle,
				currGattDescriptor,
				(ULONG)descValueDataSize,
				pDescValueBuffer.get(),
				NULL,
				BLUETOOTH_GATT_FLAG_NONE);
			OkOrDisconnected(*hLEDevice, hr);

			if (currGattDescriptor->AttributeHandle < 255) {
				BTH_LE_GATT_DESCRIPTOR_VALUE newValue;

				RtlZeroMemory(&newValue, sizeof(newValue));

				newValue.DescriptorType = ClientCharacteristicConfiguration;
				newValue.ClientCharacteristicConfiguration.IsSubscribeToNotification = TRUE;

				hr = BluetoothGATTSetDescriptorValue(
					hLEDevice->hHandle,
					currGattDescriptor,
					&newValue,
					BLUETOOTH_GATT_FLAG_NONE);

				if (S_OK != hr)
				{
					return false;
				}
			}
		}
	}
	return true;
}

bool BLE::ConnectService(DisconnectCallback disconnectCallback, GUID& AGuid, BLE::UID2Device& target)
{
	BLE::BLEClosingHandle hLEDevice = GetBLEHandle(disconnectCallback, AGuid);

	// Get service count
	USHORT serviceBufferCount;
	HRESULT hr = BluetoothGATTGetServices(
		hLEDevice->hHandle,
		0,
		NULL,
		&serviceBufferCount,
		BLUETOOTH_GATT_FLAG_NONE);

	// Retrieve services
	auto pServiceBuffer = std::make_unique<BTH_LE_GATT_SERVICE[]>(serviceBufferCount);
	USHORT numServices;
	hr = BluetoothGATTGetServices(
		hLEDevice->hHandle,
		serviceBufferCount,
		pServiceBuffer.get(),
		&numServices,
		BLUETOOTH_GATT_FLAG_NONE);

	if (S_OK != hr)
	{
		return false;
	}


	// Characteristic size
	USHORT charBufferSize;
	hr = BluetoothGATTGetCharacteristics(
		hLEDevice->hHandle,
		pServiceBuffer.get(),
		0,
		NULL,
		&charBufferSize,
		BLUETOOTH_GATT_FLAG_NONE);
	if (HRESULT_FROM_WIN32(ERROR_MORE_DATA) != hr)
	{
		return false;
	}

	std::unique_ptr<BTH_LE_GATT_CHARACTERISTIC[]> pCharBuffer;
	if (charBufferSize > 0) {
		pCharBuffer = std::make_unique<BTH_LE_GATT_CHARACTERISTIC[]>(charBufferSize);

		// Retrieve Characteristics
		USHORT numChars;
		hr = BluetoothGATTGetCharacteristics(
			hLEDevice->hHandle,
			pServiceBuffer.get(),
			charBufferSize,
			pCharBuffer.get(),
			&numChars,
			BLUETOOTH_GATT_FLAG_NONE);
		if (S_OK != hr)
		{
			return false;
		}
		_ASSERT(numChars == charBufferSize);
	}

	BLE::Characteristics characteristics;

	bool couldInitialize = true;
	PBTH_LE_GATT_CHARACTERISTIC currGattChar;
	for (int ii = 0; couldInitialize && ii < charBufferSize; ii++) {
		std::unique_ptr<BTH_LE_GATT_CHARACTERISTIC_VALUE[]> pCharValueBuffer;
		currGattChar = &pCharBuffer[ii];
		couldInitialize = couldInitialize && InitDescriptors(hLEDevice, currGattChar);

		characteristics.push_back(*currGattChar);
	}
	if (!couldInitialize)
	{
		return false;
	}

	target[AGuid] = { std::move(hLEDevice), characteristics };
	return true;
}