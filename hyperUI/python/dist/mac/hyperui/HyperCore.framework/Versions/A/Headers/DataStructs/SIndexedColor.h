#pragma once

/*****************************************************************************/
struct HYPERCORE_API SIndexedColor
{
	SColor myColor;
	int myIndex;

	bool operator()(const SIndexedColor& scolColor1, const SIndexedColor& scolColor2) const
	{
		long lNum1 = ((int)(scolColor1.myColor.r*255.0))*1000000 + ((int)(scolColor1.myColor.g*255.0))*1000 + ((int)(scolColor1.myColor.b*255.0));
		long lNum2 = ((int)(scolColor2.myColor.r*255.0))*1000000 + ((int)(scolColor2.myColor.g*255.0))*1000 + ((int)(scolColor2.myColor.b*255.0));
		return lNum1 < lNum2;
	}
};
typedef set < SIndexedColor, SIndexedColor > TUniqueIndexedColorSet;
typedef vector < SIndexedColor > TIndexedColorVector;
/*****************************************************************************/