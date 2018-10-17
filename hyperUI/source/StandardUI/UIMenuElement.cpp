#include "stdafx.h"

// TODO: These probably needs to be moved out to props...
#define SEPARATOR_HOR_ANIM	    "separatorHorAnim"
#define SEPARATOR_VERT_ANIM	    "separatorVertAnim"
#define MENU_HIGHLIGHT_ANIM	    "menuHiAnim"

#define HOR_SEP_THICKNESS_ADJUSTMENT	-upToScreen(4.0)

#define MENU_DISABLED_OPACITY_MULT		0.35
#define MENU_ICON_DISABLED_OPACITY_MULT		0.25

#define SEP_END_PADDING			upToScreen(3.0)
#define SUBMENU_ARROW_PADDING	upToScreen(8.0)

#define COMMON_HOR_OFFSET_AND_SPACING	upToScreen(8.0 + 3.0 + 1.5)

// Init offsets
#define HOR_MENU_INIT_OFFSET_X		upToScreen(13.0)
#define HOR_MENU_INIT_OFFSET_Y		upToScreen(10.0)
#define VERT_MENU_INIT_OFFSET_X		upToScreen(-3.5 - 1.5)		// -6
#define VERT_MENU_INIT_OFFSET_Y		upToScreen(12.5 + 4.0)		// 10
#define VERT_MENU_BOTTOM_PADDING	upToScreen(2.0)

// Highlight size
#define HOR_MENU_HIGHLIGHT_HOR_PADDING	    upToScreen(4.0)
#define HOR_MENU_HIGHLIGHT_VERT_PADDING	    -upToScreen(3.5)
#define VERT_MENU_HIGHLIGHT_HOR_PADDING	    upToScreen(-4.0)
#define VERT_MENU_HIGHLIGHT_VERT_PADDING    upToScreen(0.0)

// Highlight offset
#define HOR_MENU_HIGHLIGHT_OFFSET_X			upToScreen(0.5)
#define HOR_MENU_HIGHLIGHT_OFFSET_Y			-upToScreen(0.5)
#define VERT_MENU_HIGHLIGHT_OFFSET_X		upToScreen(0)
#define VERT_MENU_HIGHLIGHT_OFFSET_Y		-upToScreen(0.5)

// Vert menu item spacing
#define VERT_MENU_ITEM_SPACING_Y			upToScreen(14.5)		//12.5

// Submenus offset
#define HOR_SUBMENU_POS_OFFSET_X		-upToScreen(10.0)
#define HOR_SUBMENU_POS_OFFSET_Y		-upToScreen(4.0)
#define VERT_SUBMENU_POS_OFFSET_X		-upToScreen(6.0)
#define VERT_SUBMENU_POS_OFFSET_Y		-upToScreen(9.0)

// Shortcuts
#define SHORTCUT_EST_KEY_SIZE		upToScreen(3.5)
#define SHORTCUT_HOR_OFFSET			upToScreen(16.0)

