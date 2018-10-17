#pragma once
/*

The idea here is that we define a few core properties with specific values (that start
at a good offset after all possible custom props, hopefully). The rest of the properties
are per-project, and their values are assigned using line numbers. with the DECLARE_PROPERTY
macro.

This way, they can be used as types using an integer lookup and comparison, but at the same
time, the user can add their own, globally visible properties using nothing but a property
name.

*/
/********************************************************************************************/
#define STRING_ID_PROP						"id"
#define STRING_DEF_OLD_ID_PROP				"oldId"
#define STRING_TAGS_PROP					"tags"
/********************************************************************************************/
#define CORE_PROPERTIES_START_VALUE			3072
#define PropertyNull						(CORE_PROPERTIES_START_VALUE + 1024)
/********************************************************************************************/
struct SPropAdder
{
	HYPERCORE_API SPropAdder(const CHAR_TYPE* pcsToken, int iValue);
};

#define DECLARE_PROPERTY(propertyName) \
	const int propertyName = (__LINE__); \
	static const HyperCore::SPropAdder propertyName##_adder = HyperCore::SPropAdder(#propertyName, (__LINE__));

#define DECLARE_PROPERTY_EXPLICIT(propertyName, iValue) \
	const int propertyName = (iValue); \
	static const HyperCore::SPropAdder propertyName##_adder = HyperCore::SPropAdder(#propertyName, (iValue));
/********************************************************************************************/
DECLARE_PROPERTY_EXPLICIT(PropertyId, CORE_PROPERTIES_START_VALUE + 0);
DECLARE_PROPERTY_EXPLICIT(PropertyTags, CORE_PROPERTIES_START_VALUE + 1);
DECLARE_PROPERTY_EXPLICIT(PropertyOldId, CORE_PROPERTIES_START_VALUE + 2);
DECLARE_PROPERTY_EXPLICIT(PropertyUnknown, CORE_PROPERTIES_START_VALUE + 3);
/********************************************************************************************/
