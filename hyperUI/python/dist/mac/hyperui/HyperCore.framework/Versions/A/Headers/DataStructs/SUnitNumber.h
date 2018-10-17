#pragma once
/*

This class represents a united value that is responsible for its own conversion 
and stringization. Also, wow, class comments!

*/

/*****************************************************************************/
class HYPERCORE_API SUnitNumber
{
public:

	SUnitNumber();
	SUnitNumber(FLOAT_TYPE fValue, UnitType eUnits);
	SUnitNumber(const char* pcsStringRepr);

	SUnitNumber convertTo(UnitType eType, FLOAT_TYPE fRelativeToValue = FLOAT_TYPE_MAX, FLOAT_TYPE fDpi = FLOAT_TYPE_MAX) const;
	void toStringUsingFormat(const char* pcsFormat, bool bAppendUnits, string& strOut) const;
	void toString(string& strOut) const;

	bool setFromString(const char* pcsString);
	void set(FLOAT_TYPE fValue, UnitType eUnits);

	inline FLOAT_TYPE getRawValue() const { return myValue; }
	inline void setRawValue(FLOAT_TYPE fValue) { myValue = fValue; }
	FLOAT_TYPE getConvertedValue(UnitType eTargetUnits, FLOAT_TYPE fRelativeToValue = FLOAT_TYPE_MAX, FLOAT_TYPE fDpi = FLOAT_TYPE_MAX) const;

	inline UnitType getUnits() const { return myUnits; }
	void assignUnits(UnitType eOverrideUnits) { myUnits = eOverrideUnits; }

private:

	double convertToMM(FLOAT_TYPE fValue, UnitType eCurrUnits, FLOAT_TYPE fDpi) const;
	FLOAT_TYPE convertFromMM(double fValue, UnitType eTargetUnits, FLOAT_TYPE fDpi) const;

private:

	FLOAT_TYPE myValue;
	UnitType myUnits;
};
/*****************************************************************************/