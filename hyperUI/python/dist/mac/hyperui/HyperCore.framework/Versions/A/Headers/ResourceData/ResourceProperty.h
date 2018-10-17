#pragma once

#define MUTLINE_STRING_BEGIN_MARKER		"___[[["
#define MUTLINE_STRING_END_MARKER		"]]]___"

#define SIMPLER_JSON_SAVE_FORMAT

#define EXTRA_ARRYA_FRONT_INFO		2

// MUST BE in the same order as PropertyDataType 
// in PyParm.h
enum PropertyDataType
{ 
	PropertyDataNumber = 0,
	PropertyDataString,
	PropertyDataBool,
	PropertyDataFloatArray2D,
	PropertyDataStringList,
	PropertyDataNumberList,
	PropertyDataDouble,
	PropertyDataExpression,
};

// Converts property keys between template key types.
const CHAR_TYPE* convertPropertyKeyToKeyType(const PropertyMapper* pMapper, ::PropertyType eKey);
::PropertyType convertPropertyKeyToKeyType(const PropertyMapper* pMapper, const CHAR_TYPE* pcsKey);
/********************************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
class HYPERCORE_API TResourceProperty
{
public:
	TResourceProperty();
	TResourceProperty(const TResourceProperty<KEY_TYPE, STORAGE_TYPE>& rOther);
	~TResourceProperty();

	inline static ::PropertyType getIdPropertyValue(::PropertyType dummyVar) { return PropertyId; }
	inline static const CHAR_TYPE* getIdPropertyValue(const CHAR_TYPE* dummyVar) { return STRING_ID_PROP; }

	inline static ::PropertyType getTagsPropertyValue(::PropertyType dummyVar) { return PropertyTags; }
	inline static const CHAR_TYPE* getTagsPropertyValue(const CHAR_TYPE* dummyVar) { return STRING_TAGS_PROP; }

	inline static ::PropertyType getDefOldIdPropertyValue(::PropertyType dummyVar) { return PropertyOldId; }
	inline static const CHAR_TYPE* getDefOldIdPropertyValue(const CHAR_TYPE* dummyVar) { return STRING_DEF_OLD_ID_PROP; }

	inline static bool isIdProperty(::PropertyType eType) { return eType == PropertyId; }
	inline static bool isIdProperty(const CHAR_TYPE* pcsPropName) { return pcsPropName && strcmp(pcsPropName, STRING_ID_PROP) == 0; }

	inline static bool isNullProperty(::PropertyType eType) { return eType == PropertyNull; }
	inline static bool isNullProperty(const CHAR_TYPE* pcsPropName) { return pcsPropName && strlen(pcsPropName) > 0 && (strcmp(pcsPropName, PROPERTY_NONE) == 0); }

	inline static const CHAR_TYPE* getNullProperty(const CHAR_TYPE* dummyVar) { return PROPERTY_NONE; }
	inline static ::PropertyType getNullProperty(::PropertyType eDummy) { return PropertyNull; }

	static const CHAR_TYPE* mapPropertyKeyToString(const PropertyMapper* pMapper, const CHAR_TYPE* pcsProp) { return pcsProp; }
	static const CHAR_TYPE* mapPropertyKeyToString(const PropertyMapper* pMapper, ::PropertyType eProp);

	static const CHAR_TYPE* mapStringToPropertyType(const PropertyMapper* pMapper, const STRING_TYPE& strProp, const CHAR_TYPE* dummyVar) { return strProp.c_str(); }
	static const CHAR_TYPE* mapStringToPropertyType(const PropertyMapper* pMapper, const CHAR_TYPE* pcsProp, const CHAR_TYPE* dummyVar) { return pcsProp; }
	static ::PropertyType mapStringToPropertyType(const PropertyMapper* pMapper, const STRING_TYPE& strProp, ::PropertyType eDummy);
	static ::PropertyType mapStringToPropertyType(const PropertyMapper* pMapper, const CHAR_TYPE* pcsProp, ::PropertyType eDummy);

	inline static bool arePropertiesEqual(::PropertyType eProp1, ::PropertyType eProp2) { return eProp1 == eProp2; }
	inline static bool arePropertiesEqual(const char *eProp1, const CHAR_TYPE* eProp2) { return IS_STRING_EQUAL(eProp1, eProp2); }

	void reset(bool bForce, PropertyDataType eNextType = PropertyDataNumber);

public:
	inline void setPropertyName(KEY_TYPE eType) { myPropName = eType; }
	void setNum(FLOAT_TYPE fVal);
	void setDouble(double dVal);
	void setBool(bool bValue);
	void setString(const CHAR_TYPE* str);
	void setStringOrExpression(const CHAR_TYPE* str);
	void setFloatArray(FLOAT_TYPE* fArray, int iW, int iH);
	void addEnumValue(const CHAR_TYPE* pcsValue);
	void addEnumValueAsLong(GTIME lValue);
	void setEnumValue(int iIndex, const CHAR_TYPE* pcsValue);
	void addNumericEnumValue(FLOAT_TYPE fValue);
	void setNumericEnumValue(int iIndex, FLOAT_TYPE fValue);
	void setFromString(const CHAR_TYPE* pcsValue);
	void setAsLong(GTIME lValue);
	void setAsColor(const SColor& scolValue, bool bAddAlpha = true);
	void setFromVector(const vector < FLOAT_TYPE >& vecNumbers);
	void setFromVector(const TStringVector& vecStrings);
	void setAsRect(const SRect2D& srRect);
	void setExpression(const CHAR_TYPE* pcsExpression);

	inline TResourceProperty& operator=(FLOAT_TYPE fValue) { this->setNum(fValue); return *this; }
	inline TResourceProperty& operator=(double fValue) { this->setDouble(fValue); return *this; }
	inline TResourceProperty& operator=(int iValue) { this->setNum(iValue); return *this; }
	inline TResourceProperty& operator=(const CHAR_TYPE* pcsValue) { this->setString(pcsValue); return *this; }
	inline TResourceProperty& operator=(bool bValue) { this->setBool(bValue); return *this; }
	inline TResourceProperty& operator=(const SColor& scolValue) { this->setAsColor(scolValue); return *this; }
	inline TResourceProperty& operator=(const SRect2D& srRect) { this->setAsRect(srRect); return *this; }

	int getNumEnumValues() const;
	const CHAR_TYPE* getEnumValue(int iIndex) const;
	int getNumNumericEnumValues() const;
	FLOAT_TYPE getNumericEnumValue(int iIndex) const;
	int findStringInEnumProp(const CHAR_TYPE* pcsString) const;
	int findNumInEnumProp(FLOAT_TYPE fNum) const;
	int countStringInEnumProp(const CHAR_TYPE* pcsString) const;

	inline KEY_TYPE getPropertyName() const { return myPropName; }
	inline FLOAT_TYPE getNum() const
	{
		if(myDataType == PropertyDataExpression)
			return myExpression->getResultAsNumber();

		_ASSERT(myDataType == PropertyDataNumber || myDataType == PropertyDataDouble);
		if(myDataType == PropertyDataNumber)
			return myNumValue;
		else
			return myDoubleValue;
	}
	inline double getDouble() const
	{
		if(myDataType == PropertyDataExpression)
			return myExpression->getResultAsDoubleNumber();

		_ASSERT(myDataType == PropertyDataNumber || myDataType == PropertyDataDouble);
		if(myDataType == PropertyDataNumber)
			return myNumValue;
		else
			return myDoubleValue;
	}
	inline bool getBool() const
	{
		if(myDataType == PropertyDataExpression)
			return myExpression->getResultAsBool();

		_ASSERT(myDataType == PropertyDataBool);
		return myBoolValue;
	}
	inline const CHAR_TYPE* getString() const
	{
		if(myDataType == PropertyDataExpression)
			return myExpression->getResultAsString();

		_ASSERT(myDataType == PropertyDataString);
		if(myStrValue)
			return myStrValue->c_str();
		else
			return "";
	}
	inline unsigned int getStringLength() const
	{
		if(myDataType == PropertyDataExpression)
			return strlen(myExpression->getResultAsString());

		_ASSERT(myDataType == PropertyDataString);
		if(myStrValue)
			return myStrValue->length();
		else
			return 0;
	}	
	inline const IExpression* getExpression() const
	{
		_ASSERT(myDataType == PropertyDataExpression);
		return myExpression;
	}
	inline IExpression* getExpression()
	{
		_ASSERT(myDataType == PropertyDataExpression);
		return myExpression;
	}

	FLOAT_TYPE* getFloatArray(int& iW, int &iH) const;

	bool areEqualByValue(const TResourceProperty& rOther) const;
	TResourceProperty& operator=(const TResourceProperty& rOther);

	void getAsString(STRING_TYPE& strOut, const CHAR_TYPE* pcsOptFloatConvFormat = NULL) const;
	FLOAT_TYPE getAsNumber() const;
	bool getAsColor(SColor& colOut) const;
	GTIME getAsLong() const;
	double getAsDouble() const;
	GTIME getEnumPropValueAsLong(int iIndex) const;
	void getAsStringVector(TStringVector& vecOut, const CHAR_TYPE* pcsOptFloatConvFormat = NULL) const;
	void getAsVector3(SVector3D& svOut) const;
	void getAsNumVector(vector < FLOAT_TYPE >& vecNumbers) const;
	void getAsRect(SRect2D& srOut) const;
	void removeEnumValue(int iIndex);

	inline operator FLOAT_TYPE() const { return this->getAsNumber(); }
	inline operator int() const { return (int)this->getAsNumber(); }
	inline operator bool() const { return this->getBool(); }
	inline operator double() const { return this->getAsDouble(); }
	inline operator const CHAR_TYPE*() const { return this->getString(); }
	inline operator SColor() const { SColor rCol; this->getAsColor(rCol); return rCol; }
	inline operator SRect2D() const { SRect2D rect; this->getAsRect(rect); return rect; }

	void saveToStringNative(STRING_TYPE& strOut, int iDbSaveLevel = 1) const;
	void saveToStringJSON(STRING_TYPE& strOut) const;
	inline PropertyDataType getPropertyDataType() const { return myDataType; }

	bool getIsNone() const;
	bool getIsLessThan(const TResourceProperty& rOther) const;
	bool getIsGreaterThan(const TResourceProperty& rOther) const;
	bool getIsEqual(const TResourceProperty& rOther) const;

	template < class MY_KEY_TYPE, class MY_STORAGE_TYPE, class OTHER_KEY_TYPE, class OTHER_STORAGE_TYPE >
	void copyValueFrom(const TResourceProperty<OTHER_KEY_TYPE, OTHER_STORAGE_TYPE> &rOther)
	{
		this->reset(true);
		myDataType = rOther.myDataType;

		// Look at the type
		if(rOther.myDataType == PropertyDataNumber)
		{
			myNumValue = rOther.myNumValue;
		}
		else if(rOther.myDataType == PropertyDataDouble)
		{
			myDoubleValue = rOther.myDoubleValue;
		}
		else if(rOther.myDataType ==  PropertyDataBool)
		{
			myBoolValue = rOther.myBoolValue;
		}
		else if(rOther.myDataType == PropertyDataString)
		{
			myStrValue = new string(*rOther.myStrValue);
		}
		else if(rOther.myDataType == PropertyDataFloatArray2D)
		{
			int iTempArrayW = rOther.myFloatArray[0];
			int iTempArrayH = rOther.myFloatArray[1];

			myFloatArray = new FLOAT_TYPE[iTempArrayH*iTempArrayW + EXTRA_ARRYA_FRONT_INFO];
			memcpy(myFloatArray, rOther.myFloatArray, sizeof(FLOAT_TYPE)*(iTempArrayH*iTempArrayW + EXTRA_ARRYA_FRONT_INFO));
		}	
		else if(rOther.myDataType == PropertyDataStringList)
		{
			myStringList = new TStringVector;
			*myStringList = *rOther.myStringList;

		}
		else if(rOther.myDataType == PropertyDataNumberList)
		{
			myNumberList = new TFloatVector;
			*myNumberList = *rOther.myNumberList;	
		}
		else if(rOther.myDataType == PropertyDataExpression)
		{
			myExpression = rOther.myExpression->cloneSelf();
		}
		ELSE_ASSERT;
	}
    
    // Stupid GCC wants this.
    template <class MY_KEY_TYPE, class MY_STORAGE_TYPE > friend class TResourceProperty;
 
private:

	union
	{
		FLOAT_TYPE myNumValue;
		double myDoubleValue;
		bool myBoolValue;
		FLOAT_TYPE *myFloatArray;
		string *myStrValue;
		TStringVector *myStringList;
		TFloatVector *myNumberList;		
		IExpression* myExpression;
	};

	STORAGE_TYPE myPropName;
	PropertyDataType myDataType;
};
/*****************************************************************************/
typedef TResourceProperty< PropertyType, PropertyType > ResourceProperty;
typedef TResourceProperty< const CHAR_TYPE*, RESOURCEITEM_STRING_TYPE > StringResourceProperty;
