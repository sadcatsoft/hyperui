#include "stdafx.h"

namespace HyperCore
{
PropertyMapper* PropertyMapper::theInstance = NULL;
/********************************************************************************************/
PropertyMapper::PropertyMapper()
{
	myOrderedListUncompressed.resize(CORE_PROPERTIES_START_VALUE*2);
}
/*****************************************************************************/
PropertyMapper* PropertyMapper::getInstance()
{
	if(!theInstance)
		theInstance = new PropertyMapper;
	return theInstance;
}
/********************************************************************************************/
PropertyMapper::~PropertyMapper()
{

}
/********************************************************************************************/
PropertyType PropertyMapper::mapProperty(const CHAR_TYPE* pcsProp) const
{
	TStringPropDefMap::const_iterator mi = myMap.find(pcsProp);
	if(mi == myMap.end())
	{
		// All properties must be added to the g_pAllProperties table first.
		gLog("ERROR: Property not found: %s. Please ensure all properties are declared using the DECLARE_PROPERTY() macro.\n", pcsProp);
		_ASSERT(0);
		return PropertyUnknown;
	}
	else
	{
		return mi->second.myType;
	}
}
/********************************************************************************************/
PropertyType PropertyMapper::mapProperty(const STRING_TYPE& strProperty) const
{
    TStringPropDefMap::const_iterator mi = myMap.find(strProperty);
    if(mi == myMap.end())
    {
		// All properties must be added to the g_pAllProperties table first.
		_ASSERT(0);
		return PropertyUnknown;
    }
    else
    {
		return mi->second.myType;
    }
}
/********************************************************************************************/
bool PropertyMapper::isStringPropertyEqual(const STRING_TYPE& strValue, PropertyType ePropType) const
{
	return (strValue == getPropertyString(ePropType));
}
/********************************************************************************************/
const CHAR_TYPE* PropertyMapper::getPropertyString(PropertyType ePropType) const
{
	_ASSERT(ePropType < myOrderedListUncompressed.size());
	_ASSERT(myOrderedListUncompressed[ePropType].myName.length() > 0);
	return myOrderedListUncompressed[ePropType].myName.c_str();
}
/*****************************************************************************/
void PropertyMapper::addProperty(PropertyType ePropType, const CHAR_TYPE* pcsStringValue)
{
	SPropertyDef rNewDef(pcsStringValue, ePropType);

	// Make sure it starts with a lower-case letter
	CHAR_TYPE* pcsPtr = const_cast<CHAR_TYPE*>(rNewDef.myName.c_str());
	pcsPtr[0] = TOLOWER(pcsPtr[0]);
	myMap[pcsPtr] = rNewDef;

	//gLog("Adding property: %s with value %d\n", rNewDef.myName.c_str(), (int)ePropType);

	if(myOrderedListUncompressed.size() <= (int)ePropType)
		myOrderedListUncompressed.resize((int)ePropType*2);

	// If this fails, we're overwriting something somehow.
	// What probably happened is that we've added a new property to the enum type,
	// but haven't added one to the array of actual text-prop definitions above.
	// Thus, the property list is longer and oversteps the bounds of the array.
	//_ASSERT(myOrderedListUncompressed[(int)ePropType].myName.length() == 0);
	_ASSERT(myOrderedListUncompressed[(int)ePropType].myName.length() == 0 || myOrderedListUncompressed[(int)ePropType].myName == pcsPtr);

	// Make sure we have something meaningful to write.
	_ASSERT(STRLEN(pcsStringValue) > 0);

	myOrderedListUncompressed[(int)ePropType] = rNewDef;
}
/********************************************************************************************/
SPropAdder::SPropAdder(const CHAR_TYPE* pcsToken, int iValue)
{
	static const int iSkipFirstChars = STRLEN(STR_LIT("Property"));
	PropertyMapper::getInstance()->addProperty((PropertyType)iValue, pcsToken + iSkipFirstChars);
}
/********************************************************************************************/
};