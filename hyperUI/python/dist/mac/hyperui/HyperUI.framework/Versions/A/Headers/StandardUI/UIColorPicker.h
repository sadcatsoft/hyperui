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
#pragma once

enum ActiveColorModeType
{
	ActiveColorModeRed = 0,
	ActiveColorModeGreen,
	ActiveColorModeBlue,
	ActiveColorModeHue,
	ActiveColorModeSat,
	ActiveColorModeBri,

	ActiveColorModeLastPlaceholder,
};

/********************************************************************************************/
class HYPERUI_API UIColorPicker : public UIElement
{
public:
	DECLARE_STANDARD_UIELEMENT(UIColorPicker, UiElemColorPicker);
	virtual void updateOwnData(SUpdateInfo& rRefreshInfo);

	inline void getCurrentColor(SColor& scolOut) const { scolOut = myCurrentColor; }
	void setColor(const SColor& scolNew, bool bSetHSB = true, bool bSetRGB = true, bool bUpdateTargetElem = true);
	inline void setInitialColor(const SColor& scolNew) { myInitialColor = scolNew; }

	void refreshComplementaryEnableStatus();

	virtual void postInit();
	virtual void onRefreshStoredValueFromUICallback(UIElement* pSourceElem, bool bIsChangingContinuously);
	virtual bool handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData);

	inline void setTargetForColorChange(const char* pcsElemId, UNIQUEID_TYPE idWindow) { myElemId = pcsElemId; myElemWindowId = idWindow; }

	void cancelColorSelection();
	void setRecentColors(const TUniqueIndexedColorSet* pColorSet);

	int getNumComplementaryColors();
	void setNumComplementaryColors(int iNumColors);

private:

	void setChildTargetIdentifiers();
	void computeComplementaryColors();

	void updateTargetElem(const SColor& scolNewColor);

	ActiveColorModeType getCurrentColorMode();

private:

	SColor myCurrentColor;
	// Used for cancelling
	SColor myInitialColor;

	string myElemId;
	UNIQUEID_TYPE myElemWindowId;

	static ActiveColorModeType theLastActiveColorMode;
};
/********************************************************************************************/
