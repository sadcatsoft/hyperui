#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
UndoItemUIValueChange::UndoItemUIValueChange(UIElement * pElem)
{
	myTargetElementName = pElem->getStringProp(PropertyId);

	// Store the current value here
	storeValue(pElem, myInitElemValue, myInitSecInfo);
}
/*****************************************************************************/
UndoItemUIValueChange::~UndoItemUIValueChange()
{

}
/*****************************************************************************/
void UndoItemUIValueChange::undo(IBaseObject* pContext, UIElement* pElement)
{
	if(!pElement)
		ASSERT_RETURN;

	// Find the element
	UIElement* pFinalElem = NULL;
	if(myTargetElementName == pElement->getStringProp(PropertyId))
		pFinalElem = pElement;

	if(!pFinalElem)
		pFinalElem = pElement->getChildById(myTargetElementName.c_str());

	if(!pFinalElem)
		ASSERT_RETURN;

	// Copy the saved value onto the UI...
	storeValue(pFinalElem, myRedoElemValue, myRedoSecInfo);
	retrieveValue(myInitElemValue, myInitSecInfo, pFinalElem);
}
/*****************************************************************************/
void UndoItemUIValueChange::redo(IBaseObject* pContext, UIElement* pElement)
{
	if(!pElement)
		ASSERT_RETURN;

	// Find the element
	UIElement* pFinalElem = NULL;
	if(myTargetElementName == pElement->getStringProp(PropertyId))
		pFinalElem = pElement;

	if(!pFinalElem)
		pFinalElem = pElement->getChildById(myTargetElementName.c_str());

	if(!pFinalElem)
		ASSERT_RETURN;

	retrieveValue(myRedoElemValue, myRedoSecInfo, pFinalElem);
}
/*****************************************************************************/
void UndoItemUIValueChange::storeValue(UIElement* pElem, ResourceProperty& rItem, int& iSecInfo)
{
	if(dynamic_cast<UITextFieldElement*>(pElem))
	{
		UITextFieldElement* pCast = dynamic_cast<UITextFieldElement*>(pElem);
		if(pCast->getIsNumeric())
		{
			// What to do about units...? Use a string...?
			rItem.setNum(pCast->getNumericValue(UnitLastPlaceholder));
		}
		else
		{
			string strTemp;
			pCast->getTextAsString(strTemp);
			rItem.setString(strTemp.c_str());
		}
	}
	else if(dynamic_cast<UIDropdown*>(pElem))
	{
		UIDropdown* pCast = dynamic_cast<UIDropdown*>(pElem);
		rItem.setString(pCast->getSelectedId());
	}
	else if(as<UIColorPicker>(pElem))
	{
		SColor scolTemp;
		as<UIColorPicker>(pElem)->getCurrentColor(scolTemp);
		iSecInfo = as<UIColorPicker>(pElem)->getNumComplementaryColors();
		rItem.setAsColor(scolTemp);
	}
	else if(as<UIGradientEdit>(pElem))
	{
		string strTemp;
		const SGradient *pGrad = as<UIGradientEdit>(pElem)->getGradient();
		pGrad->saveToString(strTemp);
		rItem.setString(strTemp.c_str());
	}
	else if(as<UICurveEditor>(pElem))
	{
		string strTemp;
		const SCubicCurve* pCurve = as<UICurveEditor>(pElem)->getCurve();
		pCurve->saveToString(strTemp);
		rItem.setString(strTemp.c_str());
	}
	else 
	{
		_ASSERT(0);
	}
}
/*****************************************************************************/
void UndoItemUIValueChange::retrieveValue(ResourceProperty& rFromItem, int iSecInto, UIElement* pToElem)
{
	if(dynamic_cast<UITextFieldElement*>(pToElem))
	{
		UITextFieldElement* pCast = dynamic_cast<UITextFieldElement*>(pToElem);
		
		if(pCast->getIsNumeric())
		{
			// Units?
			pCast->changeValueTo(rFromItem.getNum(), NULL, true, false);
		}
		else
			pCast->setText(rFromItem.getString());
			
	}
	else if(dynamic_cast<UIDropdown*>(pToElem))
	{
		UIDropdown* pCast = dynamic_cast<UIDropdown*>(pToElem);
		pCast->selectItem(rFromItem.getString());
	}
	else if(as<UIColorPicker>(pToElem))
	{
		SColor scolTemp;
		rFromItem.getAsColor(scolTemp);
		as<UIColorPicker>(pToElem)->setNumComplementaryColors(iSecInto);
		as<UIColorPicker>(pToElem)->setColor(scolTemp);
	}
	else if(as<UIGradientEdit>(pToElem))
	{
		SGradient rGrad;
		rGrad.loadFromString(rFromItem.getString());
		as<UIGradientEdit>(pToElem)->setGradient(rGrad);
	}
	else if(as<UICurveEditor>(pToElem))
	{
		SCubicCurve rCurve;
		rCurve.loadFromString(rFromItem.getString());
		as<UICurveEditor>(pToElem)->setCurve(rCurve);
	}
	else 
	{
		_ASSERT(0);
	}
}
/*****************************************************************************/
void UndoItemUIValueChange::generateDescriptiveString(string& strOut)
{
	string strTemp;
	strOut = "Changing " + myTargetElementName;
	myInitElemValue.getAsString(strTemp);
	strOut += " from [" + strTemp;
	myRedoElemValue.getAsString(strTemp);
	strOut += "]  to [" + strTemp;
	strOut += "]";
}
/*****************************************************************************/
};