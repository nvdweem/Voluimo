#pragma once

#include "LEDMatrix.h"
#include <thread>

class Controller;

class NuimoTicker
{
public:
	typedef std::function<void()> DoneCallback;
	NuimoTicker(Controller& controller, LEDMatrix::Ticker& ticker);
	~NuimoTicker();

// Operations
	void Start(int speed, int callbackDelay = 0);
	void Stop();
	void Callback(DoneCallback);

private:
// Operations
	LEDMatrix::Matrix GetMatrixFor(int x) const;
	bool OKMatrix(const LEDMatrix::Matrix&) const;

// Members
	Controller&       MController;
	LEDMatrix::Ticker MTicker;
	int               MSpeed;
	std::thread       MRunner;
	bool              MRunning;
	DoneCallback      MDoneCallback;
};

