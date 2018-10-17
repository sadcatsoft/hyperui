#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
UIUnitedNumeric::UIUnitedNumeric(UIPlane* pParentPlane)
	: UIElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void UIUnitedNumeric::onAllocated(IBaseObject* pData)
{
	UIElement::onAllocated(pData);
}
/*****************************************************************************/
bool UIUnitedNumeric::handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData)
{
	bool bDidHandle = false;
	if(strAction == UIA_DROPDOWN_ITEM_SELECTED_BEFORE_TEXT_UPDATE)
	{
		// Must be our numeric one. Note that we deliberatly not
		// set the handled flag since we may want this message 
		// to propagate to our parent in case it wants to do something
		// else.
		// bDidHandle = true;

		// We must find our text field and set num precision digits.
		UITextFieldElement* pField = getChildAndSubchild<UITextFieldElement>("defLabeledNumUpdown", "defNumBox");
		if(pField)
		{
			const char* pcsSelUnitId = as<UIDropdown>(pSourceElem)->getSelectedId();
			UnitType eNewUnits = mapStringToType(pcsSelUnitId, g_pcsUnitSuffixes, UnitLastPlaceholder);
			const char* pcsFormat = "%d";
			if(eNewUnits != UnitLastPlaceholder && eNewUnits != UnitPixels)
				pcsFormat = "%g";
			pField->setStringProp(PropertyTextFormat, pcsFormat);
		}
		ELSE_ASSERT;
	}

	return bDidHandle;
}
/*****************************************************************************/
};