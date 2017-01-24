#pragma once
#include <string>
#include <vector>
#include <unordered_map>

struct LEDMatrix
{
	typedef std::vector<std::vector<bool>>         Number;    // 0-9
	typedef Number                                 Character; // 0-9 A-Z
	typedef std::unordered_map<wchar_t, Character> Characters;// a => Character, b=> Character, ...
	typedef Number                                 Matrix;    // 9x9
	typedef Matrix                                 Ticker;    // ?x9

// Operations
	static const Matrix  CreateMatrix();
	static const Number& FromNumber(int nr, bool shortNr);
	static void          AddNumber(Matrix& target, int& x, int y, char nr, bool shortNr);
	static void          AddNumber(Matrix& target, int& x, int y, int nr, bool shortNr);
	static void          AddToMatrix(Matrix& target, int& x, int y, const Character& c);
	static Ticker        CreateText(std::wstring text);

// Statics
	static std::string SMuted;
	static std::string SLogo;

	static Number One;
	static Number ShortOne;
	static Number Two;
	static Number Three;
	static Number Four;
	static Number Five;
	static Number Six;
	static Number Seven;
	static Number Eight;
	static Number Nine;
	static Number Zero;

	static Characters CharacterMap;

private: 
	LEDMatrix();
};
