#pragma once

/********************************************************************************************/
struct SPropertyDef
{
	SPropertyDef() { }
	SPropertyDef(const CHAR_TYPE* pcsName, PropertyType eProp)
	{
		myName = pcsName;
		myType = eProp;
	}

	STRING_TYPE myName;
	PropertyType myType;
};
/********************************************************************************************/
typedef map < STRING_TYPE, SPropertyDef > TStringPropDefMap;
typedef vector < PropertyType > TPropTypeVector;
/********************************************************************************************/
class PropertyMapper
{
public:
	HYPERCORE_API static PropertyMapper* getInstance();
    HYPERCORE_API ~PropertyMapper();

    HYPERCORE_API PropertyType mapProperty(const STRING_TYPE& strProperty) const;
	HYPERCORE_API PropertyType mapProperty(const CHAR_TYPE* pcsProp) const;

	HYPERCORE_API bool isStringPropertyEqual(const STRING_TYPE& strValue, PropertyType ePropType) const;
	HYPERCORE_API const CHAR_TYPE* getPropertyString(PropertyType ePropType) const;
	inline const CHAR_TYPE* getPropertyString(const CHAR_TYPE* pcsPropName) const { return pcsPropName; }

	HYPERCORE_API void addProperty(PropertyType ePropType, const CHAR_TYPE* pcsStringValue);

private:
    PropertyMapper();

private:

    TStringPropDefMap myMap;
	// For reverse lookup - may be in different order than the g_ list.
	vector < SPropertyDef > myOrderedListUncompressed;

	static PropertyMapper* theInstance;
};
/********************************************************************************************/
