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
class HYPERUI_API UIRoundProgressElement : public UIElement
{
public:

	DECLARE_STANDARD_UIELEMENT(UIRoundProgressElement, UiElemRoundProgress);

	virtual void onPreRenderChildren(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

	void setMinProgress(FLOAT_TYPE fValue) { myMinProgress = fValue; }
	void setMaxProgress(FLOAT_TYPE fValue) { myMaxProgress = fValue; }
	void setProgress(FLOAT_TYPE fValue, bool bAnimated = false, FLOAT_TYPE fOverrideTime = -1, bool bContinueFromCurrentAnim = false, FLOAT_TYPE fOffsetTime = -1.0);
	void stopCurrProgAnim();

protected:

	virtual void renderProgressAnimInternal(SVector2D& svCenter, FLOAT_TYPE fAnimValue, FLOAT_TYPE fFinalOpacity, FLOAT_TYPE fScale);

protected:

	FLOAT_TYPE myMinProgress, myMaxProgress, myCurrProgress;
	AnimatedValue myAnim;
};
/*****************************************************************************/
