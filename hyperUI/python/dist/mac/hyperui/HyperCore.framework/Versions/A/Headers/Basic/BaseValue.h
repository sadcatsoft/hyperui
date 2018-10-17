#pragma once

// Base class used to pass various values around
/*****************************************************************************/
template < class TYPE >
class TBaseValue : public IBaseObject
{
public:
	TBaseValue() { myValue = (TYPE)0; }
	TBaseValue(TYPE iValue) { myValue = iValue; }
	virtual ~TBaseValue() { }

	inline void setValue(TYPE iValue) { myValue = iValue; }
	inline TYPE getValue() { return myValue; }
	inline const TYPE getValue() const { return myValue; }

private:
	TYPE myValue;
};
/*****************************************************************************/
typedef TBaseValue  < int > BaseInt;
typedef TBaseValue  < const char*  > BaseCharPtr;
/*****************************************************************************/
