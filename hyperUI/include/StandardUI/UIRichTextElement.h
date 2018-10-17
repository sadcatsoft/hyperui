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

#define RTE_LINE_BREAK			"{br}"
/*****************************************************************************/
enum RichObjectType
{
	RichObjectText = 0,
	RichObjectUserElem
};
/*****************************************************************************/
struct HYPERUI_API SRichObject
{
	SRichObject() { myElem = NULL; }
	~SRichObject() { if(myElem) delete myElem; }

	string myString;
	RichObjectType myType;

	SVector2D myPos;
	UIElement* myElem;
};
typedef vector < SRichObject* > TRichObjects;
/*****************************************************************************/
class HYPERUI_API UIRichTextElement : public UIZoomWindowElement
{
public:
	DECLARE_STANDARD_UIELEMENT_NO_DESTRUCTOR_DEF(UIRichTextElement, UiElemRichText);
	virtual void onDeallocated(void);

	//virtual void render(SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

	virtual UIElement* setTextForChild(const char* pcsChildId, const char* pcsText, SColor* pOptColor = NULL);
	virtual UIElement* setTextForChild(const char* pcsChildId, const char* pcsSubChildId, const char* pcsText, SColor* pOptColor = NULL);
	virtual UIElement* setTextForChild(const char* pcsChildId, string& strText, SColor* pOptColor = NULL, FLOAT_TYPE fShortenTextMaxLen = 0);
	virtual UIElement* setIsVisibleForChild(const char* pcsChildId, bool bIsVisible);

	virtual void setText(const char* pcsText);
	virtual void postInit(void);

	virtual FLOAT_TYPE getMinZoom(void) { return 1.0; }
	virtual FLOAT_TYPE getMaxZoom(void) { return 1.0; }

	virtual UIElement* getChildAtRecursive(FLOAT_TYPE fX, FLOAT_TYPE fY, bool bFirstNonPassthrough, bool bGlidingOnly);

	UIElement* getObjectById(const char* pcsId);
	void hideObjectGroup(const char* pcsGroupId);

	virtual void updateOwnData(SUpdateInfo& rRefreshInfo);
	virtual bool applyMouseWheelDelta(FLOAT_TYPE fDeltaMulted);
	virtual bool getAllowWheelScroll() const { return true; }

	void scrollOnSlider(UISliderElement* pSlider);

	void getEntireText(string& strOut);

protected:

	virtual void renderZoomedContents(const SRect2D& srWindowRect, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);
	virtual void getContentSize(SVector2D& svOut);

	virtual UIElement* getChildAtRecursiveInternal(const SVector2D& svPoint, const SVector2D& svScroll, bool bFirstNonPassthrough, bool bIgnoreChildren, bool bGlidingOnly);
	UIElement* getObjectAt(FLOAT_TYPE fScreenX, FLOAT_TYPE fScreenY);

	void updateRelatedSlider();

private:
	void parseRichText(const char* pcsText, TRichObjects& rVecOut);
	void copyTokenPropsToResourceItem(TCharPtrVector& rTokens, TStringStringMap& mapTempTokens, ResourceItem& rPropsOut);

private:

	TRichObjects myObjects;
	FLOAT_TYPE myTotalHeight;
	bool myAreObjectsDirty;

	static TCharPtrVector theTokens;
	static TCharPtrVector theInnerTokens;
};
/*****************************************************************************/
