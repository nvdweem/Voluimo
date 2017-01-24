#include "stdafx.h"
#include "Controller.h"
#include "NuimoTicker.h"

NuimoTicker::NuimoTicker(Controller& controller, LEDMatrix::Ticker& ticker)
	: MController(controller),
	  MTicker(ticker),
	  MSpeed(0),
	  MRunner(),
	  MRunning(false),
	  MDoneCallback()
{
}


NuimoTicker::~NuimoTicker()
{
	Stop();
}

// Operations

bool NuimoTicker::OKMatrix(const LEDMatrix::Matrix& matrix) const
{
	bool hasTrue = false;
	for (auto line : matrix)
	{
		for (auto c : line)
		{
			hasTrue = hasTrue || c;
		}
	}
	return hasTrue;
}

void NuimoTicker::Start(int speed, int callbackDelay)
{
	MSpeed = speed;
	Stop();

	MRunning = true;
	MRunner = std::thread([&, callbackDelay]() {
		int position = 0;
		bool first = true;
		while (MRunning)
		{
			auto matrix = GetMatrixFor(position);
			MController.SendMatrix(matrix, (char)((2*(first ? 6 * MSpeed : MSpeed)) / 100));
			if (!OKMatrix(matrix))
			{
				break;
			}

			position++;
			Sleep(first ? 10*MSpeed : MSpeed);
			first = false;
		}

		if (MRunning && MDoneCallback)
		{
			Sleep(callbackDelay);
			MDoneCallback();
		}
	});
}

void NuimoTicker::Stop()
{
	if (MRunner.joinable())
	{
		MRunning = false;
		MRunner.join();
	}
}

void NuimoTicker::Callback(DoneCallback cb)
{
	MDoneCallback = cb;
}

LEDMatrix::Matrix NuimoTicker::GetMatrixFor(int pos) const
{
	auto result = LEDMatrix::CreateMatrix();

	bool done = false;
	for (size_t y = 0; !done && y < result.size(); y++)
	{
		if (MTicker.size() <= y)
		{
			continue;
		}

		for (size_t x = 0; x < result[y].size(); x++)
		{
			if (MTicker[y].size() <= x + pos)
			{
				continue;
			}
			result[y][x] = MTicker[y][x + pos];
		}
	}
	return result;
}