namespace HyperUI
{
/*****************************************************************************/
UIMenuElement::UIMenuElement(UIPlane* pParentPlane)
	: UIElement(pParentPlane), myDummyButton(pParentPlane)
{
    onAllocated(pParentPlane);
}
/*****************************************************************************/
void UIMenuElement::onAllocated(IBaseObject* pData)
{
    UIElement::onAllocated(pData);
	myDoShowChildrenOnGlide = false;

	myDummyButton.onAllocated(pData);

	myDefaultProvider.setSourceItem(this);
	myEnableStatusCalllback = NULL;
	myAreShortcutsDirty = false;
	myIsDirty = false;
	myPrevSelectedEntry = -1;
	myIconScale = 1.0;
}
/*****************************************************************************/
UIMenuElement::~UIMenuElement()
{
	onDeallocated();
}
/*****************************************************************************/
void UIMenuElement::onDeallocated(void)
{
	delete myEnableStatusCalllback;
	myEnableStatusCalllback = NULL;
}
/*****************************************************************************/
void UIMenuElement::postInit(void)
{
	initMenuFrom();
    UIElement::postInit();
}
/*****************************************************************************/
FLOAT_TYPE UIMenuElement::initMenuFrom(IMenuContentsProvider* pOptContentsProvider, FLOAT_TYPE fOptWidth, FLOAT_TYPE fOptMaxHeight, FLOAT_TYPE fOptMaxTextLength)
{
#ifdef _DEBUG
	if(IS_OF_TYPE("newNodeSubmenu"))
	{
		int bp = 0;
	}
#endif
	FLOAT_TYPE fTotalMenuSize = 0;
	killAllHighlights();
	myAllowPressing = false;
	const char* pcsFont = this->getStringProp(PropertyFont);
	int iFontSize = this->getNumProp(PropertyFontSize);

	bool bIsHorizontal = !this->getBoolProp(PropertyIsVertical);
	const FLOAT_TYPE fHorSpacing = getCommonHorOffset();
	//const FLOAT_TYPE fInitVertOffset = upToScreen(20.0);
	///const FLOAT_TYPE fVertSpacing = upToScreen(12.5);

	// Hor spacing after the icon
	const FLOAT_TYPE fHorIconSpacing = -upToScreen(1.0);

	const FLOAT_TYPE fSeparatorThickness = upToScreen(2.0);
	const FLOAT_TYPE fMinSepLength = upToScreen(10.0);

	SVector2D svSize;
	svSize.x = this->getNumProp(PropertyWidth);
	svSize.y = this->getNumProp(PropertyHeight);
	//this->getBoxSize(svSize);

	FLOAT_TYPE fMaxWidth = fMinSepLength;

	bool bHaveSubmenus = false;

	myParentMenuId = "";
	myPrevSelectedEntry = -1;

	myNoIconSpacing = upToScreen(6.0);
	if(doesPropertyExist(PropertyMenuLeftNoIconPadding))
		myNoIconSpacing = this->getNumProp(PropertyMenuLeftNoIconPadding);

	myIconScale = 1.0;
	if(doesPropertyExist(PropertyMenuIconScale))
		myIconScale = getNumProp(PropertyMenuIconScale);

	string strTemp;

	// Get the entries
	bool bIsSeparator;
	SVector2D svDimsOut;
	SVector2D svIconDims;
	SVector2D svPrevPosOffset(fHorSpacing, 0.0);

	SVector2D svInitOffsets;
	if(this->doesPropertyExist(PropertyMenuInitOffsets))
	{
		svInitOffsets.x = this->getNumericEnumPropValue(PropertyMenuInitOffsets, 0);
		svInitOffsets.y = this->getNumericEnumPropValue(PropertyMenuInitOffsets, 1);
	}
	else
	{
		if(bIsHorizontal)
		{
			svInitOffsets.x = HOR_MENU_INIT_OFFSET_X;
			svInitOffsets.y = HOR_MENU_INIT_OFFSET_Y;
		}
		else
		{
			svInitOffsets.x = VERT_MENU_INIT_OFFSET_X;
			svInitOffsets.y = VERT_MENU_INIT_OFFSET_Y;
		}
	}
	svPrevPosOffset += svInitOffsets;

	SVector2D svHighlightPadding;
	getHighlightPadding(svHighlightPadding);

	FLOAT_TYPE fCurrHorIconOffset, fMaxHorIconOffset = 0;
	// This prevents too narrow spacing between text start and left menu edge for menus with no icons
	if(!bIsHorizontal)
		fMaxHorIconOffset = myNoIconSpacing;

	int iLongestNumCharsInShortcut = 0;
	int iLongestNumCharsInText = 0;

	KeyManager* pKeyMon = KeyManager::getInstance();

	myEntries.clear();

	IMenuContentsProvider* pMenuContentsProvider = pOptContentsProvider;
	if(as<IMenuContentsProvider>(this))
		pMenuContentsProvider = as<IMenuContentsProvider>(this);
	if(!pMenuContentsProvider)
		pMenuContentsProvider = &myDefaultProvider;

	Window* pParentWindow = getParentWindow();
	bool bIsFullLicense = pParentWindow->getIsFullLicense();
	bool bIsAppStoreBuild = false;
#ifdef APPSTORE_BUILD
	bIsAppStoreBuild = true;
#endif

	const char* pcsCheckmarkAnim = this->getStringProp(PropertyMenuCheckmarkImage);

	int iCurr, iNum = pMenuContentsProvider->getNumItems();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		SMenuEntry rEntry;
		rEntry.myId = pMenuContentsProvider->getItemId(iCurr);

#if !defined(_DEBUG) && !defined(_DEBUG)
		// Skip any debug submenus.
		if(rEntry.myId.length() > strlen(DEBUG_MENU_SUFFIX) && IS_STRING_EQUAL(rEntry.myId.c_str() + (rEntry.myId.length() - strlen(DEBUG_MENU_SUFFIX)), DEBUG_MENU_SUFFIX) )
			continue;
#endif

		// Also skip any menu items found only in non-full versions
		if((bIsFullLicense || bIsAppStoreBuild) && rEntry.myId.length() > strlen(NONFULLONLY_MENU_SUFFIX) && IS_STRING_EQUAL(rEntry.myId.c_str() + (rEntry.myId.length() - strlen(NONFULLONLY_MENU_SUFFIX)), NONFULLONLY_MENU_SUFFIX) )
			continue;

#ifdef APPSTORE_BUILD
		if(rEntry.myId.length() > strlen(NONAPPSTOREONLY_MENU_SUFFIX) && IS_STRING_EQUAL(rEntry.myId.c_str() + (rEntry.myId.length() - strlen(NONAPPSTOREONLY_MENU_SUFFIX)), NONAPPSTOREONLY_MENU_SUFFIX) )
			continue;
#endif

		// Skip any consequent separataors
		if(myEntries.size() > 0 && rEntry.myId == SEPARATOR_ID && myEntries[myEntries.size() - 1].myId == SEPARATOR_ID)
			continue;

		rEntry.mySubmenu = pMenuContentsProvider->getItemSubmenu(iCurr);
		rEntry.myLabel = pMenuContentsProvider->getItemLabel(iCurr);
		rEntry.myAction = pMenuContentsProvider->getItemAction(iCurr);
		rEntry.myActionValue = pMenuContentsProvider->getItemActionValue(iCurr);
		rEntry.myActionValue2 = pMenuContentsProvider->getItemActionValue2(iCurr);
		rEntry.myActionValue3 = pMenuContentsProvider->getItemActionValue3(iCurr);
		rEntry.myIcon = pMenuContentsProvider->getItemIcon(iCurr);
		rEntry.myIsCheckmark = pMenuContentsProvider->getIsItemCheckmark(iCurr);
		rEntry.myMacTag = -1;

		if(fOptMaxTextLength > 0)
			RenderUtils::shortenStringFromTheEnd(getParentWindow(), rEntry.myLabel, this->getCachedFont(), this->getCachedFontSize(), fOptMaxTextLength, "...", true);

		if(rEntry.mySubmenu == PROPERTY_NONE)
			rEntry.mySubmenu = "";
		if(rEntry.myLabel == PROPERTY_NONE)
			rEntry.myLabel = "";
		if(rEntry.myActionValue == PROPERTY_NONE)
			rEntry.myActionValue = "";
		if(rEntry.myActionValue2 == PROPERTY_NONE)
			rEntry.myActionValue2 = "";
		if(rEntry.myActionValue3 == PROPERTY_NONE)
			rEntry.myActionValue3 = "";
		if(rEntry.myIcon == PROPERTY_NONE)
			rEntry.myIcon = "";

		rEntry.myIsEnabled = true;
		rEntry.myIsChecked = false;

#ifdef _DEBUG
		if(rEntry.myAction == "uiaToggleMaximizeWorkspaceMode")
		{
			int bp = 0;
		}
#endif
		pKeyMon->getShortcutTextByUiAction(rEntry.myAction.c_str(), rEntry.myActionValue.c_str(), rEntry.myActionValue3.c_str(), rEntry.myShortcut);
		if(rEntry.myShortcut.length() > iLongestNumCharsInShortcut)
			iLongestNumCharsInShortcut = rEntry.myShortcut.length();

		if(rEntry.myLabel.length() > iLongestNumCharsInText)
			iLongestNumCharsInText = rEntry.myLabel.length();

		if(!bHaveSubmenus && rEntry.mySubmenu.length() > 0)
			bHaveSubmenus = true;

		bIsSeparator = (rEntry.myId == SEPARATOR_ID);

		svDimsOut.set(0, 0);
		svIconDims.set(0, 0);
		if(bIsSeparator)
		{
			rEntry.myTextX = 0;
			rEntry.myIconX = 0;
			if(bIsHorizontal)
			{
				rEntry.myTotalSize.x = fSeparatorThickness;
				rEntry.myTotalSize.y = svSize.y - SEP_END_PADDING*2;
			}
			else
			{
				// TBD later.
				rEntry.myTotalSize.x = -1;
				rEntry.myTotalSize.y = fSeparatorThickness;
			}
		}
		else
		{	
			if(rEntry.myLabel.length() > 0 && rEntry.myLabel != PROPERTY_NONE)
				RenderUtils::measureText(getParentWindow(), rEntry.myLabel.c_str(), pcsFont, iFontSize, 0, svDimsOut);

			if( (rEntry.myIcon.length() > 0 && rEntry.myIcon != PROPERTY_NONE) || rEntry.myIsCheckmark)
			{
				string* pFinalPtr;
				if(rEntry.myIsCheckmark)
				{
					theCommonString = pcsCheckmarkAnim;
					pFinalPtr = &theCommonString;
				}
				else
					pFinalPtr = &rEntry.myIcon;

				_ASSERT(pFinalPtr->length() > 0);

				int iColonPos = pFinalPtr->find(":");
				if(iColonPos != string::npos)
				{
					char *pTemp = const_cast<char*>(pFinalPtr->c_str());
					pTemp[iColonPos] = 0;
				}

				svIconDims.x = getTextureManager()->getWidth(pFinalPtr->c_str());
				svIconDims.y = getTextureManager()->getHeight(pFinalPtr->c_str());

				if(iColonPos != string::npos)
				{
					char *pTemp = const_cast<char*>(pFinalPtr->c_str());
					pTemp[iColonPos] = ':';
				}
			}
			


			rEntry.myTotalSize.x = svIconDims.x + svDimsOut.x;
			if(svIconDims.x > 0)
				rEntry.myTotalSize.x += fHorIconSpacing;

			rEntry.myTotalSize.y = max(svIconDims.y, svDimsOut.y);

			if(bIsHorizontal && svSize.y > rEntry.myTotalSize.y)
				rEntry.myTotalSize.y = svSize.y;
			else if(!bIsHorizontal)
				rEntry.myTotalSize.y = VERT_MENU_ITEM_SPACING_Y;

			if(rEntry.myTotalSize.x > fMaxWidth)
				fMaxWidth = rEntry.myTotalSize.x;

			fCurrHorIconOffset = svIconDims.x;

			rEntry.myIconX = svIconDims.x/2.0;
			rEntry.myTextX = svIconDims.x;

			if(rEntry.myTextX > 0)
			{
				rEntry.myTextX += fHorIconSpacing;
				fCurrHorIconOffset += fHorIconSpacing;
			}

			if(fCurrHorIconOffset > fMaxHorIconOffset)
				fMaxHorIconOffset = fCurrHorIconOffset;
		}

		if(bIsHorizontal)
		{
			rEntry.myPosOffset.x = svPrevPosOffset.x;
			rEntry.myPosOffset.y = svPrevPosOffset.y;

			svPrevPosOffset.x += fHorSpacing + rEntry.myTotalSize.x;
		}
		else
		{
			rEntry.myPosOffset.x = svPrevPosOffset.x;
			rEntry.myPosOffset.y = svPrevPosOffset.y;
			svPrevPosOffset.y += VERT_MENU_ITEM_SPACING_Y;
			if(bIsSeparator)
			{
				rEntry.myPosOffset.y += HOR_SEP_THICKNESS_ADJUSTMENT/2.0;
				svPrevPosOffset.y += HOR_SEP_THICKNESS_ADJUSTMENT;
			}
		}

		myEntries.push_back(rEntry);
	}

