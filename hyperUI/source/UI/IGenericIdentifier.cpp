#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
// IParmBasedIdentifier
/*****************************************************************************/
template < class PROPERTY_TYPE >
void IParmBasedIdentifier< PROPERTY_TYPE >::refreshUIFromParm(UIElement* pTargetElem, PROPERTY_TYPE* pParm)
{
	if(!pParm || !pTargetElem)
		return;

	pTargetElem->setIsUpdatingFromParm(true);

	if(as<UITextFieldElement>(pTargetElem))
	{
		const char* pcsFormat = NULL;
		if(pTargetElem->doesPropertyExist(PropertyTextFormat))
			pcsFormat = pTargetElem->getStringProp(PropertyTextFormat);

		if(pParm->getPropertyDataType() == PropertyDataExpression)
			pParm->getExpression()->getRawExpressionWithPrefix(mySharedString);
		else
			pParm->getAsString(mySharedString, pcsFormat);

		pTargetElem->setText(mySharedString.c_str());

		as<UITextFieldElement>(pTargetElem)->updateLinkedElement(false, false);
		
	}
	else if(as<UIButtonElement>(pTargetElem))
	{
		// Checkbox & buttons
		as<UIButtonElement>(pTargetElem)->setIsPushed(pParm->getBool(), true);		
	}
	else if(as<UIColorSwatch>(pTargetElem))
	{
		SColor scol;
		pParm->getAsColor(scol);
		as<UIColorSwatch>(pTargetElem)->setColor(scol);
	}
	else if(as<UIGradientEdit>(pTargetElem))
	{
		SGradient rGrad;
		rGrad.loadFromString(pParm->getString());
		as<UIGradientEdit>(pTargetElem)->setGradient(rGrad);
	}
	else if(as<UIColorList>(pTargetElem))
	{
		SColorList rColList;
		rColList.loadFromString(pParm->getString());
		as<UIColorList>(pTargetElem)->setColorList(rColList);
	}
	else if(as<UICurveEditor>(pTargetElem))
	{
		SCubicCurve rCurve;
		rCurve.loadFromString(pParm->getString());
		as<UICurveEditor>(pTargetElem)->setCurve(rCurve);
	}
	else if(as<UIDropdown>(pTargetElem))
	{
		as<UIDropdown>(pTargetElem)->selectItem(pParm->getString());
	}
	else if(pTargetElem->getElemType() == UiElemStatic)
	{
		// Nothing
	}

	pTargetElem->setIsUpdatingFromParm(false);
}
/*****************************************************************************/
template < class PROPERTY_TYPE >
void IParmBasedIdentifier< PROPERTY_TYPE >::refreshParmFromUI(UIElement* pSourceElem, PROPERTY_TYPE* pParm, bool bIsChangingContinuously)
{
	if(!pParm)
		return;

	if(as<UIExpressionTextFieldElement>(pSourceElem) && as<UIExpressionTextFieldElement>(pSourceElem)->getHaveValidExpression())
	{
		UIExpressionTextFieldElement* pCastElem = as<UIExpressionTextFieldElement>(pSourceElem);
		pCastElem->getTextAsStringOrExpression(mySharedString);
		pParm->setExpression(mySharedString.c_str());
	}
	else if(as<UITextFieldElement>(pSourceElem))
	{
		UITextFieldElement* pCastElem = as<UITextFieldElement>(pSourceElem);
		if(pCastElem->getIsNumeric())
			pParm->setNum(pCastElem->getNumericValue(DEFAULT_STORAGE_UNITS));
		else
		{
			pCastElem->getTextAsString(mySharedString);
			pParm->setString(mySharedString.c_str());
		}
	}                                                                                                                         
	else if(as<UIDropdown>(pSourceElem))
	{
		UIDropdown* pCastElem = as<UIDropdown>(pSourceElem);
		pParm->setString(pCastElem->getSelectedId());
	}
	else if(as<UIColorSwatch>(pSourceElem))
	{
		UIColorSwatch* pCastElem = as<UIColorSwatch>(pSourceElem);
		SColor rCol;
		pCastElem->getColor(rCol);
		pParm->setAsColor(rCol);
	}
	else if(as<UICheckboxElement>(pSourceElem))
	{
		UICheckboxElement* pCastElem = as<UICheckboxElement>(pSourceElem);
		pParm->setBool(pCastElem->getIsPushed());
	}
	else if(as<UIButtonElement>(pSourceElem))
	{
		UIButtonElement *pCastElem = as<UIButtonElement>(pSourceElem);
		pParm->setBool(pCastElem->getIsPushed());
	}
	else if(as<UIGradientEdit>(pSourceElem))
	{
		const SGradient *pGrad = as<UIGradientEdit>(pSourceElem)->getGradient();
		pGrad->saveToString(mySharedString);
		pParm->setString(mySharedString.c_str());
	}
	else if(as<UIColorList>(pSourceElem))
	{
		const SColorList *pColList = as<UIColorList>(pSourceElem)->getColorList();
		pColList->saveToString(mySharedString);
		pParm->setString(mySharedString.c_str());
	}
	else if(as<UIDropdown>(pSourceElem))
	{		
		pParm->setString(as<UIDropdown>(pSourceElem)->getSelectedId());
	}
	else if(as<UICurveEditor>(pSourceElem))
	{
		as<UICurveEditor>(pSourceElem)->getCurve()->saveToString(mySharedString);
		pParm->setString(mySharedString.c_str());
	}
	else if(pSourceElem->getElemType() == UiElemStatic)
	{
		// Nothing
	}
}
/*****************************************************************************/
template class IParmBasedIdentifier< StringResourceProperty >;
template class IParmBasedIdentifier< ResourceProperty >;
};