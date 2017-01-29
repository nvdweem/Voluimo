#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <thread>

#pragma warning( push )
#pragma warning( disable : 4251 )  

#define NUIMO_API __declspec(dllexport)

namespace Nuimo
{
	// Structures / enums
	struct NuimoInfo
	{
		bool        success;
		std::string name;
		std::string color;
		std::string hardwareVersion;
		std::string firmwareVersion;
	};

	enum TouchType
	{
		ESwipeLeft = 0,
		ESwipeRight = 1,
		ESwipeUp = 2,
		ESwipeDown = 3,
		ETouchLeft = 4,
		ETouchRight = 5,
		ETouchUp = 6,
		ETouchDown = 7,
		ELongTouchLeft = 8,
		ELongTouchRight = 9,
		ELongTouchUp = 10,
		ELongTouchDown = 11,
	};
	enum ClickType
	{
		EDown = 1,
		EUp = 0
	};

	// Callbacks
	typedef std::vector<std::vector<bool>>   LEDS;
	typedef std::function<void(ClickType)>   ClickCallback;
	typedef std::function<void(TouchType)>   TouchCallback;
	typedef std::function<void(int rotated)> RotateCallback;

	// Device
	class NUIMO_API Device
	{
		typedef std::pair<void*, void*> UnknownContextPair; // Don't want the actual types here since that would require the BLE headers
	public:
		typedef std::function<void()>   DisconnectCallback;

		Device();
		Device(const Device& device) = delete;
		Device& operator=(const Device& device) = delete;
		~Device();

		bool Connect(DisconnectCallback callback);
		int  BatteryLevel();
		NuimoInfo Info();
		bool LEDMatrix(const LEDS& leds, bool fade, unsigned char brightness, unsigned char displayTime);
		bool LEDMatrix(std::string& leds, bool fade, unsigned char brightness, unsigned char displayTime);

		// Get/set callback
		void ClickCallback(ClickCallback cb);
		void TouchCallback(TouchCallback cb);
		void RotateCallback(RotateCallback cb);

	private:
		bool AddCallbacks();
		void Callback(void* _params);
		void KeepAlive();

		// Datamembers
		void*                 MPMap;
		Nuimo::ClickCallback  MClickCallback;
		Nuimo::TouchCallback  MTouchCallback;
		Nuimo::RotateCallback MRotateCallback;
		std::vector<std::unique_ptr<UnknownContextPair>> MCallbackContexts;

		std::thread           MKeepAlive;
		bool                  MAlive;
	};
}
#pragma warning( pop )