	// See if we have any shortcuts, and estimate their length
	FLOAT_TYPE fEstShortcutLength = iLongestNumCharsInShortcut*SHORTCUT_EST_KEY_SIZE + SHORTCUT_HOR_OFFSET;
	FLOAT_TYPE fEstTotalLength = fEstShortcutLength + iLongestNumCharsInText*SHORTCUT_EST_KEY_SIZE + fMaxHorIconOffset;

	if(!bIsHorizontal)
	{
		FLOAT_TYPE fVertSubmenuArrowPadding = 0;
		if(bHaveSubmenus)
			fVertSubmenuArrowPadding = fHorSpacing; // SUBMENU_ARROW_PADDING;

		if(fMaxWidth < fEstTotalLength)
			fMaxWidth = fEstTotalLength;

		// Size everything to our contents.
		SVector2D svNewSize(fMaxWidth + fHorSpacing*2.0 + fVertSubmenuArrowPadding, svPrevPosOffset.y + VERT_MENU_BOTTOM_PADDING);
		// Hack to avoid gaps in selection...
		if(((int)svNewSize.x) % 2 == 0)
			svNewSize.x += 1.0;

		if(fOptWidth > 0)
			svNewSize.x = fOptWidth;

		_ASSERT(svNewSize.x > 0 && svNewSize.y > 0);
		this->setNumProp(PropertyWidth, svNewSize.x);
		if(fOptMaxHeight > 0 && svNewSize.y > fOptMaxHeight)
		{
			this->setNumProp(PropertyHeight, fOptMaxHeight);
			///bDoesFitCompletly = (svNewSize.y <= fOptMaxHeight);
		}
		else
			this->setNumProp(PropertyHeight, svNewSize.y); // - fVertSpacing + fInitVertOffset);
		this->resetEvalCache(false);

		fTotalMenuSize = svNewSize.y;
	}
	else
	{
		// See if we need to stretch
		FLOAT_TYPE fNewWidth = svPrevPosOffset.x + svInitOffsets.x;
		if(fNewWidth > svSize.x || this->getBoolProp(PropertyMenuAllowShrinking))
		{
			this->setNumProp(PropertyWidth, fNewWidth);
			this->resetEvalCache(false);
		}
		fTotalMenuSize = fNewWidth;
	}


	// Set any entries with negative size to be offset from max size.
	// They're likely separators.
	FLOAT_TYPE fNewSize = this->getNumProp(PropertyWidth);
	SMenuEntry *pItem;
	iNum = myEntries.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pItem = &myEntries[iCurr];
		if(pItem->myTotalSize.x < 0)
		{
			pItem->myTotalSize.x = fMaxWidth - SEP_END_PADDING*2.0;
			if(!bIsHorizontal)
				pItem->myPosOffset.x = fHorSpacing;
		}
		else if(!bIsHorizontal && bHaveSubmenus)
			pItem->myTotalSize.x = fNewSize + svHighlightPadding.x*2.0;
		else if(!bIsHorizontal && fMaxHorIconOffset > 0 && pItem->myIcon.length() == 0 && !pItem->myIsCheckmark)
			pItem->myTotalSize.x += fMaxHorIconOffset;

		if(!bIsHorizontal && fMaxHorIconOffset > 0 && pItem->myLabel.length() > 0 && pItem->myIcon.length() == 0 && !pItem->myIsCheckmark)
			pItem->myTextX += fMaxHorIconOffset;

		if(!bIsHorizontal && pItem->myLabel.length() > 0 && iLongestNumCharsInShortcut > 0)
			pItem->myTotalSize.x = fNewSize + VERT_MENU_HIGHLIGHT_HOR_PADDING*2.0;
	}

	myIsDirty = false;

	if(getUIPlane()->getParentWindow()->getDidFinishPrelimInit())
		this->onMenuChanged();

	return fTotalMenuSize;
}
/*****************************************************************************/
void UIMenuElement::postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
    SVector2D svPos;
    FLOAT_TYPE fFinalOpac, fLocScale;
    this->getLocalPosition(svPos, &fFinalOpac, &fLocScale);
    fFinalOpac *= fOpacity;
    SVector2D svSize;
    this->getBoxSize(svSize);
    svPos += svScroll;
    svSize *= fLocScale*fScale;
    if(this->getParent())
	svPos *= fScale;

    svPos -= svSize*0.5;

 	SVector2D svExtraShiftOffset;
 	this->getExtraShiftOffset(svExtraShiftOffset);
	svPos.y += svExtraShiftOffset.y;

	bool bDoScissorRect = getDoScissorRect();
	if(bDoScissorRect)
	{
		SVector2D svSizeOffset, svShiftOffset;
		getAutoScrollingRectangleSizeAndShiftOffsets(svSizeOffset, svShiftOffset);

		SRect2D srGlobalRect;
		getGlobalRectangle(srGlobalRect);
		RenderUtils::beginScissorRectangle(srGlobalRect.x + svShiftOffset.x, srGlobalRect.y + svShiftOffset.y, srGlobalRect.w + svSizeOffset.x, srGlobalRect.h + svSizeOffset.y, getDrawingCache());
	}

    const char* pcsFont = this->getCachedFont();
    if(!pcsFont || strlen(pcsFont) == 0)
		pcsFont = MAIN_FONT_MED;
	int iFontSize = this->getCachedFontSize();
