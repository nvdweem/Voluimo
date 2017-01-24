#include "stdafx.h"
#include "LEDMatrix.h"
#include "LEDCharacters.h"
#include <algorithm>

LEDMatrix::LEDMatrix()
{
}

const LEDMatrix::Matrix LEDMatrix::CreateMatrix()
{
	Matrix matrix;
	matrix.push_back(std::vector<bool>(9));
	matrix.push_back(std::vector<bool>(9));
	matrix.push_back(std::vector<bool>(9));
	matrix.push_back(std::vector<bool>(9));
	matrix.push_back(std::vector<bool>(9));
	matrix.push_back(std::vector<bool>(9));
	matrix.push_back(std::vector<bool>(9));
	matrix.push_back(std::vector<bool>(9));
	matrix.push_back(std::vector<bool>(9));
	return matrix;
}

const LEDMatrix::Number& LEDMatrix::FromNumber(int nr, bool shortNr)
{
	_ASSERT(nr >= 0 && nr <= 9);
	switch (nr)
	{
	case 0: return Zero;
	case 1: return shortNr ? ShortOne : One;
	case 2: return Two;
	case 3: return Three;
	case 4: return Four;
	case 5: return Five;
	case 6: return Six;
	case 7: return Seven;
	case 8: return Eight;
	case 9: return Nine;
	}
	throw 0;
}

void LEDMatrix::AddNumber(Matrix& target, int& x, int y, char nr, bool shortNr)
{
	return AddNumber(target, x, y, nr - '0', shortNr);
}
void LEDMatrix::AddNumber(Matrix& target, int& x, int y, int nr, bool shortNr)
{
	const Number& matrix = FromNumber(nr, shortNr);
	AddToMatrix(target, x, y, matrix);
	x++;
}

void LEDMatrix::AddToMatrix(Matrix& target, int& x, int y, const Character& c)
{
	for (size_t fy = 0; fy < c.size(); fy++)
	{
		for (size_t fx = 0; fx < c[fy].size(); fx++)
		{
			target[y + fy][x + fx] = c[fy][fx];
		}
	}
	x += (int)c[0].size();
}

LEDMatrix::Ticker LEDMatrix::CreateText(std::wstring text)
{
	Ticker ticker;
	ticker.push_back(std::vector<bool>());
	ticker.push_back(std::vector<bool>());
	ticker.push_back(std::vector<bool>());
	ticker.push_back(std::vector<bool>());
	ticker.push_back(std::vector<bool>());
	ticker.push_back(std::vector<bool>());
	ticker.push_back(std::vector<bool>());
	std::transform(text.begin(), text.end(), text.begin(), ::tolower);

	for (auto c : text)
	{
		auto cmi = CharacterMap.find(c);
		if (cmi == CharacterMap.end())
		{
			cmi = CharacterMap.find('?');
		}

		Character matrix = cmi->second;
		for (size_t fy = 0; fy < matrix.size(); fy++)
		{
			for (size_t fx = 0; fx < matrix[fy].size(); fx++)
			{
				ticker[fy].push_back(matrix[fy][fx]);
			}
			ticker[fy].push_back(false);
		}
	}

	return ticker;
}
