#include "stdafx.h"

// Millimeters per inch
#define MM_PER_INCH		25.4

namespace HyperCore
{
/*****************************************************************************/
SUnitNumber::SUnitNumber()
{
	myValue = 0;
	myUnits = UnitLastPlaceholder;
}
/*****************************************************************************/
SUnitNumber::SUnitNumber(FLOAT_TYPE fValue, UnitType eUnits)
{
	myValue = fValue;
	myUnits = eUnits;
}
/*****************************************************************************/
SUnitNumber::SUnitNumber(const char* pcsStringRepr)
{
	_ASSERT(pcsStringRepr);
	setFromString(pcsStringRepr);
}
/*****************************************************************************/
FLOAT_TYPE SUnitNumber::getConvertedValue(UnitType eTargetUnits, FLOAT_TYPE fRelativeToValue, FLOAT_TYPE fDpi) const
{
	return convertTo(eTargetUnits, fRelativeToValue, fDpi).getRawValue();
}
/*****************************************************************************/
SUnitNumber SUnitNumber::convertTo(UnitType eType, FLOAT_TYPE fRelativeToValue, FLOAT_TYPE fDpi) const
{
	// If we are a unitless value, it's ok to have this call - it's more convenient and
	// consistent for the caller. Just return self.
	if(myUnits == UnitLastPlaceholder)
		return *this;

	_ASSERT(eType != UnitLastPlaceholder);

	if(eType == myUnits)
		return *this;

	SUnitNumber rResult;
	rResult.myUnits = eType;

	// Convert all values to mm using double precision, then convert back.
	// Except the percentage values.
	if(myUnits == UnitPercent)
	{
		if(fRelativeToValue != FLOAT_TYPE_MAX)
			rResult.myValue = myValue*fRelativeToValue;
		else
			rResult.myValue = myValue;
		return rResult;
	}
	else if(eType == UnitPercent)
	{
		// The target is percentage:
		_ASSERT(fRelativeToValue != FLOAT_TYPE_MAX);
		rResult.myValue = myValue/fRelativeToValue;
		return rResult;
	}

	// Else: if we're here, we're not dealing with percentages.
	// So convert them all to mm and then to the needed units:

	// Now, if we're converting to or from pixels, we change
	// pixels per inch (the usual meaning) to pixels per centimeter.
	FLOAT_TYPE fDpiMultFactor = 1.0;
	if( (myUnits == UnitPixels && (eType == UnitMillimeters || eType == UnitCentimeters))
		|| (eType == UnitPixels && (myUnits == UnitMillimeters || myUnits == UnitCentimeters)))
		fDpiMultFactor = 2.54;

	double fMMValue = convertToMM(myValue, myUnits, fDpi*fDpiMultFactor);
	rResult.myValue = convertFromMM(fMMValue, eType, fDpi*fDpiMultFactor);

	return rResult;
}
/*****************************************************************************/
double SUnitNumber::convertToMM(FLOAT_TYPE fValue, UnitType eCurrUnits, FLOAT_TYPE fDpi) const
{
	double dRes = fValue;
	_ASSERT(eCurrUnits != UnitPercent);
	if(eCurrUnits == UnitPixels)
	{
		_ASSERT(fDpi != FLOAT_TYPE_MAX && fDpi > 0);
		// Convert to inches and then to mm:
		dRes = fValue/fDpi*MM_PER_INCH;		
	}
	else if(eCurrUnits == UnitCentimeters)
		dRes = fValue*10.0;
	else if(eCurrUnits == UnitInches)
		dRes = fValue*MM_PER_INCH;
	else if(eCurrUnits == UnitMillimeters)
		dRes = fValue;
	else if(eCurrUnits == UnitPoints)
	{
		// Convert to inches first, then to mm:	
		dRes = fValue/72.0*MM_PER_INCH;
	}
	ELSE_ASSERT;

	return dRes;
}
/*****************************************************************************/
FLOAT_TYPE SUnitNumber::convertFromMM(double fValue, UnitType eTargetUnits, FLOAT_TYPE fDpi) const
{
	FLOAT_TYPE fRes = fValue;
	_ASSERT(eTargetUnits != UnitPercent);
	if(eTargetUnits == UnitPixels)
	{
		_ASSERT(fDpi != FLOAT_TYPE_MAX && fDpi > 0);
		// Convert to inches then use the DPI
		fRes = (fValue/MM_PER_INCH)*fDpi;
	}
	else if(eTargetUnits == UnitCentimeters)
		fRes = fValue/10.0;
	else if(eTargetUnits == UnitInches)
		fRes = fValue/MM_PER_INCH;
	else if(eTargetUnits == UnitMillimeters)
		fRes = fValue;
	else if(eTargetUnits == UnitPoints)
	{
		// Convert to inches first, then to points:
		fRes = (fValue/MM_PER_INCH)*72.0;
	}
	ELSE_ASSERT;

	return fRes;
}
/*****************************************************************************/
void SUnitNumber::toStringUsingFormat(const char* pcsFormat, bool bAppendUnits, string& strOut) const
{
	if(!pcsFormat)
	{
		toString(strOut);
		return;
	}

	const int iConstBuffSize = 1024;
	char* pcsFinalBuffPtr;
	char pcsStaticBuff[iConstBuffSize];
	pcsFinalBuffPtr = pcsStaticBuff;

	if(strstr(pcsFormat, "%d"))
	{
		int iTemp = myValue;
		sprintf(pcsFinalBuffPtr, pcsFormat, iTemp);
	}
	else
		sprintf(pcsFinalBuffPtr, pcsFormat, myValue);

	strOut = pcsFinalBuffPtr;
	if(myUnits != UnitLastPlaceholder && bAppendUnits)
		strOut += g_pcsUnitSuffixes[myUnits];
}
/*****************************************************************************/
void SUnitNumber::toString(string& strOut) const
{
// 	if(fabs(  myValue - (FLOAT_TYPE)((int)myValue)) < FLOAT_EPSILON)
// 		getNiceNumber(myValue, strOut);
// 	else
// 		getNiceNumber(myValue, -1, false, strOut);
	StringUtils::numberToString(myValue, strOut);

	if(myUnits != UnitLastPlaceholder)
		strOut += g_pcsUnitSuffixes[myUnits];
}
/*****************************************************************************/
void SUnitNumber::set(FLOAT_TYPE fValue, UnitType eUnits)
{
	myValue = fValue;
	myUnits = eUnits;
//	_ASSERT(eUnits != UnitLastPlaceholder);
}
/*****************************************************************************/
bool SUnitNumber::setFromString(const char* pcsString)
{
	myValue = 0;
	myUnits = UnitLastPlaceholder;
	if(!pcsString)
		ASSERT_RETURN_FALSE;

	// Now, attempt to find the first numeric digi at the back
	int iCurr, iLen = strlen(pcsString);
	if(iLen == 0)
		return false;

	for(iCurr = iLen - 1; iCurr >= 0; iCurr--)
	{
		if(isdigit(pcsString[iCurr]))
			break;
	}

	// Note that if iCurr is < 0, we have no digits at all!
	if(iCurr < 0)
		return false;

	// Attempt to find the suffix from string. iCurr is now at
	UnitType eNewUnits = UnitLastPlaceholder;
	if(iCurr < iLen - 1)
		eNewUnits = mapStringToType(&pcsString[iCurr + 1], g_pcsUnitSuffixes, UnitLastPlaceholder);
	myValue = atof(pcsString);

	// If we pass in a value with no units, keep current.
	if(eNewUnits != UnitLastPlaceholder)
		myUnits = eNewUnits;

	return true;
}
/*****************************************************************************/
};