// 	if(iFontSize <= 0)
// 		iFontSize = MAIN_FONT_MED_DEF_SIZE;

	SColor scolShortcutText(0.475, 0.475, 0.475, 1.0);
    SColor scolText;
    this->getAsColor(PropertyTextColor, scolText);
	SColor scolSelText;
	if(doesPropertyExist(PropertySelectedTextColor))
		this->getAsColor(PropertySelectedTextColor, scolSelText);

    SColor *pShadowColor = NULL;
    SColor scolActualShadowColor;
    if(this->doesPropertyExist(PropertyTextShadowColor))
    {
		this->getAsColor(PropertyTextShadowColor, scolActualShadowColor);
		scolActualShadowColor.alpha *= fFinalOpac;
		pShadowColor = &scolActualShadowColor;
    }

	SVector2D svTextOffset;
	svTextOffset.x = this->getNumProp(PropertyTextOffsetX);
	svTextOffset.y = this->getNumProp(PropertyTextOffsetY);

	// Take into account the percentage offset
	svTextOffset.x += this->getNumProp(PropertyTextPercOffsetX)*svSize.x;
	svTextOffset.y += this->getNumProp(PropertyTextPercOffsetY)*svSize.y;

	SColor scolTextFinal;
	SColor *pFinalTextColPtr;
	SColor* pFinalShadowColor;
    SVector2D svShadowOffset;
    getCachedShadowOffset(svShadowOffset);

    scolText.alpha *= fFinalOpac;

	GTIME lTime = Application::getInstance()->getGlobalTime(getClockType());

    int iSepHorW, iSepHorH;
    getTextureManager()->getTextureRealDims(SEPARATOR_HOR_ANIM, iSepHorW, iSepHorH);
    int iSepVertW, iSepVertH;
    getTextureManager()->getTextureRealDims(SEPARATOR_VERT_ANIM, iSepVertW, iSepVertH);

	FLOAT_TYPE fHighlightOpacity;
    bool bIsHorizontal = !this->getBoolProp(PropertyIsVertical);

    int iCornerW, iCornerH;
    getTextureManager()->getCornerSize(MENU_HIGHLIGHT_ANIM, iCornerW, iCornerH);

	scolTextFinal = scolText;
	bool bIsItemEnabled = true;

	FLOAT_TYPE fEnableOpMult;

	SVector2D svHighlightPadding;
	getHighlightPadding(svHighlightPadding);

	SVector2D svHighlightOffset;
	getHighlightOffset(svHighlightOffset);

	const char* pcsCheckmarkAnim = this->getStringProp(PropertyMenuCheckmarkImage);

    // Render out items
    const char* pcsSepAnim;
    FLOAT_TYPE fSepHorOffset;
    FLOAT_TYPE fSepScaleX, fSepScaleY;
    SMenuEntry* pEntry;
    SVector2D svHighlightSize;
    int iCurr, iNum = myEntries.size();
    for(iCurr = 0; iCurr < iNum; iCurr++)
    {
		pEntry = &myEntries[iCurr];
		bIsItemEnabled = pEntry->myIsEnabled;

		if(pEntry->myId ==PROPERTY_NONE)
			continue;

		if(pEntry->myId == SEPARATOR_ID)
		{
			// Draw separator bitmap
			if(bIsHorizontal)
			{
				fSepScaleX = 1.0;
				fSepScaleY = pEntry->myTotalSize.y/(FLOAT_TYPE)iSepVertH*fScale;
				pcsSepAnim = SEPARATOR_VERT_ANIM;
				fSepHorOffset = 0;
			}
			else
			{
				fSepScaleX = pEntry->myTotalSize.x/(FLOAT_TYPE)iSepHorW*fScale;
				fSepScaleY = 1.0;
				pcsSepAnim = SEPARATOR_HOR_ANIM;
				fSepHorOffset = SEP_END_PADDING;
			}
			getDrawingCache()->addSprite(pcsSepAnim, svPos.x + pEntry->myPosOffset.x + pEntry->myTotalSize.x/2.0 + fSepHorOffset, svPos.y + pEntry->myPosOffset.y, fFinalOpac, 0, fSepScaleX, fScale, 1.0, true);
		}
		else
		{
			if(bIsItemEnabled)
				fEnableOpMult = 1.0;
			else
				fEnableOpMult = MENU_ICON_DISABLED_OPACITY_MULT;

			if(pEntry->myIcon.length() > 0)
				getDrawingCache()->addSprite(pEntry->myIcon.c_str(), (int)(svPos.x + pEntry->myPosOffset.x + pEntry->myIconX), (int)(svPos.y + pEntry->myPosOffset.y), fFinalOpac*fEnableOpMult, 0, fScale*myIconScale, 1.0, true);

			if(pEntry->myIsCheckmark && pEntry->myIsChecked)
			{
				theCommonString = pcsCheckmarkAnim;
				getDrawingCache()->addSprite(theCommonString.c_str(), (int)(svPos.x + pEntry->myPosOffset.x + pEntry->myIconX), (int)(svPos.y + pEntry->myPosOffset.y), fFinalOpac*fEnableOpMult, 0, fScale*myIconScale, 1.0, true);
			}
	        
			fHighlightOpacity = pEntry->myHighlightAnim.getValue();
			if(fHighlightOpacity > FLOAT_EPSILON && bIsItemEnabled)
				pFinalShadowColor = NULL;
			else
				pFinalShadowColor = pShadowColor;

			if(pEntry->myLabel.length() > 0)
			{	
				scolTextFinal.alpha = scolText.alpha;
				if(!bIsItemEnabled)
					scolTextFinal.alpha *= MENU_DISABLED_OPACITY_MULT;

				scolSelText.alpha = scolText.alpha;
				if(myPrevSelectedEntry == iCurr)
					pFinalTextColPtr = &scolSelText;
				else
					pFinalTextColPtr = &scolTextFinal;

				getDrawingCache()->addText(pEntry->myLabel.c_str(), pcsFont, iFontSize, svPos.x + pEntry->myPosOffset.x + pEntry->myTextX + svTextOffset.x, svPos.y + pEntry->myPosOffset.y + svTextOffset.y, 
						*pFinalTextColPtr, HorAlignLeft, VertAlignCenter,  0, NULL, pFinalShadowColor, &svShadowOffset, 0, fScale);
			}

			if(pEntry->myShortcut.length() > 0)
			{
				scolShortcutText.alpha = fFinalOpac;
				if(!bIsItemEnabled)
					scolShortcutText.alpha *= MENU_DISABLED_OPACITY_MULT;
				getDrawingCache()->addText(pEntry->myShortcut.c_str(), pcsFont, iFontSize, svPos.x + svSize.x - getCommonHorOffset(), svPos.y + pEntry->myPosOffset.y + svTextOffset.y, 
					scolShortcutText, HorAlignRight, VertAlignCenter,  0, NULL, pFinalShadowColor, &svShadowOffset, 0, fScale);
			}

			if(!bIsHorizontal && pEntry->mySubmenu.length() > 0)
				getDrawingCache()->addSprite("menuArrow", svPos.x + svSize.x - getCommonHorOffset() - upToScreen(2.0), svPos.y + pEntry->myPosOffset.y, fFinalOpac*0.8, 0, fScale, 1.0, true);

			if(fHighlightOpacity > FLOAT_EPSILON && bIsItemEnabled)
			{
				// Highlight it. We need to figure out the right size of the highlight.
				FLOAT_TYPE fPosX = svPos.x + pEntry->myPosOffset.x + pEntry->myTotalSize.x/2.0;
				FLOAT_TYPE fPosY = svPos.y + pEntry->myPosOffset.y;
				svHighlightSize = pEntry->myTotalSize;
				if(bIsHorizontal)
				{
					svHighlightSize.x += svHighlightPadding.x*2.0;
					svHighlightSize.y += svHighlightPadding.y*2.0;
					fPosX += svHighlightOffset.x;
					fPosY += svHighlightOffset.y;
				}
				else
				{
					svHighlightSize.x = svSize.x + svHighlightPadding.x*2.0;
					svHighlightSize.y += svHighlightPadding.y*2.0;

					fPosX = svPos.x + svSize.x/2.0;

					fPosX += svHighlightOffset.x;
					fPosY += svHighlightOffset.y;

				}

				// Hacks to avoid single pixel lines when fading in
				if(((int)svHighlightSize.x) % 2 == 1)
					svHighlightSize.x += 1.0;
				if(!bIsHorizontal)
					fPosX += 1.0;

				getDrawingCache()->addScalableButton(MENU_HIGHLIGHT_ANIM, (int)fPosX, 
					fPosY, 
					(int)svHighlightSize.x, svHighlightSize.y, 
					fFinalOpac*fHighlightOpacity, iCornerW, iCornerH);
			}
		}
    }

	if(bDoScissorRect)
		RenderUtils::endScissorRectangle(getDrawingCache());

    UIElement::postRender(svScroll, fOpacity, fScale);
}
/*****************************************************************************/
void UIMenuElement::onPressed(TTouchVector& vecTouches)
{
    UIElement::onPressed(vecTouches);

    if(getIsEnabled())
    {
	myAllowPressing = true;
//	this->setPushed(true);
    }
}
/*****************************************************************************/
void UIMenuElement::onMouseEnter(TTouchVector& vecTouches)
{
    UIElement::onMouseEnter(vecTouches);	
}
/*****************************************************************************/
void UIMenuElement::onMouseLeave(TTouchVector& vecTouches)
{
    UIElement::onMouseLeave(vecTouches);	
    myAllowPressing = false;

	killAllHighlights();

//     if(!(this->getIsRadio() && this->getIsPushed()))
// 	if(getIsEnabled())
// 	    this->setPushed(false);
}
/*****************************************************************************/
void UIMenuElement::onCursorGlideLeave(TTouchVector& vecTouches)
{
	killAllHighlights();
}
/*****************************************************************************/
void UIMenuElement::onCursorGlide(TTouchVector& vecTouches)
{
    UIElement::onCursorGlide(vecTouches);
    if(vecTouches.size() > 0 && !this->getIsHiding())
    {
		// See if we're over an item
		int iNewHighlightEntry = getEntryAt(vecTouches[0].myPoint.x, vecTouches[0].myPoint.y);	
		if(iNewHighlightEntry != myHighlightEntry && myHighlightEntry >= 0)
			myEntries[myHighlightEntry].deselect(false);
		if(iNewHighlightEntry >= 0)
			myEntries[iNewHighlightEntry].select();

		if(myHighlightEntry != iNewHighlightEntry && iNewHighlightEntry >= 0 && getTopLevelMenuParent()->getDoShowChildrenOnGlide())
			openSubmenu(&myEntries[iNewHighlightEntry], true, false);

		myHighlightEntry = iNewHighlightEntry;
    }
}
/*****************************************************************************/
void UIMenuElement::onReleased(TTouchVector& vecTouches, bool bIgnoreActions)
{
    UIElement::onReleased(vecTouches, bIgnoreActions);

    if(vecTouches.size() > 0)
    {
		SRect2D srExcludingRect;
		getSelectionClickExclusionRectangle(srExcludingRect);
		int iEntry = getEntryAt(vecTouches[0].myPoint.x, vecTouches[0].myPoint.y, &srExcludingRect);
		if(iEntry >= 0)
		{
			SMenuEntry *pEntry = &myEntries[iEntry];

			// We've hit something. See what.
			if(pEntry->mySubmenu.length() > 0)
			{
				if(openSubmenu(pEntry, false, true))
					getTopLevelMenuParent()->setDoShowChildrenOnGlide(true);
				else
					getTopLevelMenuParent()->setDoShowChildrenOnGlide(false);
			}
			else if(pEntry->myAction.length() > 0 && pEntry->myIsEnabled)
			{
				getTopLevelMenuParent()->setDoShowChildrenOnGlide(false);

				// Do it!
				this->executEntry(pEntry);

				// Kill the highlight on self and all predecessors
				this->killHighlightRecursiveUp();		
			}
			else
			{
				getTopLevelMenuParent()->setDoShowChildrenOnGlide(false);
				customMenuItemAction(pEntry->myId.c_str());
				this->killHighlightRecursiveUp();
			}

			// If it's a checkmark, refresh our check status:
			if(pEntry->myIsCheckmark)
				refreshEnableStatusFor(pEntry);
		}
    }

    myAllowPressing = false;
}
/*****************************************************************************/
void UIMenuElement::executEntry(const SMenuEntry* pEntry)
{
	UIPlane* pPlane = getUIPlane();
	myDummyButton.setStringProp(PropertyAction, pEntry->myAction.c_str());
	myDummyButton.setStringProp(PropertyImage, pEntry->myIcon.c_str());
	if(pEntry->myActionValue.length() > 0)
		myDummyButton.setStringProp(PropertyActionValue, pEntry->myActionValue.c_str());
	if(pEntry->myActionValue2.length() > 0)
		myDummyButton.setStringProp(PropertyActionValue2, pEntry->myActionValue2.c_str());
	if(pEntry->myActionValue3.length() > 0)
		myDummyButton.setStringProp(PropertyActionValue3, pEntry->myActionValue3.c_str());
	if(this->getHaveGenericDataSource())
		myDummyButton.setGenericDataSource(this->getGenericDataSource());
	if(!this->getHaveGenericDataSource() && this->doesPropertyExist(PropertyTargetDataSource))
	{
		this->getAsString(PropertyTargetDataSource, theCommonString3);
		myDummyButton.setGenericDataSource(theCommonString3.c_str());
	}
	pPlane->onButtonClicked(&myDummyButton, MouseButtonLeft);
}
/*****************************************************************************/
void UIMenuElement::setEntryTag(int iEntryIndex, int iTag)
{
	if(iEntryIndex >= 0 && iEntryIndex < myEntries.size())
		myEntries[iEntryIndex].myMacTag = iTag;
	ELSE_ASSERT;	
}
/*****************************************************************************/
int UIMenuElement::findSubmenuIndex(const char* pcsId)
{
	if(!IS_VALID_STRING_AND_NOT_NONE(pcsId))
		return -1;

	int iCurr, iNum = myEntries.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(myEntries[iCurr].mySubmenu == pcsId)
			return iCurr;
	}

	return -1;
}
/*****************************************************************************/
UIMenuElement* UIMenuElement::getParentMenuElem()
{
	// Note that it is possible that there are several parents that exist. We return 
	// the first one.
	UIPlane* pPlane = getUIPlane();
	TUIElementSet vecMenuElems;
	pPlane->getAllElementsOfType<UIMenuElement>(vecMenuElems);

	// Should this be the old, if it exists?
	const char* pcsOwnId = this->getStringProp(PropertyId);

	TUIElementSet::iterator si;
	for(si = vecMenuElems.begin(); si != vecMenuElems.end(); si++)
	{
		if(as<UIMenuElement>((*si))->findSubmenuIndex(pcsOwnId) >= 0)
			return as<UIMenuElement>((*si));
	}
	return NULL;
}
/*****************************************************************************/
void UIMenuElement::removeItem(int iIndex)
{
	int iNumIds = this->getEnumPropCount(PropertyMenuIds);
	if(iIndex < 0 || iIndex >= iNumIds)
		ASSERT_RETURN;

	this->removeEnumValue(PropertyMenuIds, iIndex);
	if(this->doesPropertyExist(PropertyMenuLabels))
		this->removeEnumValue(PropertyMenuLabels, iIndex);
	if(this->doesPropertyExist(PropertyMenuActions))
		this->removeEnumValue(PropertyMenuActions, iIndex);
	if(this->doesPropertyExist(PropertyMenuActionValues))
		this->removeEnumValue(PropertyMenuActionValues, iIndex);
	if(this->doesPropertyExist(PropertyMenuActionValues2))
		this->removeEnumValue(PropertyMenuActionValues2, iIndex);
	if(this->doesPropertyExist(PropertyMenuActionValues3))
		this->removeEnumValue(PropertyMenuActionValues3, iIndex);
	if(this->doesPropertyExist(PropertyMenuIcons))
		this->removeEnumValue(PropertyMenuIcons, iIndex);
	if(this->doesPropertyExist(PropertyMenuSubitems))
		this->removeEnumValue(PropertyMenuSubitems, iIndex);
	if(this->doesPropertyExist(PropertyMenuIsCheckmark))
		this->removeEnumValue(PropertyMenuIsCheckmark, iIndex);

	myEntries.erase(myEntries.begin() + iIndex);	

	if(getUIPlane()->getParentWindow()->getDidFinishPrelimInit())
		this->onMenuChanged();
}
/*****************************************************************************/
int UIMenuElement::getEntryAt(FLOAT_TYPE fX, FLOAT_TYPE fY, const SRect2D* pOptExclusionRect)
{
    SVector2D svPos;
    FLOAT_TYPE fGlobalScale;
    this->getGlobalPosition(svPos, NULL, &fGlobalScale);
    SVector2D svOwnSize;
    this->getBoxSize(svOwnSize);
    svOwnSize *= fGlobalScale;
    svPos -= svOwnSize*0.5;

	SVector2D svExtraShiftOffset;
	this->getExtraShiftOffset(svExtraShiftOffset);
	svPos.y += svExtraShiftOffset.y;

	bool bIsHorizontal = !this->getBoolProp(PropertyIsVertical);

    SVector2D svScreenPos(fX, fY);

	if(pOptExclusionRect && pOptExclusionRect->w > 0 && pOptExclusionRect->h > 0 && pOptExclusionRect->doesContain(svScreenPos))
		return -1;

    SVector2D svCenter;
    SRect2D srCurrRect;
    SMenuEntry* pEntry;
    int iCurr, iNum = myEntries.size();
    for(iCurr = 0; iCurr < iNum; iCurr++)
    {
		pEntry = &myEntries[iCurr];
		if(pEntry->myId == SEPARATOR_ID)
			continue;
		
		srCurrRect.h = pEntry->myTotalSize.y;

		if(bIsHorizontal)
			srCurrRect.w = pEntry->myTotalSize.x;
		else
			srCurrRect.w = svOwnSize.x;

		srCurrRect.x = pEntry->myPosOffset.x + svPos.x;
		srCurrRect.y = pEntry->myPosOffset.y + svPos.y - srCurrRect.h/2;

		if(srCurrRect.doesContain(svScreenPos))
			return iCurr;
    }

    return -1;
}
/*****************************************************************************/
void UIMenuElement::setupAsSubmenu(UIMenuElement* pParentMenu, SMenuEntry* pEntry)
{
	// Make sure we refresh it in case we have dynamic elements that affect size.
	this->updateDataRecursive();

    SVector2D svPos;
    FLOAT_TYPE fGlobalScale;
    pParentMenu->getGlobalPosition(svPos, NULL, &fGlobalScale);
    SVector2D svParentSize;
    pParentMenu->getBoxSize(svParentSize);
    svParentSize *= fGlobalScale;
    svPos -= svParentSize*0.5;

    bool bIsParentHorizontal = !pParentMenu->getBoolProp(PropertyIsVertical);

	myParentMenuId = pParentMenu->getStringProp(PropertyId);

    SRect2D srCurrRect;
    srCurrRect.w = pEntry->myTotalSize.x;
    srCurrRect.h = pEntry->myTotalSize.y;
    srCurrRect.x = pEntry->myPosOffset.x + svPos.x;
    srCurrRect.y = pEntry->myPosOffset.y + svPos.y - srCurrRect.h/2;

    // So now we have our parent rectangle.
    // We need to set our position
    SVector2D svOwnSize;
    this->getBoxSize(svOwnSize);
    
	SVector2D svScreenDims;
	getParentWindow()->getSize(svScreenDims);

    SVector2D svNewPos;
    if(bIsParentHorizontal)
    {
		// Set us up under the box
		svNewPos.x = srCurrRect.x + srCurrRect.w/2.0 + svOwnSize.x/2.0 + HOR_SUBMENU_POS_OFFSET_X;
		svNewPos.y = srCurrRect.y + srCurrRect.h + svOwnSize.y/2.0 + HOR_SUBMENU_POS_OFFSET_Y;
    }
    else
    {
		// Set up to the right
		svNewPos.x = srCurrRect.x + srCurrRect.w + svOwnSize.x/2.0 + VERT_SUBMENU_POS_OFFSET_X;
		svNewPos.y = srCurrRect.y + svOwnSize.y/2.0 + VERT_SUBMENU_POS_OFFSET_Y;

		// See if we're outside of the screen and do the left thingie.
		if(svNewPos.x + svOwnSize.x/2.0 > svScreenDims.x)
			svNewPos.x = srCurrRect.x - svOwnSize.x/2.0;

		// Same for bottom
		if(svNewPos.y + svOwnSize.y/2.0 > svScreenDims.y)
			svNewPos.y = svScreenDims.y - svOwnSize.y/2.0;
    }

    killAllHighlights();

	setMenuPosition(svNewPos);
}
/*****************************************************************************/
void UIMenuElement::setMenuPosition(SVector2D& svPos)
{
	// Our submenus are global, and so we need to subract screen center now:
	// But only if they are global. They're not for dropdowns.
	SVector2D svScreenSize;
	if(!this->getParent())
		getParentWindow()->getSize(svScreenSize);

	this->setNumProp(PropertyX, svPos.x - svScreenSize.x/2.0);
	this->setNumProp(PropertyY, svPos.y - svScreenSize.y/2.0);
	this->resetEvalCache(true);
}
/*****************************************************************************/
void UIMenuElement::killAllHighlights()
{
	myHighlightEntry = -1;

	UIElement* pSrcElem = getTopmostParent<UIElement>();
	if(!pSrcElem)
		pSrcElem = this;

	bool bInstant = !pSrcElem->getIsBeingShown() && !pSrcElem->getIsFullyShown();

	GTIME lTime = Application::getInstance()->getGlobalTime(getClockType());
	FLOAT_TYPE fHighlightOpacity;
	int iItem, iNum = myEntries.size();
	for(iItem = 0; iItem < iNum; iItem++)
	{
		fHighlightOpacity = myEntries[iItem].myHighlightAnim.getValue();
		// The second clause is to prevent items that have just been set to animate to 1.0 but are currently at 0.0
		// from never being deselected.
		if(fHighlightOpacity > FLOAT_EPSILON || myEntries[iItem].myHighlightAnim.getEndValue() > 0.5)
			myEntries[iItem].deselect(bInstant);
	}
}
/*****************************************************************************/
void UIMenuElement::killHighlightRecursiveUp()
{
	killAllHighlights();

	UIMenuElement *pParent = getUIPlane()->getElementById<UIMenuElement>(myParentMenuId.c_str());
	if(pParent)
		pParent->killHighlightRecursiveUp();
}
/*****************************************************************************/
void UIMenuElement::getHierarchyChain(UIElement* pStartElem, TUIElementVector& rVecOut)
{
	rVecOut.clear();
	if(!pStartElem)
		return;

	UIElement* pExtra;
	UIMenuElement* pMenuElem = this, *pLastMenuElem;
	do 
	{
		_ASSERT(pMenuElem);
		rVecOut.push_back(pMenuElem);
		pExtra = pMenuElem->getExtraHidingException();
		if(pExtra)
			rVecOut.push_back(pExtra);

		pLastMenuElem = pMenuElem;
		pMenuElem = getUIPlane()->getElementById<UIMenuElement>(pMenuElem->getParentMenuId());

	} while (pMenuElem);

/*

	UIElement* pCurr = pStartElem;
	do 
	{
		rVecOut.push_back(pCurr);
		pCurr = pCurr->getParent<UIElement>();
	} while (pCurr);
*/
}
/*****************************************************************************/
UIElement* UIMenuElement::getSubmenuForName(const char* pcsSubmenuName)
{
	UIPlane* pPlane = getUIPlane();
	UIElement* pSubmenuElem = pPlane->getElementById<UIElement>(pcsSubmenuName);

	// Now, if we couldn't find it by type, assume the name token was a layer name and find the
	// first elem with that. We use that for new node menu in net editor, for example.
	if(!pSubmenuElem)
		pSubmenuElem = pPlane->getTopLevelElementByLayerName(pcsSubmenuName);

	return pSubmenuElem;
}
/*****************************************************************************/
bool UIMenuElement::openSubmenu(SMenuEntry* pEntry, bool bAlwaysCloseOthers, bool bCloseIfAlreadyOpen)
{
	// Find the element, get the layer from that.
	UIPlane* pPlane = getUIPlane();
	UIElement* pSubmenuElem = this->getSubmenuForName(pEntry->mySubmenu.c_str());
	if(pSubmenuElem)
	{
		// If it's already open, ignore it.
		if(pSubmenuElem->getIsBeingShown() || pSubmenuElem->getIsFullyShown())
		{
			// We actually want to close it...
			if(bCloseIfAlreadyOpen)
			{
				pPlane->hideUI(pSubmenuElem->getStringProp(PropertyLayer));
				// Prevent all menus from being closed
				pPlane->setHideAllMenus(false);
			}
			return false;
		}

		dynamic_cast<UIMenuElement*>(pSubmenuElem)->setupAsSubmenu(this, pEntry);

		getHierarchyChain(pSubmenuElem, theElemVector);
		pPlane->hideAllWithTag("submenu", &theElemVector, false, getUIPlane(), AnimOverActionGenericCallback);

		pPlane->showUI(pSubmenuElem->getStringProp(PropertyLayer), false, this->getGenericDataSource());
		pPlane->setHideAllMenus(false);

		return true;
	}
	else if(bAlwaysCloseOthers)
	{
		hideAllButSelfChain();
		return true;
	}

	_ASSERT(0);
	return true;
}
/*****************************************************************************/
UIMenuElement* UIMenuElement::getTopLevelMenuParent()
{
	UIMenuElement* pMenuElem = this, *pLastMenuElem;
	do 
	{
		pLastMenuElem = pMenuElem;
		pMenuElem = getUIPlane()->getElementById<UIMenuElement>(pMenuElem->getParentMenuId());

	} while (pMenuElem);

	return pLastMenuElem;
}
/*****************************************************************************/
FLOAT_TYPE UIMenuElement::getCommonHorOffset()
{
	if(this->doesPropertyExist(PropertyMenuItemSpacing))
		return this->getNumProp(PropertyMenuItemSpacing);
	else
		return COMMON_HOR_OFFSET_AND_SPACING;
}
/*****************************************************************************/
void UIMenuElement::getHighlightPadding(SVector2D& svHighlightPadding)
{
	bool bIsHorizontal = !this->getBoolProp(PropertyIsVertical);
	if(this->doesPropertyExist(PropertyMenuHighlightPadding))
	{
		svHighlightPadding.x = this->getNumericEnumPropValue(PropertyMenuHighlightPadding, 0);
		svHighlightPadding.y = this->getNumericEnumPropValue(PropertyMenuHighlightPadding, 1);
	}
	else
	{
		if(bIsHorizontal)
		{
			svHighlightPadding.x = HOR_MENU_HIGHLIGHT_HOR_PADDING;
			svHighlightPadding.y = HOR_MENU_HIGHLIGHT_VERT_PADDING;
		}
		else
		{
			svHighlightPadding.x = VERT_MENU_HIGHLIGHT_HOR_PADDING;
			svHighlightPadding.y = VERT_MENU_HIGHLIGHT_VERT_PADDING;
		}
	}
}
/*****************************************************************************/
void UIMenuElement::getHighlightOffset(SVector2D& svHighlightOffset)
{
	bool bIsHorizontal = !this->getBoolProp(PropertyIsVertical);
	if(this->doesPropertyExist(PropertyMenuHighlightOffset))
	{
		svHighlightOffset.x = this->getNumericEnumPropValue(PropertyMenuHighlightOffset, 0);
		svHighlightOffset.y = this->getNumericEnumPropValue(PropertyMenuHighlightOffset, 1);
	}
	else
	{
		if(bIsHorizontal)
		{
			svHighlightOffset.x = HOR_MENU_HIGHLIGHT_OFFSET_X;
			svHighlightOffset.y = HOR_MENU_HIGHLIGHT_OFFSET_Y;
		}
		else
		{
			svHighlightOffset.x = VERT_MENU_HIGHLIGHT_OFFSET_X;
			svHighlightOffset.y = VERT_MENU_HIGHLIGHT_OFFSET_Y;
		}
	}
}
/*****************************************************************************/
void UIMenuElement::onStartShowing(void)
{
#ifdef _DEBUG
	if(IS_OF_TYPE("toolVectorEditConvertSubmenu"))
	{
		int bp = 0;
	}
#endif
	UIElement::onStartShowing();

	if(myIsDirty)
		initMenuFrom();

	refreshMenuItemEnabledStatus(true);
	ensureShortcutsValid();
}
/*****************************************************************************/
void UIMenuElement::refreshMenuItemEnabledStatus(bool bForce)
{
#if !defined(ENABLE_NATIVE_OSX_MENU)
	if(!bForce)
	{
		// We don't even need a dirty flag, since 
		// we'll be refreshed by the on start showing
		// call...
		if(this->getIsFullyHidden())
			return;
	}
#endif

	// Find an item with callback
	const IMenuEnableStatusCallback* pCurrCallback = getEnableStatusCallback();

	SMenuEntry* pEntry;
	int iCurrEntry, iNum = myEntries.size();
	for(iCurrEntry = 0; iCurrEntry < iNum; iCurrEntry++)
	{
		pEntry = &myEntries[iCurrEntry];
		if(pCurrCallback)
		{
			pEntry->myIsEnabled = pCurrCallback->getIsMenuItemEnabled(pEntry, this->getGenericDataSource());
			if(pEntry->myIsCheckmark)
				pEntry->myIsChecked = pCurrCallback->getIsMenuItemChecked(pEntry, this->getGenericDataSource());
		}
		else
		{
			pEntry->myIsEnabled = true;
			pEntry->myIsChecked = false;
		}
	}

	if(getUIPlane()->getParentWindow()->getDidFinishPrelimInit())
		this->onMenuStatusRefreshed();
}
/*****************************************************************************/
void UIMenuElement::iterateOverMenuItems(IMenuVisitor& rVisitor)
{
	// Visit every item we have
	SMenuEntry* pEntry;
	UIMenuElement* pSubmenu;
	int iItem, iNumItems = myEntries.size();
	for(iItem = 0; iItem < iNumItems; iItem++)
	{
		pEntry = &myEntries[iItem];
		rVisitor.visit(pEntry, this, iItem);
		
		if(pEntry->mySubmenu.length() > 0)
		{
			// This is a submenu. Recurse.
			pSubmenu = as<UIMenuElement>(this->getSubmenuForName(pEntry->mySubmenu.c_str()));
			if(pSubmenu)
			{
				pSubmenu->iterateOverMenuItems(rVisitor);
			}
			ELSE_ASSERT;
		}
	}
}
/*****************************************************************************/
bool UIMenuElement::getDoShowChildrenOnGlide()
{
	// Since we currently only call this for top parent, we
	// can always show children on glide for any top parent
	// with vertical menus
	return  myDoShowChildrenOnGlide || this->getBoolProp(PropertyIsVertical);
}
/*****************************************************************************/
UIElement* UIMenuElement::getExtraHidingException()
{
	UIElement *pExtraException = NULL;
	if(myExtraHidingException.length() > 0)
		pExtraException = getUIPlane()->getElementById(myExtraHidingException.c_str(), true, true);
	return pExtraException;
}
/*****************************************************************************/
void UIMenuElement::ensureShortcutsValid()
{
	if(!myAreShortcutsDirty)
		return;

	// Go over all of our menu items and refresh shortcuts
	KeyManager* pKeyMon = KeyManager::getInstance();
	SMenuEntry *pEntry;
	int iCurr, iNum = myEntries.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pEntry = &myEntries[iCurr];
		pKeyMon->getShortcutTextByUiAction(pEntry->myAction.c_str(), pEntry->myActionValue.c_str(), pEntry->myActionValue3.c_str(), pEntry->myShortcut);
	}

	if(getUIPlane()->getParentWindow()->getDidFinishPrelimInit())
		this->onMenuShortcutsChanged();

	myAreShortcutsDirty = false;
}
/*****************************************************************************/
const IMenuEnableStatusCallback* UIMenuElement::getEnableStatusCallback()
{
	UIPlane* pUiPlane = getUIPlane();
	const IMenuEnableStatusCallback* pCurrCallback = myEnableStatusCalllback;

	if(!pCurrCallback)
		pCurrCallback = as<IMenuEnableStatusCallback>(this);

	UIMenuElement *pMenuElem = this;
	while(!pCurrCallback && pMenuElem && pUiPlane)
	{
		pMenuElem = pUiPlane->getElementById<UIMenuElement>(pMenuElem->getParentMenuId());
		if(pMenuElem)
			pCurrCallback = pMenuElem->getOwnedEnableStatusCallback();
	}

	if(!pCurrCallback)
	{
		// Try to get the part of this type:
		pCurrCallback = getParentOfType<IMenuEnableStatusCallback>();
	}

	return pCurrCallback;
}

