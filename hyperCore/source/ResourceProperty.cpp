#include "stdafx.h"

namespace HyperCore
{
template < class KEY_TYPE, class STORAGE_TYPE >
const CHAR_TYPE* TResourceProperty<KEY_TYPE, STORAGE_TYPE>::mapPropertyKeyToString(const PropertyMapper* pMapper, PropertyType eProp) { return pMapper->getPropertyString(eProp); }

template < class KEY_TYPE, class STORAGE_TYPE >
PropertyType TResourceProperty<KEY_TYPE, STORAGE_TYPE>::mapStringToPropertyType(const PropertyMapper* pMapper, const STRING_TYPE& strProp, PropertyType eDummy) { return pMapper->mapProperty(strProp); }

template < class KEY_TYPE, class STORAGE_TYPE >
::PropertyType TResourceProperty<KEY_TYPE, STORAGE_TYPE>::mapStringToPropertyType(const PropertyMapper* pMapper, const CHAR_TYPE* pcsProp, ::PropertyType eDummy) { return pMapper->mapProperty(pcsProp); }

const CHAR_TYPE* convertPropertyKeyToKeyType(const PropertyMapper* pMapper, ::PropertyType eKey) { return pMapper->getPropertyString(eKey); }
::PropertyType convertPropertyKeyToKeyType(const PropertyMapper* pMapper, const CHAR_TYPE* pcsKey) { return pMapper->mapProperty(pcsKey); }

/*****************************************************************************/
// ResourceProperty
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
TResourceProperty<KEY_TYPE, STORAGE_TYPE>::TResourceProperty()
{ 
	myStringList = NULL;
	myNumberList = NULL;
	
	myFloatArray = NULL;

	myPropName = getNullProperty((KEY_TYPE)0);
	myDataType = PropertyDataNumber;

	// This may be required since a number may not be the largest variable on 
	// 64-bit systems, and so some higher bits of the pointer may remain set
	// to something, failing our if tests and pointing to invalid memory.
	myNumValue = 0;
	myStrValue = NULL;

	// Save some time calling this.
	//reset(true);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
TResourceProperty<KEY_TYPE, STORAGE_TYPE>::TResourceProperty(const TResourceProperty<KEY_TYPE, STORAGE_TYPE>& rOther) 
{
	myStrValue = NULL;
	myStringList = NULL;
	myNumberList = NULL;
	
	myFloatArray = NULL;
	myDataType = PropertyDataNumber;

	*this = rOther;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
TResourceProperty<KEY_TYPE, STORAGE_TYPE>::~TResourceProperty()
{
	// We don't need to do anything if we're a number of a boolean.
	if(myDataType != PropertyDataNumber && myDataType != PropertyDataBool)
		reset(true);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::reset(bool bForce, PropertyDataType eNextType)
{
	// DO NOT RESET the prop name here!
	if(eNextType != myDataType || bForce)
	{
		if(myDataType == PropertyDataNumber
			|| myDataType == PropertyDataDouble
			|| myDataType == PropertyDataBool)
		{
			// Nothing
		}
		else if(myDataType == PropertyDataString)
		{
			if(myStrValue)
				delete myStrValue;	
		}
		else if(myDataType == PropertyDataFloatArray2D)
		{
			if(myFloatArray)
				delete[] myFloatArray;
		}	
		else if(myDataType == PropertyDataStringList)
		{
			if(myStringList)
				delete myStringList;	
		}
		else if(myDataType == PropertyDataNumberList)
		{
			if(myNumberList)
				delete myNumberList;	
		}
		else if(myDataType == PropertyDataExpression)
		{
			if(myExpression)
				delete myExpression;
		}
		ELSE_ASSERT;
	}

	if(myDataType == PropertyDataNumber || myDataType == PropertyDataDouble ||
		myDataType == PropertyDataBool || eNextType != myDataType || bForce) 
	{
		//myNumValue = 0;
		myDoubleValue = 0;
		// This may be required since a number may not be the largest variable on 
		// 64-bit systems, and so some higher bits of the pointer may remain set
		// to something, failing our if tests and pointing to invalid memory.
		myStrValue = 0;
	}
	
	myDataType = PropertyDataNumber;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
TResourceProperty<KEY_TYPE, STORAGE_TYPE>& TResourceProperty<KEY_TYPE, STORAGE_TYPE>::operator=(const TResourceProperty<KEY_TYPE, STORAGE_TYPE>& rOther)
{
	myPropName = rOther.myPropName;
	this->copyValueFrom<KEY_TYPE, STORAGE_TYPE, KEY_TYPE, STORAGE_TYPE>(rOther);	
	return *this;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::setNum(FLOAT_TYPE fVal)
{
	reset(false, PropertyDataNumber);

	myNumValue = fVal;
	myDataType = PropertyDataNumber;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::setDouble(double dVal)
{
	reset(false, PropertyDataDouble);

	myDoubleValue = dVal;
	myDataType = PropertyDataDouble;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
FLOAT_TYPE* TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getFloatArray(int& iW, int &iH) const
{
	_ASSERT(myDataType == PropertyDataFloatArray2D);
	iW = myFloatArray[0];
	iH = myFloatArray[1];
	return (myFloatArray + EXTRA_ARRYA_FRONT_INFO);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::setFloatArray(FLOAT_TYPE* fArray, int iW, int iH)
{
	// NOTE: Resetting will also reset the property name, and we don't want
	// that. So save it and the set it again.
	KEY_TYPE eOldProp = myPropName;
	reset(true);
	myPropName = eOldProp;

	if(iW <= 0 || iH <= 0)
		return;

	myFloatArray = new FLOAT_TYPE[iW*iH + EXTRA_ARRYA_FRONT_INFO];
	memcpy(myFloatArray + EXTRA_ARRYA_FRONT_INFO, fArray, sizeof(FLOAT_TYPE)*(iW*iH));

	myFloatArray[0] = iW;
	myFloatArray[1] = iH;

	_ASSERT(iW >= 0 && iW <= 65535);
	_ASSERT(iH >= 0 && iH <= 65535);
	
	myDataType = PropertyDataFloatArray2D;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::setExpression(const CHAR_TYPE* pcsExpression)
{
	reset(true, PropertyDataExpression);

	_ASSERT(myExpression == NULL);
	_ASSERT(pcsExpression);
	myExpression = IExpression::createExpressionFrom(pcsExpression);
	_ASSERT(myExpression != NULL);
	if(myExpression)
		myDataType = PropertyDataExpression;
	else
		reset(true);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::setBool(bool bValue)
{
	reset(false, PropertyDataBool);

	myBoolValue = bValue;
	myDataType = PropertyDataBool;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::setString(const CHAR_TYPE* str)
{
	reset(false, PropertyDataString);

	if(!myStrValue)
		myStrValue = new string;
	*myStrValue = str;
	myDataType = PropertyDataString;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::setStringOrExpression(const CHAR_TYPE* str)
{
	if(IS_VALID_STRING_AND_NOT_NONE(ExpressionTypeManager::getInstance()->getExpressionType(str)))
		setExpression(str);
	else
		setString(str);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::setEnumValue(int iIndex, const CHAR_TYPE* pcsValue)
{
	_ASSERT(myStringList);
	_ASSERT(iIndex >= 0 && iIndex <= (int)myStringList->size());
	(*myStringList)[iIndex] = pcsValue;	
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::setNumericEnumValue(int iIndex, FLOAT_TYPE fValue)
{
	_ASSERT(myNumberList);
	_ASSERT(iIndex >= 0 && iIndex <= (int)myNumberList->size());
	(*myNumberList)[iIndex] = fValue;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::setAsLong(GTIME lValue)
{
	CHAR_TYPE pcsBuff[128];
	sprintf(pcsBuff, "%c%ld", SAVE_LONG_AS_STRING_SYMBOL, lValue);
	this->setString(pcsBuff);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::setFromVector(const vector < FLOAT_TYPE >& vecNumbers)
{
	reset(false, PropertyDataNumberList);
	myDataType = PropertyDataNumberList;
	if(!myNumberList)
		myNumberList = new TFloatVector;
	*myNumberList = vecNumbers;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::setAsRect(const SRect2D& srRect)
{
	reset(false, PropertyDataNumberList);
	myDataType = PropertyDataNumberList;
	if(!myNumberList)
		myNumberList = new TFloatVector;
	myNumberList->resize(4);
	(*myNumberList)[0] = srRect.x;
	(*myNumberList)[1] = srRect.y;
	(*myNumberList)[2] = srRect.w;
	(*myNumberList)[3] = srRect.h;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::setFromVector(const TStringVector& vecStrings)
{
	reset(false, PropertyDataStringList);
	myDataType = PropertyDataStringList;
	if(!myStringList)
		myStringList = new TStringVector;
	*myStringList = vecStrings;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::setAsColor(const SColor& scolValue, bool bAddAlpha)
{
	reset(true);

	this->addNumericEnumValue(scolValue.r);
	this->addNumericEnumValue(scolValue.g);
	this->addNumericEnumValue(scolValue.b);
	if(bAddAlpha)
		this->addNumericEnumValue(scolValue.alpha);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::setFromString(const CHAR_TYPE* pcsValue)
{
	// Automatically determine prop type
	if(!pcsValue)
		return;

	this->reset(false);

	if(StringUtils::isANumberIgnoreSpaces(pcsValue, true, false))
	{
		// This is a number
		setNum(atof(pcsValue));
		return;
	}

	// Otherwise, see if it's a boolean string.
	string strTemp(pcsValue);
	std::transform(strTemp.begin(), strTemp.end(), strTemp.begin(), ::tolower);
	if(strTemp == "true")
	{
		setBool(true);
		return;
	}
	else if(strTemp == "false")
	{
		setBool(false);
		return;
	}

	// See if this is a list
	if(strTemp.find('{') != string::npos && strTemp.find('}') != string::npos)
	{
		// This is either list of numbers or strings.
		strTemp = pcsValue;
		TStringVector vecTokens;
		bool bIsNumberList = true;
		TokenizeUtils::tokenizeString(strTemp, "\t=, ", vecTokens);
		int iCurr, iNum = vecTokens.size();
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
			if(vecTokens[iCurr] == "{" || vecTokens[iCurr] == "}")
				continue;

			if(!StringUtils::isANumber(vecTokens[iCurr], true))
			{
				bIsNumberList = false;
				break;
			}
		}

		if(bIsNumberList)
		{
			for(iCurr = 0; iCurr < iNum; iCurr++)
			{
				if(vecTokens[iCurr] == "{" || vecTokens[iCurr] == "}")
					continue;
				this->addNumericEnumValue(atof(vecTokens[iCurr].c_str()));
			}
		}
		else
		{
			for(iCurr = 0; iCurr < iNum; iCurr++)
			{
				if(vecTokens[iCurr] == "{" || vecTokens[iCurr] == "}")
					continue;
				this->addEnumValue(vecTokens[iCurr].c_str());
			}
		}
		return;
	}

	// This must be a string otherwise
	this->setString(pcsValue);

}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::addNumericEnumValue(FLOAT_TYPE fValue)
{
	this->reset(false, PropertyDataNumberList);

	myDataType = PropertyDataNumberList;
	if(!myNumberList)
		myNumberList = new TFloatVector;
	myNumberList->push_back(fValue);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::addEnumValue(const CHAR_TYPE* pcsValue)
{
	this->reset(false, PropertyDataStringList);

	myDataType = PropertyDataStringList;
	if(!myStringList)
		myStringList = new TStringVector;
	myStringList->EMPLACE_BACK(pcsValue);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::addEnumValueAsLong(GTIME lValue)
{
	this->reset(false, PropertyDataStringList);

	myDataType = PropertyDataStringList;
	if(!myStringList)
		myStringList = new TStringVector;

	CHAR_TYPE pcsBuff[128];
	sprintf(pcsBuff, "%c%ld", SAVE_LONG_AS_STRING_SYMBOL, lValue);
	myStringList->EMPLACE_BACK(pcsBuff);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
FLOAT_TYPE TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getAsNumber() const
{
	switch(myDataType)
	{
		case PropertyDataString:
		{
			const CHAR_TYPE* pcsStart = myStrValue->c_str();
			while(pcsStart[0] != 0 && !isdigit(pcsStart[0]) && pcsStart[0] != '-')
				pcsStart++;
			if(pcsStart)
				return atof(pcsStart);
			else
				return 0;
		}
		break;
		case PropertyDataBool: 
		{
			if(myBoolValue)
				return 1;
			else
				return 0;
		}
		break;
		case PropertyDataExpression:
		{
			const CHAR_TYPE* pcsRes = NULL;
			if(myExpression)
				pcsRes = myExpression->getResultAsString();
			ELSE_ASSERT;

			if(pcsRes)
				return atof(pcsRes);
			else
				return 0.0;
		}
		break;
		case PropertyDataNumber: return myNumValue;
		case PropertyDataDouble: return myDoubleValue;
		// Don't know how to convert the rest.
		default:_ASSERT(0);return 0;
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
GTIME TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getEnumPropValueAsLong(int iIndex) const
{
	if(!myStringList)
		return 0;

	_ASSERT(myDataType == PropertyDataStringList);
	const CHAR_TYPE* pcsStr = (*myStringList)[iIndex].c_str();
	_ASSERT(pcsStr[0] == SAVE_LONG_AS_STRING_SYMBOL);
	// The first letter is a special symbol, skip it
	return atol(pcsStr + 1);

}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
GTIME TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getAsLong() const
{
	if(myDataType == PropertyDataExpression)
		return myExpression->getResultAsNumber();

	_ASSERT(myDataType ==  PropertyDataString);
	if(myDataType !=  PropertyDataString)
		return 0;

	const CHAR_TYPE* pcsStr = myStrValue->c_str();
	_ASSERT(pcsStr[0] == SAVE_LONG_AS_STRING_SYMBOL);
	// The first letter is a special symbol, skip it
	return atol(pcsStr + 1);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
double TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getAsDouble() const
{
	if(myDataType == PropertyDataExpression)
		return myExpression->getResultAsDoubleNumber();

	if(myDataType == PropertyDataDouble)
		return myDoubleValue;

	_ASSERT(myDataType == PropertyDataString);
	if(myDataType !=  PropertyDataString)
		return 0;

	const CHAR_TYPE* pcsStr = myStrValue->c_str();
	_ASSERT(pcsStr[0] == SAVE_DOUBLE_AS_STRING_SYMBOL);
	// The first letter is a special symbol, skip it
	return atof(pcsStr + 1);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
bool TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getAsColor(SColor& colOut) const
{
	bool bResult = false;
	switch(myDataType)
	{
		case PropertyDataNumber: colOut.set(myNumValue, myNumValue, myNumValue, 1);bResult = true;break;
		case PropertyDataDouble: colOut.set(myDoubleValue, myDoubleValue, myDoubleValue, 1);break;
		case PropertyDataNumberList: 
		{
			_ASSERT(myNumberList);
			_ASSERT(myNumberList->size() == 3 || myNumberList->size() == 4);
			if(myNumberList->size() == 3)
				colOut.set((*myNumberList)[0], (*myNumberList)[1], (*myNumberList)[2], 1);
			else
				colOut.set((*myNumberList)[0], (*myNumberList)[1], (*myNumberList)[2], (*myNumberList)[3]);
			bResult = true;
		}
		break;
		case PropertyDataString:  
		{
			bResult = colOut.fromHex(myStrValue->c_str() + 1);
			if(!bResult)
				bResult = colOut.fromFloatString(*myStrValue);
		}
		break;
			
		default:_ASSERT(0);colOut.set(0,0,0,1);break;
	}

	// See if we're in a 255 format and convert to 1.0-based range:
	// Note that alpha is always assumed to be in a 1.0 range.
	if(colOut.r > 1.0 || colOut.g > 1.0 || colOut.b > 1.0)
	{
		colOut.r /= 255.0;
		colOut.g /= 255.0;
		colOut.b /= 255.0;
	}

	return bResult;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::removeEnumValue(int iIndex)
{
	_ASSERT(myDataType == PropertyDataStringList || 
		myDataType == PropertyDataNumberList);

	if(myDataType == PropertyDataStringList)
	{
		myStringList->erase(myStringList->begin() + iIndex);
	}
	else if(myDataType == PropertyDataNumberList)
	{
		myNumberList->erase(myNumberList->begin() + iIndex);
	}
	ELSE_ASSERT;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getAsNumVector(vector < FLOAT_TYPE >& vecNumbers) const
{
	// The latter is in case we have an empty list
	_ASSERT(myDataType == PropertyDataNumberList || (myDataType == PropertyDataNumber && myNumValue == 0) );

	if(!myNumberList || myDataType != PropertyDataNumberList)
	{
		vecNumbers.clear();
		return;
	}

	vecNumbers = *myNumberList;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getAsRect(SRect2D& srOut) const
{
	_ASSERT(myDataType == PropertyDataNumberList || (myDataType == PropertyDataNumber && myNumValue == 0) );

	if(!myNumberList || myDataType != PropertyDataNumberList || myNumberList->size() < 4)
	{
		srOut.reset();
		return;
	}

	srOut.x = (*myNumberList)[0];
	srOut.y = (*myNumberList)[1];
	srOut.w = (*myNumberList)[2];
	srOut.h = (*myNumberList)[3];
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getAsVector3(SVector3D& svOut) const
{
	_ASSERT(myDataType == PropertyDataNumberList);
	_ASSERT(myNumberList);

	if(!myNumberList)
		return;

	_ASSERT(myNumberList->size() == 3);
	if(myNumberList->size() < 3)
		return;

	svOut.set((*myNumberList)[0], (*myNumberList)[1], (*myNumberList)[2]);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getAsStringVector(TStringVector& vecOut, const CHAR_TYPE* pcsOptFloatConvFormat) const
{
	if(myDataType != PropertyDataStringList && myDataType != PropertyDataNumberList)
	{
		string strTemp;
		this->getAsString(strTemp, pcsOptFloatConvFormat);
		vecOut.push_back(strTemp);
	}
	else if(myDataType == PropertyDataStringList)
	{
		if(myStringList)
			vecOut = *myStringList;
	}
	else if(myDataType == PropertyDataNumberList)
	{
		CHAR_TYPE pcsBuff[64];
		if(myNumberList)
		{
			FLOAT_TYPE fNum;
			int iNum;
			int curr, num = myNumberList->size();
			for(curr = 0; curr < num; curr++)
			{
				fNum = (*myNumberList)[curr];
				if(  fabs((FLOAT_TYPE)((int)fNum) - fNum) <= FLOAT_EPSILON)
				{
					iNum = (int)fNum;
					sprintf(pcsBuff, "%d", iNum);
				}
				else
					sprintf(pcsBuff, "%f", fNum);
				vecOut.EMPLACE_BACK(pcsBuff);
			}
		}
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getAsString(STRING_TYPE& strOut, const CHAR_TYPE* pcsOptFloatConvFormat) const
{
	switch(myDataType)
	{
		case PropertyDataString:
		{
			// CAREFUL! The retarded GCC silently uses ref counting between strings, so assigning one string
			// to another will result in them sharing the same pointer, even though the docs explicitly say
			// it must be a copy! Bastards. I hate them all.
			_ASSERT(strOut.length() == 0 || strOut.c_str() != myStrValue->c_str());          
			strOut = myStrValue->c_str();
			_ASSERT(strOut.length() == 0 || strOut.c_str() != myStrValue->c_str());
		}
		break;
		case PropertyDataBool: 
		{
			if(myBoolValue)
				strOut = TRUE_VALUE;
			else
				strOut = FALSE_VALUE;
		}
		break;
		case PropertyDataExpression:
		{
			const CHAR_TYPE* pcsRes = NULL;
			if(myExpression)
				pcsRes = myExpression->getResultAsString();
			ELSE_ASSERT;

			if(pcsRes)
				strOut = pcsRes;
			else
				strOut = "";
		}
		break;
		case PropertyDataNumber: 
		{
			CHAR_TYPE pcsBuff[128];
			if( ((FLOAT_TYPE)((int)myNumValue)) == myNumValue)
			{
				int iTemp = myNumValue;
				sprintf(pcsBuff, "%d", iTemp);
			}
			else
			{
				if(pcsOptFloatConvFormat)
				{
					if(strstr(pcsOptFloatConvFormat, "d"))
					{
						// We need to cast to int to properly display it
						int iVal = myNumValue;
						sprintf(pcsBuff, pcsOptFloatConvFormat, iVal);
					}
					else
						sprintf(pcsBuff, pcsOptFloatConvFormat, myNumValue);
				}
				else
					sprintf(pcsBuff, "%f", myNumValue);

			}
			strOut = pcsBuff;			
		}
		break;
		case PropertyDataDouble: 
		{
			CHAR_TYPE pcsBuff[128];
			if( ((double)((int)myDoubleValue)) == myDoubleValue)
			{
				int iTemp = myDoubleValue;
				SPRINTF(pcsBuff, 128, STR_LIT("%d"), iTemp);
			}
			else
			{
				if(pcsOptFloatConvFormat)
					SPRINTF(pcsBuff, 128, pcsOptFloatConvFormat, myDoubleValue);
				else
					SPRINTF(pcsBuff, 128, STR_LIT("%.17g"), myDoubleValue);
			}
			strOut = pcsBuff;			
		}
		break;
		case PropertyDataStringList:
		{
			int iCurr, iNum = myStringList->size();
			strOut = "{ ";
			for(iCurr = 0; iCurr < iNum; iCurr++)
			{
				strOut += (*myStringList)[iCurr];
				if(iCurr + 1 < iNum)
					strOut += ", ";

			}
			strOut += " }";
		}
		break;
		case PropertyDataNumberList:
		{

			CHAR_TYPE pcsBuff[128];
			FLOAT_TYPE fValue;
			int iCurr, iNum = myNumberList->size();
			strOut = "{ ";
			for(iCurr = 0; iCurr < iNum; iCurr++)
			{
				fValue = (*myNumberList)[iCurr];
				if( ((FLOAT_TYPE)((int)fValue)) == fValue)
				{
					int iTemp = fValue;
					sprintf(pcsBuff, "%d", iTemp);
				}
				else
				{
					if(pcsOptFloatConvFormat)
						sprintf(pcsBuff, pcsOptFloatConvFormat, fValue);
					else
						sprintf(pcsBuff, "%f", fValue);

				}

				strOut += pcsBuff;
				if(iCurr + 1 < iNum)
					strOut += ", ";

			}
			strOut += " }";
		}
		break;
		// Don't know how to convert the rest.
		default:_ASSERT(0);strOut = ""; break;
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
int TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getNumEnumValues() const
{
	if(myDataType == PropertyDataStringList)
		return myStringList->size();
	else
		return 0;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
const CHAR_TYPE* TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getEnumValue(int iIndex) const
{
	if(!myStringList)
		return "";

	_ASSERT(myDataType == PropertyDataStringList);
	if(myDataType != PropertyDataStringList)
		return "";

	if(iIndex < 0 || iIndex >= myStringList->size())
		return "";
	else
		return (*myStringList)[iIndex].c_str();
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
int TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getNumNumericEnumValues() const
{
	if(myDataType == PropertyDataNumberList)
		return myNumberList->size();
	else
		return 0;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
int TResourceProperty<KEY_TYPE, STORAGE_TYPE>::countStringInEnumProp(const CHAR_TYPE* pcsString) const
{
	_ASSERT(myDataType == PropertyDataStringList);
	_ASSERT(myStringList);

	if(!myStringList)
		return 0;

	// Look for the right string.
	int iCount = 0;
	int iCurr, iNum = myStringList->size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if((*myStringList)[iCurr] == pcsString)
			iCount++;
	}

	return iCount;

}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
int TResourceProperty<KEY_TYPE, STORAGE_TYPE>::findNumInEnumProp(FLOAT_TYPE fNum) const
{
	if(!myNumberList)
		return -1;

	// Look for the right string.
	int iCurr, iNum = myNumberList->size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(fabs( (*myNumberList)[iCurr] - fNum) <= FLOAT_EPSILON)
			return iCurr;
	}

	return -1;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
int TResourceProperty<KEY_TYPE, STORAGE_TYPE>::findStringInEnumProp(const CHAR_TYPE* pcsString) const
{
	if(myDataType != PropertyDataStringList)
		return -1;

	if(!myStringList)
		return -1;
	
	// Look for the right string.
	int iCurr, iNum = myStringList->size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if((*myStringList)[iCurr] == pcsString)
			return iCurr;
	}

	return -1;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
FLOAT_TYPE TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getNumericEnumValue(int iIndex) const
{
	if(!myNumberList)
		return 0;
	
	_ASSERT(myDataType == PropertyDataNumberList);
	if(iIndex < 0 || iIndex >= myNumberList->size())
		return 0;
	else
		return (*myNumberList)[iIndex];
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::saveToStringNative(STRING_TYPE& strOut, int iDbSaveLevel) const
{
	CHAR_TYPE pcsBuff[256];
#ifdef _DEBUG
	int iLevel;
	for(iLevel = 0; iLevel < iDbSaveLevel; iLevel++)
		strOut += "\t";
#endif

#ifdef _DEBUG
	if( (myDataType == PropertyDataNumber && myNumValue == 45.4)
		|| (myDataType == PropertyDataString && (*myStrValue) == "45.4") )
	{
		int bp = 0;
	}
#endif

	_ASSERT(!isNullProperty(myPropName));
	string strGenericCachedString;
	strGenericCachedString = TResourceProperty<KEY_TYPE, STORAGE_TYPE>::mapPropertyKeyToString(PropertyMapper::getInstance(), myPropName);

	if(myDataType == PropertyDataNumber)
	{
		// See if we're actually a floating point value, or an integer
		strOut += strGenericCachedString + "=";
		if( fabs( (FLOAT_TYPE)((int)myNumValue) - myNumValue) < FLOAT_EPSILON )
		{
			int iTemp = myNumValue;
			sprintf(pcsBuff, "%d\n", iTemp);
		}
		else
			sprintf(pcsBuff, "%f\n", myNumValue);
		strOut += pcsBuff;
	}
	else if(myDataType == PropertyDataDouble)
	{
		// See if we're actually a floating point value, or an integer
		strOut += strGenericCachedString + STR_LIT("=");
		if( fabs( (double)((int)myDoubleValue) - myDoubleValue) < DOUBLE_EPSILON )
		{
			int iTemp = myDoubleValue;
			SPRINTF(pcsBuff, 256, STR_LIT("%d\n"), iTemp);
		}
		else
			SPRINTF(pcsBuff, 256, STR_LIT("%.17g\n"), myDoubleValue);
		strOut += pcsBuff;
	}
	else if(myDataType == PropertyDataString)
	{
		bool bWriteOutQuoted = false;

		// We also need to write it out quoted if it looks like
		// a number:
		if(StringUtils::isANumberIgnoreSpaces(myStrValue->c_str(), true, false) || myStrValue->length() == 0 || StringUtils::isAnUPNumber(myStrValue->c_str()))
			bWriteOutQuoted = true;

		// See if there are any newlines in the string
		if(!bWriteOutQuoted && myStrValue->find("\n") != string::npos)
			bWriteOutQuoted = true;

		strOut += strGenericCachedString + "=";
		if(bWriteOutQuoted)
			strOut += MUTLINE_STRING_BEGIN_MARKER;

		if(myStrValue->length() > 0)
			strOut += *myStrValue;

		if(bWriteOutQuoted)
			strOut += MUTLINE_STRING_END_MARKER;

		strOut += "\n";
	}
	else if(myDataType == PropertyDataBool)
	{
		strOut += strGenericCachedString + "=";
		if(myBoolValue)
			strOut += "True\n";
		else
			strOut += "False\n";
	}
	else if(myDataType == PropertyDataExpression)
	{
		strOut += strGenericCachedString + "=";
		if(myExpression)
		{
			myExpression->saveToString(strGenericCachedString);
			strOut += strGenericCachedString;
		}
		else
			strOut += PROPERTY_NONE;
		strOut += "\n";
	}
	else if(myDataType == PropertyDataStringList)
	{
		int iCurr, iNum = myStringList->size();

		bool bWriteOutQuoted = false;
		// See if there are any newlines in the string
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
			if((*myStringList)[iCurr].find("\n") != string::npos || (*myStringList)[iCurr].find(",") != string::npos)
			{
				bWriteOutQuoted = true;
				break;
			}

			// Alternatively, if the first item would look like a number, we also have to quote, otherwise
			// we'll think the list is numeric			
			if(iCurr == 0 && (StringUtils::isANumberIgnoreSpaces((*myStringList)[iCurr].c_str(), true, false) || StringUtils::isAnUPNumber((*myStringList)[iCurr])) )
			{
				bWriteOutQuoted = true;
				break;
			}
		}

		strOut += strGenericCachedString + " = { ";
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
			if(iCurr != 0)
				strOut += ", ";

			if(bWriteOutQuoted)
				strOut += MUTLINE_STRING_BEGIN_MARKER;

			if((*myStringList)[iCurr].length() > 0)
				strOut += (*myStringList)[iCurr];
			else
				strOut += PROPERTY_NONE;

			if(bWriteOutQuoted)
			{
				strOut += " ";
				strOut += MUTLINE_STRING_END_MARKER;
			}
		}
		strOut += " }\n";
	}
	else if(myDataType == PropertyDataNumberList)
	{
		int iCurr, iNum = myNumberList->size();
		strOut += strGenericCachedString + " = { ";
		float fArrayValue;
		int iArrayValue;
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
			if(iCurr != 0)
				strOut += ", ";
			fArrayValue = (*myNumberList)[iCurr];

			if( fabs( (FLOAT_TYPE)((int)fArrayValue) - fArrayValue) < FLOAT_EPSILON )
			{
				iArrayValue = fArrayValue;
				sprintf(pcsBuff, "%d", iArrayValue);
			}
			else
				sprintf(pcsBuff, "%f", fArrayValue);
			strOut += pcsBuff;
		}
		strOut += " }\n";
	}
	else if(myDataType == PropertyDataFloatArray2D)
	{
		// It's an array! Who-hoo!
		// Most likely it's a map.
		strOut += "<" + strGenericCachedString + " type=\"array2d\">\n";
		int iCurrW, iCurrH;
		float fArrayValue;
		int iArrayValue;

		int iTempArrayW = myFloatArray[0];
		int iTempArrayH = myFloatArray[1];
		FLOAT_TYPE* fRealArrayStart = myFloatArray + EXTRA_ARRYA_FRONT_INFO;

		for(iCurrH = 0; iCurrH < iTempArrayH; iCurrH++)
		{
			strOut += "\t\t";
			for(iCurrW = 0; iCurrW < iTempArrayW; iCurrW++)
			{
				fArrayValue = GET_ARRAY_ELEM(iCurrW, iCurrH, fRealArrayStart, iTempArrayW);
				if( fabs( (FLOAT_TYPE)((int)fArrayValue) - fArrayValue) < FLOAT_EPSILON )
				{
					iArrayValue = fArrayValue;
					sprintf(pcsBuff, "%d ", iArrayValue);
				}
				else
					sprintf(pcsBuff, "%f ", fArrayValue);
				strOut += pcsBuff;
			}
			strOut += "\n";
		}
		strOut += "\t</" + strGenericCachedString + ">\n";
	}
	else
	{
		_ASSERT(0);
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceProperty<KEY_TYPE, STORAGE_TYPE>::saveToStringJSON(STRING_TYPE& strOut) const
{
	CHAR_TYPE pcsBuff[256];

	_ASSERT(!isNullProperty(myPropName));
	STRING_TYPE strGenericCachedString;
	strGenericCachedString = TResourceProperty<KEY_TYPE, STORAGE_TYPE>::mapPropertyKeyToString(PropertyMapper::getInstance(), myPropName);
	strGenericCachedString = STR_LIT("\"") + strGenericCachedString + STR_LIT("\"");

	if(myDataType == PropertyDataNumber)
	{
		// See if we're actually a floating point value, or an integer
		strOut += strGenericCachedString + STR_LIT(":");
		SPRINTF(pcsBuff, 256, STR_LIT("%g"), myNumValue);
		strOut += pcsBuff;
	}
	else if(myDataType == PropertyDataDouble)
	{
		// See if we're actually a floating point value, or an integer
		strOut += strGenericCachedString + STR_LIT(":");
		SPRINTF(pcsBuff, 256, STR_LIT("%.17g"), myDoubleValue);
		strOut += pcsBuff;
	}
	else if(myDataType == PropertyDataString)
	{
		strOut += strGenericCachedString + STR_LIT(":");

		if(myStrValue->length() > 0 && *myStrValue != PROPERTY_NONE)
		{
			STRING_TYPE strGenericCachedString2;
			strGenericCachedString2 = *myStrValue;
			StringUtils::removeControlCharacters(strGenericCachedString2, true);
			strOut += STR_LIT("\"") + strGenericCachedString2 + STR_LIT("\"");
		}
		else
			strOut += STR_LIT("null");
	}
	else if(myDataType == PropertyDataExpression)
	{
		strOut += strGenericCachedString + "=";
		if(myExpression)
		{
			myExpression->saveToString(strGenericCachedString);
			StringUtils::removeControlCharacters(strGenericCachedString, true);
			strOut += STR_LIT("\"") + strGenericCachedString + STR_LIT("\"");
		}
		else
			strOut += STR_LIT("null");
	}
	else if(myDataType == PropertyDataBool)
	{
		strOut += strGenericCachedString + STR_LIT(":");
		if(myBoolValue)
			strOut += STR_LIT("true");
		else
			strOut += STR_LIT("false");
	}
	else if(myDataType == PropertyDataStringList)
	{
		int iCurr, iNum = myStringList->size();

		strOut += strGenericCachedString + STR_LIT(" : [ ");
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
			if(iCurr != 0)
				strOut += STR_LIT(", ");

			if((*myStringList)[iCurr].length() > 0 && (*myStringList)[iCurr] != PROPERTY_NONE)
			{
				STRING_TYPE strGenericCachedString2;
				strGenericCachedString2 = (*myStringList)[iCurr];
				//gReplaceString(strGenericCachedString2, "\"", "\\\"");
				StringUtils::removeControlCharacters(strGenericCachedString2, true);
				strOut += STR_LIT("\"") + strGenericCachedString2 + STR_LIT("\"");
			}
			else
				strOut += STR_LIT("null");
		}
		strOut += STR_LIT(" ]");
	}
	else if(myDataType == PropertyDataNumberList)
	{
		int iCurr, iNum = myNumberList->size();
		strOut += strGenericCachedString + STR_LIT(" : [ ");
		float fArrayValue;
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
			if(iCurr != 0)
				strOut += STR_LIT(", ");
			fArrayValue = (*myNumberList)[iCurr];
			SPRINTF(pcsBuff, 256, STR_LIT("%g"), fArrayValue);
			strOut += pcsBuff;
		}
		strOut += STR_LIT(" ]\n");
	}
	else if(myDataType == PropertyDataFloatArray2D)
	{
		// 2D arrays not supported in JSON
		_ASSERT(0);
	}
	ELSE_ASSERT;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
bool TResourceProperty<KEY_TYPE, STORAGE_TYPE>::areEqualByValue(const TResourceProperty& rOther) const
{
	if(myDataType != rOther.myDataType)
		return false;

	// Otherwise, it's the same type. Compare the values...
	if(myDataType == PropertyDataNumber)
	{
		return fabs(myNumValue - rOther.myNumValue) < FLOAT_EPSILON;
	}
	else if(myDataType == PropertyDataString)
	{
		return *myStrValue == *rOther.myStrValue;
	}
	else if(myDataType == PropertyDataBool)
	{
		return myBoolValue == rOther.myBoolValue;
	}
	else if(myDataType == PropertyDataExpression)
	{
		return myExpression->isEqualTo(rOther.myExpression);
	}
	else if(myDataType == PropertyDataFloatArray2D)
	{
		int iTempArrayW = myFloatArray[0];
		int iTempArrayH = myFloatArray[1];

		int iTempArrayW2 = rOther.myFloatArray[0];
		int iTempArrayH2 = rOther.myFloatArray[1];

		if(iTempArrayH != iTempArrayH2 || iTempArrayW != iTempArrayW2)
			return false;

		int iCurr, iNumElems = iTempArrayH*iTempArrayW + EXTRA_ARRYA_FRONT_INFO;
		for(iCurr = 0; iCurr < iNumElems; iCurr++)
		{
			if(fabs(myFloatArray[iCurr] - rOther.myFloatArray[iCurr]) >= FLOAT_EPSILON)
				return false;
		}
		return true;
	}
	else if(myDataType == PropertyDataStringList)
	{
		if(myStringList->size() != rOther.myStringList->size())
			return false;

		int iCurr, iNum = myStringList->size();
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
			if((*myStringList)[iCurr] != (*rOther.myStringList)[iCurr])
				return false;
		}

		return true;
	}
	else if(myDataType == PropertyDataNumberList)
	{
		if(myNumberList->size() != rOther.myNumberList->size())
			return false;

		int iCurr, iNum = myNumberList->size();
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
			if(fabs((*myNumberList)[iCurr] - (*rOther.myNumberList)[iCurr]) >= FLOAT_EPSILON)
				return false;
		}

		return true;
	}
	ELSE_ASSERT_RETURN_NULL;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
bool TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getIsNone() const
{
	if(myDataType != PropertyDataString)
		return false;
	else
		return (*myStrValue) == PROPERTY_NONE;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
bool TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getIsLessThan(const TResourceProperty& rOther) const
{
	if(myDataType != rOther.getPropertyDataType() 
		|| myDataType == PropertyDataBool 
		|| myDataType == PropertyDataFloatArray2D
		|| myDataType == PropertyDataStringList
		|| myDataType == PropertyDataNumberList
		|| myDataType == PropertyDataExpression
		)
	{
		// Special case for double/float
		double dVal1 = DOUBLE_TYPE_MAX, dVal2 = DOUBLE_TYPE_MAX;
		if(myDataType == PropertyDataNumber)
			dVal1 = myNumValue;
		else if(myDataType == PropertyDataDouble)
			dVal1 = myDoubleValue;

		if(rOther.getPropertyDataType() == PropertyDataNumber)
			dVal2 = myNumValue;
		else if(rOther.getPropertyDataType() == PropertyDataDouble)
			dVal2 = myDoubleValue;

		if(dVal1 != DOUBLE_TYPE_MAX && dVal2 != DOUBLE_TYPE_MAX)
			return dVal1 < dVal2;

		// Convert both to strings and compare them that way. Whatever,
		// we shouldn't be sorting by properties of different types 
		// just because we can.
		STRING_TYPE str1, str2;
		this->getAsString(str1);
		this->getAsString(str2);
		return str1 < str2;
	}

	// Otherwise, go by the prop value:
	if(myDataType == PropertyDataNumber)
		return myNumValue < rOther.myNumValue;
	else if(myDataType == PropertyDataDouble)
		return myDoubleValue < rOther.myDoubleValue;
	else if(myDataType == PropertyDataString)
		return (*myStrValue) < (*rOther.myStrValue);

	_ASSERT(0);
	return false;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
bool TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getIsGreaterThan(const TResourceProperty& rOther) const
{
	if(myDataType != rOther.getPropertyDataType() 
		|| myDataType == PropertyDataBool 
		|| myDataType == PropertyDataFloatArray2D
		|| myDataType == PropertyDataStringList
		|| myDataType == PropertyDataNumberList
		|| myDataType == PropertyDataExpression
		)
	{
		// Special case for double/float
		double dVal1 = DOUBLE_TYPE_MAX, dVal2 = DOUBLE_TYPE_MAX;
		if(myDataType == PropertyDataNumber)
			dVal1 = myNumValue;
		else if(myDataType == PropertyDataDouble)
			dVal1 = myDoubleValue;

		if(rOther.getPropertyDataType() == PropertyDataNumber)
			dVal2 = myNumValue;
		else if(rOther.getPropertyDataType() == PropertyDataDouble)
			dVal2 = myDoubleValue;

		if(dVal1 != DOUBLE_TYPE_MAX && dVal2 != DOUBLE_TYPE_MAX)
			return dVal1 > dVal2;

		// Convert both to strings and compare them that way. Whatever,
		// we shouldn't be sorting by properties of different types 
		// just because we can.
		STRING_TYPE str1, str2;
		this->getAsString(str1);
		this->getAsString(str2);
		return str1 > str2;
	}

	// Otherwise, go by the prop value:
	if(myDataType == PropertyDataNumber)
		return myNumValue > rOther.myNumValue;
	else if(myDataType == PropertyDataDouble)
		return myDoubleValue > rOther.myDoubleValue;
	else if(myDataType == PropertyDataString)
		return (*myStrValue) > (*rOther.myStrValue);

	_ASSERT(0);
	return false;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
bool TResourceProperty<KEY_TYPE, STORAGE_TYPE>::getIsEqual(const TResourceProperty& rOther) const
{
	if(myDataType != rOther.getPropertyDataType())
		return false;

	if(myDataType == PropertyDataNumber)
		return myNumValue == rOther.myNumValue;
	else if(myDataType == PropertyDataDouble)
		return myDoubleValue == rOther.myDoubleValue;
	else if(myDataType == PropertyDataString)
		return myStrValue == rOther.myStrValue || (*myStrValue) == (*rOther.myStrValue);
	else if(myDataType == PropertyDataBool)
		return myBoolValue == rOther.myBoolValue;
	else
	{
		// Special case for double/float
		double dVal1 = DOUBLE_TYPE_MAX, dVal2 = DOUBLE_TYPE_MAX;
		if(myDataType == PropertyDataNumber)
			dVal1 = myNumValue;
		else if(myDataType == PropertyDataDouble)
			dVal1 = myDoubleValue;

		if(rOther.getPropertyDataType() == PropertyDataNumber)
			dVal2 = myNumValue;
		else if(rOther.getPropertyDataType() == PropertyDataDouble)
			dVal2 = myDoubleValue;

		if(dVal1 != DOUBLE_TYPE_MAX && dVal2 != DOUBLE_TYPE_MAX)
			return fabs(dVal1 - dVal2) <= DOUBLE_EPSILON;

		// Convert both to strings and compare them that way. Close enough...
		STRING_TYPE str1, str2;
		this->getAsString(str1);
		this->getAsString(str2);
		return str1 == str2;
	}

	_ASSERT(0);
	return false;
}
/*****************************************************************************/
template class TResourceProperty<PropertyType, PropertyType>;
template class TResourceProperty<const CHAR_TYPE*, RESOURCEITEM_STRING_TYPE >;
/*****************************************************************************/
};