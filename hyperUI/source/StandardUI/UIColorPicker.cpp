/*****************************************************************************

Disclaimer: This software is supplied to you by Sad Cat Software
("Sad Cat") in consideration of your agreement to the following terms, and 
your use, installation, modification or redistribution of this Sad Cat software
constitutes acceptance of these terms.  If you do not agree with these terms,
please do not use, install, modify or redistribute this Sad Cat software.

This software is provided "as is". Sad Cat Software makes no warranties, 
express or implied, including without limitation the implied warranties
of non-infringement, merchantability and fitness for a particular
purpose, regarding Sad Cat's software or its use and operation alone
or in combination with other hardware or software products.

In no event shall Sad Cat Software be liable for any special, indirect,
incidental, or consequential damages (including, but not limited to, 
procurement of substitute goods or services; loss of use, data, or profits;
or business interruption) arising in any way out of the use, reproduction,
modification and/or distribution of Sad Cat's software however caused and
whether under theory of contract, tort (including negligence), strict
liability or otherwise, even if Sad Cat Software has been advised of the
possibility of such damage.

Copyright (C) 2012, Sad Cat Software. All Rights Reserved.

*****************************************************************************/
#include "stdafx.h"

namespace HyperUI
{

const char* const g_pcsActiveModeCheckboxNames[ActiveColorModeLastPlaceholder] =
{
	"cpdRadioRed",
	"cpdRadioGreen",
	"cpdRadioBlue",
	"cpdRadioHue",
	"cpdRadioSat",
	"cpdRadioBri",
};

ActiveColorModeType UIColorPicker::theLastActiveColorMode = ActiveColorModeHue;
/*****************************************************************************/
UIColorPicker::UIColorPicker(UIPlane* pParentPlane)
	:  UIElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void UIColorPicker::onAllocated(IBaseObject* pData)
{
	 UIElement::onAllocated(pData);
	 myElemWindowId = -1;
}
/*****************************************************************************/
void UIColorPicker::postInit()
{
	UIElement::postInit();
	setChildTargetIdentifiers();
	setColor(SColor::White, true, true, false);

	// Set action types
	getChildById<UIColorSwatch>("cpdNewComplColor1")->setClickAction(UIColorSwatch::ClickActionExecuteUiAction);
	getChildById<UIColorSwatch>("cpdNewComplColor2")->setClickAction(UIColorSwatch::ClickActionExecuteUiAction);
	getChildById<UIColorSwatch>("cpdNewComplColor3")->setClickAction(UIColorSwatch::ClickActionExecuteUiAction);
	getChildById<UIColorSwatch>("cpdNewComplColor4")->setClickAction(UIColorSwatch::ClickActionExecuteUiAction);
	getChildById<UIColorSwatch>("cpdNewComplColor5")->setClickAction(UIColorSwatch::ClickActionExecuteUiAction);

	getChildById<UIColorSwatch>("cpdOldColorSwatch")->setClickAction(UIColorSwatch::ClickActionExecuteUiAction);
	getChildById<UIColorSwatch>("cpdNewColorSwatch")->setClickAction(UIColorSwatch::ClickActionExecuteUiAction);
}
/*****************************************************************************/
void UIColorPicker::updateOwnData(SUpdateInfo& rRefreshInfo)
{
	refreshComplementaryEnableStatus();
	computeComplementaryColors();

	UIButtonElement* pButton = this->getChildAndSubchild<UIButtonElement>(g_pcsActiveModeCheckboxNames[theLastActiveColorMode], "defLabel");
	if(pButton)
		pButton->simulateClick();
}
/*****************************************************************************/
bool UIColorPicker::handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData)
{
	bool bHandled = false;
	if(strAction == COLOR_CHANGED_ACTION)
	{
		// Change our color
		SColor scolNew;
		pSourceElem->getAsColor(PropertyActionValue3, scolNew);
		setColor(scolNew);

		bHandled = true;
	}
	else if(strAction == "uiaChangeColorModel")
	{
		const char* pcsMode = pSourceElem->getStringProp(PropertyActionValue);
		UIColorPalette* pPalette = getChildById<UIColorPalette>("cpdColorPalette");
		pPalette->changeModeTo(pcsMode);

		theLastActiveColorMode = getCurrentColorMode();
	}
	else if(strAction == COLOR_SWATCH_CLICK_ACTION)
	{
		// Get the current swatch color and set it to us:
		SColor scolNew;
		if(as<UIColorSwatch>(pSourceElem))
			as<UIColorSwatch>(pSourceElem)->getColor(scolNew);
		else if(as<UIRecentColorsPalette>(pSourceElem))
			as<UIRecentColorsPalette>(pSourceElem)->getLastSelectedColor(scolNew);
		ELSE_ASSERT;
		setColor(scolNew);
	}
	else if(strAction == "uiaCopyHexColorToClipboard")
	{
		myCurrentColor.toHex(theCommonString, false);
		ClipboardManager::setClipboardText(theCommonString.c_str());
	}

	return bHandled;
}
/*****************************************************************************/
void UIColorPicker::setColor(const SColor& scolNew, bool bSetHSB, bool bSetRGB, bool bUpdateTargetElem)
{
	myCurrentColor = scolNew;
	myCurrentColor.wrapToRange();

	// Set the color onto all our elems
	int iTemp;

	// RGB
	if(bSetRGB)
	{
		iTemp = myCurrentColor.r*255.0;
		StringUtils::numberToString(iTemp, theCommonString);
		setTextForChild("cpdRadioRed", "defNumBox", theCommonString.c_str());

		iTemp = myCurrentColor.g*255.0;
		StringUtils::numberToString(iTemp, theCommonString);
		setTextForChild("cpdRadioGreen", "defNumBox", theCommonString.c_str());

		iTemp = myCurrentColor.b*255.0;
		StringUtils::numberToString(iTemp, theCommonString);
		setTextForChild("cpdRadioBlue", "defNumBox", theCommonString.c_str());
	}

	// HSB
	FLOAT_TYPE fH, fS, fB;
	myCurrentColor.toHSB(fH, fS, fB);

	if(bSetHSB)
	{
		iTemp = fH*360.0;
		StringUtils::numberToString(iTemp, theCommonString);
		setTextForChild("cpdRadioHue", "defNumBox", theCommonString.c_str());

		iTemp = fS*100.0;
		StringUtils::numberToString(iTemp, theCommonString);
		setTextForChild("cpdRadioSat", "defNumBox", theCommonString.c_str());

		iTemp = fB*100.0;
		StringUtils::numberToString(iTemp, theCommonString);
		setTextForChild("cpdRadioBri", "defNumBox", theCommonString.c_str());
	}
	
	// Web
	myCurrentColor.toHex(theCommonString, false);
	//theCommonString += "#";
	setTextForChild("cpdWebColor", "defTextBox", theCommonString.c_str());

	computeComplementaryColors();

	// Main color
	UIColorSwatch* pColorSwatch;
	pColorSwatch = as<UIColorSwatch>(getChildById("cpdNewColorSwatch"));
	if(pColorSwatch)
		pColorSwatch->setColor(myCurrentColor);

	// Get palette, set its color
	UIColorPalette* pPalette = getChildById<UIColorPalette>("cpdColorPalette");
	pPalette->setBaseColor(myCurrentColor);

	if(bUpdateTargetElem)
		updateTargetElem(myCurrentColor);
}
/*****************************************************************************/
void UIColorPicker::refreshComplementaryEnableStatus()
{
	int iNumComplColors = getChildAndSubchild("cpdNumComplementaryColors", "defNumBox")->getTextAsNumber();
	int iCurr;
	UIElement* pElem;
	SColor scolDisabledColor(0.2, 0.2, 0.2, 1.0);
	for(iCurr = 0; iCurr < 5; iCurr++)
	{
		StringUtils::numberToString(iCurr + 1, theCommonString);
		theCommonString = "cpdNewComplColor" + theCommonString;
		pElem = getChildById(theCommonString.c_str());
		pElem->setIsEnabled(iCurr < iNumComplColors);

		as<UIColorSwatch>(pElem)->setColor(scolDisabledColor);
	}
}
/*****************************************************************************/
void UIColorPicker::setChildTargetIdentifiers()
{
	getChildAndSubchild("cpdRadioHue", "defNumBox")->setTargetUiElem(this);
	getChildAndSubchild("cpdRadioSat", "defNumBox")->setTargetUiElem(this);
	getChildAndSubchild("cpdRadioBri", "defNumBox")->setTargetUiElem(this);

	getChildAndSubchild("cpdRadioRed", "defNumBox")->setTargetUiElem(this);
	getChildAndSubchild("cpdRadioGreen", "defNumBox")->setTargetUiElem(this);
	getChildAndSubchild("cpdRadioBlue", "defNumBox")->setTargetUiElem(this);

	getChildAndSubchild("cpdWebColor", "defTextBox")->setTargetUiElem(this);

	getChildAndSubchild("cpdNumComplementaryColors", "defNumBox")->setTargetUiElem(this);
}
/*****************************************************************************/
void UIColorPicker::onRefreshStoredValueFromUICallback(UIElement* pSourceElem, bool bIsChangingContinuously)
{
	if(as<UITextFieldElement>(pSourceElem))
	{
		UITextFieldElement* pCastElem = as<UITextFieldElement>(pSourceElem);
		if(pSourceElem->getParentById("cpdRadioHue")
			|| pSourceElem->getParentById("cpdRadioSat")
			|| pSourceElem->getParentById("cpdRadioBri") )
		{
			// We've changed HSB value. Read them all, assemble a color, and set it.
			FLOAT_TYPE fH, fS, fB;
			fH = getChildAndSubchild("cpdRadioHue", "defNumBox")->getTextAsNumber()/360.0;
			fS = getChildAndSubchild("cpdRadioSat", "defNumBox")->getTextAsNumber()/100.0;
			fB = getChildAndSubchild("cpdRadioBri", "defNumBox")->getTextAsNumber()/100.0;

			SColor scolFinal;
			scolFinal.fromHSB(fH, fS, fB);
			setColor(scolFinal, false, true);
		}
		else if(pSourceElem->getParentById("cpdRadioRed")
			|| pSourceElem->getParentById("cpdRadioGreen")
			|| pSourceElem->getParentById("cpdRadioBlue") )
		{
			// We've changed HSB value. Read them all, assemble a color, and set it.
			SColor scolFinal;
			scolFinal.r = getChildAndSubchild("cpdRadioRed", "defNumBox")->getTextAsNumber()/255.0;
			scolFinal.g = getChildAndSubchild("cpdRadioGreen", "defNumBox")->getTextAsNumber()/255.0;
			scolFinal.b = getChildAndSubchild("cpdRadioBlue", "defNumBox")->getTextAsNumber()/255.0;

			setColor(scolFinal, true, false);
		}
		else if(pSourceElem->getParentById("cpdWebColor"))
		{
			// Webcolor
			getChildAndSubchild("cpdWebColor", "defTextBox")->getTextAsString(theCommonString);
			SColor scolFinal;
			scolFinal.fromHex(theCommonString.c_str());

			setColor(scolFinal);
		}
		else if(pSourceElem->getParentById("cpdNumComplementaryColors"))
		{
			// Number of colors
			refreshComplementaryEnableStatus();
			computeComplementaryColors();
		}
	}
}
/*****************************************************************************/
void UIColorPicker::computeComplementaryColors()
{
	FLOAT_TYPE fH, fS, fB;
	myCurrentColor.toHSB(fH, fS, fB);

	// Compute cop
	int iNumComplColors = getChildAndSubchild("cpdNumComplementaryColors", "defNumBox")->getTextAsNumber();
	FLOAT_TYPE fHueStep = 1.0/(FLOAT_TYPE)(iNumComplColors + 1);

	FLOAT_TYPE fCurrHue = fH;
	int iCurrColor;
	SColor scolCurr;
	UIColorSwatch* pColorSwatch;
	for(iCurrColor = 0; iCurrColor < iNumComplColors; iCurrColor++)
	{
		fCurrHue += fHueStep;
		fCurrHue = HyperCore::mantissa(fCurrHue);
		scolCurr.fromHSB(fCurrHue, fS, fB);

		StringUtils::numberToString(iCurrColor + 1, theCommonString);
		theCommonString = "cpdNewComplColor" + theCommonString;
		pColorSwatch = as<UIColorSwatch>(getChildById(theCommonString.c_str()));
		if(pColorSwatch)
			pColorSwatch->setColor(scolCurr);
	}
}
/*****************************************************************************/
void UIColorPicker::updateTargetElem(const SColor& scolNewColor)
{
	Window* pWindow = WindowManager::getInstance()->findItemById(myElemWindowId);
	if(!pWindow)
		ASSERT_RETURN;

	UIElement* pElem = pWindow->getUIPlane()->getElementById<UIElement>(myElemId.c_str(), true, false);
	if(!pElem)
		ASSERT_RETURN;

	UIColorSwatch* pColor = as<UIColorSwatch>(pElem);
	if(pColor)
		pColor->changeColorFromUI(scolNewColor, true);
	ELSE_ASSERT;
}
/*****************************************************************************/
void UIColorPicker::cancelColorSelection()
{
	updateTargetElem(myInitialColor);
}
/*****************************************************************************/
void UIColorPicker::setRecentColors(const TUniqueIndexedColorSet* pColorSet)
{
	UIRecentColorsPalette* pResColsElem = getChildById<UIRecentColorsPalette>("cpdRecentColors");
	if(pResColsElem)
		pResColsElem->setRecentColors(pColorSet);
	ELSE_ASSERT;
}
/*****************************************************************************/
int UIColorPicker::getNumComplementaryColors()
{
	return getNumericValueForChild("cpdNumComplementaryColors", "defNumBox", UnitLastPlaceholder);
}
/*****************************************************************************/
void UIColorPicker::setNumComplementaryColors(int iNumColors)
{
	StringUtils::numberToString(iNumColors, theCommonString);
	setTextForChild("cpdNumComplementaryColors", "defNumBox", theCommonString.c_str());
	refreshComplementaryEnableStatus();
}
/*****************************************************************************/
ActiveColorModeType UIColorPicker::getCurrentColorMode()
{
	UIButtonElement* pButton;
	int iCurr;
	for(iCurr = 0; iCurr < ActiveColorModeLastPlaceholder; iCurr++)
	{
		pButton = this->getChildAndSubchild<UIButtonElement>(g_pcsActiveModeCheckboxNames[iCurr], "defLabel");
		if(!pButton)
			ASSERT_CONTINUE;
		if(pButton->getIsPushed())
			return (ActiveColorModeType)iCurr;
	}

	return ActiveColorModeLastPlaceholder;
}
/*****************************************************************************/
};