/*****************************************************************************/
void UIMenuElement::refreshEnableStatusFor(SMenuEntry* pEntry)
{
	const IMenuEnableStatusCallback* pCurrCallback = getEnableStatusCallback();
	if(pCurrCallback)
	{
		pEntry->myIsEnabled = pCurrCallback->getIsMenuItemEnabled(pEntry, this->getGenericDataSource());
		if(pEntry->myIsCheckmark)
			pEntry->myIsChecked = pCurrCallback->getIsMenuItemChecked(pEntry, this->getGenericDataSource());
	}
	else
	{
		pEntry->myIsEnabled = true;
		pEntry->myIsChecked = false;
	}

	if(getUIPlane()->getParentWindow()->getDidFinishPrelimInit())
		this->onMenuStatusRefreshed();
}
/*****************************************************************************/
void UIMenuElement::onTimerTick(GTIME lGlobalTime)
{
	SMenuEntry *pEntry;
	UIElement::onTimerTick(lGlobalTime);
	int iCurr, iNum = myEntries.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pEntry = &myEntries[iCurr];
		//pEntry->myHighlightAnim.checkTime(lGlobalTime);
		pEntry->myHighlightAnim.getValue();
	}
}
/*****************************************************************************/
int UIMenuElement::getEntryById(const char* pcsId)
{
	if(!IS_VALID_STRING(pcsId))
		return -1;

	int iCurr, iNum = myEntries.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(myEntries[iCurr].myId == pcsId)
			return iCurr;
	}

	return -1;
}
/*****************************************************************************/
const SMenuEntry* UIMenuElement::getEntry(int iIndex) const
{
	if(iIndex >= 0 && iIndex < myEntries.size())
		return &myEntries[iIndex];
	else
		return NULL;
}
/*****************************************************************************/
void UIMenuElement::onFinishHiding()
{
	UIElement::onFinishHiding();
	killAllHighlights();
	gLog("UIMenuElement::onFinishHiding: on %s\n", this->getId());
}
/*****************************************************************************/
void UIMenuElement::hideAllButSelfChain()
{
	UIPlane* pPlane = getUIPlane();
	getHierarchyChain(this, theElemVector);
	pPlane->hideAllWithTag("submenu", &theElemVector, false, pPlane, AnimOverActionGenericCallback);
}
/*****************************************************************************/
};
