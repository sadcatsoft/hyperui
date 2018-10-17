#include "stdafx.h"

namespace HyperCore
{
string *g_colorColorDelimeters = NULL;

template < class S, int iMaxChannelValue > const TColor<S, iMaxChannelValue> TColor<S, iMaxChannelValue>::White(iMaxChannelValue, iMaxChannelValue, iMaxChannelValue, iMaxChannelValue);
template < class S, int iMaxChannelValue > const TColor<S, iMaxChannelValue> TColor<S, iMaxChannelValue>::Red(iMaxChannelValue, 0, 0, iMaxChannelValue);
template < class S, int iMaxChannelValue > const TColor<S, iMaxChannelValue> TColor<S, iMaxChannelValue>::Green(0, iMaxChannelValue, 0, iMaxChannelValue);
template < class S, int iMaxChannelValue > const TColor<S, iMaxChannelValue> TColor<S, iMaxChannelValue>::Blue(0, 0, iMaxChannelValue, iMaxChannelValue);
template < class S, int iMaxChannelValue > const TColor<S, iMaxChannelValue> TColor<S, iMaxChannelValue>::TransparentWhite(iMaxChannelValue, iMaxChannelValue, iMaxChannelValue, 0);
template < class S, int iMaxChannelValue > const TColor<S, iMaxChannelValue> TColor<S, iMaxChannelValue>::TransparentBlack(0, 0, 0, 0);
template < class S, int iMaxChannelValue > const TColor<S, iMaxChannelValue> TColor<S, iMaxChannelValue>::Black(0, 0, 0, iMaxChannelValue);
template < class S, int iMaxChannelValue > const TColor<S, iMaxChannelValue> TColor<S, iMaxChannelValue>::Magenta(iMaxChannelValue, 0, iMaxChannelValue, iMaxChannelValue);
template < class S, int iMaxChannelValue > const TColor<S, iMaxChannelValue> TColor<S, iMaxChannelValue>::BabyBlue( 15.0/255.0*(FLOAT_TYPE)iMaxChannelValue, 156.0/255.0*(FLOAT_TYPE)iMaxChannelValue, iMaxChannelValue, iMaxChannelValue);

template < class S, int iMaxChannelValue > map < string, TColor<S, iMaxChannelValue> > * TColor<S, iMaxChannelValue>::theColorNameMap = NULL;
/*****************************************************************************/
template < class S, int iMaxChannelValue >
void TColor<S, iMaxChannelValue>::ensureColorMapInitialized()
{
	if(theColorNameMap)
		return;

	theColorNameMap = new map < string, TColor >;

	(*theColorNameMap)["aliceblue"] = SColor(240, 248, 255);
	(*theColorNameMap)["antiquewhite"] = SColor(250, 235, 215);
	(*theColorNameMap)["aqua"] = SColor( 0, 255, 255);
	(*theColorNameMap)["aquamarine"] = SColor(127, 255, 212);
	(*theColorNameMap)["azure"] = SColor(240, 255, 255);
	(*theColorNameMap)["beige"] = SColor(245, 245, 220);
	(*theColorNameMap)["bisque"] = SColor(255, 228, 196);
	(*theColorNameMap)["black"] = SColor( 0, 0, 0);
	(*theColorNameMap)["blanchedalmond"] = SColor(255, 235, 205);
	(*theColorNameMap)["blue"] = SColor( 0, 0, 255);
	(*theColorNameMap)["blueviolet"] = SColor(138, 43, 226);
	(*theColorNameMap)["brown"] = SColor(165, 42, 42);
	(*theColorNameMap)["burlywood"] = SColor(222, 184, 135);
	(*theColorNameMap)["cadetblue"] = SColor( 95, 158, 160);
	(*theColorNameMap)["chartreuse"] = SColor(127, 255, 0);
	(*theColorNameMap)["chocolate"] = SColor(210, 105, 30);
	(*theColorNameMap)["coral"] = SColor(255, 127, 80);
	(*theColorNameMap)["cornflowerblue"] = SColor(100, 149, 237);
	(*theColorNameMap)["cornsilk"] = SColor(255, 248, 220);
	(*theColorNameMap)["crimson"] = SColor(220, 20, 60);
	(*theColorNameMap)["cyan"] = SColor( 0, 255, 255);
	(*theColorNameMap)["darkblue"] = SColor( 0, 0, 139);
	(*theColorNameMap)["darkcyan"] = SColor( 0, 139, 139);
	(*theColorNameMap)["darkgoldenrod"] = SColor(184, 134, 11);
	(*theColorNameMap)["darkgray"] = SColor(169, 169, 169);
	(*theColorNameMap)["darkgreen"] = SColor( 0, 100, 0);
	(*theColorNameMap)["darkgrey"] = SColor(169, 169, 169);
	(*theColorNameMap)["darkkhaki"] = SColor(189, 183, 107);
	(*theColorNameMap)["darkmagenta"] = SColor(139, 0, 139);
	(*theColorNameMap)["darkolivegreen"] = SColor( 85, 107, 47);
	(*theColorNameMap)["darkorange"] = SColor(255, 140, 0);
	(*theColorNameMap)["darkorchid"] = SColor(153, 50, 204);
	(*theColorNameMap)["darkred"] = SColor(139, 0, 0);
	(*theColorNameMap)["darksalmon"] = SColor(233, 150, 122);
	(*theColorNameMap)["darkseagreen"] = SColor(143, 188, 143);
	(*theColorNameMap)["darkslateblue"] = SColor( 72, 61, 139);
	(*theColorNameMap)["darkslategray"] = SColor( 47, 79, 79);
	(*theColorNameMap)["darkslategrey"] = SColor( 47, 79, 79);
	(*theColorNameMap)["darkturquoise"] = SColor( 0, 206, 209);
	(*theColorNameMap)["darkviolet"] = SColor(148, 0, 211);
	(*theColorNameMap)["deeppink"] = SColor(255, 20, 147);
	(*theColorNameMap)["deepskyblue"] = SColor( 0, 191, 255);
	(*theColorNameMap)["dimgray"] = SColor(105, 105, 105);
	(*theColorNameMap)["dimgrey"] = SColor(105, 105, 105);
	(*theColorNameMap)["dodgerblue"] = SColor( 30, 144, 255);
	(*theColorNameMap)["firebrick"] = SColor(178, 34, 34);
	(*theColorNameMap)["floralwhite"] = SColor(255, 250, 240);
	(*theColorNameMap)["forestgreen"] = SColor( 34, 139, 34);
	(*theColorNameMap)["fuchsia"] = SColor(255, 0, 255);
	(*theColorNameMap)["gainsboro"] = SColor(220, 220, 220);
	(*theColorNameMap)["ghostwhite"] = SColor(248, 248, 255);
	(*theColorNameMap)["gold"] = SColor(255, 215, 0);
	(*theColorNameMap)["goldenrod"] = SColor(218, 165, 32);
	(*theColorNameMap)["gray"] = SColor(128, 128, 128);
	(*theColorNameMap)["grey"] = SColor(128, 128, 128);
	(*theColorNameMap)["green"] = SColor( 0, 128, 0);
	(*theColorNameMap)["greenyellow"] = SColor(173, 255, 47);
	(*theColorNameMap)["honeydew"] = SColor(240, 255, 240);
	(*theColorNameMap)["hotpink"] = SColor(255, 105, 180);
	(*theColorNameMap)["indianred"] = SColor(205, 92, 92);
	(*theColorNameMap)["indigo"] = SColor( 75, 0, 130);
	(*theColorNameMap)["ivory"] = SColor(255, 255, 240);
	(*theColorNameMap)["khaki"] = SColor(240, 230, 140);
	(*theColorNameMap)["lavender"] = SColor(230, 230, 250);
	(*theColorNameMap)["lavenderblush"] = SColor(255, 240, 245);
	(*theColorNameMap)["lawngreen"] = SColor(124, 252, 0);
	(*theColorNameMap)["lemonchiffon"] = SColor(255, 250, 205);
	(*theColorNameMap)["lightblue"] = SColor(173, 216, 230);
	(*theColorNameMap)["lightcoral"] = SColor(240, 128, 128);
	(*theColorNameMap)["lightcyan"] = SColor(224, 255, 255);
	(*theColorNameMap)["lightgoldenrodyellow"] = SColor(250, 250, 210);
	(*theColorNameMap)["lightgray"] = SColor(211, 211, 211);
	(*theColorNameMap)["lightgreen"] = SColor(144, 238, 144);
	(*theColorNameMap)["lightgrey"] = SColor(211, 211, 211);
	(*theColorNameMap)["lightpink"] = SColor(255, 182, 193);
	(*theColorNameMap)["lightsalmon"] = SColor(255, 160, 122);
	(*theColorNameMap)["lightseagreen"] = SColor( 32, 178, 170);
	(*theColorNameMap)["lightskyblue"] = SColor(135, 206, 250);
	(*theColorNameMap)["lightslategray"] = SColor(119, 136, 153);
	(*theColorNameMap)["lightslategrey"] = SColor(119, 136, 153);
	(*theColorNameMap)["lightsteelblue"] = SColor(176, 196, 222);
	(*theColorNameMap)["lightyellow"] = SColor(255, 255, 224);
	(*theColorNameMap)["lime"] = SColor( 0, 255, 0);
	(*theColorNameMap)["limegreen"] = SColor( 50, 205, 50);
	(*theColorNameMap)["linen"] = SColor(250, 240, 230);
	(*theColorNameMap)["magenta"] = SColor(255, 0, 255);
	(*theColorNameMap)["maroon"] = SColor(128, 0, 0);
	(*theColorNameMap)["mediumaquamarine"] = SColor(102, 205, 170);
	(*theColorNameMap)["mediumblue"] = SColor( 0, 0, 205);
	(*theColorNameMap)["mediumorchid"] = SColor(186, 85, 211);
	(*theColorNameMap)["mediumpurple"] = SColor(147, 112, 219);
	(*theColorNameMap)["mediumseagreen"] = SColor( 60, 179, 113);
	(*theColorNameMap)["mediumslateblue"] = SColor(123, 104, 238);
	(*theColorNameMap)["mediumspringgreen"] = SColor( 0, 250, 154);
	(*theColorNameMap)["mediumturquoise"] = SColor( 72, 209, 204);
	(*theColorNameMap)["mediumvioletred"] = SColor(199, 21, 133);
	(*theColorNameMap)["midnightblue"] = SColor( 25, 25, 112);
	(*theColorNameMap)["mintcream"] = SColor(245, 255, 250);
	(*theColorNameMap)["mistyrose"] = SColor(255, 228, 225);
	(*theColorNameMap)["moccasin"] = SColor(255, 228, 181);
	(*theColorNameMap)["navajowhite"] = SColor(255, 222, 173);
	(*theColorNameMap)["navy"] = SColor( 0, 0, 128);
	(*theColorNameMap)["oldlace"] = SColor(253, 245, 230);
	(*theColorNameMap)["olive"] = SColor(128, 128, 0);
	(*theColorNameMap)["olivedrab"] = SColor(107, 142, 35);
	(*theColorNameMap)["orange"] = SColor(255, 165, 0);
	(*theColorNameMap)["orangered"] = SColor(255, 69, 0);
	(*theColorNameMap)["orchid"] = SColor(218, 112, 214);
	(*theColorNameMap)["palegoldenrod"] = SColor(238, 232, 170);
	(*theColorNameMap)["palegreen"] = SColor(152, 251, 152);
	(*theColorNameMap)["paleturquoise"] = SColor(175, 238, 238);
	(*theColorNameMap)["palevioletred"] = SColor(219, 112, 147);
	(*theColorNameMap)["papayawhip"] = SColor(255, 239, 213);
	(*theColorNameMap)["peachpuff"] = SColor(255, 218, 185);
	(*theColorNameMap)["peru"] = SColor(205, 133, 63);
	(*theColorNameMap)["pink"] = SColor(255, 192, 203);
	(*theColorNameMap)["plum"] = SColor(221, 160, 221);
	(*theColorNameMap)["powderblue"] = SColor(176, 224, 230);
	(*theColorNameMap)["purple"] = SColor(128, 0, 128);
	(*theColorNameMap)["red"] = SColor(255, 0, 0);
	(*theColorNameMap)["rosybrown"] = SColor(188, 143, 143);
	(*theColorNameMap)["royalblue"] = SColor( 65, 105, 225);
	(*theColorNameMap)["saddlebrown"] = SColor(139, 69, 19);
	(*theColorNameMap)["salmon"] = SColor(250, 128, 114);
	(*theColorNameMap)["sandybrown"] = SColor(244, 164, 96);
	(*theColorNameMap)["seagreen"] = SColor( 46, 139, 87);
	(*theColorNameMap)["seashell"] = SColor(255, 245, 238);
	(*theColorNameMap)["sienna"] = SColor(160, 82, 45);
	(*theColorNameMap)["silver"] = SColor(192, 192, 192);
	(*theColorNameMap)["skyblue"] = SColor(135, 206, 235);
	(*theColorNameMap)["slateblue"] = SColor(106, 90, 205);
	(*theColorNameMap)["slategray"] = SColor(112, 128, 144);
	(*theColorNameMap)["slategrey"] = SColor(112, 128, 144);
	(*theColorNameMap)["snow"] = SColor(255, 250, 250);
	(*theColorNameMap)["springgreen"] = SColor( 0, 255, 127);
	(*theColorNameMap)["steelblue"] = SColor( 70, 130, 180);
	(*theColorNameMap)["tan"] = SColor(210, 180, 140);
	(*theColorNameMap)["teal"] = SColor( 0, 128, 128);
	(*theColorNameMap)["thistle"] = SColor(216, 191, 216);
	(*theColorNameMap)["tomato"] = SColor(255, 99, 71);
	(*theColorNameMap)["turquoise"] = SColor( 64, 224, 208);
	(*theColorNameMap)["violet"] = SColor(238, 130, 238);
	(*theColorNameMap)["wheat"] = SColor(245, 222, 179);
	(*theColorNameMap)["white"] = SColor(255, 255, 255);
	(*theColorNameMap)["whitesmoke"] = SColor(245, 245, 245);
	(*theColorNameMap)["yellow"] = SColor(255, 255, 0);
	(*theColorNameMap)["yellowgreen"] = SColor(154, 205, 50);

	typename map < string, TColor<S, iMaxChannelValue> >::iterator mi;
	for(mi = theColorNameMap->begin(); mi != theColorNameMap->end(); mi++)
	{
		mi->second.r *= (FLOAT_TYPE)iMaxChannelValue/255.0;
		mi->second.g *= (FLOAT_TYPE)iMaxChannelValue/255.0;
		mi->second.b *= (FLOAT_TYPE)iMaxChannelValue/255.0;
	}
}
/*****************************************************************************/
inline char HexCharToInt(char n)
{
	if (n >= '0' && n <= '9')
		return (n-'0');
	else
	{
		if (n >= 'a' && n <= 'f')
			return (n-'a'+10);
		else if (n >= 'A' && n <= 'F')
			return (n-'A'+10);
		else
			return 0;
	}
}
/*****************************************************************************/
inline int HexTwoBytesToInt(char c1, char c2) 
{	
	return HexCharToInt(c1)*16 + HexCharToInt(c2); 
}
/*****************************************************************************/
FLOAT_TYPE Color_HueToRgb(FLOAT_TYPE M1, FLOAT_TYPE M2, FLOAT_TYPE Hue)
{
	FLOAT_TYPE Channel;
	if (Hue < 0.0)
		Hue += 1.0;
	else if (Hue > 1.0)
		Hue -= 1.0;

	if ((6.0 * Hue) < 1.0)
		Channel = (M1 + (M2 - M1) * Hue * 6.0);
	else if ((2.0 * Hue) < 1.0)
		Channel = (M2);
	else if ((3.0 * Hue) < 2.0)
		Channel = (M1 + (M2 - M1) * ((2.0F / 3.0F) - Hue) * 6.0);
	else
		Channel = (M1);

	return Channel;
}
/********************************************************************************************/
template < class S, int iMaxChannelValue >
bool TColor<S, iMaxChannelValue>::fromNamedColor(const char* pString)
{
	if(!theColorNameMap)
		ensureColorMapInitialized();

	typename map < string, TColor<S, iMaxChannelValue> >::iterator mi = theColorNameMap->find(pString);
	if(mi == theColorNameMap->end())
		return false;
	else
		*this = mi->second;
	return true;
}
/*****************************************************************************/
template < class S, int iMaxChannelValue >
bool TColor<S, iMaxChannelValue>::fromHex(const char* pString)
{
	int iStringLen = strlen(pString);
	if(iStringLen < 6)
		return false;

	FLOAT_TYPE dTemp;

	dTemp = ((FLOAT_TYPE)HexTwoBytesToInt(pString[0], pString[1]))/(FLOAT_TYPE)255.0;
	r = dTemp*(FLOAT_TYPE)iMaxChannelValue;
	dTemp = ((FLOAT_TYPE)HexTwoBytesToInt(pString[2], pString[3]))/(FLOAT_TYPE)255.0;
	g = dTemp*(FLOAT_TYPE)iMaxChannelValue;
	dTemp = ((FLOAT_TYPE)HexTwoBytesToInt(pString[4], pString[5]))/(FLOAT_TYPE)255.0;
	b = dTemp*(FLOAT_TYPE)iMaxChannelValue;

	if(iStringLen >= 8)
	{
		dTemp = ((FLOAT_TYPE)HexTwoBytesToInt(pString[6], pString[7]))/(FLOAT_TYPE)255.0;
		alpha = dTemp*(FLOAT_TYPE)iMaxChannelValue;
	}
	else
		alpha = 1.0;

	return true;
}
/*****************************************************************************/
template < class S, int iMaxChannelValue >
void TColor<S, iMaxChannelValue>::fromHLS(FLOAT_TYPE Hue, FLOAT_TYPE Lumination, FLOAT_TYPE Saturation)
{
	FLOAT_TYPE M1, M2;
	if (Saturation == 0)
	{
		r = Lumination;
		g = Lumination;
		b = Lumination;
	}
	else
	{
		if (Lumination <= 0.5)
			M2 = Lumination * (1.0 + Saturation);
		else
			M2 = Lumination + Saturation - Lumination * Saturation;

		M1 = (2.0 * Lumination - M2);

		r = Color_HueToRgb(M1, M2, Hue + (1.0F / 3.0F));
		g = Color_HueToRgb(M1, M2, Hue);
		b = Color_HueToRgb(M1, M2, Hue - (1.0F / 3.0F));
	}
}

/*****************************************************************************/
template < class S, int iMaxChannelValue >
bool TColor<S, iMaxChannelValue>::fromFloatString(const string& strText)
{
	if(!g_colorColorDelimeters)
		g_colorColorDelimeters = new string(" {},%()");

	string strSafeCopy(strText.c_str());
	// One more for faulty values
	const int iMaxTokens = 5;
	char* pcsTokens[iMaxTokens];
	int iNumTokensParsed = TokenizeUtils::tokenizeStringToCharPtrsInPlace(strSafeCopy.c_str(), *g_colorColorDelimeters, pcsTokens, iMaxTokens);
	if(iNumTokensParsed != 3 && iNumTokensParsed != 4)
		return false;

	if(!StringUtils::isANumber(pcsTokens[0], true))
		return false;

	r = atof(pcsTokens[0]);
	g = atof(pcsTokens[1]);
	b = atof(pcsTokens[2]);
	if(iNumTokensParsed > 3)
		alpha = atof(pcsTokens[3]);
	else
		alpha = iMaxChannelValue;

	return true;
}
/*****************************************************************************/
template < class S, int iMaxChannelValue >
void TColor<S, iMaxChannelValue>::fromHSB(FLOAT_TYPE h, FLOAT_TYPE s, FLOAT_TYPE v)
{
	if(s <= 0.0)
	{
		r = g = b = v;
		return;
	}

	h *= 360.0;
	int hi = (int)(h/60.0) % 6;
	FLOAT_TYPE f = h/60.0 - hi;

	FLOAT_TYPE p = v*(1.0-s);
	FLOAT_TYPE q = v*(1.0-f*s);
	FLOAT_TYPE t = v*(1.0-(1.0-f)*s);

	if( hi == 0)
	{
		r = v;
		g = t;
		b = p;
	}
	else if( hi == 1)
	{
		r = q;
		g = v;
		b = p;
	}
	else if( hi == 2)
	{
		r = p;
		g = v;
		b = t;
	}
	else if( hi == 3)
	{
		r = p;
		g = q;
		b = v;
	}
	else if( hi == 4)
	{
		r = t;
		g = p;
		b = v;
	}
	else if( hi == 5)
	{
		r = v;
		g = p;
		b = q;
	}
}
/*****************************************************************************/
template < class S, int iMaxChannelValue >
void TColor<S, iMaxChannelValue>::toHSB(FLOAT_TYPE& dH, FLOAT_TYPE& dS, FLOAT_TYPE& dB) const
{
	FLOAT_TYPE dMax = max(max(r,g), b);
	FLOAT_TYPE dMin = min(min(r,g), b);
	FLOAT_TYPE h,s,v;

	if(dMin == dMax)
		h = 0.0;
	else if(dMax == r && g>=b)
		h = 60.0 * (g-b)/(dMax-dMin);
	else if(dMax == r && g<b)
		h = 60.0 * (g-b)/(dMax-dMin) + 360.0;
	else if(dMax == g)
		h = 60.0 * (b-r)/(dMax-dMin) + 120.0;
	else if(dMax == b)
		h = 60.0* (r-g)/(dMax-dMin)+240.0;
	h /= 360.0;

	if(dMax == 0.0)
		s = 0.0;
	else
		s = (dMax - dMin)/dMax;
	v = dMax;

	dH = h;
	dS = s;
	dB = v;
}
/*****************************************************************************/
template < class S, int iMaxChannelValue >
void TColor<S, iMaxChannelValue>::toHCL(FLOAT_TYPE& fH, FLOAT_TYPE& fC, FLOAT_TYPE& fL) const
{
	FLOAT_TYPE dMax = max(max(r,g), b);
	FLOAT_TYPE dMin = min(min(r,g), b);

	fC = dMax - dMin;
	// Polygon space
	FLOAT_TYPE fTempH;
	if(dMin == dMax)
		fTempH = 0;
	else if(dMax == r)
		fTempH = fmod((FLOAT_TYPE)(g - b)/fC, (FLOAT_TYPE)6.0);
	else if(dMax == g)
		fTempH = (b - r)/fC + 2.0;
	else if(dMax == b)
		fTempH = (r - g)/fC + 4.0;
	fH = fTempH*60.0/360.0;

/*
	// Circle space
	FLOAT_TYPE fAlpha = (2.0*r - g - b)*0.5;
	const FLOAT_TYPE fSquareRootOf3 = 1.7320508075688772935274463415059/2.0;
	FLOAT_TYPE fBeta = fSquareRootOf3*(g - b);
	fH = F_ATAN2(fBeta, fAlpha)/360.0;
	fC = F_SQRT(fAlpha*fAlpha + fBeta*fBeta);
*/
	fL = 0.21*r + 0.72*g + 0.07*b;
}
/*****************************************************************************/
template < class S, int iMaxChannelValue >
void TColor<S, iMaxChannelValue>::fromHCL(FLOAT_TYPE fH, FLOAT_TYPE fC, FLOAT_TYPE fL)
{
	FLOAT_TYPE fQuadrant = fH*360.0/60.0;
	FLOAT_TYPE fX = fC*(1.0 - fabs((FLOAT_TYPE)fmod(fQuadrant, (FLOAT_TYPE)2.0) - 1.0));

	r = g = b = 0.0;
	if(fQuadrant >= 0.0 && fQuadrant < 1.0)
	{
		r = fC;
		g = fX;
	}
	else if(fQuadrant >= 1.0 && fQuadrant < 2.0)
	{
		r = fX;
		g = fC;
	}
	else if(fQuadrant >= 2.0 && fQuadrant < 3.0)
	{
		g = fC;
		b = fX;
	}
	else if(fQuadrant >= 3.0 && fQuadrant < 4.0)
	{
		g = fX;
		b = fC;
	}
	else if(fQuadrant >= 4.0 && fQuadrant < 5.0)
	{
		r = fX;
		b = fC;
	}
	else if(fQuadrant >= 5.0 && fQuadrant < 6.0)
	{
		r = fC;
		b = fX;
	}

	FLOAT_TYPE fm = fL - (0.21*r + 0.72*g + 0.07*b);
	r += fm;
	g += fm;
	b += fm;
}
/*****************************************************************************/
template < class S, int iMaxChannelValue >
void TColor<S, iMaxChannelValue>::toCMYK(FLOAT_TYPE& fC, FLOAT_TYPE& fM, FLOAT_TYPE& fY, FLOAT_TYPE& fK) const
{
	FLOAT_TYPE color[4] = { r, g, b, alpha };
	if (color[0]==0 && color[1]==0 && color[2]==0)
	{
		color[0] = 0;
		color[1] = 0;
		color[2] = 0;
		color[3] = 1.0;
	}
	else if (color[0]==1.0 && color[1]==1.0 && color[2]==1.0)
	{
		color[0] = 0;
		color[1] = 0;
		color[2] = 0;
		color[3] = 0;
	}
	else
	{
		color[0] = 1.0 - color[0];
		color[1] = 1.0 - color[1];
		color[2] = 1.0 - color[2];

		FLOAT_TYPE minK = min(color[0], min(color[1], color[2]));
		color[0] = (color[0] - minK) / (1.0 - minK);
		color[1] = (color[1] - minK) / (1.0 - minK);
		color[2] = (color[2] - minK) / (1.0 - minK);
		color[3] = minK;
	}

	fC = color[0];
	fM = color[1];
	fY = color[2];
	fK = color[3];
}
/*****************************************************************************/
template < class S, int iMaxChannelValue >
void TColor<S, iMaxChannelValue>::fromCMYK(FLOAT_TYPE fC, FLOAT_TYPE fM, FLOAT_TYPE fY, FLOAT_TYPE fK)
{
	const FLOAT_TYPE color[4] = { fC, fM, fY, fK };
	r = 1.0 - (color[0] * (1.0 - color[3]) + color[3]);
	g = 1.0 - (color[1] * (1.0 - color[3]) + color[3]);
	b = 1.0 - (color[2] * (1.0 - color[3]) + color[3]);
}
/*****************************************************************************/
template < class S, int iMaxChannelValue >
void TColor<S, iMaxChannelValue>::fromTemperature(FLOAT_TYPE fTemperature)
{
	fTemperature = HyperCore::clampToRange<FLOAT_TYPE>(1000, 40000, fTemperature)/100.0;

	FLOAT_TYPE fTemp;

	// Red
	if(fTemperature <= 66.0)
		r = iMaxChannelValue;
	else
	{
		fTemp = fTemperature - 60;
		fTemp = (329.698727446 * pow((S)fTemp, (S)-0.1332047592)/255.0)*iMaxChannelValue;
		r = HyperCore::clampToRange<S>(0, iMaxChannelValue, fTemp);
	}

	// Green
	if(fTemperature <= 66.0)
		fTemp = (99.4708025861 * log(fTemperature) - 161.1195681661)/255.0*iMaxChannelValue;
	else
	{
		fTemp = fTemperature - 60;
		fTemp = (288.1221695283 * pow((S)fTemp, (S)-0.0755148492))/255.0*iMaxChannelValue;
	}
	g = HyperCore::clampToRange<S>(0, iMaxChannelValue, fTemp);

	// Blue
	if(fTemperature >= 66.0)
		b = iMaxChannelValue;
	else if(fTemperature <= 19.0)
		b = 0;
	else
	{
		fTemp = fTemperature - 10;
		fTemp = (138.5177312231 * log(fTemp) - 305.0447927307)/255.0*iMaxChannelValue;
		b = HyperCore::clampToRange<S>(0, iMaxChannelValue, fTemp);
	}

	// Alpha
	alpha = iMaxChannelValue;
}
/*****************************************************************************/
template < class S, int iMaxChannelValue >
void TColor<S, iMaxChannelValue>::toHLS(FLOAT_TYPE& Hue, FLOAT_TYPE& Lumination, FLOAT_TYPE& Saturation)
{
	FLOAT_TYPE Delta;
	FLOAT_TYPE Max, Min;

	Max     = max(max(r, g), b);
	Min     = min(min(r, g), b);

	Hue        = 0;
	Lumination = (Max + Min) / 2.0F;
	Saturation = 0;

	if (Max == Min)
		return;

	Delta = (Max - Min);

	if (Lumination < 0.5)
		Saturation = Delta / (Max + Min);
	else
		Saturation = Delta / (2.0 - Max - Min);

	if (r == Max)
		Hue = (g - b) / Delta;
	else if (g == Max)
		Hue = 2.0 + (b - r) / Delta;
	else
		Hue = 4.0 + (r - g) / Delta;

	Hue /= 6.0;

	if (Hue < 0.0)
		Hue += 1.0;
}
/*****************************************************************************/
template < class S, int iMaxChannelValue >
void TColor<S, iMaxChannelValue>::toHex(string& strOut, bool bIncludeAlpha) const
{
	char pcsRes[9];
	toHex(pcsRes, bIncludeAlpha);
	strOut = pcsRes;
}
/*****************************************************************************/
template < class S, int iMaxChannelValue >
void TColor<S, iMaxChannelValue>::toHex(char* pcsRes, bool bIncludeAlpha) const
{
	int iVal;
	iVal = (r/(FLOAT_TYPE)iMaxChannelValue)*255.0;
	sprintf(&pcsRes[0], "%02x", iVal);

	iVal = (g/(FLOAT_TYPE)iMaxChannelValue)*255.0;
	sprintf(&pcsRes[2], "%02x", iVal);

	iVal = (b/(FLOAT_TYPE)iMaxChannelValue)*255.0;
	sprintf(&pcsRes[4], "%02x", iVal);

	if(bIncludeAlpha)
	{
		iVal = (alpha/(FLOAT_TYPE)iMaxChannelValue)*255.0;
		sprintf(&pcsRes[6], "%02x", iVal);
		pcsRes[8] = 0;
	}
	else
	{
		pcsRes[6] = 0;
	}
}
/*****************************************************************************/
FLOAT_TYPE *g_fColorBalanceHghlights = NULL;
FLOAT_TYPE *g_fColorBalanceMidtones = NULL;
FLOAT_TYPE *g_fColorBalanceShadows = NULL;

const int iLastColorStep = 65535;
void ensureColorBalanceArraysValid()
{
	if(g_fColorBalanceHghlights)
		return;

	g_fColorBalanceHghlights = new FLOAT_TYPE[iLastColorStep + 1];
	g_fColorBalanceMidtones = new FLOAT_TYPE[iLastColorStep + 1];
	g_fColorBalanceShadows = new FLOAT_TYPE[iLastColorStep + 1];

	// Fill them in
	const FLOAT_TYPE fA = 64.0/255.0*(FLOAT_TYPE)iLastColorStep;
	const FLOAT_TYPE fB = 85.0/255.0*(FLOAT_TYPE)iLastColorStep;
	const FLOAT_TYPE fScale = 1.785;

	FLOAT_TYPE fLow, fMid;
	int iStep;
	for(iStep = 0; iStep <= iLastColorStep; iStep++)
	{
		fLow = HyperCore::clampToRange<FLOAT_TYPE>(0, 1, (iStep - fB) / -fA + .5)*fScale;
		fMid = HyperCore::clampToRange<FLOAT_TYPE>(0, 1, (iStep - fB) /  fA  + .5)*HyperCore::clampToRange<FLOAT_TYPE>(0, 1, (iStep + fB - iLastColorStep) / -fA + .5)*fScale;

		// Aribtrary scaling...
		fLow *= (FLOAT_TYPE)iLastColorStep/4;
		fMid *= (FLOAT_TYPE)iLastColorStep/4;

		g_fColorBalanceShadows[iStep] = fLow;
		g_fColorBalanceMidtones[iStep] = fMid;
		g_fColorBalanceHghlights[iLastColorStep - iStep] = fLow;
	}
}

template < class S, int iMaxChannelValue >
void TColor<S, iMaxChannelValue>::adjustColorBalance(FLOAT_TYPE fCyanRedShadows, FLOAT_TYPE fMagentaGreeShadows, FLOAT_TYPE fYellowBlueShadows,
	FLOAT_TYPE fCyanRedMidtones, FLOAT_TYPE fMagentaGreeMidtones, FLOAT_TYPE fYellowBlueMidtones,
	FLOAT_TYPE fCyanRedHighlights, FLOAT_TYPE fMagentaGreeHighlights, FLOAT_TYPE fYellowBlueHighlights,
	bool bPreserveLuminocity)
{
	ensureColorBalanceArraysValid();

	FLOAT_TYPE fH, fS, fL1, fL2;
	int iColorIndex;
	FLOAT_TYPE fCurrCol;

	if(bPreserveLuminocity)
		toHLS(fH, fL1, fS);

	fCurrCol = r*(FLOAT_TYPE)iLastColorStep;
	iColorIndex = HyperCore::clampToRange<FLOAT_TYPE>(0, iLastColorStep, fCurrCol);
	fCurrCol += fCyanRedShadows * g_fColorBalanceShadows[iColorIndex];
	fCurrCol += fCyanRedMidtones * g_fColorBalanceMidtones[iColorIndex];
	fCurrCol += fCyanRedHighlights * g_fColorBalanceHghlights[iColorIndex];
	r = HyperCore::clampToRange<FLOAT_TYPE>(0, 1, fCurrCol/(FLOAT_TYPE)iLastColorStep);

	fCurrCol = g*(FLOAT_TYPE)iLastColorStep;
	iColorIndex = HyperCore::clampToRange<FLOAT_TYPE>(0, iLastColorStep, fCurrCol);
	fCurrCol += fMagentaGreeShadows * g_fColorBalanceShadows[iColorIndex];
	fCurrCol += fMagentaGreeMidtones * g_fColorBalanceMidtones[iColorIndex];
	fCurrCol += fMagentaGreeHighlights * g_fColorBalanceHghlights[iColorIndex];
	g = HyperCore::clampToRange<FLOAT_TYPE>(0, 1, fCurrCol/(FLOAT_TYPE)iLastColorStep);

	fCurrCol = b*(FLOAT_TYPE)iLastColorStep;
	iColorIndex = HyperCore::clampToRange<FLOAT_TYPE>(0, iLastColorStep, fCurrCol);
	fCurrCol += fYellowBlueShadows * g_fColorBalanceShadows[iColorIndex];
	fCurrCol += fYellowBlueMidtones * g_fColorBalanceMidtones[iColorIndex];
	fCurrCol += fYellowBlueHighlights * g_fColorBalanceHghlights[iColorIndex];
	b = HyperCore::clampToRange<FLOAT_TYPE>(0, 1, fCurrCol/(FLOAT_TYPE)iLastColorStep);

	if(bPreserveLuminocity)
	{
		toHLS(fH, fL2, fS);
		fromHLS(fH, fL1, fS);
		clamp();
	}
}
/*****************************************************************************/
template < class S, int iMaxChannelValue >
bool TColor<S, iMaxChannelValue>::fromCSS(const char* pcsString)
{
	if(!pcsString)
		return false;

	int iLen = strlen(pcsString);
	if(iLen == 0)
		return false;

	bool bRes = false;

	// color    ::= "#" hexdigit hexdigit hexdigit (hexdigit hexdigit hexdigit)?
	//	| "rgb(" wsp* integer comma integer comma integer wsp* ")"
	//	| "rgb(" wsp* integer "%" comma integer "%" comma integer "%" wsp* ")"
	//	| color-keyword
	if(pcsString[0] == '#')
		bRes = fromHex(pcsString + 1);
	else if(iLen >= 3 && tolower(pcsString[0]) == 'r' && tolower(pcsString[1]) == 'g' && tolower(pcsString[2]) == 'b')
	{
		// rgb, rgba, or percentages
		int iBracketStart;
		for(iBracketStart = 3; pcsString[iBracketStart] && pcsString[iBracketStart] != '('; iBracketStart++);

		if(iBracketStart < iLen)
			bRes = fromFloatString(pcsString + iBracketStart);

		// Now, if we have no percentages, convert the first r,g,b values from 0 to 255 to 0 to 1.
		if(bRes && strchr(pcsString, '%') == NULL)
		{
			r = r/255.0*iMaxChannelValue;
			g = g/255.0*iMaxChannelValue;
			b = b/255.0*iMaxChannelValue;
			alpha *= iMaxChannelValue;
		}
	}
	else
		bRes = fromNamedColor(pcsString);

	return bRes;
}
/*****************************************************************************/
template class TColor< float, 1 >;
/*****************************************************************************/
};