#pragma once

/*****************************************************************************/
struct HYPERCORE_API SIntVector2D
{
	SIntVector2D()
	{
		x = y = 0;
	}

	SIntVector2D(int iX, int iY)
	{
		x = iX;
		y = iY;
	}

	SIntVector2D(const SIntVector2D& rOther)
	{
		x = rOther.x;
		y = rOther.y;
	}

	inline void set(int iX, int iY)
	{
		x = iX;
		y = iY;
	}

	inline SIntVector2D& operator=(const SVector2D& svVec)
	{
		x = svVec.x;
		y = svVec.y;
		return *this;
	}

	inline SIntVector2D& operator*=(FLOAT_TYPE fValue)
	{
		x *= fValue;
		y *= fValue;
		return *this;
	}

	inline bool operator==(const SIntVector2D& siOther) const {	return (x == siOther.x) && (y == siOther.y); }
	inline bool operator!=(const SIntVector2D& siOther) const { return !((*this) == siOther); }

	inline void clampToImageSize(int iW, int iH)
	{
		x = HyperCore::clampToRange<int>(0, iW - 1, x);
		y = HyperCore::clampToRange<int>(0, iH - 1, y);
	}

	int x, y;
};
/*****************************************************************************/
typedef list < SIntVector2D > TIntPointList;
typedef vector < SIntVector2D > TIntPointVector;
