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

/*****************************************************************************/
class HYPERUI_API UIPopupElement : public UIElement
{
public:

	DECLARE_STANDARD_UIELEMENT(UIPopupElement, UiElemPopup);

	// Override this for custom size controls
	virtual bool getContentSize(SVector2D& svOut);

	virtual void onPostUpdateChildData();
	virtual void postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

	void getTotalPadding(SVector2D& svPaddingOut);

	static void drawNose(DrawingCache* pDrawingCache, const char* pcsAnim, const SVector2D& svOwnPos, const SVector2D& svOwnSize, const SVector2D& svNoseOffset, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale, SideType eSide);
};
/*****************************************************************************/
