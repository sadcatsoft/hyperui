#include "stdafx.h"

#define DEFAULT_FLOAT_CONV_FORMAT		"%.1f"

#if !defined(WIN32) && !defined(LINUX)
//#import "SoundManager.h"
//extern SoundManager *g_pSoundManager;
#endif

#define DOUBLE_CLICK_MAX_TIME		(0.66)
#define DOUBLE_CLICK_MAX_DIST		upToScreen(1.5)

#define WINDOW_SLIDE_TIME				0.60  // 0.24

#define PIXEL_TOLERANCE					upToScreen(6) // 9

#define WINDOW_TITLE_HOR_PADDING			upToScreen(0.5)
#define WINDOW_TITLE_VERT_PADDING			upToScreen(0.5)
#define WINDOW_TITLE_ICON_HOR_PADDING		upToScreen(2.0)
#define WINDOW_TITLE_HOR_TEXT_PADDING		upToScreen(7.0)

// #define SLIDE_WITH_FADE

#define POPUP_SPACING					upToScreen(5)
#define POPUP_MIN_SCREEN_PADDING		upToScreen(-5)

namespace HyperUI
{

bool LinkedToSelfElemIterator::getDoesSatisfyConditions(UIElement* pItem) { return pItem->getLinkedToElement() == myLinkTarget; }

TCharPtrVector UIElement::thePathVector;
/*****************************************************************************/
UIElement::UIElement(UIPlane* pParentPlane)
	: IPlaneObject(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void UIElement::onAllocated(IBaseObject* pData)
{
	IPlane* pParentPlane = dynamic_cast<IPlane*>(pData);
	if(!pParentPlane)
	{
		IPlaneObject* pSrcChild = dynamic_cast<IPlaneObject*>(pData);
		if(pSrcChild)
		{
			ResourceItem* pTopParent = pSrcChild;
			while(pTopParent->getParent())
				pTopParent = pTopParent->getParent();

			IPlaneObject* pTopParentObj = dynamic_cast<IPlaneObject*>(pTopParent);
			if(pTopParentObj)
				pParentPlane = pTopParentObj->getParentPlane();
			ELSE_ASSERT;
		}
	}

	ResourceItem::onAllocated(pData);
	IPlaneObject::onAllocated(pParentPlane);

	myIsCachedTextValid = false;
	myCachedOwnDefinition = NULL;
	myCursorType = CursorInvalidUnset;
	myPendingAnimImage = NULL;
	myAccelImage = NULL;
	myGenericDataSource = NULL;
	myForceDrawUncachedText = false;
	myIsCallingChangeValue = false;
	myIsEnabled = true;
	myCachedOpacity = 0.0;
	myCachedScale = 1.0;
	myCachedPositionTime = 0;
	myCurrentSliderIndex = 0;
	myCurrentSlidePos = 0;
	myIsMousePressed = false;
	myIsRenderable = true;
	myIsAnimating = false;
	myHidingInProgress = false;
	myHasBeenPlacedManually = false;
	myCachedFont = NULL;
	myCachedFontSize = 0;
	myTargetIdentifier = NULL;
	myIsUpdatingFromParm = false;
	//myTargetElementToUpdate = -1;
	myPushedCursorId = -1;
	myPrevChangedToValue = FLOAT_TYPE_MAX;
	myCachedAnimOpacity = 1.0;
	myCachedAnimScale = 1.0;

	myIsGettingLinkedToElemWithValidTarget = false;
	myIsTestingDropdowns = false;
	myIsTextOffsetDirty = myIsAnimOffsetDirty = true;

	myExtraScrollOffset.set(0, 0);

	myLifeTimer.setNonAnimValue(0.0);

	myCachedHasChildSliders = false;
	myStartInterpType = InterpLinear;
	myEndInterpType = InterpLinear;

	myMotionEffect = NULL;
	myCachedOffOpacity = myCachedOff2Opacity = myCachedOnOpacity = -1;

	myLastReleasedTime = 0;
	myLastReleasedPosition.set(-1, -1);
	mySizingInfos = NULL;
	myCachedProgress = -1;
	myValidRawPosCache = false;
	myIsVisible = true;
#ifdef ALLOW_UI_EDITOR
	myIsEditorSelected = false;
#endif
	myDisabledOpacityMult = DISABLED_OPACITY_MULT;

	// Check if we're set to be invisible
	if(this->doesPropertyExist(PropertyIsHidden))
		myIsVisible = (this->getBoolProp(PropertyIsHidden) == false);

	myLeavingSlideChild = NULL;
	myIncomingSlideChild = NULL;

	myHandleActionCallback = NULL;
	myCallbacks = dynamic_cast<UIPlane*>(pParentPlane);

	// Note: if we're a child of another element, we automatically
	// set our own state to 1.0 to be shown with the window to begin
	// with. At this point, we should already know whether we're a child
	// or not.
	// Ah! But not if we're actually animated.
#ifdef _DEBUG
	if(this->doesPropertyExist(PropertyId) && strcmp(this->getStringProp(PropertyId), "singlePin") == 0)
	{
		int bp = 0;
	}

#endif

	resetInitialStateAnim();
}
/*****************************************************************************/
UIElement::~UIElement()
{
#ifdef _DEBUG
		/*
    gLog("Deallocating %x named %s\n", this, this->getDebugName());
	
    if(IS_STRING_EQUAL(this->getDebugName(), "defRowIcon9052"))
    {
        int bp = 0;
    }
	*/
#endif
	if(myPushedCursorId >= 0)
	{
		getParentWindow()->popCursor(myPushedCursorId);
		myPushedCursorId = -1;
	}

	// See if we're the glide or the last mouse button element and invalidate it
    if(getParentWindow())
        getParentWindow()->onUiElementDeleted(this);

	if(myMotionEffect)
		delete myMotionEffect;
	myMotionEffect = NULL;

    delete mySizingInfos;
	mySizingInfos = NULL;

	if(myGenericDataSource)
		delete myGenericDataSource;
	myGenericDataSource = NULL;

	{
		//SCOPED_LOCK(myAccelImageLock);
		SCOPED_MUTEX_LOCK(&myAccelImageLock);
		delete myAccelImage;
		myAccelImage = NULL;
	}

	{
		//SCOPED_LOCK(myPendingAnimImageLock);
		SCOPED_MUTEX_LOCK(&myPendingAnimImageLock);
		delete myPendingAnimImage;
		myPendingAnimImage = NULL;
	}

	clearTargetUpdateIdentifier();
}
/*****************************************************************************/
void UIElement::resetInitialStateAnim()
{
    if((this->getParent() && this->getBoolProp(PropertyAnimateChild) == false)) //  || this->getElemType() == UiElemTableCell)
		myStateAnim.setNonAnimValue(1.0);
    else
		myStateAnim.setNonAnimValue(0.0);
}
/*****************************************************************************/
void UIElement::postInit()
{
	myCursorType = getOwnCursorType();

	// False since the caller likely already recurses for us.
	//cacheVariables(false);
	resetEvalCache(false);
	resetUiElemCachedVars(false);

#ifdef _DEBUG
	string strTempPath;
	this->getFullPath(strTempPath);
	myStateAnim.setDebugName(strTempPath.c_str());
	strTempPath += " - Lifetimer";
	myLifeTimer.setDebugName(strTempPath.c_str());
#endif

	if(this->doesPropertyExist(PropertyMotionEffect))
	{
		// See what the effect is and apply it.
		MotionEffectType eEffectType = mapStringToType(this->getStringProp(PropertyMotionEffect), g_pcsMotionEffectTokens, MotionEffectShake);
		this->applyEffect(eEffectType, this->getNumericEnumPropValue(PropertyMotionEffectParms, 0));
	}

	// If this window has title bar, see how large it is:
	updateTitleBarVars();

	myDefaultCancelChild = "";
	myDefaultActionChild = "";
	if(!this->getParent())
	{
		UIElement* pChild = this->findChildByPropertyValue(PropertyIsDefaultAcceptButton, true);
		if(pChild)
			myDefaultActionChild = pChild->getStringProp(PropertyId);

		pChild = this->findChildByPropertyValue(PropertyIsDefaultCancelButton, true);
		if(pChild)
			myDefaultCancelChild = pChild->getStringProp(PropertyId);

	}
}
/*****************************************************************************/
void UIElement::resetUiElemCachedVars(bool bRecursive)
{
	// Cache own vars
	if(this->doesPropertyExist(PropertyTextVertAlign))
	{
		theSharedString2 = this->getStringProp(PropertyTextVertAlign);
		//myVertAlign = mapStringToVertAlign(theSharedString2);
		myTextVertAlign = mapStringToType<VertAlignType>(theSharedString2.c_str(), g_pcsVertAlignStrings, VertAlignCenter);
	}
	else
		myTextVertAlign = VertAlignCenter;

	if(this->doesPropertyExist(PropertyTextHorAlign))
	{
		theSharedString2 = this->getStringProp(PropertyTextHorAlign);
		//myHorAlign = mapStringToHorAlign(theSharedString2);
		myTextHorAlign = mapStringToType<HorAlignType>(theSharedString2.c_str(), g_pcsHorAlignStrings, HorAlignCenter);
	}
	else
		myTextHorAlign = HorAlignCenter;

	myCachedFontSize = this->getNumProp(PropertyFontSize);

	if(this->doesPropertyExist(PropertyFont))
		myCachedFont = this->getStringProp(PropertyFont);
	else
		myCachedFont = "";

	myBackgroundMode = mapStringToType<BackgroundModeType>(this->getStringProp(PropertyBackMode), g_pcsBackgroundModeTypeStrings, BackgroundModeNone);
	
	myBackgroundColor.r = -1;
	if(this->doesPropertyExist(PropertyBackColor))
		this->getAsColor(PropertyBackColor, myBackgroundColor);

	myBorderColor.r = -1;
	if(this->doesPropertyExist(PropertyBorderColor))
		this->getAsColor(PropertyBorderColor, myBorderColor);

	myBorderThickness = this->getNumProp(PropertyBorderThickness);

	markOffsetsDirty();

	myShadowOffset.x =  this->getNumProp(PropertyTextShadowOffsetX);
	myShadowOffset.y =  this->getNumProp(PropertyTextShadowOffsetY);

	myRotation = 0;
	if(this->doesPropertyExist(PropertyRotation))
		myRotation = this->getNumProp(PropertyRotation);

	myOverlayColor.r = FLOAT_TYPE_MAX;
	if(this->doesPropertyExist(PropertyOverlayColor))
		this->getAsColor(PropertyOverlayColor, myOverlayColor);

	this->getTopAnimNameNoFrameNum(theSharedString3);
	SkinManager::getInstance()->mapOverlayColor(theSharedString3.c_str(), myOverlayColor);

	myCachedFillParentXLessPadding = -1;
	if(this->doesPropertyExist(PropertyFillXLess))
		myCachedFillParentXLessPadding = this->getNumProp(PropertyFillXLess);

	myCachedFillParentYLessPadding = -1;
	if(this->doesPropertyExist(PropertyFillYLess))
		myCachedFillParentYLessPadding = this->getNumProp(PropertyFillYLess);

	myCachedAnimOpacity = 1.0;
	if(this->doesPropertyExist(PropertyImageOpacity))
		myCachedAnimOpacity = this->getNumProp(PropertyImageOpacity);

	myCachedAnimScale = 1.0;
	if(this->doesPropertyExist(PropertyImageScale))
		myCachedAnimScale = this->getNumProp(PropertyImageScale);

	myCachedHasChildSliders = this->doesPropertyExist(PropertyUioSlidingChildren);

	myCachedDoesSupportAutoScrolling = this->getBoolProp(PropertySupportsAutoScrollingX) || this->getBoolProp(PropertySupportsAutoScrollingY);
	myCachedDoPostponeRendering = this->getBoolProp(PropertyPostponeRendering);

	if(doesPropertyExist(PropertyForceDrawingTextUncached))
		myForceDrawUncachedText = this->getBoolProp(PropertyForceDrawingTextUncached);

	myChildrenLayoutType = mapStringToType<AutolayoutType>(this->getStringProp(PropertyLayout), g_pcsAutolayoutTypeStrings, AutolayoutNone);
	myCachedHasNoDefinition = this->getBoolProp(PropertyUioHasNoDefinition);

	if(this->doesPropertyExist(PropertyStartAnimStyle))
		myStartInterpType = mapStringToType<InterpType>(this->getStringProp(PropertyStartAnimStyle), g_pcsInterpTypeTokens, InterpLinear);
	if(this->doesPropertyExist(PropertyEndAnimStyle))
		myEndInterpType = mapStringToType<InterpType>(this->getStringProp(PropertyEndAnimStyle), g_pcsInterpTypeTokens, InterpLinear);

	myTextWidth = this->getNumProp(PropertyTextWidth);
	if(this->doesPropertyExist(PropertyTextWidthLessPadding))
		myTextWidth = -1;
	onTextWidthUpdated();

	if(bRecursive)
	{
		UIElement* pChild;
		int iCurrChild, iNumChildren = this->getNumChildren();
		for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
		{
			pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
			pChild->resetUiElemCachedVars(bRecursive);
		}
	}
}
/*****************************************************************************/
void UIElement::resetAllCaches(bool bRecursive)
{
	this->resetUiElemCachedVars(bRecursive);
	this->resetPosCache(bRecursive);
	this->resetEvalCache(bRecursive);
}
/*****************************************************************************/
void UIElement::onTimerTick(GTIME lGlobalTime)
{
	//myStateAnim.checkTime(lGlobalTime);

	FLOAT_TYPE fCurrValue = myStateAnim.getValue();
	bool bIsAnimating = myStateAnim.getIsAnimating(false);

#ifdef _DEBUG
	if(strcmp(this->getStringProp(PropertyId), "fileSubmenu") == 0)
	{
		int bp = 0;
	}

	if(strcmp(this->getStringProp(PropertyId), "newGameChoice") == 0 && fCurrValue >= 0.9)
	{
		int bp = 0;
	}

	if(strcmp(this->getStringProp(PropertyId), "builderMenu") == 0 && fCurrValue >= 0.9)
	{
		int bp = 0;
	}
	if(strcmp(this->getStringProp(PropertyId), "builderMenu") == 0)
	{
		int bp = 0;
	}
	if(strcmp(this->getStringProp(PropertyId), "builderMenu") == 0 && fCurrValue >= 1.0)
	{
		int bp = 0;
		bool bIsAnimating = myStateAnim.getIsAnimating(false);
	}
#endif

	if(myIsAnimating && !bIsAnimating)
	{
		// We have just finished showing this element.
		// Call a method on it.
		if(fCurrValue > 0.0)
			this->onFinishShowingRecursive();
//		else
//			this->onFinishHidingRecursive();
	}
	myIsAnimating = bIsAnimating;

	if(myMotionEffect && myMotionEffect->isDone())
	{
		delete myMotionEffect;
		myMotionEffect = NULL;
	}

	// Call children
	int iSliderIdx;
	UIElement* pChild;
	bool bAllowTimer;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));

		// If the child is a slider, do not timer tick it unless it's the current one.
		bAllowTimer = true;
		if(myCachedHasChildSliders)
		{
			iSliderIdx = this->findStringInEnumProp(PropertyUioSlidingChildren, pChild->getStringProp(PropertyId));
			if(iSliderIdx != myCurrentSliderIndex)
				bAllowTimer = false;
		}

		if(bAllowTimer)
			pChild->onTimerTick(lGlobalTime);
	}

	// Do this last since it may hide us.
//	myLifeTimer.checkTime(lGlobalTime);
}
/*****************************************************************************/
void UIElement::renderChildren(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	onPreRenderChildren(svScroll, fOpacity, fScale);

	SVector2D svPos;
	FLOAT_TYPE fFinalOpac, fLocScale;
	this->getLocalPosition(svPos, &fFinalOpac, &fLocScale);
	fFinalOpac *= fOpacity;

	bool bHaveParent = (this->getParent() != NULL);

	SVector2D svTotalOffset;
	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
/*
		if(!pChild)
		{
			// Something didn't work - we have a non-UIElement child.
			_ASSERT(0);
			continue;
		}
*/


		if(bHaveParent)
			svTotalOffset = svScroll + svPos*fScale;
		else
			svTotalOffset = svScroll + svPos;
		if(pChild->getIsVisible())
			pChild->render(svTotalOffset, fFinalOpac, fScale*fLocScale);
	}
	onPostRenderChildren(svScroll, fOpacity, fScale);
}
/*****************************************************************************/
bool UIElement::onTextPreRender( string& strText, SColor& scolText, int &iCursorPosOut)
{
	// Do nothing here, overriden in other methods.
	// Render shadow if any.
	iCursorPosOut = -1;
	return true;
}
/*****************************************************************************/
void UIElement::renderTextInternalFinal(const string& strText, const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale, bool bSelected)
{
	FLOAT_TYPE fCursorOpacity = this->getCursorOpacity();
	bool bTextExists = strText.length() > 0;
	//bool bIsDataSourceValid = myTextDataSource.getIsValid();
	if(!bTextExists && fCursorOpacity <= 0.0)
		return;

	// Renders the text of this element.
	SVector2D svPos;
	FLOAT_TYPE fFinalOpac, fLocScale;
	this->getLocalPosition(svPos, &fFinalOpac, &fLocScale);
	fFinalOpac *= fOpacity;

	// Grab the text offset properties.
	const char* pcsFont = myCachedFont;
	int iFontSize = myCachedFontSize;
	//const char* pcsText = "";
	SColor scolSelCol(-1,-1,-1);
	SColor scolText;

	if(this->doesPropertyExist(PropertySelectedTextColor))
		this->getAsColor(PropertySelectedTextColor, scolSelCol);

	if(bSelected && this->doesPropertyExist(PropertySelectedTextColor))
		scolText = scolSelCol;
	else
		this->getAsColor(PropertyTextColor, scolText);

	if(scolSelCol.r < 0)
		scolSelCol = scolText;

	SkinManager::getInstance()->mapTextColor(scolText);
	SkinManager::getInstance()->mapTextColor(scolSelCol);

	theSharedString2 = strText;
	if(this->doesPropertyExist(PropertyTextFormat))
	    formatText(theSharedString2);

	int iCursorPosition = -1;
	recomputeTextOffset();
	bool bAllowShadow = onTextPreRender(theSharedString2, scolText, iCursorPosition) && SkinManager::getInstance()->getDoAllowTextShadow();


	SVector2D svFinalTextPos;
	if(this->getParent())
		svFinalTextPos = svScroll + svPos*fScale + myTextOffset*fLocScale*fScale;
	else
		svFinalTextPos = svScroll + svPos + myTextOffset*fLocScale;

	// See if we want the shadow
	SColor *pShadowColor = NULL;
	SColor scolActualShadowColor(-1, -1, -1, -1);
	if(bAllowShadow)
	{
		if(bSelected && this->doesPropertyExist(PropertyTextSelShadowColor))
			this->getAsColor(PropertyTextSelShadowColor, scolActualShadowColor);
		// Note that this used to say !bSelected...
		else if(this->doesPropertyExist(PropertyTextShadowColor) && !this->getIsNone(PropertyTextShadowColor))
			this->getAsColor(PropertyTextShadowColor, scolActualShadowColor);
	}

	if(scolActualShadowColor.r >= 0)
	{
		SkinManager::getInstance()->mapTextColor(scolActualShadowColor);

		scolActualShadowColor.alpha *= fFinalOpac;
		pShadowColor = &scolActualShadowColor;
	}

	SVector2D svSelShadow;
	SVector2D *pShadowOffset = &myShadowOffset;
	if(bSelected)
	{
		if(this->doesPropertyExist(PropertyTextSelShadowOffsetX) || this->doesPropertyExist(PropertyTextSelShadowOffsetY))
		{
			svSelShadow.x = this->getNumProp(PropertyTextSelShadowOffsetX);
			svSelShadow.y = this->getNumProp(PropertyTextSelShadowOffsetY);
			pShadowOffset = &svSelShadow;
		}
	}

	scolText.alpha *= fFinalOpac;

	if(myTextWidth < 0)
		ensureFlexibleWidthValid();

	SVector2D svRelCursorPos;
	if(fCursorOpacity > 0.0 || getHaveSelection())
	{
		getRelativeCursorPos(svRelCursorPos);
		renderSelection(svFinalTextPos, fFinalOpac, fScale*fLocScale, false);
	}

	SVector2D svScrolledText = svFinalTextPos;
	this->getScrolledTextPosition(svScrolledText);
	modifyTextBeforeRender(svScrolledText, theSharedString2);
#ifdef _DEBUG
	bool bDbIsStencil = RenderUtils::isInsideStencil();
	bool bDbIsScissor = RenderUtils::isInsideScissor();
#endif
	getDrawingCache()->addOrMeasureText(theSharedString2.c_str(), pcsFont, iFontSize, svScrolledText.x,
		svScrolledText.y, scolText, myTextHorAlign, myTextVertAlign, 0.0, NULL,
		pShadowColor, pShadowOffset, getTextWidthForRendering(), myForceDrawUncachedText, &scolSelCol, fScale*fLocScale);
	myLastRenderedTextPos = svFinalTextPos;

	// Measure the string and draw cursor
	if(fCursorOpacity > 0.0)
	{
		SColor scolCursor(1,1,1, fCursorOpacity);
		bool bIsCursorAtEnd = (iCursorPosition == theSharedString2.length());

		// Measure the cut line:
/*
		SVector2D svSize;
		SCachedTextObject rTextInfo;
		rTextInfo.myCenter = svFinalTextPos;
		rTextInfo.myHorAlign = myHorAlign;
		rTextInfo.myVertAlign = myVertAlign;
		rTextInfo.myBaseInfo.myFont = pcsFont;
		// Cut the string to the point where the cursor is to be drawn
		_ASSERT(iCursorPosition >= 0 && iCursorPosition <= theSharedString2.length());
		if(iCursorPosition > theSharedString2.length())
			iCursorPosition = theSharedString2.length();
		theSharedString2 = theSharedString2.substr(0, iCursorPosition);
		RenderUtils::measureText(theSharedString2.c_str(), rTextInfo, svSize);
*/


		// Get the line height for the font
#ifdef USE_FREETYPE_FONTS
		FLOAT_TYPE fLinePixHeight = getParentWindow()->getFontManager()->getFontInfo(pcsFont)->getLineHeight(getCachedFontSize());
#else

		ResourceCollection* pFonts = ResourceManager::getInstance()->getCollection(ResourceRasterFonts);
		ResourceItem* pItem = pFonts->getItemById(pcsFont);
		FLOAT_TYPE fLinePixHeight = pItem->getNumProp(PropertyFtHeight);
#endif
		FLOAT_TYPE fCursorVertShift = 0;
		if(myTextVertAlign == VertAlignTop)
			fCursorVertShift += fLinePixHeight/2.0;
		else if(myTextVertAlign == VertAlignBottom)
			fCursorVertShift -= fLinePixHeight/2.0;

		// Note: the below assumes a single line of text
		// Note some hacks for cursor size and position.

#ifdef USE_FREETYPE_FONTS
		getDrawingCache()->addRectangle(svFinalTextPos.x + svRelCursorPos.x + upToScreen(0.5),
			HyperCore::roundToInt(svFinalTextPos.y - fLinePixHeight/2.0 + fCursorVertShift) + svRelCursorPos.y,
			bIsCursorAtEnd ? upToScreen(4.0) : upToScreen(1.0),
			fLinePixHeight, scolCursor);
#else
		// Thin cursor
		getDrawingCache()->addRectangle(svFinalTextPos.x + svRelCursorPos.x + upToScreen(0.5),
			roundToInt(svFinalTextPos.y - fLinePixHeight/2.0 + upToScreen(1.5) + fCursorVertShift) + svRelCursorPos.y,
			bIsCursorAtEnd ? upToScreen(4.0) : upToScreen(1.0),
			fLinePixHeight - upToScreen(4.0), scolCursor);
#endif

/*
		getDrawingCache()->addRectangle(svFinalTextPos.x + svSize.x + (bIsCursorAtEnd ? upToScreen(1.5) : upToScreen(0.5)),
			roundToInt(svFinalTextPos.y - fLinePixHeight/2.0 + upToScreen(1.5) + fCursorVertShift),
			bIsCursorAtEnd ? upToScreen(4.0) : upToScreen(1.0),
			fLinePixHeight - upToScreen(4.0), scolCursor);
*/
	}

	onTextPostRender();
}
/*****************************************************************************/
void UIElement::renderTextInternal(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale, bool bSelected)
{
	FLOAT_TYPE fCursorOpacity = this->getCursorOpacity();
	bool bTextExists = false;
	ensureCachedTextValid();
	if(getCachedTextRaw().length() > 0)
		bTextExists = true;

	if(!bTextExists && fCursorOpacity <= 0.0)
		return;

	renderTextInternalFinal(getCachedTextRaw(), svScroll, fOpacity, fScale, bSelected);
}
/*****************************************************************************/
void UIElement::renderInternal(const string &strAnim, const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{

#ifdef _DEBUG
	if(IS_OF_TYPE("brushSettingsHost"))
	{
		int bp = 0;
	}
	if(IS_OF_TYPE("mainWnd"))
	{
		int bp = 0;
	}
	if(IS_OF_TYPE("imgWndStatusBar"))
	{
		int bp = 0;
	}
#endif

	bool bHaveValidAnim = !(strAnim == PROPERTY_NONE || strAnim.length() <= 0);
	bool bHaveFlatPainting = (myBackgroundColor.r >= 0.0 || (myBorderColor.r >= 0 && myBorderThickness > 0));
	if(!bHaveValidAnim && !myAccelImage && !myPendingAnimImage && !bHaveFlatPainting)
		return;

	SVector2D svPos;
	FLOAT_TYPE fFinalOpac, fLocScale;
	this->getLocalPosition(svPos, &fFinalOpac, &fLocScale);
	fFinalOpac *= fOpacity*myCachedAnimOpacity;

#ifdef HARDCORE_LINUX_DEBUG_MODE
	fFinalOpac *= 0.6;
#endif

	if(myIsAnimOffsetDirty)
		recomputeAnimOffset();
	svPos += myAnimOffset;

	if(this->getParent())
		svPos *= fScale;

	if(bHaveFlatPainting)
	{
		SVector2D svOwnSize;
		getBoxSize(svOwnSize);

		// If this fails, you forgot to specify the size on the element.
		_ASSERT(svOwnSize.x > 0 && svOwnSize.y > 0);

		svOwnSize = svOwnSize*fScale*fLocScale;

		// We do these horrible flushes because if the same element has other
		// stuff such as bitmaps, the rectangles get drawn over it...
		getDrawingCache()->flush();

		// First, fill the background with our background color
		SColor scolTemp;
		if(myBackgroundColor.r >= 0.0)
		{
			scolTemp = myBackgroundColor;
			scolTemp.alpha *= fFinalOpac;
			getDrawingCache()->addRectangle(svPos.x + svScroll.x - svOwnSize.x/2.0, svPos.y + svScroll.y - svOwnSize.y/2.0, svOwnSize.x, svOwnSize.y, scolTemp);
		}
		
		if(myBorderColor.r >= 0 && myBorderThickness > 0)
		{
			scolTemp = myBorderColor;
			scolTemp.alpha *= fFinalOpac;
			getDrawingCache()->addRectangle(svPos.x + svScroll.x - svOwnSize.x/2.0, svPos.y + svScroll.y - svOwnSize.y/2.0, svOwnSize.x, svOwnSize.y, scolTemp, myBorderThickness);
		}

		getDrawingCache()->flush();
	}

	if(myBackgroundMode == BackgroundModeScale && bHaveValidAnim)
	{
		// We have the flag to scale the image.
		// For this, we need to know the image's actual size in the
		// bitmap, as well as our own size. Note that it has to take scaling
		// into account.
		SVector2D svOwnSize;
		getBoxSize(svOwnSize);

		// If this fails, you forgot to specify the size on the element.
		_ASSERT(svOwnSize.x > 0 && svOwnSize.y > 0);

		svOwnSize = svOwnSize*fScale*fLocScale;

		// Now, find out the image size.
		int iRealW, iRealH;
		getTextureManager()->getTextureRealDims(strAnim.c_str(), iRealW, iRealH);

		FLOAT_TYPE fExtraScaleX, fExtraScaleY;
		fExtraScaleX = svOwnSize.x/(FLOAT_TYPE)iRealW*fScale*fLocScale;
		fExtraScaleY = svOwnSize.y/(FLOAT_TYPE)iRealH*fScale*fLocScale;

//		if(fExtraScaleY < 1.0)
//			fExtraScaleY = 1.0;

		// Hack to avoid bug when resizing
		if(fabs(fExtraScaleX) < FLOAT_EPSILON)
			fExtraScaleX = 1.0;
		if(fabs(fExtraScaleY) < FLOAT_EPSILON)
			fExtraScaleY = 1.0;

		getDrawingCache()->addSprite(strAnim.c_str(), svPos.x + svScroll.x, svPos.y + svScroll.y,
			fFinalOpac, 0, fExtraScaleX, fExtraScaleY, 0, true);

	}
	else if(myBackgroundMode == BackgroundModeFill && bHaveValidAnim)
	{
		SVector2D svOwnSize;
		getBoxSize(svOwnSize);

		// If this fails, you forgot to specify the size on the element.
		//_ASSERT(svOwnSize.x > 0 && svOwnSize.y > 0);

		svOwnSize = svOwnSize*fScale*fLocScale;

		int iCornerW, iCornerH;
		getTextureManager()->getCornerSize(strAnim.c_str(), iCornerW, iCornerH);

		// We must fill the back
		if(iCornerW > 0 && iCornerH > 0)
		{
			// This is more complex - drawing a window with unique corners.
			getDrawingCache()->addScalableButton(strAnim.c_str(), svPos.x + svScroll.x, svPos.y + svScroll.y, svOwnSize.x, svOwnSize.y, fFinalOpac, iCornerW, iCornerH, myOverlayColor.r == FLOAT_TYPE_MAX ? NULL : &myOverlayColor);
		}
		else
			getDrawingCache()->fillArea(strAnim.c_str(), svPos.x - svOwnSize.x/2.0 + svScroll.x, svPos.y - svOwnSize.y/2.0 + svScroll.y, svOwnSize.x, svOwnSize.y, fFinalOpac, fScale*fLocScale, fScale*fLocScale);
	}
	else
	{
		if(myPendingAnimImage)
		{
			//SCOPED_LOCK(myPendingAnimImageLock);
			SCOPED_MUTEX_LOCK(&myPendingAnimImageLock);
			//SCOPED_LOCK_NAMED(myAccelImageLock, rAccelLock);
			myAccelImageLock.lock();
			delete myAccelImage;
			myAccelImage = getParentWindow()->allocateAccelImage();
			myAccelImage->setFromImage(myPendingAnimImage);
			delete myPendingAnimImage;
			myPendingAnimImage = NULL;
			myAccelImageLock.unlock();
		}

		if(myAccelImage)
		{
			//SCOPED_LOCK(myAccelImageLock);
			SCOPED_MUTEX_LOCK(&myAccelImageLock);
			myAccelImage->setSamplingMode(TextureSamplingNearestNeighbour);
			SRect2D srTestBox;
			myAccelImage->getBBox(svPos.x + svScroll.x, svPos.y + svScroll.y, fScale*fLocScale*myCachedAnimScale, fScale*fLocScale*myCachedAnimScale,  srTestBox);
			// Again, on NVidia, we crap out if there's too many of these.
			// Help and test with the scissor rect
			if(RenderUtils::doesIntersectScissorRect(srTestBox))
			{
				getDrawingCache()->flush();
				myAccelImage->render(getParentWindow(), svPos.x + svScroll.x, svPos.y + svScroll.y, fScale*fLocScale*myCachedAnimScale, fScale*fLocScale*myCachedAnimScale, fFinalOpac);
			}
		}
		else if(bHaveValidAnim)
		{
			getDrawingCache()->addSprite(strAnim.c_str(), (int)(svPos.x + svScroll.x), (int)(svPos.y + svScroll.y),
				fFinalOpac, myRotation, fScale*fLocScale*myCachedAnimScale, 0, true, myOverlayColor.r == FLOAT_TYPE_MAX ? NULL : &myOverlayColor);
		}
	}
}
/*****************************************************************************/
void UIElement::render(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
#ifdef _DEBUG

	if(IS_OF_TYPE("defDialogInner12392"))
	{
		int bp = 0;
	}

	if(IS_OF_TYPE("defRowIcon244"))
	{
		int bp = 0;
	}
	if(as<UIDropdown>(getParent()))
	{
		int bp = 0;
	}
	if(this->doesPropertyExist(PropertyOldId) && strcmp(this->getStringProp(PropertyOldId), "defButton") == 0)
	{
		int bp = 0;
	}
	if(this->doesPropertyExist(PropertyOldId) && strcmp(this->getStringProp(PropertyOldId), "defButton") == 0 && as<UIDropdown>(getParent()))
	{
		int bp = 0;
	}

#endif

	FLOAT_TYPE fProgress = myStateAnim.getValue();

	// We skip rendering elements with no progress (i.e. the hidden ones).
	// However, this would also affect the children, since they obey parent's
	// progress and their own is not animated. So we also check if the element
	// has a parent.
	if(fProgress <= FLOAT_EPSILON && !this->getParent())
		return;

	// If we have no layer, don't render us.
	const char* pcsLayer = this->getStringProp(PropertyLayer);
	if(strlen(pcsLayer) <=0 || strcmp(pcsLayer, PROPERTY_NONE) == 0)
		return;


	// See if we want to postpone rendering
	// Bad. There's a copy in UIButtonElement, and anything that overrides render() won't use this...
	if(this->getDoPostponeRendering() && getUIPlane()->getRenderingPass() != RenderingPostopnedElements)
	{
		getUIPlane()->addPostponedElement(this, svScroll, fOpacity, fScale);
		return;
	}

	preRender(svScroll, fOpacity, fScale);

	//theSharedString = this->getStringProp(PropertyObjAnim);
	this->getFullTopAnimName(theSharedString);
	SVector2D svAnimOffsetWithScroll;
	getTopAnimOffset(svAnimOffsetWithScroll);
	svAnimOffsetWithScroll += svScroll;
	renderInternal(theSharedString, svAnimOffsetWithScroll, fOpacity, fScale);
	renderTextInternal(svScroll, fOpacity, fScale, this->getIsTextSelected());

	renderTitleBar(svScroll, fOpacity, fScale);

	SVector2D svDummy;
	FLOAT_TYPE fLocScale;
	FLOAT_TYPE fLocOpacity;
	this->getLocalPosition(svDummy, &fLocOpacity, &fLocScale);

	renderChildren(svScroll, fOpacity, fScale*fLocScale);

	postRender(svScroll, fOpacity*fLocOpacity, fScale);
}
/*****************************************************************************/
void UIElement::renderBBox()
{
#if defined(DEBUG_OUTLINE_OBJECTS) && !defined(ALLOW_UI_EDITOR)
	SVector2D svSize;
	SColor scolOutlineCol(0,0,1, 0.5);
	this->getBoxSize(svSize);
	renderBBoxOfSize(svSize, scolOutlineCol, true);
#endif

#ifdef ALLOW_UI_EDITOR
	// See if we're selected
	bool bRenderOwnBox = false;
	bool bRenderAnimBox = false;
	SColor scolOwnBox, scolAnimBox;
	scolAnimBox.set(1, 0, 0, 0.4);
	scolOwnBox.set(1, 0, 0, 0.4);
	if(myIsEditorSelected)
	{
		bRenderOwnBox = true;
		bRenderAnimBox = true;

		scolOwnBox.set(0, 1, 0, 1);
	}
	else if(UIEditor::getInstance()->getShowBBoxes())
	{
		bRenderOwnBox = true;
	}


	SVector2D svTempBox;
	if(bRenderAnimBox)
	{
		this->getAnimBoxSize(svTempBox);
		renderBBoxOfSize(svTempBox, scolAnimBox, false);
	}
	if(bRenderOwnBox)
	{
		this->getBoxSize(svTempBox);
		if(svTempBox.x == 0)
			this->getAnimBoxSize(svTempBox);
		renderBBoxOfSize(svTempBox, scolOwnBox, true);
	}
#endif
}
/*****************************************************************************/
void UIElement::renderBBoxOfSize(SVector2D& svSize, const SColor& scol, bool bTakeShiftIntoAccount)
{
	SRect2D srBBox;
	SColor scolOutlineCol;
	scolOutlineCol = scol;
	SVector2D svPos;
	FLOAT_TYPE fOpacity;
	this->getGlobalPosition(svPos, &fOpacity, NULL);

	if(bTakeShiftIntoAccount)
	{
		SVector2D svBoxShift;
		svBoxShift.x = this->getNumProp(PropertyBoxOffsetX);
		svBoxShift.y = this->getNumProp(PropertyBoxOffsetY);
		svPos += svBoxShift;
	}

	//scolOutlineCol.alpha *= fOpacity;
	getDrawingCache()->addRectangle(svPos.x - svSize.x/2.0, svPos.y - svSize.y/2.0, svSize.x, svSize.y, scolOutlineCol, 1);
}
/*****************************************************************************/
void UIElement::getGlobalPosition(SVector2D& svOut, FLOAT_TYPE* fOpacityOut, FLOAT_TYPE* fScaleOut)
{
#ifdef _DEBUG
	if(IS_OF_TYPE("srcTranslateX"))
	{
		int bp = 0;
	}
#endif
	// Get own local position, then get parent's
	SVector2D svParentPos;
	FLOAT_TYPE fParentOpacity;
	this->getLocalPosition(svOut, fOpacityOut, fScaleOut);

	UIElement* pParent = FAST_CAST<UIElement*>(this->getParent());
	if(!pParent)
	{
		SVector2D svWndSize;
		getParentWindow()->getSize(svWndSize);
		svOut += svWndSize*0.5;
		return;
	}

	FLOAT_TYPE fLocScale;
	if(fOpacityOut)
	{
		pParent->getGlobalPosition(svParentPos, &fParentOpacity, &fLocScale);
		(*fOpacityOut) *= fParentOpacity;
	}
	else
		pParent->getGlobalPosition(svParentPos, NULL, &fLocScale);

	if(pParent->getParent())
		svOut += svParentPos*fLocScale;
	else
		svOut += svParentPos;

	if(fScaleOut)
		*fScaleOut *= fLocScale;
}
/*****************************************************************************/
void UIElement::getLocalPosition(SVector2D& svOut, FLOAT_TYPE* fOpacityOut, FLOAT_TYPE* fScaleOut)
{
#ifdef _DEBUG
	if(IS_OF_TYPE("blendModeLabel"))
	{
		int bp = 0;
	}
#endif
	GTIME lTime = Application::getInstance()->getGlobalTime(ClockUiPrimary);

	if(myCachedPositionTime == lTime)
	{
		svOut = myCachedLocalPosition;
		UIElement* pParent = FAST_CAST<UIElement*>(this->getParent());
		if(pParent)
		{
			SVector2D svParentExtraOffset;
			pParent->getExtraShiftOffset(svParentExtraOffset);
			svOut += svParentExtraOffset;
		}


		if(fOpacityOut)
			*fOpacityOut = myCachedOpacity;
		if(fScaleOut)
			*fScaleOut = myCachedScale;
		return;
	}

	FLOAT_TYPE fProgress = myStateAnim.getValue();

	if(fProgress == myCachedProgress && myValidRawPosCache && !myMotionEffect)
	{
		svOut = myCachedLocalPosition;
		UIElement* pParent = FAST_CAST<UIElement*>(this->getParent());
		if(pParent)
		{
			SVector2D svParentExtraOffset;
			pParent->getExtraShiftOffset(svParentExtraOffset);
			svOut += svParentExtraOffset;
		}
		if(fOpacityOut)
		{
			*fOpacityOut = myCachedOpacity;

// 			if(myMotionEffect && fProgress == 1)
// 				myMotionEffect->evaluateOpacity(*fOpacityOut);
		}

		if(fScaleOut)
			*fScaleOut = myCachedScale;
		return;
	}

	///FLOAT_TYPE fProgress = myStateAnim.getValue(lTime);

	// See where we are in the transition
	SVector2D svOffPos, svOnPos, svOff2Pos;
	FLOAT_TYPE fOffScale, fOnScale, fOff2Scale;
#ifdef _DEBUG
	//if(IS_OF_TYPE("mainWnd"))
	//if(IS_OBJ_OF_TYPE(pItem, "blendModeLabel"))
	const char* pcsId = this->getStringProp(PropertyId);
	if(strstr(pcsId, "blendModeLabel"))
	{
	    int bp = 0;
	}
#endif
	getRawPositions(svOffPos, svOnPos, svOff2Pos, fOffScale, fOnScale, fOff2Scale);

	// We need to know whether:
	// 1) The item is going from 0->1, i.e. appearing on the screen.
	// 2) It is going 1->0, i.e. disappearing
	// 3) Is not being moved, being at 0 or 1.
	FLOAT_TYPE fStartValue = myStateAnim.getStartValue();
	FLOAT_TYPE fEndValue = myStateAnim.getEndValue();

	//FLOAT_TYPE fOnOpacity, fOffOpacity, fOff2Opacity;
	//if(fOpacityOut)
	FLOAT_TYPE fOffOpac, fOnOpac, fOff2Opac;
	getRawOpacitites(fOffOpac, fOnOpac, fOff2Opac);

/*
	if(fProgress >= 1)
	{
		svOut = svOnPos;
		myCachedOpacity = fOnOpacity;
	}
	else if(fProgress <= 0.0)
	{
		svOut = svOff2Pos;
		myCachedOpacity = fOff2Opacity;
	}
	else */ if(fProgress <= 0 || fStartValue > fEndValue)
	{
		// We're disappearing
		svOut = svOnPos*fProgress + svOff2Pos*(1.0 - fProgress);
		myCachedOpacity = myCachedOnOpacity*fProgress + myCachedOff2Opacity*(1.0 - fProgress);
		myCachedScale = fOnScale*fProgress + fOff2Scale*(1.0 - fProgress);
	}
	else if(fProgress >= 1 || fStartValue < fEndValue)
	{
		svOut = svOnPos*fProgress + svOffPos*(1.0 - fProgress);
		myCachedOpacity = myCachedOnOpacity*fProgress + myCachedOffOpacity*(1.0 - fProgress);
		myCachedScale = fOnScale*fProgress + fOffScale*(1.0 - fProgress);
	}

	if(!getIsEnabled())
		myCachedOpacity *= myDisabledOpacityMult;

	if(myMotionEffect && fProgress == 1)
		myMotionEffect->evaluateOpacity(myCachedOpacity);


	myCachedProgress = fProgress;
	if(fOpacityOut)
	{
		*fOpacityOut = myCachedOpacity;
	}
//		myCachedOpacity = *fOpacityOut;
	myCachedLocalPosition = svOut;
	myCachedPositionTime = lTime;
	if(fScaleOut)
		*fScaleOut = myCachedScale;

	UIElement* pParent = FAST_CAST<UIElement*>(this->getParent());
	if(pParent)
	{
		SVector2D svParentExtraOffset;
		pParent->getExtraShiftOffset(svParentExtraOffset);
		svOut += svParentExtraOffset;
	}

}
/*****************************************************************************/
void UIElement::onPressed(TTouchVector& vecTouches)
{
	myIsMousePressed = true;
	if(vecTouches.size() > 0)
		myPressedPos = vecTouches[0].myPoint;
}
/*****************************************************************************/
void UIElement::onMouseEnter(TTouchVector& vecTouches)
{

}
/*****************************************************************************/
bool UIElement::allowDragDropStart(SVector2D& svPos)
{
	return this->getBoolProp(PropertyIsDraggable);
}
/*****************************************************************************/
void UIElement::onMouseLeave(TTouchVector& vecTouches)
{
#ifdef INITIATE_DRAG_AND_DROP_ON_MOUSE_LEAVE
	if(myIsMousePressed && this->allowDragDropStart(vecTouches[0].myPoint))
	{
		// Tell the D&D manager we're being dragged.
		getParentWindow()->getDragDropManager()->beginDrag(this);
	}
#endif
	myIsMousePressed = false;
	myPressedPos.set(0,0);
}
/*****************************************************************************/
void UIElement::onMouseMove(TTouchVector& vecTouches)
{
#ifndef INITIATE_DRAG_AND_DROP_ON_MOUSE_LEAVE
	if(myIsMousePressed && vecTouches.size() > 0
		&& getParentWindow()->getDragDropManager()->isDragging() == false)
	{
		SVector2D svDist;
		svDist = myPressedPos - vecTouches[0].myPoint;
		//if(svDist.lengthSquared() > DRAG_DROP_MIN_DIST*DRAG_DROP_MIN_DIST
		if(svDist.lengthSquared() > getMinDistanceBeforeDragStart()*getMinDistanceBeforeDragStart()
			&& this->allowDragDropStart(vecTouches[0].myPoint))
		{
			// Tell the D&D manager we're being dragged.
			getParentWindow()->getDragDropManager()->beginDrag(this);
		}
	}
#endif
}
/*****************************************************************************/
void UIElement::onReleased( TTouchVector& vecTouches, bool bIgnoreActions )
{
	myIsMousePressed = false;
	if(!getAllowKeepingFocus())
	{
		// Kill focus if we're not a text element.
		getParentWindow()->setFocusElement(NULL);
	}

	// See if we're actually double-clicking
	if(vecTouches.size() > 0 && getParentWindow()->getCurrMouseButtonDown() == MouseButtonLeft)
	{
		GTIME lCurrTime = Application::getInstance()->getGlobalTime(ClockUniversal);
		if(lCurrTime - myLastReleasedTime < Application::secondsToTicks(DOUBLE_CLICK_MAX_TIME)
			&& (vecTouches[0].myPoint - myLastReleasedPosition).lengthSquared() <= (DOUBLE_CLICK_MAX_DIST*DOUBLE_CLICK_MAX_DIST) )
		{
			onDoubleClick(vecTouches, bIgnoreActions);
			myLastReleasedPosition.set(-1, -1);
			myLastReleasedTime = 0;
		}
		else
		{
			myLastReleasedPosition = vecTouches[0].myPoint;
			myLastReleasedTime = lCurrTime;
		}
	}

}
/*****************************************************************************/
bool UIElement::isInsideLocal(FLOAT_TYPE fX, FLOAT_TYPE fY, bool bClickableOnly)
{
	if(bClickableOnly && this->getElemType() == UiElemStatic)
		return false;

	SRect2D srRect;
	this->getGlobalRectangle(srRect);
	return srRect.doesContain(fX, fY);

	/*


	FLOAT_TYPE fLocScale;
	SVector2D svPos, svSize;
	this->getBoxSize(svSize);
	this->getLocalPosition(svPos, NULL, &fLocScale);

	// Get the box shift
	SVector2D svBoxShift;
	svBoxShift.x = this->getNumProp(PropertyBoxOffsetX);
	svBoxShift.y = this->getNumProp(PropertyBoxOffsetY);
	svPos += svBoxShift;

	svSize *= fLocScale;

	svPos = svPos - svSize/2.0;


//	if(this->getParent())
//		svPos *= fScale;

	if(fX >= svPos.x && fX <= svPos.x + svSize.x &&
		fY >= svPos.y && fY <= svPos.y + svSize.y)
		return true;
	else
		return false;
		*/
}
/*****************************************************************************/
void UIElement::resetToInitState()
{
	if(myPushedCursorId >= 0)
	{
		getParentWindow()->popCursor(myPushedCursorId);
		myPushedCursorId = -1;
	}

	this->onAllocated(this->getUIPlane());
	this->postInit();

	// Do children
	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
/*
		if(!pChild)
		{
			_ASSERT(0);
			continue;
		}
*/
		pChild->resetToInitState();
	}

	this->resetSliderChildrenVisibility();
}
/*****************************************************************************/
void UIElement::onFinishShowing()
{
	// See if we've got music to play
	if(this->doesPropertyExist(PropertyMusic))
	{
		TStringVector levPlaylist;
		this->getAsStringVector(PropertyMusic, levPlaylist);
		//gStartPlaylist(levPlaylist);
	}
}
/*****************************************************************************/
void UIElement::onFinishShowingRecursive()
{
	// Call on ourselves, then call on all our children.
	onFinishShowing();

	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
/*
		if(!pChild)
		{
			// Something didn't work - we have a non-UIElement child.
			_ASSERT(0);
			continue;
		}
*/

		if(pChild->getIsVisible() == false)
			continue;
		pChild->onFinishShowingRecursive();
	}
}
/*****************************************************************************/
void UIElement::onStartShowing()
{
	resetSliderPosition();
	SUpdateInfo rInfo;
	rInfo.myPosition = 0;
	updateDataRecursive(rInfo);

	// See if we have to create an Undo Manager
	if(this->getBoolProp(PropertyHaveOwnUndoManager))
	{
		Window* pParentWindow = getParentWindow();
		if(pParentWindow)
			pParentWindow->createAndPushOverrideUndoManager(this);
	}
}
/*****************************************************************************/
void UIElement::onStartShowingRecursive()
{
	// Call on ourselves, then call on all our children.
	onStartShowing();

	bool bHasDemoTag, bHasNonDemoTag;
#ifdef DEMO_MODE
	bool bIsInDemo = !GameEngine::isDemoUnlocked();
#else
	bool bIsInDemo = false;
#endif

	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
/*
		if(!pChild)
		{
			// Something didn't work - we have a non-UIElement child.
			_ASSERT(0);
			continue;
		}
*/

		bHasDemoTag = pChild->hasTag("demoonly");
		bHasNonDemoTag = pChild->hasTag("notindemo");
		_ASSERT(bHasNonDemoTag == false || bHasDemoTag == false);
		if(bHasDemoTag || bHasNonDemoTag)
		{
			bool bIsHidden = pChild->getBoolProp(PropertyIsHidden);

			if( ((bIsInDemo && bHasDemoTag) || (!bIsInDemo && bHasNonDemoTag)) && !bIsHidden)
				pChild->setIsVisible(true);
			else
				pChild->setIsVisible(false);
		}

		if(pChild->getIsVisible() == false)
			continue;
		pChild->onStartShowingRecursive();
	}

}
/*****************************************************************************/
void UIElement::onStartHiding()
{
	if(this->doesPropertyExist(PropertyMusic))
	{
#if !defined(WIN32) && !defined(LINUX)
//		if(!g_pSoundManager->getIsMusicPaused())
//			g_pSoundManager->pauseMusic();
#endif
	}

	// Note that this assumes only one undo manager
	if(this->getBoolProp(PropertyHaveOwnUndoManager))
	{
		Window* pParentWindow = getParentWindow();
		if(pParentWindow)
			pParentWindow->destroyAndPopOverrideUndoManager(this);
	}
}
/*****************************************************************************/
void UIElement::onFinishHiding()
{
	// If we're a child that is animated we need to set our
	// anim value to 0, so that the next time we're called,
	// we actually start from 0. This may happen if we have a time
	// offset that's too long.
	if(this->getBoolProp(PropertyAnimateChild) && getParent())
	{
		myStateAnim.setNonAnimValue(0);
	}

	// If we have a callback, call that now
	if(this->doesPropertyExist(PropertyOnHiddenCallback))
	{
		UIElement* pTarget = getUIPlane()->getElementById<UIElement>(this->getStringProp(PropertyOnHiddenCallback), true, true);
		if(pTarget)
			pTarget->onChildFinishHiding(this);
	}

	myHidingInProgress = false;
}
/*****************************************************************************/
void UIElement::onStartHidingRecursive()
{
	// Call on ourselves, then call on all our children.
	onStartHiding();

	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
/*
		if(!pChild)
		{
			// Something didn't work - we have a non-UIElement child.
			_ASSERT(0);
			continue;
		}
*/

		if(pChild->getIsVisible() == false)
			continue;
		pChild->onStartHidingRecursive();
	}
}
/*****************************************************************************/
void UIElement::onFinishHidingRecursive()
{
	// Call on ourselves, then call on all our children.
	onFinishHiding();

	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
/*
		if(!pChild)
		{
			// Something didn't work - we have a non-UIElement child.
			_ASSERT(0);
			continue;
		}
*/

		if(pChild->getIsVisible() == false)
			continue;
		pChild->onFinishHidingRecursive();
	}
}
/*****************************************************************************/
void UIElement::show(bool bImmediate, AnimationOverCallback* pCallBack, AnimOverActionType eAnimOverAction, bool bResetElements)
{
	GTIME lTime = Application::getInstance()->getGlobalTime(ClockUiPrimary);
	if(myStateAnim.getValue() == 1.0 && !myStateAnim.getIsAnimating(false))
		return;

	FLOAT_TYPE fTimeToLive = this->getNumProp(PropertyTimeToLive);

#ifdef _DEBUG
	if(strcmp(this->getStringProp(PropertyId), "singlePin") == 0)
	{
		int bp = 0;
	}
#endif

	if(bImmediate)
	{
		myStateAnim.setNonAnimValue(1.0);
		if(fTimeToLive > 0.0)
			myLifeTimer.setAnimation(0, 1, fTimeToLive, getClockType(), AnimOverActionGenericCallback, this);
	}
	else
	{
		// Start the anim towards 1.0.
		FLOAT_TYPE fCurrValue = myStateAnim.getValue();

		// Figure out the portion of time we have left
		FLOAT_TYPE fPortion = 1.0 - fCurrValue;
		if(fPortion < 0.0)
			return;
		if(fPortion > 1.0)
			fPortion = 1.0;

		FLOAT_TYPE fRealTransTime = fPortion*this->getNumProp(PropertyTransitionTime);
		FLOAT_TYPE fTimeOffset = getTimeOffset(true);
		myStateAnim.setAnimation(fCurrValue, 1.0, fRealTransTime, getClockType(), eAnimOverAction, pCallBack, fTimeOffset);
		myStateAnim.setStartInterpType(myStartInterpType);
		myStateAnim.setEndInterpType(myEndInterpType);

		if(fTimeToLive > 0.0)
			myLifeTimer.setAnimation(0, 1, fTimeToLive + fRealTransTime, getClockType(), AnimOverActionGenericCallback, this);
	}

	// See if any children need to be animated
	int iCurrChild, iNumChildren = getNumChildren();
	UIElement* pChild;
	bool bShowChild;
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(getChild(iCurrChild));
/*
		if(!pChild)
		{
			_ASSERT(0);
			continue;
		}
*/

		bShowChild = false;
		if(pChild->getBoolProp(PropertyAnimateChild))
		{
			bShowChild = true;
			if(myCachedHasChildSliders && getCurrentSlideElement<UIElement>() != pChild)
				bShowChild = false;
		}

		if(bShowChild)
			pChild->show(bImmediate, NULL, AnimOverActionNone, bResetElements);

	}

	if(bResetElements && getParent() == NULL)
		onStartShowingRecursive();
}
/*****************************************************************************/
void UIElement::setAllChildrenInvisible()
{
    int iCurrChild, iNumChildren = getNumChildren();
    UIElement* pChild;
    for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
    {
		pChild = FAST_CAST<UIElement*>(getChild(iCurrChild));
		pChild->setIsVisible(false);
    }
}
/*****************************************************************************/
void UIElement::hide(bool bImmediate, AnimationOverCallback* pCallBack, AnimOverActionType eAnimOverAction, FLOAT_TYPE fExtraOffset)
{
	GTIME lTime = Application::getInstance()->getGlobalTime(ClockUiPrimary);
	if(myStateAnim.getValue() == 0.0 && !myStateAnim.getIsAnimating(false))
		return;

	if(bImmediate)
	{
		myStateAnim.setNonAnimValue(0.0);
		return;
	}

	myHidingInProgress = true;

	// Start the anim towards 1.0.
	FLOAT_TYPE fCurrValue = myStateAnim.getValue();

	// Figure out the portion of time we have left
	FLOAT_TYPE fPortion = fCurrValue;
	if(fPortion < 0.0)
		return;
	if(fPortion > 1.0)
		fPortion = 1.0;

	FLOAT_TYPE fRealTransTime = fPortion*this->getNumProp(PropertyTransitionTime);
	FLOAT_TYPE fTimeOffset = getTimeOffset(false);
	myStateAnim.setAnimation(fCurrValue, 0.0, fRealTransTime, ClockUiPrimary, eAnimOverAction, pCallBack, fTimeOffset + fExtraOffset);
	myStateAnim.setStartInterpType(myStartInterpType);
	myStateAnim.setEndInterpType(myEndInterpType);

	// See if any children need to be animated
	int iCurrChild, iNumChildren = getNumChildren();
	UIElement* pChild;
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(getChild(iCurrChild));
		if(pChild->getBoolProp(PropertyAnimateChild))
			pChild->hide(bImmediate, NULL, AnimOverActionNone, fExtraOffset);
	}

	if(getParent() == NULL)
		onStartHidingRecursive();
}
/*****************************************************************************/
bool UIElement::getIsBeingHidden()
{
	if(this->getIsAnimating() == false)
	{
		//return getIsFullyHidden();
		return false;
	}
	else
	{
		_ASSERT(myStateAnim.getEndTime() > Application::getInstance()->getGlobalTime(ClockUiPrimary));
		if(myStateAnim.getStartValue() > myStateAnim.getEndValue())
			return true;
		else
			return false;
	}
}
/*****************************************************************************/
bool UIElement::getIsBeingShown()
{
	if(this->getIsAnimating() == false)
	{
		//return getIsFullyShown();
		return false;
	}
	else
	{
		_ASSERT(myStateAnim.getEndTime() >= Application::getInstance()->getGlobalTime(ClockUiPrimary));
		if(myStateAnim.getStartValue() < myStateAnim.getEndValue())
			return true;
		else
			return false;
	}
}
/*****************************************************************************/
bool UIElement::getIsFullyHidden()
{
	if(getIsAnimating())
		return false;
	if(myStateAnim.getValue() <= 0.0)
		return true;
	else
		return false;
}
/*****************************************************************************/
bool UIElement::getIsFullyShown()
{
	if(getIsAnimating())
		return false;
	if(myStateAnim.getValue() >= 1.0)
		return true;
	else
		return false;
}
/*****************************************************************************/
bool UIElement::getIsAnimating()
{
	if(myStateAnim.getEndTime() < Application::getInstance()->getGlobalTime(getClockType()))
		return false;
	else
		return true;
}
/*****************************************************************************/
void UIElement::getRawPositions(SVector2D& svOffOut, SVector2D& svOnOut, SVector2D& svOff2Out,
								FLOAT_TYPE& fOffScale, FLOAT_TYPE& fOnScale, FLOAT_TYPE& fOff2Scale)
{

	if(!myValidRawPosCache)
	{
#ifdef _DEBUG
	    if(strcmp(this->getStringProp(PropertyId), "toolParentWindow") == 0)
	    {
		int bp = 0;
	    }
#endif
		myOnPosCache.x = this->getNumProp(PropertyX);
		myOnPosCache.y = this->getNumProp(PropertyY);

		myOffPosCache = myOnPosCache;

		if(this->doesPropertyExist(PropertyOffPosX))
		    myOffPosCache.x = this->getNumProp(PropertyOffPosX);
		if(this->doesPropertyExist(PropertyOffPosY))
		    myOffPosCache.y = this->getNumProp(PropertyOffPosY);

		if(this->doesPropertyExist(PropertyOffScale))
			myOffScaleCache = this->getNumProp(PropertyOffScale);
		else
			myOffScaleCache = 1.0;

		if(this->doesPropertyExist(PropertyOffOutPosX))
		{
			myOff2PosCache.x = this->getNumProp(PropertyOffOutPosX);
			myOff2PosCache.y = this->getNumProp(PropertyOffOutPosY);
		}
		else
			myOff2PosCache = myOffPosCache;

		if(this->doesPropertyExist(PropertyOffOutScale))
			myOff2ScaleCache = this->getNumProp(PropertyOffOutScale);
		else
			myOff2ScaleCache = myOffScaleCache;


		if(this->doesPropertyExist(PropertyScale))
			myOnScaleCache = this->getNumProp(PropertyScale);
		else
			myOnScaleCache = 1.0;

		FLOAT_TYPE fDummy;
		getRawOpacitites(fDummy, fDummy, fDummy);

		myValidRawPosCache = true;
	}

	svOffOut = myOffPosCache;
	svOnOut = myOnPosCache;
	svOff2Out = myOff2PosCache;

	fOffScale = myOffScaleCache;
	fOff2Scale = myOff2ScaleCache;
	fOnScale = myOnScaleCache;

	if(myMotionEffect)
	{
		// Apply the effect.
		myMotionEffect->evaluate(svOnOut);
	}

}
/*****************************************************************************/
UIElement* UIElement::getChildAtRecursive(FLOAT_TYPE fX, FLOAT_TYPE fY, bool bFirstNonPassthrough, bool bGlidingOnly)
{
	SVector2D svPos;
	SVector2D svPoint(fX, fY);
	// We get it starting at the parent, since we account for our
	// own position in the recursive call below.
	UIElement* pParent = FAST_CAST<UIElement*>(this->getParent());
	if(pParent)
	{
		this->getGlobalPosition(svPos);
		// Subtract own scale from it since
		// it will re-added in the rec call below,
		// and if we don't do that, we'll add it twice.
		SVector2D svLocPos;
		FLOAT_TYPE fLocScale;
		this->getLocalPosition(svLocPos, NULL, &fLocScale);
		svPos = svPos - svLocPos*fLocScale;
	}
	else
	{
		SVector2D svWndSize;
		getParentWindow()->getSize(svWndSize);
		svPos += svWndSize*0.5;
	}

	// Special case: if we have an opened drop down menu, it overrides
	// pretty much anything else. So first we find if that's the case,
	// and if so, whether we're in the bbox of that guy. This is necessary
	// because the drop down part may extend beyond the parent element,
	// and otherwise we'll never test it.
	// We need to go through all drop downs and test them...
	UIElement* pDDResult = getUIPlane()->testShownDropdownsRecursive(svPoint, bFirstNonPassthrough, bGlidingOnly);
//	UIElement* pDDResult = testShownDropdownsRecursive(svPoint, bFirstNonPassthrough, bGlidingOnly);
	if(pDDResult)
		return pDDResult;

	return this->getChildAtRecursiveInternal(svPoint, svPos, bFirstNonPassthrough, false, bGlidingOnly);
}
/*****************************************************************************/
UIElement* UIElement::testShownDropdownsRecursive(const SVector2D& svPoint, bool bFirstNonPassthrough, bool bGlidingOnly)
{
	// We need this since testShownDropdownsRecursive() may call getChildAtRecursive() on us, which
	// wilil call testShownDropdownsRecursive(), and so on, in an infinite loop.
	if(myIsTestingDropdowns)
		return NULL;

	myIsTestingDropdowns = true;

	// Test self, then call on children
	UIDropdownMenu* pDropdown = dynamic_cast<UIDropdownMenu*>(this);
	if( (pDropdown || this->getBoolProp(PropertyIsDropdownPart))
		&& this->getIsVisible() && this->getIsEnabled() && this->getIsFullyShown())
	{
		UIElement* pDDResult = this->getChildAtRecursive(svPoint.x, svPoint.y, bFirstNonPassthrough, bGlidingOnly);
		if(pDDResult)
		{
			// Now, we may actually be over the scrollbar of that drop down
			UIElement* pCurrBar;
			UIElement* pBarRes;
			LinkedToSelfElemIterator ei;
			for(ei = pDDResult->linkedToSelfElemIteratorFirst(); !ei.isEnd(); ei++)
			{
				pCurrBar = ei.getItem();
				pBarRes = pCurrBar->getChildAtRecursive(svPoint.x, svPoint.y, bFirstNonPassthrough, bGlidingOnly);
				if(pBarRes)
				{
					pDDResult = pBarRes;
					break;
				}
			}

			myIsTestingDropdowns = false;
			return pDDResult;
		}
	}

	// Otherwise, go through children.
	UIElement* pFoundSubchild = NULL;
	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	//for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	// Start from the other end so that topmost children get first clicks.
	for(iCurrChild = iNumChildren - 1; iCurrChild >= 0; iCurrChild--)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;

		if(!pChild->getIsVisible())
			continue;

		pFoundSubchild = pChild->testShownDropdownsRecursive(svPoint, bFirstNonPassthrough, bGlidingOnly);
		if(pFoundSubchild)
		{
			myIsTestingDropdowns = false;
			return pFoundSubchild;
		}
	}

	myIsTestingDropdowns = false;
	return NULL;
}
/*****************************************************************************/
UIElement* UIElement::getChildAtRecursiveInternal(const SVector2D& svPoint, const SVector2D& svScroll, bool bFirstNonPassthrough, bool bIgnoreChildren, bool bGlidingOnly)
{
	SRect2D srSelfRect;

#ifdef _DEBUG
	if(!bGlidingOnly)
	{
		if(this->doesPropertyExist(PropertyId) && IS_STRING_EQUAL(this->getStringProp(PropertyId), "testWnd"))
		{
			int bp = 0;
		}
		if(this->doesPropertyExist(PropertyId) && IS_STRING_EQUAL(this->getStringProp(PropertyId), "gradTest"))
		{
			int bp = 0;
		}

	}
#endif

	if(!this->getIsVisible())
		return NULL;

	FLOAT_TYPE fLocScale;
	SVector2D svPos;
	this->getLocalPosition(svPos, NULL, &fLocScale);

	SVector2D svTotalOffset;
	if(this->getParent())
		svTotalOffset = svScroll + svPos*fLocScale;
	else
		svTotalOffset = svScroll + svPos;

	// Note that we have to re-add the parent's extra scrolling, if any.
	// Should it be recursive..?
	// This is still wrong - the title in the window offsets the UI,too.
	// But in that case, somehow we don't need to subtract it...
	// So the problem is that we have a parmHost which itself contains mainWin with actual parms.
	// We scroll parmHOst, but mainWin is non-stretchy and is the same size as parmHost. So when we scroll
	// too far, we run out of it...
//	SVector2D svParentExtraScrolling;
// 	if(this->getParent())
// 		 this->getParent<UIElement>()->getExtraShiftOffset(svParentExtraScrolling);

	// Box offset!
	SVector2D svBoxOffset;
	this->getBoxOffset(svBoxOffset);

	srSelfRect.w = this->getNumProp(PropertyWidth);
	srSelfRect.h = this->getNumProp(PropertyHeight);
	srSelfRect.x = svTotalOffset.x - srSelfRect.w/2.0 + svBoxOffset.x;
	srSelfRect.y = svTotalOffset.y - srSelfRect.h/2.0 + svBoxOffset.y;

	// Go through all the children
	if(!srSelfRect.doesContain(svPoint))
		return NULL;
	else if(bIgnoreChildren)
	{
		if(this->getBoolProp(PropertyIsPassthrough))
			return NULL;
		else
			return this;

	}

	UIElement* pFoundSubchild = NULL;
	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	//for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	// Start from the other end so that topmost children get first clicks.
	for(iCurrChild = iNumChildren - 1; iCurrChild >= 0; iCurrChild--)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
/*
		if(!pChild)
		{
			// Something didn't work - we have a non-UIElement child.
			_ASSERT(0);
			continue;
		}
*/

		if(pChild->getIsVisible() == false)
			continue;

		if(pChild->getStateAnimProgress() < FLOAT_EPSILON)
			continue;

		if(bFirstNonPassthrough)
			pFoundSubchild = pChild->getChildAtRecursiveInternal(svPoint, svTotalOffset, false, true, bGlidingOnly);
		else
			pFoundSubchild = pChild->getChildAtRecursiveInternal(svPoint, svTotalOffset, false, false, bGlidingOnly);
		if(pFoundSubchild)
			break;
	}

	if(pFoundSubchild)
		return pFoundSubchild;
	else
	{
		if(this->getBoolProp(PropertyIsPassthrough))
			return NULL;
		else
			return this;
	}
}
/*****************************************************************************/
void UIElement::defineLayersOnChildren(const char* pcsLayerName, int iLevel)
{
	if(iLevel > 0)
	{
		const char* pcsCurrName = NULL;
		if(this->doesPropertyExist(PropertyLayer))
			pcsCurrName = this->getStringProp(PropertyLayer);
		if(!pcsCurrName || strlen(pcsCurrName) <= 0 || strcmp(pcsCurrName, PROPERTY_NONE) == 0)
			this->setStringProp(PropertyLayer, pcsLayerName);
	}

	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		pChild->defineLayersOnChildren(pcsLayerName, iLevel + 1);
	}
}
/*****************************************************************************
void UIElement::setTextDataSource(DataSource& rSource)
{
	myTextDataSource = rSource;
}
/*****************************************************************************
void UIElement::setObjAnimDataSource(DataSource& rSource)
{
	myObjAnimDataSource = rSource;
}
/*****************************************************************************/
UIElement* UIElement::getChildById(const char *pcsId, bool bRecurse, bool bIncludeOldDefType)
{
	UIElement* pChild, *pCElem;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
/*
		if(!pChild)
		{
			// Something didn't work - we have a non-UIElement child.
			_ASSERT(0);
			continue;
		}
*/

		if(strcmp(pChild->getStringProp(PropertyId), pcsId) == 0)
			return pChild;
		else if(bIncludeOldDefType && pChild->doesPropertyExist(PropertyOldId) && strcmp(pChild->getStringProp(PropertyOldId), pcsId) == 0)
			return pChild;
		else if(bRecurse)
		{
			pCElem = pChild->getChildById(pcsId, true, bIncludeOldDefType);
			if(pCElem)
				return pCElem;
		}
	}

	return NULL;

}
/*****************************************************************************/
UIElement* UIElement::getChildByElemType(const char *pcsElemType, bool bRecurse)
{
    UIElement* pChild, *pCElem;
    int iCurrChild, iNumChildren = this->getNumChildren();
    for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
    {
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;

		if(strcmp(pChild->getStringProp(PropertyElemType), pcsElemType) == 0)
			return pChild;
		else if(bRecurse)
		{
			pCElem = pChild->getChildByElemType(pcsElemType, true);
			if(pCElem)
			return pCElem;
		}
    }

    return NULL;

}
/*****************************************************************************/
void UIElement::resetSliderChildrenVisibility()
{
	UIElement* pChild;
	const char* pcsChildName;
	int iCurr, iNum = this->getEnumPropCount(PropertyUioSlidingChildren);
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pcsChildName = this->getEnumPropValue(PropertyUioSlidingChildren, iCurr);
		// Get immediate children only.
		pChild = this->getChildById(pcsChildName, false);

		if(!pChild)
			ASSERT_CONTINUE;

		// If the child isn't a currently selected element, set it to be invisible.
		pChild->setIsVisible(iCurr == myCurrentSliderIndex);

		// Also, make sure we set its state animation progress value to 0.0.
		if(iCurr != myCurrentSliderIndex)
			pChild->myStateAnim.setNonAnimValue(0.0);
	}
}
/*****************************************************************************/
void UIElement::resetSliderPosition()
{
	if(myCachedHasChildSliders == false)
		return;

	const char* pcsCurrChildName = this->getEnumPropValue(PropertyUioSlidingChildren, myCurrentSliderIndex);
	const char* pcsAppearingChildName = this->getEnumPropValue(PropertyUioSlidingChildren, 0);

	UIElement* pCurrChildElem = this->getChildById(pcsCurrChildName, false);
	UIElement* pNextChildElem = this->getChildById(pcsAppearingChildName, false);

	if(pCurrChildElem != pNextChildElem)
	{
		SVector2D svElemSize, svCurrCenter;
		svElemSize.x = pCurrChildElem->getNumProp(PropertyWidth);
		svElemSize.y = pCurrChildElem->getNumProp(PropertyHeight);

		// Instead, we get the on position:
		svCurrCenter.x = pCurrChildElem->getNumProp(PropertyX);
		svCurrCenter.y = pCurrChildElem->getNumProp(PropertyY);

		SVector2D svNextChildPos;
		svNextChildPos.x = svCurrCenter.x + (svElemSize.x);
		svNextChildPos.y = svCurrCenter.y;

		pNextChildElem->setNumProp(PropertyX, svCurrCenter.x);
		pNextChildElem->setNumProp(PropertyY, svCurrCenter.y);
		pNextChildElem->setNumProp(PropertyOffPosX, svNextChildPos.x);
		pNextChildElem->setNumProp(PropertyOffPosY, svNextChildPos.y);

		pCurrChildElem->hide(true);
		pNextChildElem->show(true, NULL, AnimOverActionNone, false);

		pCurrChildElem->setIsVisible(false);
		pNextChildElem->setIsVisible(true);
	}

	// Note: we must tell us to refresh the data, too.
	SUpdateInfo rRefreshInfo;
	rRefreshInfo.myPosition = 0;
	pNextChildElem->updateDataRecursive(rRefreshInfo);

	myCurrentSlidePos = 0;
	myCurrentSliderIndex = 0;
}
/*****************************************************************************/
void UIElement::slide(int iDir, int iTableRowCalling, bool bIsVertical)
{
	// Get our slider props. Note that we need to store our current slider
	// position somehow.
	bool bIsInfinite = this->getBoolProp(PropertyUioIsSliderInfinite);

	int iNumSliders = this->getEnumPropCount(PropertyUioSlidingChildren);
	int iNewSliderIndex = myCurrentSliderIndex + iDir;
	int iNewSliderPos = myCurrentSlidePos + iDir;

	int iNumInfSides = this->getNumInfiniteSlides();

	bool bIsValid = true;
	if(bIsInfinite)
	{
		iNewSliderPos = iNewSliderPos % iNumInfSides;
		iNewSliderIndex = iNewSliderIndex % iNumSliders;

		if(iNewSliderIndex < 0)
			iNewSliderIndex += iNumSliders;
		if(iNewSliderPos < 0)
			iNewSliderPos += iNumInfSides;
	}
	else
	{
		if(iNewSliderIndex < 0 || iNewSliderIndex >= iNumSliders)
			bIsValid =  false;
	}

	if(!bIsValid)
		return;

	const char* pcsCurrChildName = this->getEnumPropValue(PropertyUioSlidingChildren, myCurrentSliderIndex);
	const char* pcsAppearingChildName = this->getEnumPropValue(PropertyUioSlidingChildren, iNewSliderIndex);

	UIElement* pCurrChildElem = this->getChildById(pcsCurrChildName, false);
	UIElement* pNextChildElem = this->getChildById(pcsAppearingChildName, false);

	// Position it, set the animation. We assume the child is the same as the currently
	// shown element in size.
	SVector2D svElemSize, svCurrCenter;
	svElemSize.x = pCurrChildElem->getNumProp(PropertyWidth);
	svElemSize.y = pCurrChildElem->getNumProp(PropertyHeight);

	// Instead, we get the on position:
	svCurrCenter.x = pCurrChildElem->getNumProp(PropertyX);
	svCurrCenter.y = pCurrChildElem->getNumProp(PropertyY);

	//FLOAT_TYPE fOffset = upToScreen(20.0);
	FLOAT_TYPE fOffset = 0;

	SVector2D svNextChildPos, svGoneChildPos;
	if(bIsVertical)
	{
		svNextChildPos.x = svCurrCenter.x;
		svNextChildPos.y = svCurrCenter.y + (svElemSize.y + fOffset)*iDir;

		svGoneChildPos.x = svCurrCenter.x;
		svGoneChildPos.y = svCurrCenter.y + (svElemSize.y + fOffset)*(iDir*-1);

	}
	else
	{
		svNextChildPos.x = svCurrCenter.x + (svElemSize.x + fOffset)*iDir;
		svNextChildPos.y = svCurrCenter.y;

		svGoneChildPos.x = svCurrCenter.x + (svElemSize.x + fOffset)*(iDir*-1);
		svGoneChildPos.y = svCurrCenter.y;
	}
	// Shamelessly reset its positional properties.
	pNextChildElem->setNumProp(PropertyX, svCurrCenter.x);
	pNextChildElem->setNumProp(PropertyY, svCurrCenter.y);
	pNextChildElem->setNumProp(PropertyOffPosX, svNextChildPos.x);
	pNextChildElem->setNumProp(PropertyOffPosY, svNextChildPos.y);

#ifdef SLIDE_WITH_FADE
	pNextChildElem->setNumProp(PropertyOpacity, 1.0);
	pNextChildElem->setNumProp(PropertyOffOpacity, 0.0);
	pNextChildElem->setNumProp(PropertyOffOutOpacity, 0.0);

	pCurrChildElem->setNumProp(PropertyOpacity, 1.0);
	pCurrChildElem->setNumProp(PropertyOffOpacity, 0.0);
	pCurrChildElem->setNumProp(PropertyOffOutOpacity, 0.0);
#endif

	pCurrChildElem->setNumProp(PropertyX, svCurrCenter.x);
	pCurrChildElem->setNumProp(PropertyY, svCurrCenter.y);
	pCurrChildElem->setNumProp(PropertyOffOutPosX, svGoneChildPos.x);
	pCurrChildElem->setNumProp(PropertyOffOutPosY, svGoneChildPos.y);

	// Reset their caches
	pNextChildElem->resetPosCache(true);
	pCurrChildElem->resetPosCache(true);

	// Override transition times
	pCurrChildElem->setNumProp(PropertyTransitionTime, WINDOW_SLIDE_TIME);
	pNextChildElem->setNumProp(PropertyTransitionTime, WINDOW_SLIDE_TIME);

	pCurrChildElem->setIsVisible(true);
	pNextChildElem->setIsVisible(true);

	SUpdateInfo rRefreshInfo;
	rRefreshInfo.myIsSliding = true;
	if(iTableRowCalling >= 0)
		rRefreshInfo.myPosition = iTableRowCalling;
	else
		rRefreshInfo.myPosition = iNewSliderPos;
	pNextChildElem->updateDataRecursive(rRefreshInfo);

	AnimOverActionType eHideAction = AnimOverActionNone;
	AnimOverActionType eShowAction = AnimOverActionNone;
	AnimationOverCallback* pHideCallback = NULL;
	AnimationOverCallback* pShowCallback = NULL;

	int iOutTransTime = pCurrChildElem->getNumProp(PropertyTransitionTime);
	int iNextTransTime = pNextChildElem->getNumProp(PropertyTransitionTime);

	if(iNextTransTime > iOutTransTime)
	{
		pShowCallback = this;
		eShowAction = AnimOverActionFinishedSliding;
	}
	else
	{
		pHideCallback = this;
		eHideAction = AnimOverActionFinishedSliding;
	}

	// Now, tell one to show and the other one to hide.
	pCurrChildElem->hide(false, pHideCallback, eHideAction);
	pNextChildElem->show(false, pShowCallback, eShowAction, false);

	myLeavingSlideChild = pCurrChildElem;
	myIncomingSlideChild = pNextChildElem;

	myCurrentSlidePos = iNewSliderPos;
	myCurrentSliderIndex = iNewSliderIndex;
}
/*****************************************************************************/
void UIElement::animationOver(AnimatedValue *pAValue, string* pData)
{
	IPlaneObject::animationOver(pAValue, pData);

	// Now, see what the action was
	if(pAValue->getAnimOverAction() == AnimOverActionFinishedSliding &&
		myLeavingSlideChild && myIncomingSlideChild)
	{
		myLeavingSlideChild->setIsVisible(false);

		// We can stop drawing our own region now.
		myLeavingSlideChild = NULL;
		myIncomingSlideChild = NULL;
	}
	else if(pAValue == &myLifeTimer)
	{
		this->getUIPlane()->hideUI(this->getStringProp(PropertyLayer));
	}

}
/*****************************************************************************/
bool UIElement::isChildLeaving(UIElement* pChild)
{
	if(myLeavingSlideChild == pChild)
		return true;
	else
		return false;
}
/*****************************************************************************/
bool UIElement::isChildEntering(UIElement* pChild)
{
	if(myIncomingSlideChild == pChild)
		return true;
	else
		return false;
}
/*****************************************************************************/
void UIElement::preRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	// If we support auto-scroll, start the scissor rect
	if(myCachedDoesSupportAutoScrolling)
	{
		SRect2D srGlobalRect;
		getGlobalRectangle(srGlobalRect);

		SVector2D svSizeOffset, svShiftOffset;
		RenderUtils::beginScissorRectangle(srGlobalRect.x + svShiftOffset.x, srGlobalRect.y + svShiftOffset.y, srGlobalRect.w + svSizeOffset.x, srGlobalRect.h + svSizeOffset.y, getDrawingCache());
	}

	// If we have a parent, see if we're sliding.
	UIElement* pParent = FAST_CAST<UIElement*>(this->getParent());
	if(pParent)
	{
		if(pParent->isChildLeaving(this) || pParent->isChildEntering(this))
		{
			SRect2D srParent, srSelf, srOverlap;
			SVector2D svTemp;

			// Get the parent rectangle.
			FLOAT_TYPE fGlobScale;
			pParent->getGlobalPosition(svTemp, NULL, &fGlobScale);
			srParent.w = pParent->getNumProp(PropertyWidth)*fGlobScale;
			srParent.h = pParent->getNumProp(PropertyHeight)*fGlobScale;
			srParent.x = svTemp.x - srParent.w/2.0;
			srParent.y = svTemp.y - srParent.h/2.0;

			// Now, get self
			this->getGlobalPosition(svTemp, NULL, &fGlobScale);
			srSelf.w = this->getNumProp(PropertyWidth)*fGlobScale;
			srSelf.h = this->getNumProp(PropertyHeight)*fGlobScale;
			srSelf.x = svTemp.x - srSelf.w/2.0;
			srSelf.y = svTemp.y - srSelf.h/2.0;

			srSelf.getOverlap(srParent, srOverlap);

			if(srOverlap.w > 0 && srOverlap.h > 0)
				RenderUtils::beginStencilRectangle(getDrawingCache(), srOverlap.x, srOverlap.y,
						srOverlap.w, srOverlap.h);
		}


	}

}
/*****************************************************************************/
void UIElement::postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	// Finish the region if the need must be.
	//if(myLeavingSlideChild && myIncomingSlideChild)
	UIElement* pParent = FAST_CAST<UIElement*>(this->getParent());
	if(pParent)
	{
		if(pParent->isChildLeaving(this) || pParent->isChildEntering(this))
		{
			if(RenderUtils::isInsideStencil())
				RenderUtils::endStencil(getDrawingCache());
		}
	}

	if(myCachedDoesSupportAutoScrolling)
		RenderUtils::endScissorRectangle(getDrawingCache());

	renderBBox();
}
/*****************************************************************************/
int UIElement::getSliderPosition()
{
	return myCurrentSlidePos;
}
/*****************************************************************************/
bool UIElement::allowRendering()
{
	if(!myIsRenderable || !getIsVisible())
		return false;

	// Apparently, we also have to check for parents - so always render children?
	ResourceItem* pParent = getParent();
	if(pParent)
		return true;

	// Now, we're in the case where there's no parent.
	GTIME lCurrTime = Application::getInstance()->getGlobalTime(ClockUiPrimary);
	if(fabs(myStateAnim.getValue()) > FLOAT_EPSILON)
		return true;

	// Alternatively, if our state's end anim value is the current clock time, render even if
	// we're zero state. Otherwise, we never invoke the last checkTime() call in AnimValue,
	// and are always "active" in the activity manager after, say, a submenu is hidden.
	if(myStateAnim.getEndTime() == lCurrTime)
		return true;

	return false;

	/*
	// We chechk in rednering for this.
	return myIsRenderable && getIsVisible()

		&&
		(this->getParent()
		|| (fabs(myStateAnim.getValue(Application::getInstance()->getGlobalTime(ClockUiPrimary))) > FLOAT_EPSILON && !this->getParent())
		);
		*/
	//  && fabs(myStateAnim.getValue(Application::getInstance()->getGlobalTime(ClockUiPrimary))) > FLOAT_EPSILON;
}
/*****************************************************************************/
void UIElement::setPushedForRadioGroup(const char* pcsGroup, UIButtonElement* pException, bool bInstant)
{
    UIButtonElement* pAsButton = dynamic_cast<UIButtonElement*>(this);
    if(pAsButton && pException != pAsButton && this->doesPropertyExist(PropertyRadioGroup) &&
		strcmp(this->getRadioGroupName(), pcsGroup) == 0)
    {
		// Got it!
		pAsButton->setIsPushed(false, bInstant);
		// Note that we *don't* send the event out here. This is because it
		// will send it out for *each* radio group member, even if nothing changed,
		// and even if there are hundreds of them (as could be possible in the
		// thumbnail grid). This can cause us to spend a lot of time recomputing
		// (for example, we'll recompute the brush preview for every single cell).
		// Instead, we already have one call in UIButtonElement::onReleased()
		// that will send it out.
		pAsButton->handleTargetElementUpdate(false, false);

		UIPlane* pUIPlane = getUIPlane();

		// Special case - if the button's action was to show a layer, we hide it.
		if(strcmp(pAsButton->getStringProp(PropertyAction), UIA_SHOW_LAYER) == 0)
		{
			const char* pcsButtonLayer = pAsButton->getStringProp(PropertyActionValue);
			if(pUIPlane->getIsLayerShown(pcsButtonLayer))
				pUIPlane->hideUI(pcsButtonLayer);
		}
    }
}
/*****************************************************************************/
void UIElement::setPushedForRadioGroupRecursive(const char* pcsGroup, UIButtonElement* pException, bool bInstant)
{
    setPushedForRadioGroup(pcsGroup, pException, bInstant);

	// Now, do the children
	UIElement* pElem;
	int iCurr, iNum = this->getNumChildren();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pElem = FAST_CAST<UIElement*>(this->getChild(iCurr));
		pElem->setPushedForRadioGroupRecursive(pcsGroup, pException, bInstant);
	}
}
/*****************************************************************************/
FLOAT_TYPE UIElement::getTimeOffset(bool bIsForShowing)
{
	if(!bIsForShowing && this->doesPropertyExist(PropertyTimeOffsetWhenHiding))
		return this->getNumProp(PropertyTimeOffsetWhenHiding);
	return this->getNumProp(PropertyTimeOffset);
}
/*****************************************************************************/
void UIElement::setIsEnabled(bool bValue)
{
	myValidRawPosCache = false;
	myIsEnabled = bValue;
	// Reset the cache since enable opacity is multed into cached one.
	// Non-recursive since we'll be recusring anyway below.
	resetEvalCache(false);

	// Disable all children as well
	UIElement* pElem;
	int iCurr, iNum = this->getNumChildren();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pElem = FAST_CAST<UIElement*>(this->getChild(iCurr));
		if(!pElem)
			ASSERT_CONTINUE;

		pElem->setIsEnabled(bValue);
	}
}
/*****************************************************************************/
void UIElement::updateEnableStatusRecursive()
{
	if(getHaveValidTargetElement())
	{
		bool bIsEnabled = myTargetIdentifier->getIsEnabled(this);
		setIsEnabled(bIsEnabled);

		// See if we need to disabled controls linked to us:
		bool bDisableLinkedControls = false;
		if(bIsEnabled)
			bDisableLinkedControls = myTargetIdentifier->getDisableLinkedControls(this);

		UIElement* pCurrLinkedElem;

		// Also set any parents which are logical contianers
		// UPTO THE FIRST ONE! We don't want to disable the entire parent container...
		UIElement* pParent;
		for(pParent = this->getParent<UIElement>(); pParent; pParent = pParent->getParent<UIElement>())
		{
			if(pParent->getBoolProp(PropertyIsLogicalElemsContainer))
			{
				pParent->setIsEnabled(bIsEnabled);
				break;
			}
		}

		// Note that the above call is recursive, so we first set the parent,
		// then the linked elements, since some of the children such as sliders
		// may be disabled while the parent is enabled, and we don't want to override
		// that.
		LinkedToSelfElemIterator rIter;
		// Counts self as child, and that's it...
		for(rIter = linkedToSelfElemIteratorFirst(); !rIter.isEnd(); rIter++)
		{
			pCurrLinkedElem = rIter.getItem();
			// Enable/disable it, too:
			pCurrLinkedElem->setIsEnabled(bIsEnabled && !bDisableLinkedControls);
		}


		// Do not continue setting statuses for children.
		if(!bIsEnabled)
			return;
	}

	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;

		pChild->updateEnableStatusRecursive();
	}
}
/*****************************************************************************/
void UIElement::updateVisibleStatusRecursive()
{
	if(getHaveValidTargetElement())
	{
		bool bIsVisible = myTargetIdentifier->getIsVisible();
		setIsVisible(bIsVisible);

		UIElement* pCurrLinkedElem;
		LinkedToSelfElemIterator rIter;
		// Counts self as child, and that's it...
		for(rIter = linkedToSelfElemIteratorFirst(); !rIter.isEnd(); rIter++)
		{
			pCurrLinkedElem = rIter.getItem();
			// Enable/disable it, too:
			pCurrLinkedElem->setIsVisible(bIsVisible);
		}

		// Also set any parents which are logical contianers
		// UPTO THE FIRST ONE! We don't want to disable the entire parent container...
		if(!this->getBoolProp(PropertyIsLogicalElemsContainer))
		{
			UIElement* pParent;
			for(pParent = this->getParent<UIElement>(); pParent; pParent = pParent->getParent<UIElement>())
			{
				if(pParent->getBoolProp(PropertyIsLogicalElemsContainer))
				{
					pParent->setIsVisible(bIsVisible);
					break;
				}
			}
		}

		// Do not continue setting statuses for children.
		if(!bIsVisible)
			return;
	}

	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;

		pChild->updateVisibleStatusRecursive();
	}
}
/*****************************************************************************/
void UIElement::updateDataRecursive(SUpdateInfo& rRefreshInfo)
{
	this->updateOwnData(rRefreshInfo);

	updateChildDataRecursive(rRefreshInfo);

	this->onPostUpdateChildData();
}
/*****************************************************************************/
void UIElement::setOverlayColor(SColor& scolValue)
{
	myOverlayColor = scolValue;
	getTopAnimNameNoFrameNum(theSharedString3);
	SkinManager::getInstance()->mapOverlayColor(theSharedString3.c_str(), myOverlayColor);
	if(scolValue.r != FLOAT_TYPE_MAX)
		this->setAsColor(PropertyOverlayColor, scolValue);
	else if(this->doesPropertyExist(PropertyOverlayColor))
		this->removeProperty(PropertyOverlayColor);

}
/*****************************************************************************/
void UIElement::setRotation(FLOAT_TYPE fDegAngle)
{
	myRotation = fDegAngle;
	this->setNumProp(PropertyRotation, fDegAngle);
}
/*****************************************************************************/
void UIElement::updateChildDataRecursive(SUpdateInfo& rRefreshInfo)
{
	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;
		pChild->updateDataRecursive(rRefreshInfo);
	}
}
/*****************************************************************************/
void UIElement::resetOnReloadDebug()
{
	myCachedOwnDefinition = NULL;
}
/*****************************************************************************/
void UIElement::resetEvalCache(bool bRecursive)
{
	IPlaneObject::resetEvalCache(bRecursive);

	myIsCachedTextValid = false;
	myCachedPositionTime = 0;
	myCachedProgress = -1;
	myCachedOffOpacity = -1;
	resetPosCache(bRecursive);

	markOffsetsDirtyRecursive();
}
/*****************************************************************************/
void UIElement::applyEffect(MotionEffectType eType, FLOAT_TYPE fSecDuration)
{
	if(myMotionEffect)
		delete myMotionEffect;
	myMotionEffect = IMotionEffect::createEffect(eType, fSecDuration);
}
/*****************************************************************************/
FLOAT_TYPE UIElement::getScrollPixelTolerance()
{
	return PIXEL_TOLERANCE;
}
/*****************************************************************************/
void UIElement::adjustToScreen()
{
	// Note: there's a similar copy in GameEngine::adjustOverlayUI().

	// See if this guy has hints. If not, we can't do anything.

	// Except check if he needs to be re-sized to the actual screen size.
	SVector2D svScreenSize;
	getParentWindow()->getSize(svScreenSize);

	// Also, somewhere need to make sure that elements meant to be
	// full screen are actually full-screen.
	SVector2D svCurrSize, svOrigSize;
	//FLOAT_TYPE fMultFactor = upToScreenUnclamped(100.0, getParentWindow())/100.0;
	FLOAT_TYPE fMultFactorClamped = upToScreenOriginal(100.0)/100.0;

	// These elements were created from text from scratch, and, by definition,
	// have no definition. :)
	if(this->getHasNoDefinition())
		return;

	bool bDidChangeSize = false;
	ResourceItem *pOwnDef = this->getOwnDefinition();
	_ASSERT(pOwnDef);

	svCurrSize.x = pOwnDef->getNumProp(PropertyWidth);
	svCurrSize.y = pOwnDef->getNumProp(PropertyHeight);

	bool bDoResetCachedDims = false;
	FLOAT_TYPE fFillParentXLessPadding = getFillParentXLessPadding();
	FLOAT_TYPE fFillParentYLessPadding = getFillParentYLessPadding();


	//	this->getBoxSize(svCurrSize);
	svOrigSize = svCurrSize*(1.0/fMultFactorClamped);

	const FLOAT_TYPE fReferenceScreenW = 480;
	const FLOAT_TYPE fReferenceScreenH = 320;
	// Multiplier to roughly set the distance from the
	// edges to be the same as on the iPad or iPhone 4.
	// As the screen grows (but no the button size) we don't
	// want to keep offsetting them further and further,
	// as would be the case with a simple ratio.
	// This assumes the button size is roughly meant
	// for an iPad.
	//const FLOAT_TYPE fMaxSpacingMult = min(2.0f, fMultFactor);
	//const FLOAT_TYPE fMaxSpacingMult = 2.0;
	// Since now we're not getting the screen multiple from the simple window resolution,
	// but it is an actual density multiplier, just use it. Use 2.0*it because in this app
	// we actually assume 2.0* for the screen multiplier of 1.
	const FLOAT_TYPE fMaxSpacingMult = 2.0*(FLOAT_TYPE)getScreenDensityScalingFactor();
	//const FLOAT_TYPE fVertMult = (svScreenSize.y*fReferenceScreenW)/(fReferenceScreenH*svScreenSize.x);
	const FLOAT_TYPE fVertMult = 1.0;

#ifdef _DEBUG
	if(IS_OF_TYPE("testElem"))
	{
		int bp = 0;
	}
#endif

	// Hardcode the original reference resolution since
	// we're comparing the original screen
	bool bForceCenteringX = false;
	bool bForceCenteringY = false;

	SVector2D svCurrAlignToSize;

	UIElement* pParent = getParent<UIElement>();
	if(pParent)
	{
		// See what the parent's size is.
		SVector2D svParentSize;
		FAST_CAST<UIElement*>(pParent)->getBoxSize(svParentSize);
		svCurrAlignToSize = svParentSize;
	}
	else
		svCurrAlignToSize = svScreenSize;

	// Note that somewhere (with the viewer) we hit a case where mutliple nested splitters
	// would not resize correctly when dragged. Sometimes. The below checks were added
	// which fixed the problem, but broke other splitters which only contained
	// regular windows in Bloom.
	// When disabled again, the problem was not reproducible. But keep this in mind.

	// See if we fill our guys...
	if(fFillParentXLessPadding >= 0.0)
	{
		this->setNumProp(PropertyWidth, svCurrAlignToSize.x - fFillParentXLessPadding*2.0);
		bDidChangeSize = true;
		bDoResetCachedDims = true;
		if(!pParent)
			bForceCenteringX = true;
	}

#ifdef _DEBUG
	if(IS_OF_TYPE("toolSplitter"))
	{
		int bp = 0;
	}
	if(IS_OF_TYPE("infoSplitter"))
	{
		int bp = 0;
	}
#endif

	if(fFillParentYLessPadding >= 0.0)
	{
		this->setNumProp(PropertyHeight, svCurrAlignToSize.y - fFillParentYLessPadding*2.0);
		bDidChangeSize = true;
		bDoResetCachedDims = true;
		if(!pParent)
			bForceCenteringY = true;
	}

	bool bIsTableCell = as<UITableCellElement>(this) != NULL;
	if(!bIsTableCell && (bForceCenteringX || bForceCenteringY || this->doesPropertyExist(PropertyAlign)))
	{

		// Then, see what the hints are.
		bool bHaveHorAlign = false;
		bool bHaveVertAlign = false;
		HorAlignType eHorHint;
		VertAlignType eVertHint;
		bool bHaveHintsProp = this->doesPropertyExist(PropertyAlign);

		if(bHaveHintsProp)
		{
			theCommonString = this->getEnumPropValue(PropertyAlign, 0);
			eHorHint = mapStringToType<HorAlignType>(theCommonString.c_str(), g_pcsHorAlignStrings, HorAlignCenter);
			bHaveHorAlign = (theCommonString != PROPERTY_NONE);
		}
		if(bForceCenteringX && !bHaveHorAlign)
		{
			eHorHint = HorAlignCenter;
			bHaveHorAlign = true;
		}

		if(bHaveHintsProp)
		{
			theCommonString = this->getEnumPropValue(PropertyAlign, 1);
			eVertHint = mapStringToType<VertAlignType>(theCommonString.c_str(), g_pcsVertAlignStrings, VertAlignCenter);
			bHaveVertAlign = (theCommonString != PROPERTY_NONE);
		}
		if(bForceCenteringY && !bHaveVertAlign)
		{
			eVertHint = VertAlignCenter;
			bHaveVertAlign = true;
		}

		if(myHasBeenPlacedManually)
			bHaveVertAlign = bHaveHorAlign = false;

		SVector2D svPos, svOrigPos;
		svPos.x = pOwnDef->getNumProp(PropertyX);
		svPos.y = pOwnDef->getNumProp(PropertyY);
		svOrigPos = svPos*(1.0/fMultFactorClamped);


		// See where the element would be, in terms of ratio, on a
		// 3:2 ratio.
		FLOAT_TYPE fHorOrigDist, fVertOrigDist;

#if defined(WIN32) && defined(_DEBUG)
		if(IS_OF_TYPE("testElem"))
		{
			int bp = 0;
		}
		if(IS_OF_TYPE("testElem2"))
		{
			int bp = 0;
		}
		if(IS_OF_TYPE("mainWnd"))
		{
			int bp = 0;
		}
		if(IS_OF_TYPE("imgWndStatusBar"))
		{
			int bp = 0;
		}
#endif
		if(eHorHint == HorAlignLeft)
			fHorOrigDist = -svCurrAlignToSize.x/2.0 + svPos.x + svCurrSize.x/2.0;
		else if(eHorHint == HorAlignRight)
			fHorOrigDist = svCurrAlignToSize.x/2.0 + svPos.x - svCurrSize.x/2.0;
		else
			fHorOrigDist = svOrigPos.x*fMultFactorClamped;

		if(eVertHint == VertAlignTop)
			fVertOrigDist = -svCurrAlignToSize.y/2.0 + svPos.y + svCurrSize.y/2.0;
		else if(eVertHint == VertAlignBottom)
			fVertOrigDist = svCurrAlignToSize.y/2.0 + svPos.y - svCurrSize.y/2.0;
		else
			fVertOrigDist = svOrigPos.y*fMultFactorClamped*fVertMult;

		// Then, align it to correct edges. Note that this will be screen
		// for elements with no parent or parent edges.

		// We need to do shifts because we don't know where the off position is.
		FLOAT_TYPE fShift;
		if(bHaveHorAlign)
		{
			fShift = fHorOrigDist - svPos.x;
			this->setNumProp(PropertyX, fShift + pOwnDef->getNumProp(PropertyX));
			if(this->doesPropertyExist(PropertyOffPosX))
				this->setNumProp(PropertyOffPosX, fShift + pOwnDef->getNumProp(PropertyOffPosX));
			if(this->doesPropertyExist(PropertyOffOutPosX))
				this->setNumProp(PropertyOffOutPosX, fShift + pOwnDef->getNumProp(PropertyOffOutPosX));
			bDoResetCachedDims = true;
		}

		if(bHaveVertAlign)
		{
			fShift = fVertOrigDist - svPos.y;
			this->setNumProp(PropertyY, fShift + pOwnDef->getNumProp(PropertyY));
			if(this->doesPropertyExist(PropertyOffPosY))
				this->setNumProp(PropertyOffPosY, fShift + pOwnDef->getNumProp(PropertyOffPosY));
			if(this->doesPropertyExist(PropertyOffOutPosY))
				this->setNumProp(PropertyOffOutPosY, fShift + pOwnDef->getNumProp(PropertyOffOutPosY));
			bDoResetCachedDims = true;
		}

	} // if we has hints

	// Now, if we have a parent that arranges ourselves, ask for our size and position:
	if(pParent && pParent->getChildrenLayoutType() == AutolayoutRow)
	{
	    SChildSizingInfo rMyInfo;
	    if(FAST_CAST<UIElement*>(pParent)->getChildSpacingInfo(this, rMyInfo))
	    {
			if(!rMyInfo.myIsHidden)
			{
				this->setNumProp(PropertyWidth, rMyInfo.myPixelWidth);
				this->setNumProp(PropertyX, rMyInfo.myCumulStartPos + rMyInfo.myPixelWidth/2.0);
				this->setNumProp(PropertyOffPosX, rMyInfo.myCumulStartPos + rMyInfo.myPixelWidth/2.0);
				this->setNumProp(PropertyOffOutPosX, rMyInfo.myCumulStartPos + rMyInfo.myPixelWidth/2.0);
				bDidChangeSize = true;
				bDoResetCachedDims = true;
			}
	    }
	}

	// Reset *all* caches.
	// We may have changed the size, so reset it anyway. It's
	// not a big performance hit.
	if(bDoResetCachedDims)
		this->resetEvalCache(true);

	ensureFlexibleWidthValid();

	markOffsetsDirty();

	// Auto layout the children, but just the immediate ones, so that when
	// we go to adjust them to screen, they have a valid size if they're a splitter.
	// Note that we don't need the recursive autolayout call here because
	// the current call is already recursive
	autoLayoutChildren();
	adjustChildrenToScreen();

	if(bDidChangeSize)
	{
		onSizeChangedInternal();
		onSizeChanged();
	}

	// If we're a popup, reposition:
	if(this->doesPropertyExist(PropertyPopupSourceUiElement))
	{
		UIElement* pRelTo = getUIPlane()->getElementById(this->getStringProp(PropertyPopupSourceUiElement));
		SideType eSide = mapStringToType(this->getStringProp(PropertyPopupPosition), g_pcsSideStrings, SideTop);
		positionAsPopupRelativeTo(pRelTo, eSide, this->getNumProp(PropertyPopupOffset), false);
	}

}
/*****************************************************************************/
void UIElement::adjustChildrenToScreen()
{

	// See if we need to adjust children
	if(this->getChildrenLayoutType() == AutolayoutRow)
	    recomputeChildSizes();

	// Call self on all children
	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;

		pChild->adjustToScreen();
	}
}
/*****************************************************************************/
void UIElement::setStateAnimCallbackData(const char* pcsData)
{
	myStateAnim.setCallbackData(pcsData);
}
/*****************************************************************************/
FLOAT_TYPE UIElement::getStateAnimProgress()
{
	return myStateAnim.getValue();
}
/*****************************************************************************/
bool UIElement::getIsHiding()
{
	if(myStateAnim.getEndTime() <= Application::getInstance()->getGlobalTime(ClockUiPrimary))
		return false;
	else
	{
		// See if our value is going from greater to lesser
		if(myStateAnim.getStartValue() > myStateAnim.getEndValue())
			return true;
		else
			return false;
	}
}
/*****************************************************************************/
ResourceItem* UIElement::getOwnDefinition()
{
#ifdef _DEBUG
	if(strcmp(this->getStringProp(PropertyId), "srcFile") == 0)
	{
		int bp = 0;
	}
#endif

	if(myCachedOwnDefinition)
		return myCachedOwnDefinition;

    // First, try our current name
	const char* pcsOwnId = this->getStringProp(PropertyId);
    ResourceItem* pRes = getOwnDefinitionInternal(pcsOwnId);
    if(!pRes)
    {
		// We  may be a dynamically created element, in which case our type name
		// will be our template name with some digits at the end. Strip the digits
		// and look that up.
		const char* pcsOldId = this->getStringProp(PropertyOldId);
		pRes = getOwnDefinitionInternal(pcsOldId);

		// If this fails too, just do a blind search through all UI collections.
		// Note that this may be extremely slow.
		if(!pRes)
			pRes = UIElement::getDefinitionBlindSearch(getUIPlane()->getInitCollectionType(), pcsOldId);

/*
		string strTempName(pcsOwnTypeName);
		gStripEndingDigits(strTempName);
		pRes = getOwnDefinitionInternal(strTempName.c_str());

		// If this fails too, just do a blind search through all UI collections.
		// Note that this may be extremely slow.
		if(!pRes)
			pRes = getDefinitionBlindSearch(strTempName.c_str());
			*/
    }

	myCachedOwnDefinition = pRes;
    return pRes;
}
/*****************************************************************************/
void UIElement::setCachedOwnDef(ResourceItem* pDef, bool bRecursive)
{
	myCachedOwnDefinition = pDef;

	if(!pDef || !bRecursive)
		return;

	ResourceItem* pSubDef;
	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = as<UIElement>(this->getChild(iCurrChild));

		pSubDef = pDef->getChildById(pChild->getStringProp(PropertyId), false);
		if(!pSubDef)
			pSubDef = pDef->getChildById(pChild->getStringProp(PropertyOldId), false);

		pChild->setCachedOwnDef(pSubDef, true);
	}
}
/*****************************************************************************/
ResourceItem* UIElement::getDefinitionBlindSearch(ResourceType eColl, const char* pcsSelfId)
{
	ResourceCollection* pColl;
	ResourceItem* pRes;
	//ResourceType eColls[] = { getUIPlane()->getInitCollectionType(), ResourceSepWindowUIElements, ResourceLastPlaceholder };
//	int iCurr;
	//for(iCurr = 0; eColls[iCurr] != ResourceLastPlaceholder; iCurr++)
	{

		//pColl = ResourceManager::getInstance()->getCollection(eColls[iCurr]);
		pColl = ResourceManager::getInstance()->getCollection(eColl);
		pRes = pColl->findItemWithPropValue(PropertyId, pcsSelfId, true);
		if(pRes)
			return pRes;
		pRes = pColl->findItemWithPropValue(PropertyOldId, pcsSelfId, true);
		if(pRes)
			return pRes;

	}
	return NULL;
}
/*****************************************************************************/
ResourceItem* findUiItemInCollections(ResourceType eInitCollType, const char* pcsId, ResourceType iExtraCollectionId = -1)
{
	//ResourceType eColls[] = { getUIPlane()->getInitCollectionType(), ResourceSepWindowUIElements, ResourceLastPlaceholder };
	//ResourceType eCollsNodes[] = { ResourceNodeDefinitions, ResourceTools, ResourceLastPlaceholder };

	int iCurr;
	ResourceItem* pRes = NULL;
	ResourceCollection* pColl;

	if(iExtraCollectionId >= 0)
	{
		pColl = ResourceManager::getInstance()->getCollection(iExtraCollectionId);
		pRes = pColl->getItemById(pcsId);
		if(pRes)
			return pRes;
	}

//	for(iCurr = 0; eColls[iCurr] != ResourceLastPlaceholder; iCurr++)
	{
		//pColl = ResourceManager::getInstance()->getCollection(eColls[iCurr]);
		pColl = ResourceManager::getInstance()->getCollection(eInitCollType);
		pRes = pColl->getItemById(pcsId);
		if(pRes)
			return pRes;
	}

	return NULL;
}
/*****************************************************************************/
ResourceItem* UIElement::getOwnDefinitionInternal(const char* pcsSelfId)
{
	ResourceType eInitCollType = getUIPlane()->getInitCollectionType();
	if(!this->getParent())
	{
		ResourceItem* pOrigParentDefinition = findUiItemInCollections(eInitCollType, pcsSelfId);
		return pOrigParentDefinition;
	}

	// Thing is, we may be nested more than one level deep, in which case
	// we won't find the parent. So we need to find the topmost parent,
	// and then find a child within it - recursively.
	const char* pcsTopmostType = NULL;
	//bool bLookInNodesCollection = false;
	ResourceType iExtraCollectionId = -1;
	if(this->doesPropertyExist(PropertyParentElementId))
	{
		pcsTopmostType = this->getStringProp(PropertyParentElementId);
		iExtraCollectionId = this->getNumProp(PropertyParentElementCollection);
		//bLookInNodesCollection = true;
	}
	else
	{
		UIElement *pPrevParent = NULL, *pTopmostParent  = FAST_CAST<UIElement*>(this->getParent());

		// Since table cells are defined separately, stop when we find one.
		while(pTopmostParent && dynamic_cast<UITableCellElement*>(pTopmostParent) == NULL && !pTopmostParent->doesPropertyExist(PropertyParentElementId))
		{
			pPrevParent = pTopmostParent;
			pTopmostParent = FAST_CAST<UIElement*>(pTopmostParent->getParent());
		}

		if(!pTopmostParent)
			pTopmostParent = pPrevParent;

		_ASSERT(pTopmostParent);
		if(!pTopmostParent)
			return NULL;

		if(pTopmostParent->doesPropertyExist(PropertyParentElementId))
		{
			pcsTopmostType = pTopmostParent->getStringProp(PropertyParentElementId);
			iExtraCollectionId = pTopmostParent->getNumProp(PropertyParentElementCollection);
			//bLookInNodesCollection = true;
		}
		else
			pcsTopmostType = pTopmostParent->getStringProp(PropertyId);
	}

	ResourceItem* pOrigParentDefinition = findUiItemInCollections(eInitCollType, pcsTopmostType, iExtraCollectionId);

	// Note that for children of table cells (for example), we may run into a situation where the parent cell
	// name is numbered, and so the cell def won't be found. If we just fail, we will work - but by doing
	// a very slow blind search. So instead, we try to see if this a numbered def, and if so, find one
	// with out the end numbers.
	if(!pOrigParentDefinition && StringUtils::doesEndInDigits(pcsTopmostType))
	{
		string strTempName(pcsTopmostType);
		StringUtils::stripTrailingDigits(strTempName);
		pOrigParentDefinition = findUiItemInCollections(eInitCollType, strTempName.c_str(), iExtraCollectionId);
	}

	//_ASSERT(pOrigParentDefinition);
	if(!pOrigParentDefinition)
		return NULL;

	if(strcmp(pOrigParentDefinition->getStringProp(PropertyId), pcsSelfId) == 0)
		return pOrigParentDefinition;

	// Now, find  the child in this parent.
	ResourceItem* pRes = pOrigParentDefinition->getChildById(pcsSelfId, true);
	if(!pRes)
		pRes = findUiItemInCollections(eInitCollType, pcsSelfId);

	return pRes;

}
/*****************************************************************************/
ResourceItem* UIElement::getParentDefinition()
{
	if(!this->getParent())
		return NULL;

	UIElement* pUiElemParent = FAST_CAST<UIElement*>(this->getParent());
	if(!pUiElemParent)
		return NULL;
	return pUiElemParent->getOwnDefinition();

/*
	if(!this->getParent())
		return NULL;

	ResourceCollection* pColl = ResourceManager::getInstance()->getCollection(ResourceUIElements);

	// Thing is, we may be nested more than one level deep, in which case
	// we won't find the parent. So we need to find the topmost parent,
	// and then find a child within it - recursively.
	UIElement *pPrevParent = NULL, *pTopmostParent  = FAST_CAST<UIElement*>(this->getParent());
	while(pTopmostParent)
	{
		pPrevParent = pTopmostParent;
		pTopmostParent = FAST_CAST<UIElement*>(pTopmostParent->getParent());
	}
	pTopmostParent = pPrevParent;

	_ASSERT(pTopmostParent);
	if(!pTopmostParent)
		return NULL;

	const char* pcsTopmostType = pTopmostParent->getStringProp(PropertyId);

	ResourceItem* pOrigParentDefinition = pColl->findItemByType(pcsTopmostType);
	if(!pOrigParentDefinition)
	{
		// We may be in prelim ui elems
		pColl = ResourceManager::getInstance()->getCollection(ResourcePrelimUIElements);
		pOrigParentDefinition = pColl->findItemByType(pcsTopmostType);
	}

	_ASSERT(pOrigParentDefinition);
	if(!pOrigParentDefinition)
		return NULL;

	if(strcmp(pOrigParentDefinition->getStringProp(PropertyId), this->getParent()->getStringProp(PropertyId)) == 0)
		return pOrigParentDefinition;

	// Now, find  the child in this parent.
	return pOrigParentDefinition->
	getChildByType(this->getParent()->getStringProp(PropertyId), true);
	*/
}
/*****************************************************************************
bool UIElement::getMouseOverCoords(SVector2D& svOut)
{
	if(getParentWindow()->getNumActiveTouches() > 0)
		return false;

	getParentWindow()->getLastMousePos(svOut);

	SRect2D srRect;
	SVector2D svScroll;
	getLocalRectangle(svScroll, 1.0, srRect);

	if(srRect.doesContain(svOut))
		return true;
	else
		return false;
}
/*****************************************************************************/
void UIElement::resetPosCache(bool bRecursive)
{
	myValidRawPosCache = false;

	if(bRecursive)
	{
		UIElement* pChild;
		int iCurrChild, iNumChildren = this->getNumChildren();
		for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
		{
			pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
			pChild->resetPosCache(bRecursive);
		}
	}
}
/*****************************************************************************/
void UIElement::insertIntoMap(TElementAlphaMap& rMap, TUIElemsUniqueCountHash* pOptUniqueCountMap)
{
#ifdef _DEBUG
	// Test for duplicates
	const char* pcsDbValue = this->getStringProp(PropertyId);
	UIElement* const * pDup = rMap.findSimple(this->getStringProp(PropertyId));
	if(pDup)
		gLog("\nERROR: Two or more elements with the same id %s have been found. Please keep all ids globally unique.\n", pcsDbValue);
	_ASSERT(pDup == NULL);
#endif
	
	const char* pcsId = this->getStringProp(PropertyId);
	rMap.insert(pcsId, this);
	if(pOptUniqueCountMap)
	{
		int* pCountPtr = pOptUniqueCountMap->find(pcsId);
		if(pCountPtr)
			*pCountPtr = (*pCountPtr) + 1;
		else
			pOptUniqueCountMap->insertVal(pcsId, 1);
	}

	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;
		pChild->insertIntoMap(rMap, pOptUniqueCountMap);
	}

}
/*****************************************************************************/
void UIElement::removeFromMap(TElementAlphaMap& rMap)
{
#ifdef _DEBUG
	UIElement*const* pExistingValue = rMap.findSimple(this->getStringProp(PropertyId));
	_ASSERT( (*pExistingValue) == this);
#endif
	rMap.remove(this->getStringProp(PropertyId));

	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;
		pChild->removeFromMap(rMap);
	}
}
/*****************************************************************************/
void UIElement::getGlobalRectangle(SRect2D &srWindowRect)
{
	SVector2D svTempPos;
	FLOAT_TYPE fGlobScale;
	this->getGlobalPosition(svTempPos, NULL, &fGlobScale);

	SVector2D svBoxSize;
	this->getBoxSize(svBoxSize);
	svBoxSize *= fGlobScale;

	srWindowRect.w = svBoxSize.x;
	srWindowRect.h = svBoxSize.y;
	srWindowRect.x = svTempPos.x - svBoxSize.x/2.0;
	srWindowRect.y = svTempPos.y - svBoxSize.y/2.0;
}
/*****************************************************************************
void UIElement::getLocalRectangle(const SVector2D& svScroll, FLOAT_TYPE fScale, SRect2D &srWindowRect)
{
	ADD SCREEN CENTER TO CALLERS
	SVector2D svPos;
	FLOAT_TYPE fLocScale;
	this->getLocalPosition(svPos, NULL, &fLocScale);

	if(this->getParent())
		svPos *= fLocScale;

	FLOAT_TYPE fSelfW = this->getNumProp(PropertyWidth)*fScale*fLocScale;
	FLOAT_TYPE fSelfH = this->getNumProp(PropertyHeight)*fScale*fLocScale;

	srWindowRect.x = svPos.x + svScroll.x - fSelfW/2.0;
	srWindowRect.y = svPos.y + svScroll.y - fSelfH/2.0;
	srWindowRect.w = fSelfW;
	srWindowRect.h = fSelfH;
}
/*****************************************************************************/
void UIElement::getWindowRect(SRect2D& srRectOut)
{
	SVector2D svSize;
	getBoxSize(svSize);
	SVector2D svPos;
	FLOAT_TYPE  fLocScale;
	this->getLocalPosition(svPos, NULL, &fLocScale);

	// Compensated for the added offset
	SVector2D svCenterOffset;
	UIElement* pParent = FAST_CAST<UIElement*>(this->getParent());
	if(pParent)
	{
		SVector2D svParentExtraOffset;
		pParent->getExtraShiftOffset(svParentExtraOffset);
		svPos -= svParentExtraOffset;

		SVector2D svOwnSize;
		FAST_CAST<UIElement*>(this->getParent())->getBoxSize(svOwnSize);
		svCenterOffset = svOwnSize*(0.5);
	}

	srRectOut.w = svSize.x;
	srRectOut.h = svSize.y;
	srRectOut.x = svPos.x - srRectOut.w/2.0 + svCenterOffset.x;
	srRectOut.y = svPos.y - srRectOut.h/2.0 + svCenterOffset.y;
}
/*****************************************************************************/
void UIElement::setCenter(FLOAT_TYPE fX, FLOAT_TYPE fY)
{
#ifdef _DEBUG
	if(IS_OF_TYPE("toolParentWindow"))
	{
		int bp = 0;
	}
#endif

	myHasBeenPlacedManually = true;

	if(fX != FLOAT_TYPE_MAX)
		this->setNumProp(PropertyX, fX);
	if(fY != FLOAT_TYPE_MAX)
		this->setNumProp(PropertyY, fY);

	// Reset all caches
	this->resetEvalCache(true);
}
/*****************************************************************************/
void UIElement::setWindowRect(SRect2D& srRect)
{
    SVector2D svCenterOffset(0, 0);

//     _ASSERT(srRect.w > 0);
//     _ASSERT(srRect.h > 0);

    // If we have a parent, we're relative to its center
    if(this->getParent())
    {
		SVector2D svOwnSize;
		FAST_CAST<UIElement*>(this->getParent())->getBoxSize(svOwnSize);
		svCenterOffset = svOwnSize*(-0.5);
    }
#ifdef _DEBUG
	if(IS_OF_TYPE("toolParentWindow"))
	{
		int bp = 0;
	}
    if(strcmp(this->getStringProp(PropertyId), "toolParentWindow") == 0)
    {
	int bp = 0;
    }
#endif

    myHasBeenPlacedManually = true;

    this->setNumProp(PropertyX, srRect.x + srRect.w/2.0 + svCenterOffset.x);
    this->setNumProp(PropertyY, srRect.y + srRect.h/2.0 + svCenterOffset.y);

//	_ASSERT(srRect.w > 0 && srRect.h > 0);
	SVector2D svCurrSize;
	getBoxSize(svCurrSize);
	bool bDidSizeChange = ((int)svCurrSize.x != (int)srRect.w) || ((int)svCurrSize.y != (int)srRect.h);


    this->setNumProp(PropertyWidth, srRect.w);
    this->setNumProp(PropertyHeight, srRect.h);

    // Reset all caches
    this->resetEvalCache(false);
	if(bDidSizeChange)
	{
	//if(bRecursive)
	{
		autoLayoutChildren();
		this->adjustChildrenToScreen();
	}
	}

	onSizeChangedInternal();
	onSizeChanged();
}
/*****************************************************************************/
const char* UIElement::getTitle()
{
    if(this->doesPropertyExist(PropertyTitle))
		return this->getStringProp(PropertyTitle);
    else
		return this->getStringProp(PropertyId);
}
/*****************************************************************************/
void UIElement::invalidateSizingInfos()
{
	if(this->getChildrenLayoutType() == AutolayoutRow)
	{
		delete mySizingInfos;
		mySizingInfos = NULL;
	}
}
/*****************************************************************************/
void UIElement::recomputeChildSizes()
{
#ifdef _DEBUG
	if(IS_OF_TYPE("toolOptsHost"))
	{
		int bp = 0;
	}
#endif
    if(!mySizingInfos)
		mySizingInfos = new TChildSizingInfos;
    mySizingInfos->clear();

    SVector2D svTempPos;
    FLOAT_TYPE fGlobScale;
    this->getGlobalPosition(svTempPos, NULL, &fGlobScale);

    SVector2D svBoxSize;
    this->getBoxSize(svBoxSize);
    svBoxSize *= fGlobScale;

    // Now, figure out how to distribute the children according to their
    // weights or pixel sizes. First, the fixed-size elements must remain
    // as such. This means the weights only share the remaining space.
    FLOAT_TYPE fCurrValue;
    int iChild, iNumChildren = this->getNumChildren();
    FLOAT_TYPE fAccumFixedWidth = 0;
	UIElement* pChild;
	FLOAT_TYPE svHorMargin[2];
    for(iChild = 0; iChild < iNumChildren; iChild++)
    {
		SChildSizingInfo rInfo;

		pChild = as<UIElement>(this->getChild(iChild));
		if(!pChild->getIsVisible())
		{
			rInfo.myIsHidden = true;
			mySizingInfos->push_back(rInfo);
			continue;
		}

		fCurrValue = pChild->getNumProp(PropertyLayoutWidth);
		if(fCurrValue == 0)
			fCurrValue = pChild->getNumProp(PropertyWidth);
		svHorMargin[0] = pChild->getNumProp(PropertyMarginLeft);
		svHorMargin[1] = pChild->getNumProp(PropertyMarginRight);
		_ASSERT(fCurrValue > 0);

		rInfo.myHorMargins[0] = svHorMargin[0];
		rInfo.myHorMargins[1] = svHorMargin[1];
		rInfo.myPixelWidth = fCurrValue;
		if(fCurrValue > 1)
		{
			// Assume it's a fixed width value.
			fAccumFixedWidth += fCurrValue + svHorMargin[0] + svHorMargin[1];
		}
		else
		{
			// It's a percentage. Skip for now.
		}

		mySizingInfos->push_back(rInfo);
    }

    // Now actually compute the length.
    FLOAT_TYPE fStartXPos = -svBoxSize.x/2.0;
    SChildSizingInfo* pInfo;
    for(iChild = 0; iChild < iNumChildren; iChild++)
    {
		pInfo = &(*mySizingInfos)[iChild];
		if(pInfo->myIsHidden)
			continue;

		// If it's a percentage, compute this here.
		// Note that it's a percentage of the remaining area, not the whole size.
		if(pInfo->myPixelWidth <= 1.0)
			pInfo->myPixelWidth = (svBoxSize.x - fAccumFixedWidth)*pInfo->myPixelWidth;
		pInfo->myCumulStartPos = fStartXPos + pInfo->myHorMargins[0];

		fStartXPos += pInfo->myPixelWidth + pInfo->myHorMargins[0] + pInfo->myHorMargins[1];
    }
}
/*****************************************************************************/
bool UIElement::getChildSpacingInfo(UIElement* pElem, SChildSizingInfo& rInfoOut)
{
	if(!mySizingInfos)
		recomputeChildSizes();

    _ASSERT(mySizingInfos);

    int iChild, iNumChildren = this->getNumChildren();
    for(iChild = 0; iChild < iNumChildren; iChild++)
    {
		if(getChild(iChild) == dynamic_cast<ResourceItem*>(pElem))
		{
			_ASSERT(iChild < mySizingInfos->size());
			rInfoOut = (*mySizingInfos)[iChild];
			return true;
		}
    }

    // Asking for a child with no info
    _ASSERT(0);
    return false;
}
/*****************************************************************************/
void UIElement::formatNumber(const SUnitNumber& rNumber, bool bAppendUnits, string& strOut)
{
	_ASSERT(this->doesPropertyExist(PropertyTextFormat));
	const char* pcsFormat = this->getStringProp(PropertyTextFormat);

	rNumber.toStringUsingFormat(this->getStringProp(PropertyTextFormat), bAppendUnits, strOut);
/*
	const int iConstBuffSize = 1024;
	char* pcsFinalBuffPtr;
	char pcsStaticBuff[iConstBuffSize];
	pcsFinalBuffPtr = pcsStaticBuff;

	if(strstr(pcsFormat, "%d"))
	{
		int iTemp = fNumber;
		sprintf(pcsFinalBuffPtr, pcsFormat, iTemp);
	}
	else
		sprintf(pcsFinalBuffPtr, pcsFormat, fNumber);
	strOut = pcsFinalBuffPtr;
	*/
}
/*****************************************************************************/
void UIElement::formatText(string& strInOut)
{
    _ASSERT(this->doesPropertyExist(PropertyTextFormat));

    const char* pcsFormatString = this->getStringProp(PropertyTextFormat);
    if(!strstr(pcsFormatString, "%s"))
		return;

	const int iConstBuffSize = 1024;
	int iLenNeeded = strInOut.length() + strlen(pcsFormatString) + 128;
	char* pcsFinalBuffPtr;
	char* pcsAllocBuff = NULL;
	char pcsStaticBuff[iConstBuffSize];

	if(iLenNeeded < iConstBuffSize)
		pcsFinalBuffPtr = pcsStaticBuff;
	else
	{
		pcsAllocBuff = new char[iLenNeeded];
		pcsFinalBuffPtr = pcsAllocBuff;
	}

    sprintf(pcsFinalBuffPtr, pcsFormatString, strInOut.c_str());
    strInOut = pcsFinalBuffPtr;

	if(pcsAllocBuff)
		delete[] pcsAllocBuff;
}
/*****************************************************************************/
UIElement* UIElement::findChildWithTag(const char* pcsTag, bool bIncludeSelf)
{
	if(bIncludeSelf)
	{
		if(this->hasTag(pcsTag))
			return this;
	}

	UIElement* pTempRes;
	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;

		pTempRes = pChild->findChildWithTag(pcsTag, true);
		if(pTempRes)
			return pTempRes;
	}

	return NULL;
}
/*****************************************************************************/
UIElement* UIElement::findChildByPropertyValue(PropertyType eProp, bool bValue)
{
	if(this->doesPropertyExist(eProp))
	{
		// See if we qualify
		if(this->getBoolProp(eProp) == bValue)
			return this;
	}

	UIElement* pTempRes;
	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;

		pTempRes = pChild->findChildByPropertyValue(eProp, bValue);
		if(pTempRes)
			return pTempRes;
	}

	return NULL;
}
/*****************************************************************************/
UIElement* UIElement::findNextChildByPropertyValue(PropertyType eProp, FLOAT_TYPE fCurrValue, FLOAT_TYPE fLesserThan)
{
    UIElement* pRes = NULL;
    FLOAT_TYPE fCurrResValue = fLesserThan;
    if(this->doesPropertyExist(eProp) && this->getNumProp(eProp) > fCurrValue)
    {
		// See if we qualify
		FLOAT_TYPE fOwnValue = this->getNumProp(eProp);
		if(fOwnValue > fCurrValue && fOwnValue < fCurrResValue)
		{
			fCurrResValue = fOwnValue;
			pRes = this;
		}
    }

    UIElement* pTempRes;
    UIElement* pChild;
    int iCurrChild, iNumChildren = this->getNumChildren();
    for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
    {
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
		{
			// Something didn't work - we have a non-UIElement child.
			_ASSERT(0);
			continue;
		}
		pTempRes = pChild->findNextChildByPropertyValue(eProp, fCurrValue, fCurrResValue);
		if(pTempRes)
		{
			pRes = pTempRes;
			fCurrResValue = pTempRes->getNumProp(eProp);
		}
    }

    return pRes;
}
/*****************************************************************************/
void UIElement::autoLayoutChildrenRecursive()
{
    // Recursion - first layout our children. They may also have
	// elements that need layout first.
    UIElement* pChild;
    int iCurrChild, iNumChildren = this->getNumChildren();
    for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
    {
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		pChild->autoLayoutChildrenRecursive();
    }

	// Overridden call
	autoLayoutChildren();

}
/*****************************************************************************/
void UIElement::copyParmValuesFrom(StringResourceItem* pItem, PropertyType eParmPropertyName)
{
	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;
		pChild->copyParmValuesFromRecurive(pItem, eParmPropertyName, pChild);
	}
}
/*****************************************************************************/
void UIElement::copyParmValuesFrom(ResourceItem* pItem, PropertyType eParmPropertyName)
{
	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;
		pChild->copyParmValuesFromRecurive(pItem, eParmPropertyName, pChild);
	}
}
/*****************************************************************************/
void UIElement::copyParmValuesFromRecurive(StringResourceItem* pItem, PropertyType eParmPropertyName, UIElement* pParmContainingElem)
{
	// Set us if we have a parm name
	if(this->getBoolProp(PropertyIsParmReceptor))
	{
		// Find anyone up the stream up to the container elem with a prop name.
		UIElement* pParmNameUiElem = this;
		while(pParmNameUiElem)
		{
			if(pParmNameUiElem->doesPropertyExist(eParmPropertyName))
				break;
			pParmNameUiElem = pParmNameUiElem->getParent<UIElement>();
		}

		if(pParmNameUiElem)
		{
			const char* pcsParmName = pParmNameUiElem->getStringProp(eParmPropertyName);
			this->setTargetResourceItemParm(pItem, pcsParmName);

			_ASSERT(myTargetIdentifier);
			if(myTargetIdentifier)
				myTargetIdentifier->refreshUIFromStoredValue(this);
		}
		ELSE_ASSERT;
	}

	// Recursion
	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
		{
			// Something didn't work - we have a non-UIElement child.
			_ASSERT(0);
			continue;
		}
		pChild->copyParmValuesFromRecurive(pItem, eParmPropertyName, pParmContainingElem);
	}

}
/*****************************************************************************/
void UIElement::copyParmValuesFromRecurive(ResourceItem* pItem, PropertyType eParmPropertyName, UIElement* pParmContainingElem)
{
	// Set us if we have a parm name
	if(this->getBoolProp(PropertyIsParmReceptor))
	{
		// Find anyone up the streawm up to the containt elem with a prop name.
		UIElement* pParmNameUiElem = this;
		while(pParmNameUiElem)
		{
			if(pParmNameUiElem->doesPropertyExist(eParmPropertyName))
				break;
			pParmNameUiElem = pParmNameUiElem->getParent<UIElement>();
		}

		if(pParmNameUiElem)
		{
			const char* pcsParmName = pParmNameUiElem->getStringProp(eParmPropertyName);
			this->setTargetResourceItemParm(pItem, pcsParmName);

			_ASSERT(myTargetIdentifier);
			if(myTargetIdentifier)
				myTargetIdentifier->refreshUIFromStoredValue(this);
		}
		ELSE_ASSERT;
	}

	// Recursion
	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
		{
			// Something didn't work - we have a non-UIElement child.
			_ASSERT(0);
			continue;
		}
		pChild->copyParmValuesFromRecurive(pItem, eParmPropertyName, pParmContainingElem);
	}

}
/*****************************************************************************/
void UIElement::setExtraScrollOffset(FLOAT_TYPE fX, FLOAT_TYPE fY, bool bUpdateRelatedSlider)
{
	myExtraScrollOffset.set(fX, fY);

	if(myExtraScrollOffset.y > 0)
		myExtraScrollOffset.y = 0;

	if(bUpdateRelatedSlider)
	{
		// If this asserts, we likely forgot to add a related scroll bar (slider).
		UISliderElement* pSlider = getRelatedSlider(NULL);
		if(pSlider)
		{
			FLOAT_TYPE fMaxValue = pSlider->getMaxValue();
			if(myExtraScrollOffset.y < -fMaxValue)
				myExtraScrollOffset.y = -fMaxValue;

			pSlider->setValue(-myExtraScrollOffset.y);
		}
		ELSE_ASSERT;
	}
}
/*****************************************************************************/
void UIElement::updateTitleBarVars()
{
	myExtraScrollOffset.y = 0;
	UITabWindowElement* pParent = dynamic_cast<UITabWindowElement*>(this->getParent());
	if(this->doesPropertyExist(PropertyWindowBarImage) && !pParent)
	{
		const char* pcsTitleBarAnim = this->getStringProp(PropertyWindowBarImage);
		if(!IS_STRING_EQUAL(pcsTitleBarAnim, PROPERTY_NONE))
		{
			int iRealW, iRealH;
			getTextureManager()->getTextureRealDims(pcsTitleBarAnim, iRealW, iRealH);
			_ASSERT(iRealH > 0);
			myExtraScrollOffset.y = iRealH;
		}
	}
}
/*****************************************************************************/
int UIElement::getTitleBarHeight() const
{
	if(!this->doesPropertyExist(PropertyWindowBarImage))
		return 0;

	int iRealW, iRealH = 0;
	theCommonString = this->getStringProp(PropertyWindowBarImage);
	if(theCommonString == PROPERTY_NONE)
		return 0;
	if(getTextureManager())
		getTextureManager()->getTextureRealDims(theCommonString.c_str(), iRealW, iRealH);
	return iRealH + WINDOW_TITLE_VERT_PADDING*2;
}
/*****************************************************************************/
void UIElement::renderTitleBar(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
#ifdef _DEBUG
	if(IS_OF_TYPE("toolParentWindow"))
	{
		int bp = 0;
	}
	if(IS_OF_TYPE("defDialogWindow1224"))
	{
		int bp = 0;
	}
#endif

	if(!this->doesPropertyExist(PropertyWindowBarImage))
		return;

	// We also do not render the title if our immediate parent is a tab window.
	// In that case, the tab window itself uses the title bar and icon info
	// to render it in a tab.
	if(this->getParent<UITabWindowElement>())
		return;

	const char* pcsWindowTitleAnim = this->getStringProp(PropertyWindowBarImage);
	if(IS_STRING_EQUAL(pcsWindowTitleAnim, PROPERTY_NONE))
		return;

	SVector2D svPos;
	FLOAT_TYPE fFinalOpac, fLocScale;
	this->getLocalPosition(svPos, &fFinalOpac, &fLocScale);
	fFinalOpac *= fOpacity;

	svPos += svScroll;

	SVector2D svOwnSize;
	getBoxSize(svOwnSize);

	bool bHaveParms = this->doesPropertyExist(PropertyWindowBarParms);
	int iNumParms = this->getNumericEnumPropCount(PropertyWindowBarParms);

	FLOAT_TYPE fIconPadding = WINDOW_TITLE_ICON_HOR_PADDING;
	if(bHaveParms && iNumParms > 0)
		fIconPadding = this->getNumericEnumPropValue(PropertyWindowBarParms, 0);
	FLOAT_TYPE fTextPadding = WINDOW_TITLE_HOR_TEXT_PADDING;
	if(bHaveParms && iNumParms > 1)
		fTextPadding = this->getNumericEnumPropValue(PropertyWindowBarParms, 1);

	SVector2D svTextOffset;
	if(bHaveParms && iNumParms > 2)
		svTextOffset.x = this->getNumericEnumPropValue(PropertyWindowBarParms, 2);
	if(bHaveParms && iNumParms > 3)
		svTextOffset.y = this->getNumericEnumPropValue(PropertyWindowBarParms, 3);

	// If this fails, you forgot to specify the size on the element.
	_ASSERT(svOwnSize.x > 0 && svOwnSize.y > 0);
	svOwnSize = svOwnSize*fScale*fLocScale;

	int iRealW, iRealH;
	theCommonString = pcsWindowTitleAnim;

	getTextureManager()->getTextureRealDims(theCommonString.c_str(), iRealW, iRealH);

	// Render the background
	SVector2D svWindowPosStart;
	svWindowPosStart.x = svPos.x - svOwnSize.x/2.0 + WINDOW_TITLE_HOR_PADDING;
	svWindowPosStart.y = svPos.y - svOwnSize.y/2.0 + WINDOW_TITLE_VERT_PADDING;
	getDrawingCache()->fillArea(theCommonString.c_str(), svWindowPosStart.x, svWindowPosStart.y, svOwnSize.x - WINDOW_TITLE_HOR_PADDING*2, iRealH, fFinalOpac, fScale*fLocScale, fScale*fLocScale);

	FLOAT_TYPE fTextStartPos = svWindowPosStart.x + fTextPadding;

	// Render the icon
	if(this->doesPropertyExist(PropertyWindowBarIcon))
	{
		theCommonString2 = this->getStringProp(PropertyWindowBarIcon);
		getDrawingCache()->addSprite(theCommonString2.c_str(), svWindowPosStart.x + iRealH/2.0 + fIconPadding, svWindowPosStart.y + iRealH/2.0, fOpacity, 0, fScale, fScale, 1.0, true);

		fTextStartPos += iRealH/2.0 + fIconPadding*2.0;
	}

	SColor scolText;
	this->getAsColor(PropertyTextColor, scolText);
	scolText.alpha *= fFinalOpac;

	// Render the text
	SColor scolShadow(0, 0, 0, 1.0);
	SVector2D svShadowOffset(0, -0.5);
	svShadowOffset.upToScreen();
	const char* pcsTitle = this->getTitle();
	if(pcsTitle && strlen(pcsTitle))
	{
		const char* pcsFontPtr;
		int iFontSize = 0;
		if(this->doesPropertyExist(PropertyWindowBarFont))
		{
			pcsFontPtr = this->getStringProp(PropertyWindowBarFont);
			iFontSize = this->getNumProp(PropertyWindowBarFontSize);
		}
		else
		{
			pcsFontPtr = myCachedFont;
			iFontSize = myCachedFontSize;
		}

		getDrawingCache()->addText(pcsTitle, pcsFontPtr, iFontSize, fTextStartPos + svTextOffset.x, svWindowPosStart.y + iRealH/2.0 + svTextOffset.y, scolText, HorAlignLeft, VertAlignCenter, 0, NULL, &scolShadow, &svShadowOffset);
	}
}
/*****************************************************************************/
void UIElement::markOffsetsDirty()
{
	myIsTextOffsetDirty = myIsAnimOffsetDirty = true;
}
/*****************************************************************************/
void UIElement::recomputeTextOffset()
{
	if(!myIsTextOffsetDirty)
		return;

	recomputeAnimOffset();

	SVector2D svSize;
	this->getBoxSize(svSize);
	myTextOffset.x = this->getNumProp(PropertyTextOffsetX);
	myTextOffset.y = this->getNumProp(PropertyTextOffsetY);

	// Take into account the percentage offset
	myTextOffset.x += this->getNumProp(PropertyTextPercOffsetX)*svSize.x;
	myTextOffset.y += this->getNumProp(PropertyTextPercOffsetY)*svSize.y;

	myTextOffset += myAnimOffset;

	myIsTextOffsetDirty = false;
}
/*****************************************************************************/
void UIElement::recomputeAnimOffset()
{
	if(!myIsAnimOffsetDirty)
		return;

	SVector2D svSize;
	this->getBoxSize(svSize);

	myAnimOffset.x = this->getNumProp(PropertyImagePercOffsetX)*svSize.x + this->getNumProp(PropertyImageOffsetX);
	myAnimOffset.y = this->getNumProp(PropertyImagePercOffsetY)*svSize.y + this->getNumProp(PropertyImageOffsetY);

	myIsAnimOffsetDirty = false;
}
/*****************************************************************************/
void UIElement::markOffsetsDirtyRecursive()
{
	markOffsetsDirty();

	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;
		pChild->markOffsetsDirtyRecursive();
	}
}
/*****************************************************************************/
void UIElement::shiftAllChildrenRelativeToDefinition(FLOAT_TYPE fX, FLOAT_TYPE fY)
{
	SVector2D svNewPos;
	ResourceItem* pDef;
	UIElement *pChild;
	int iCurr, iNum = this->getNumChildren();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurr));
		if(!pChild)
		{
			_ASSERT(0);
			continue;
		}

		pDef = pChild->getOwnDefinition();
		_ASSERT(pDef);
		if(!pDef)
			continue;

		svNewPos.x = pDef->getNumProp(PropertyX);
		svNewPos.y = pDef->getNumProp(PropertyY);

		svNewPos.x += fX;
		svNewPos.y += fY;

		if(fX != 0)
			pChild->setNumProp(PropertyX, svNewPos.x);

		if(fY != 0)
			pChild->setNumProp(PropertyY, svNewPos.y);
	}
}
/*****************************************************************************/
UIElement* UIElement::getLinkedToElement()
{
	if(!this->doesPropertyExist(PropertyLinkTo))
		return NULL;

	return this->getElementAtPath(this->getStringProp(PropertyLinkTo));
}
/*****************************************************************************/
UIElement* UIElement::getLinkedToElementWithValidTarget()
{
	if(getHaveValidTargetElement())
		return this;

	if(myIsGettingLinkedToElemWithValidTarget)
		return NULL;

	myIsGettingLinkedToElemWithValidTarget = true;

	// Otherwise, recurse:
	UIElement *pRes = NULL;
	UIElement* pLinkedToElem = this->getLinkedToElement();
	if(pLinkedToElem)
		pRes = pLinkedToElem->getLinkedToElementWithValidTarget();

	myIsGettingLinkedToElemWithValidTarget = false;
	return pRes;
}
/*****************************************************************************/
void UIElement::onCursorGlideEnter(TTouchVector& vecTouches)
{
	if(myCursorType == CursorInvalidUnset)
		myCursorType = getOwnCursorType();
	_ASSERT(myCursorType != CursorInvalidUnset);
	myPushedCursorId = getParentWindow()->pushCursor(myCursorType);

//gLog("Push cursor: %s id = %ld\n", this->getStringProp(PropertyId), myPushedCursorId);
}
/*****************************************************************************/
void UIElement::onCursorGlideLeave(TTouchVector& vecTouches)
{
//gLog("Pop cursor: %s id = -1\n", this->getStringProp(PropertyId));
	getParentWindow()->popCursor(myPushedCursorId);
	myPushedCursorId = -1;
}
/*****************************************************************************/
CursorType UIElement::getOwnCursorType() const
{
	CursorType eRes = CursorLastPlaceholder;
	if(this->doesPropertyExist(PropertyCursorType))
		eRes = mapStringToType(this->getStringProp(PropertyCursorType), g_pcsCursorTypes, CursorLastPlaceholder);

	return eRes;
}
/*****************************************************************************/
void UIElement::changeValueTo(FLOAT_TYPE fValue, UIElement* pOptSourceElem, bool bAnimate, bool bIsChangingContinuously)
{
	// Invalidate the cached text in any case
	invalidateCachedText();

	if(myIsCallingChangeValue)
		return;

	myIsCallingChangeValue = true;

	// Now, if our optional calling element is updating from parm,
	// we have to set ourselves as well. This is the case when
	// we're updating the parm UI, for example.
	bool bOldIsUpdatingFromParm = myIsUpdatingFromParm;
	if(pOptSourceElem)
		myIsUpdatingFromParm = pOptSourceElem->getIsUpdatingFromParm();

	// Change the value of the related element, if any.
	UIElement* pLinkedToElem = getLinkedToElement();
	if(pLinkedToElem)
		pLinkedToElem->changeValueTo(fValue, this, bAnimate, bIsChangingContinuously);

	// See if we have a min delta value
	if(getAllowValuePropagation(fValue, bIsChangingContinuously, pOptSourceElem, pLinkedToElem))
	{
// FILE* out = fopen("db_sliders_out.txt", "a");
// fprintf(out, "%x value to %g\n", this, fValue);
// fclose(out);

		handleTargetElementUpdate(bIsChangingContinuously);
	}

	myIsUpdatingFromParm = bOldIsUpdatingFromParm;

	myIsCallingChangeValue = false;
}
/*****************************************************************************/
UIButtonElement* UIElement::getDefaultActionChild()
{
	return this->getChildById<UIButtonElement>(myDefaultActionChild.c_str(), true, false);
}
/*****************************************************************************/
UIButtonElement* UIElement::getDefaultCancelChild()
{
	return this->getChildById<UIButtonElement>(myDefaultCancelChild.c_str(), true, false);
}
/*****************************************************************************/
UIElement* UIElement::getChildAndSubchild(const char* pcsChildId, const char* pcsSubchildOldDefName)
{
	UIElement *pChild = this->getChildById(pcsChildId);
	if(!pChild)
		return NULL;

	UIElement* pSubchild = pChild->getChildById<UIElement>(pcsSubchildOldDefName, true, true);
	return pSubchild;
}
/*****************************************************************************/
IUndoItem* UIElement::createUndoItemForSelfChange()
{
	//if(myTargetElementToUpdate >= 0)
	if(myTargetIdentifier)
		return myTargetIdentifier->createUndoItemForSelfChange(this);
	else
	{
		UndoManager* pManager = NULL;
		Window* pParentWindow = getParentWindow();
		if(pParentWindow)
			pManager = pParentWindow->getLastOverrideUndoManager();

		if(pManager)
		{
			// We can still create an undo for our override manager.
			// At this point, it's probably just a value change.
			return new UndoItemUIValueChange(this);
		}
		else
			return NULL;
	}
}
/*****************************************************************************/
const char* UIElement::getUndoStringForSelfChange()
{
	if(myTargetIdentifier)
		return myTargetIdentifier->getUndoStringForSelfChange();
	else
	{
		// We're just a UI node. See if we can come up with a description of ourself.
		// TODO: Find a sibling with a label and try to guess. Or just have a separate prop?
		// This will let us know what undo to use.
		theSharedString = "UI";
	}

	theSharedString = "Change " + theSharedString + " value";
	return theSharedString.c_str();
}
/*****************************************************************************/
UIElement* UIElement::getElementLinkedToSelfWithMaxPropValue(PropertyType eProp)
{
	FLOAT_TYPE fCurrMaxValue = -FLOAT_TYPE_MAX;
	return getParent<UIElement>()->getElementLinkedToSelfWithMaxPropValueInternal(this, eProp, fCurrMaxValue);
}
/*****************************************************************************/
UIElement* UIElement::getElementLinkedToSelfWithMaxPropValueInternal(UIElement* pTargetElem, PropertyType eProp, FLOAT_TYPE& fCurrMaxValue)
{
	UIElement* pResElem = NULL;
	UIElement* pCurrRes;

	FLOAT_TYPE fValue;
	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;

		if(pChild->getLinkedToElement() != pTargetElem)
			continue;

		// Otherwise, see if it has the property
		if(!pChild->doesPropertyExist(eProp))
			continue;

		fValue = pChild->getNumProp(eProp);
		if(fValue > fCurrMaxValue)
		{
			fCurrMaxValue = fValue;
			pResElem = pChild;
		}

		// Now look in its children
		pCurrRes = pChild->getElementLinkedToSelfWithMaxPropValueInternal(pTargetElem, eProp, fCurrMaxValue);
		if(pCurrRes)
			pResElem = pCurrRes;
	}

	return pResElem;
}
/*****************************************************************************/
UIElement* UIElement::getElementLinkedToSelfWithMinPropValue(PropertyType eProp)
{
	FLOAT_TYPE fCurrMinValue = FLOAT_TYPE_MAX;
	return getParent<UIElement>()->getElementLinkedToSelfWithMinPropValueInternal(this, eProp, fCurrMinValue);
}
/*****************************************************************************/
UIElement* UIElement::getElementLinkedToSelfWithMinPropValueInternal(UIElement* pTargetElem, PropertyType eProp, FLOAT_TYPE& fCurrMinValue)
{
	UIElement* pResElem = NULL;
	UIElement* pCurrRes;

	FLOAT_TYPE fValue;
	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;

		if(pChild->getLinkedToElement() != pTargetElem)
			continue;

		// Otherwise, see if it has the property
		if(!pChild->doesPropertyExist(eProp))
			continue;

		fValue = pChild->getNumProp(eProp);
		if(fValue < fCurrMinValue)
		{
			fCurrMinValue = fValue;
			pResElem = pChild;
		}

		// Now look in its children
		pCurrRes = pChild->getElementLinkedToSelfWithMinPropValueInternal(pTargetElem, eProp, fCurrMinValue);
		if(pCurrRes)
			pResElem = pCurrRes;
	}

	return pResElem;
}
/*****************************************************************************/
void UIElement::clearTargetUpdateIdentifier()
{
	if(myTargetIdentifier)
		delete myTargetIdentifier;
	myTargetIdentifier = NULL;
}
/*****************************************************************************/
bool UIElement::getHaveValidTargetElement()
{
	return myTargetIdentifier && myTargetIdentifier->isValid();
}
/*****************************************************************************/
void UIElement::handleTargetElementUpdate(bool bIsChangingContinuously, bool bSendValueChangedEvent)
{
	// If we're updating from parm, do NOT send the events back
	// to the node, since that will cause it to recook. But we're updating
	// *from* it, not changing anything...
	if(myIsUpdatingFromParm)
		return;

	if(myTargetIdentifier)
		myTargetIdentifier->refreshStoredValueFromUI(this, bIsChangingContinuously);

	if(bSendValueChangedEvent)
		EventManager::getInstance()->sendEvent(EventValueChanged, this);
}
/*****************************************************************************/
LinkedToSelfElemIterator UIElement::linkedToSelfElemIteratorFirst(UIElement* pOptStartElem)
{
	// First, we have to find a suitable parent where to start.
	UIElement* pStart = NULL, *pPrev = this;
	if(!pOptStartElem)
	{
		for(pStart = this->getParent<UIElement>(); pStart; pStart = pStart->getParent<UIElement>())
		{
			pPrev = pStart;
			if(IS_STRING_EQUAL(pStart->getStringProp(PropertyId), PARM_HOST_ELEM_ID))
				break;
		}
	}
	else
		pPrev = pOptStartElem;

	// So now pPrev contains our start elem.
	LinkedToSelfElemIterator rIter(pPrev, this);
	return rIter;
}
/*****************************************************************************/
void UIElement::setTargetResourceItemParm(StringResourceItem* pItem, const char* pcsParmName)
{
	_ASSERT(pItem && pcsParmName);
	clearTargetUpdateIdentifier();
	if(pcsParmName && pItem)
		myTargetIdentifier = new StringResourceItemIdentifier(pItem, pcsParmName);
}
/*****************************************************************************/
void UIElement::setTargetResourceItemParm(ResourceItem* pItem, const char* pcsParmName)
{
	_ASSERT(pItem && pcsParmName);
	clearTargetUpdateIdentifier();
	if(pcsParmName && pItem)
	{
		// Convert our string property to PropertyType
		PropertyMapper* pPropertyMapper = PropertyMapper::getInstance();
		PropertyType eProp = pPropertyMapper->mapProperty(pcsParmName);
		myTargetIdentifier = new ResourceItemIdentifier(pItem, eProp);
	}
}
/*****************************************************************************/
void UIElement::setTargetUiElem(UIElement* pElem)
{
#ifdef _DEBUG
	// Sanity check: for now, check that the element
	// is my parent. It's not required at all, but
	// these are all the situations we have right now
	// and there's a bug somewhere...
	bool bDidFind = false;
	UIElement *pDbParent = this;
	while(pDbParent)
	{
		if(pDbParent == pElem)
		{
			bDidFind = true;
			break;
		}
		pDbParent = pDbParent->getParent<UIElement>();
	}

	// We now have a case like this.
//	_ASSERT(bDidFind);

#endif
	_ASSERT(pElem);
	clearTargetUpdateIdentifier();
	if(pElem)
		myTargetIdentifier = new UIElementIdentifier(pElem);
}
/*****************************************************************************/
void UIElement::setTargetIdentifier(IGenericIdentifier* pIdentifier)
{
	clearTargetUpdateIdentifier();
	myTargetIdentifier = pIdentifier;
}
/*****************************************************************************/
UIElement* UIElement::getParentById(const char* pcsId, bool bAllowOldId)
{
	ResourceItem* pItem = this->getParent();
	while(pItem)
	{
		if(IS_STRING_EQUAL(pcsId, pItem->getStringProp(PropertyId)))
			return as<UIElement>(pItem);
		if(bAllowOldId && pItem->doesPropertyExist(PropertyOldId) && IS_STRING_EQUAL(pcsId, pItem->getStringProp(PropertyOldId)))
			return as<UIElement>(pItem);
		pItem = pItem->getParent();
	}
	return NULL;
}
/*****************************************************************************/
void UIElement::setUniqueName(UIPlane* pParentPlane, bool bRecursive) // , TElementAlphaMap& rTempStringSet
{
	UIElement* pFound = pParentPlane->getElementById<UIElement>(this->getStringProp(PropertyId));
	if(pFound && pFound != this)
	{
		pParentPlane->generateUniqueName(this, theCommonString); // rTempStringSet, 
		if(!this->doesPropertyExist(PropertyOldId))
			this->setStringProp(PropertyOldId, this->getStringProp(PropertyId));
		this->setId(theCommonString.c_str());
		this->onIdChanged();
	}

	if(!bRecursive)
		return;

	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;

		pChild->setUniqueName(pParentPlane, true); // , rTempStringSet
	}
}
/*****************************************************************************/
void UIElement::getMinDims(SVector2D& svDimsOut) const
{
	svDimsOut.set(0, 0);
	if(this->doesPropertyExist(PropertyMinWidth))
		svDimsOut.x = this->getNumProp(PropertyMinWidth);
	if(this->doesPropertyExist(PropertyMinHeight))
		svDimsOut.y = this->getNumProp(PropertyMinHeight);
}
/*****************************************************************************/
void UIElement::getMaxDims(SVector2D& svDimsOut) const
{
	svDimsOut.set(0, 0);
	if(this->doesPropertyExist(PropertyMaxWidth))
		svDimsOut.x = this->getNumProp(PropertyMaxWidth);
	if(this->doesPropertyExist(PropertyMaxHeight))
		svDimsOut.y = this->getNumProp(PropertyMaxHeight);
}
/*****************************************************************************/
FLOAT_TYPE UIElement::getNumericValueForChild(const char* pcsChildId, const char* pcsSubChildId, UnitType eUnits)
{
	UIElement* pElem = this->getChildById(pcsChildId, true, true);
	if(pcsSubChildId)
		pElem = pElem->getChildById<UIElement>(pcsSubChildId, true, true);
	if(pElem)
		return pElem->getNumericValue(eUnits);
	else
		ASSERT_RETURN_NULL;
}
/*****************************************************************************/
UIElement* UIElement::setNumericValueForChild(const char* pcsChildId, const char* pcsSubChildId, FLOAT_TYPE fValue, UnitType eUnits)
{
	UIElement* pElem = this->getChildById(pcsChildId, true, true);
	if(pcsSubChildId)
		pElem = pElem->getChildById<UIElement>(pcsSubChildId, true, true);
	if(pElem)
	{
		pElem->setNumericValue(fValue, eUnits);
	}
	return pElem;
}
/*****************************************************************************/
UIElement* UIElement::setTextForChild(const char* pcsChildId, string& strText, SColor* pOptColor, FLOAT_TYPE fShortenTextMaxLen)
{
	UIElement* pElem = this->getChildById(pcsChildId, true, true);
	if(pElem)
	{
		if(strText == PROPERTY_NONE)
			strText = "";
		if(fShortenTextMaxLen > 0)
			RenderUtils::shortenStringFromTheEnd(getParentWindow(), strText, pElem->getCachedFont(), pElem->getCachedFontSize(), fShortenTextMaxLen, "...", false);
		pElem->setText(strText.c_str());
		if(pOptColor)
			pElem->setAsColor(PropertyTextColor, *pOptColor);
	}

	return pElem;
}
/*****************************************************************************/
UIElement* UIElement::setTextForChild(const char* pcsChildId, const char* pcsSubChildId, const char* pcsText, SColor* pOptColor)
{
	UIElement* pElem = this->getChildById(pcsChildId, true, true);
	if(!pElem)
		return NULL;

	pElem = pElem->getChildById(pcsSubChildId, true, true);
	if(!pElem)
		return NULL;

	if(IS_STRING_EQUAL(pcsText, PROPERTY_NONE))
		pcsText = "";
	pElem->setText(pcsText);
	if(pOptColor)
		pElem->setAsColor(PropertyTextColor, *pOptColor);

	return pElem;
}
/*****************************************************************************/
UIElement* UIElement::setTextForChild(const char* pcsChildId, const char* pcsText, SColor* pOptColor)
{
	UIElement* pElem = this->getChildById(pcsChildId, true, true);
	if(pElem)
	{
		if(IS_STRING_EQUAL(pcsText, PROPERTY_NONE))
			pcsText = "";
		pElem->setText(pcsText);
		if(pOptColor)
			pElem->setAsColor(PropertyTextColor, *pOptColor);
	}

	return pElem;
}
/*****************************************************************************/
UIElement* UIElement::setIsVisibleForChild(const char* pcsChildId, bool bIsVisible)
{
	if(!IS_VALID_STRING_AND_NOT_NONE(pcsChildId))
		return NULL;

	UIElement* pElem;
	pElem = this->getChildById(pcsChildId, true, true);
	if(pElem)
		pElem->setIsVisible(bIsVisible);
	return pElem;
}
/*****************************************************************************/
UIElement* UIElement::setIsEnabledForChild(const char* pcsChildId, bool bIsEnabled)
{
	UIElement* pElem;
	pElem = this->getChildById(pcsChildId, true, true);
	if(pElem)
		pElem->setIsEnabled(bIsEnabled);
	return pElem;
}
/*****************************************************************************/
UIElement* UIElement::setTopAnimForChild(const char* pcsChildId, const char* pcsTopAnim)
{
	UIElement* pElem;
	pElem = this->getChildById(pcsChildId, true, true);
	if(pElem && pcsTopAnim)
		pElem->setTopAnim(pcsTopAnim);
	return pElem;
}
/*****************************************************************************/
void UIElement::positionAsPopupRelativeTo(const SVector2D& svPoint, SideType eSide, FLOAT_TYPE fOffset, bool bExtraOffsetSideways, bool bRunRefresh)
{
	SVector2D svSize(0, 0);
	positionAsPopupRelativeToInternal(svPoint, svSize, eSide, fOffset, bExtraOffsetSideways, bRunRefresh);
}
/*****************************************************************************/
void UIElement::positionAsPopupRelativeTo(UIElement* pRelativeToElem, SideType eSide, FLOAT_TYPE fOffset, bool bRunRefresh)
{
	SVector2D svTargetPos;
	SVector2D svTargetSize;
	pRelativeToElem->getGlobalPosition(svTargetPos, NULL, NULL);
	pRelativeToElem->getBoxSize(svTargetSize);

	// Set the info on it to make sure we can reposition it:
	this->setStringProp(PropertyPopupSourceUiElement, pRelativeToElem->getStringProp(PropertyId));
	this->setStringProp(PropertyPopupPosition, g_pcsSideStrings[eSide]);

	positionAsPopupRelativeToInternal(svTargetPos, svTargetSize, eSide, fOffset, false, bRunRefresh);
}
/*****************************************************************************/
void UIElement::positionAsPopupRelativeToInternal(const SVector2D& svTargetPos, const SVector2D& svTargetSize, SideType eSide, FLOAT_TYPE fOffset, bool bExtraOffsetSideways, bool bRunRefresh)
{
	if(fOffset < 0)
		fOffset = upToScreen(POPUP_SPACING);

	this->setNumProp(PropertyPopupOffset, fOffset);

	// First, we need to refresh it in case it's dynamic
	SUpdateInfo rInfo;
	if(bRunRefresh)
		this->updateDataRecursive(rInfo);

	// Now, we have to make sure that our center is aligned - with some offset.
	SVector2D svOwnSize;
	getBoxSize(svOwnSize);

	SVector2D svOwnPos;
	if(eSide == SideTop)
	{
		svOwnPos.x = svTargetPos.x;
		svOwnPos.y = svTargetPos.y - svTargetSize.y/2.0 - svOwnSize.y/2.0 - fOffset;
	}
	else if(eSide == SideBottom)
	{
		svOwnPos.x = svTargetPos.x;
		svOwnPos.y = svTargetPos.y + svTargetSize.y/2.0 + svOwnSize.y/2.0 + fOffset;
	}
	else if(eSide == SideLeft)
	{
		svOwnPos.x = svTargetPos.x - svTargetSize.x/2.0 - svOwnSize.x/2.0 - fOffset;
		svOwnPos.y = svTargetPos.y;
	}
	else if(eSide == SideRight)
	{
		svOwnPos.x = svTargetPos.x + svTargetSize.x/2.0 + svOwnSize.x/2.0 + fOffset;
		svOwnPos.y = svTargetPos.y;
	}
	ELSE_ASSERT;

	if(bExtraOffsetSideways)
		svOwnPos.x = svTargetPos.x + svTargetSize.x/2.0 + svOwnSize.x/2.0 + fOffset;

	SVector2D svScreenDims;
	getParentWindow()->getSize(svScreenDims);
	//if(eSide == SideTop || eSide == SideBottom)
	{
		if(svOwnPos.x - svOwnSize.x/2.0 < POPUP_MIN_SCREEN_PADDING)
			svOwnPos.x = svOwnSize.x/2.0 + POPUP_MIN_SCREEN_PADDING;
		else if(svOwnPos.x + svOwnSize.x/2.0 >= svScreenDims.x - POPUP_MIN_SCREEN_PADDING)
			svOwnPos.x = svScreenDims.x - POPUP_MIN_SCREEN_PADDING - svOwnSize.x/2.0;
	}
	//else if(eSide == SideLeft || eSide == SideRight)
	{
		if(svOwnPos.y - svOwnSize.y/2.0 < POPUP_MIN_SCREEN_PADDING)
			svOwnPos.y = svOwnSize.y/2.0 + POPUP_MIN_SCREEN_PADDING;
		else if(svOwnPos.y + svOwnSize.y/2.0 >= svScreenDims.y - POPUP_MIN_SCREEN_PADDING)
			svOwnPos.y = svScreenDims.y - POPUP_MIN_SCREEN_PADDING - svOwnSize.y/2.0;
	}

	// This is a crude mapping to convert screen coords to this elems local coords.
	// It won't work in the general case.
	SVector2D svOldGlobalPos, svOldLocalPos;
	this->getGlobalPosition(svOldGlobalPos);
	svOldLocalPos.set(this->getNumProp(PropertyX), this->getNumProp(PropertyY));
	SVector2D svNewLocalPos = (svOldLocalPos - svOldGlobalPos) + svOwnPos;

	// This is a hack since right now we don't move elements with a parent properly.
	if(this->getParent())
	{
		if(eSide == SideTop || eSide == SideBottom)
			this->setNumProp(PropertyY, svNewLocalPos.y);
		else
			this->setNumProp(PropertyX, svNewLocalPos.x);
	}
	else
	{
		this->setNumProp(PropertyY, svNewLocalPos.y);
		this->setNumProp(PropertyX, svNewLocalPos.x);
	}

	this->resetEvalCache(true);

}
/*****************************************************************************/
void UIElement::setGenericDataSource(const char* pcsValue)
{
	if(!IS_VALID_STRING_AND_NOT_NONE(pcsValue))
	{
		if(myGenericDataSource)
			delete myGenericDataSource;
		myGenericDataSource = NULL;
		return;
	}

	if(!myGenericDataSource)
		myGenericDataSource = new string;
	*myGenericDataSource = pcsValue;
}
/*****************************************************************************/
void UIElement::getRawOpacitites(FLOAT_TYPE& fOffOpac, FLOAT_TYPE& fOnOpac, FLOAT_TYPE& fOff2Opac)
{
	if(myCachedOffOpacity < 0)
	{
		if(this->doesPropertyExist(PropertyOpacity))
			myCachedOnOpacity = this->getNumProp(PropertyOpacity);
		else
			myCachedOnOpacity = 1.0;

		if(this->doesPropertyExist(PropertyOffOpacity))
			myCachedOffOpacity = this->getNumProp(PropertyOffOpacity);
		else
			// If no off opacity, assume 0.
			myCachedOffOpacity = 0.0;
		if(this->doesPropertyExist(PropertyOffOutOpacity))
			myCachedOff2Opacity = this->getNumProp(PropertyOffOutOpacity);
		else
			myCachedOff2Opacity = myCachedOffOpacity;
	}

	fOffOpac = myCachedOffOpacity;
	fOff2Opac = myCachedOff2Opacity;
	fOnOpac = myCachedOnOpacity;
}
/*****************************************************************************/
void UIElement::autoLayoutChildren()
{
	if(myChildrenLayoutType != AutolayoutColumn)
		return;

	// This is for cases where our children may have a fill parent flag, so we are sure we know
	// their sizes.
	adjustChildrenToScreen();

	FLOAT_TYPE fSpacing = this->getNumProp(PropertyLayoutVerticalSpacing);

	// No others supported atm.
	_ASSERT(myChildrenLayoutType == AutolayoutColumn);

	// Go over each child, measure sizes, set positions
	FLOAT_TYPE fCurrPosY = upToScreen(1.0);

	// Now, if we have the vertical auto-sizing flag,
	// compute the size of our children. If any are
	// recursively needed to be laid out, they should
	// have already done so.
	UIElement* pElem;
	SVector2D svCurrBox;
	FLOAT_TYPE fPadding, fPaddingTop;

	int iCurr, iNum = this->getNumChildren();
	if(this->getBoolProp(PropertyFitHeightToChildren))
	{
		FLOAT_TYPE fChildrenHeight = 0;
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
			pElem = as<UIElement>(this->getChild(iCurr));
			if(!pElem)
				ASSERT_CONTINUE;
			if(!pElem->getIsVisible())
				continue;

			// Note that we need to call the auto-layout children here
			// even though we may end up calling it twice, because
			// when we call this function from adjustToScreen(), we
			// make the call on self first, then on children.
			// But for this guy to work,  we need to first call on children,
			// then on self.
			pElem->autoLayoutChildrenRecursive();

			pElem->getBoxSize(svCurrBox);
			fPadding = pElem->getNumProp(PropertyMargin);
			fPaddingTop = pElem->getNumProp(PropertyMarginTop);
			fChildrenHeight += svCurrBox.y + fSpacing + fPadding*2.0 + fPaddingTop;
		}

		fChildrenHeight += this->getTitleBarHeight();

		if(fChildrenHeight > 0)
		{
			// Set our height
			this->setNumProp(PropertyHeight, fChildrenHeight);
			this->resetEvalCache(true);
		}
	}

	SVector2D svOwnSize;
	this->getBoxSize(svOwnSize);
	fCurrPosY -= svOwnSize.y/2.0;

	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pElem = as<UIElement>(this->getChild(iCurr));
		if(!pElem)
			ASSERT_CONTINUE;
		if(!pElem->getIsVisible())
			continue;

		pElem->getBoxSize(svCurrBox);
		if(svCurrBox.x <= 0 && svCurrBox.y <= 0)
			continue;

		fPadding = pElem->getNumProp(PropertyMargin);
		fPaddingTop = pElem->getNumProp(PropertyMarginTop);
		pElem->setCenter(FLOAT_TYPE_MAX, fCurrPosY + svCurrBox.y/2.0 + fPadding + fPaddingTop);

		fCurrPosY += svCurrBox.y + fSpacing + fPadding*2.0 + fPaddingTop;
	}
}
/*****************************************************************************/
void UIElement::onMouseWheelRecursive(FLOAT_TYPE fDelta)
{
	UIElement *pCurr = this;
	bool bResult = false;
	while(!bResult && pCurr)
	{
		bResult = pCurr->onMouseWheel(fDelta);
		pCurr = pCurr->getParent<UIElement>();
	}
}
/*****************************************************************************/
bool UIElement::applyMouseWheelDelta(FLOAT_TYPE fDeltaMulted)
{
	SVector2D svNewScroll(myExtraScrollOffset);
	svNewScroll.y += fDeltaMulted;
	setExtraScrollOffset(svNewScroll.x, svNewScroll.y, true);
	return true;
	/*
	LinkedToSelfElemIterator sli = linkedToSelfElemIteratorFirst(pParent);
	if(!sli.isEnd())
	{
		UISliderElement* pSlider = as<UISliderElement>(sli.getItem());
		_ASSERT(pSlider);
		if(pSlider)
		{
			FLOAT_TYPE fMaxValue = pSlider->getMaxValue();
			if(myExtraScrollOffset.y < -fMaxValue)
				myExtraScrollOffset.y = -fMaxValue;

			pSlider->setValue(-myExtraScrollOffset.y);
		}
	}
	*/
}
/*****************************************************************************/
bool UIElement::onMouseWheel(FLOAT_TYPE fDelta)
{
	if(!myCachedDoesSupportAutoScrolling && !getAllowWheelScroll())
		return false;

	/*
	// Moved to getRelatedSlider()
	// Hack: go up two levels to start.
	UIElement *pParent = this->getParent<UIElement>();
	if(!pParent)
		return false;

	pParent = pParent->getParent<UIElement>();
	if(!pParent)
		return false;
		*/
	return applyMouseWheelDelta(fDelta*MOUSEWHEEL_SCROLL);
}
/*****************************************************************************/
// Valid tokens in the path:
// .. - go up one level
// . - take the element at the current level
// $0 - child at 0th index, then 1, etc.
// elemId - find the id of the element at the current parent
UIElement* UIElement::getElementAtPath(const char* pcsPath)
{
	// Note that now we are complete paths. Tokenize it and see
	// what we get.
	theSharedString = pcsPath;
	theSharedString2 = "/";
	TokenizeUtils::tokenizeStringToCharPtrsInPlace(theSharedString.c_str(), theSharedString2, thePathVector);

	UIElement *pStartElem = this;
	int iCurrToken, iNumTokens = thePathVector.size();
	if(iNumTokens <= 0)
		ASSERT_RETURN_NULL;

	const char* pcsToken;
	for(iCurrToken = 0; iCurrToken < iNumTokens; iCurrToken++)
	{
		pcsToken = thePathVector[iCurrToken];
		if(strcmp(pcsToken, "..") == 0)
			pStartElem = pStartElem->getParent<UIElement>();
		else if(strcmp(pcsToken, ".") == 0)
		{
			// This means we want the current elem. Do nothing 
			// since the result is pStartElem.
		}
		else if(strlen(pcsToken) > 1 && pcsToken[0] == '$')
		{
			int iChildIndex = atoi(pcsToken + 1);
			pStartElem = pStartElem->getChild<UIElement>(iChildIndex);
		}
		else
		{
			// This can only be a name of the element. Find it.
			pStartElem = pStartElem->getChildById(pcsToken, false, true);
		}

		if(!pStartElem)
			return NULL;
	}

	return pStartElem;
}
/*****************************************************************************/
void UIElement::getAnimBoxSize(SVector2D& svOut)
{
	svOut.set(0, 0);

	// Try to get the anim file size
	int iFrameOut;
	STRING_TYPE strFullAnimName;
	getFullTopAnimName(strFullAnimName);
	CachedSequence* pSeq = NULL;
	if(strFullAnimName.length() > 0)
		pSeq = getDrawingCache()->getCachedSequence(strFullAnimName.c_str(), &iFrameOut);

	const CHAR_TYPE* pcsFulAnimName;
	if(pSeq)
		pcsFulAnimName = pSeq->getName();
	else
		pcsFulAnimName = NULL;
	if(pcsFulAnimName && strlen(pcsFulAnimName) > 0 && !IS_STRING_EQUAL(pcsFulAnimName, PROPERTY_NONE))
	{
		svOut.x = getTextureManager()->getWidth(pcsFulAnimName);
		svOut.y = getTextureManager()->getHeight(pcsFulAnimName);
	}

	// Now, if we're *still* 0,0, see if we're a text-only element, and estimate
	// its text size.
	// This way, we can select and move them, too.
	if(this->doesPropertyExist(PropertyText) && (int)svOut.x == 0 && (int)svOut.y == 0)
	{
		this->getTextAsString(theCommonString3);
		RenderUtils::measureText(getParentWindow(), theCommonString3.c_str(), this->getCachedFont(), this->getCachedFontSize(), this->getNumProp(PropertyTextWidth), svOut);
		if(svOut.x > 0 && svOut.y > 0)
		{
			svOut.x += upToScreen(10.0);
			svOut.y += upToScreen(10.0);
		}
	}

	if((int)svOut.x == 0 && (int)svOut.y == 0)
	{
		svOut.set(16, 16);
		svOut.upToScreen();
	}
}
/*****************************************************************************/
void UIElement::getElementsListAtScreenPositionRecursive(int iX, int iY, TUIElementVector& rVecOut)
{
	// See self
	SVector2D svPos;
	//FLOAT_TYPE fOpacity;
	this->getGlobalPosition(svPos, NULL, NULL);

	// First, try own box
	SVector2D svOwnSize;
	bool bHaveSize = this->doesPropertyExist(PropertyWidth);
	if(bHaveSize)
		this->getBoxSize(svOwnSize);

	if(svOwnSize.x == 0)
		this->getAnimBoxSize(svOwnSize);

	if(svOwnSize.x > 0 && svOwnSize.y > 0)
	{
		SVector2D svTestPoint(iX, iY);
		SRect2D srOwnRect;
		srOwnRect.setFromCenterAndSize(svPos, svOwnSize);
		if(srOwnRect.doesContain(svTestPoint))
		{
			// Add self to list
			if(std::find(rVecOut.begin(), rVecOut.end(), this) == rVecOut.end())
				rVecOut.push_back(this);
		}
	}


	// Recurse on children
	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;
		pChild->getElementsListAtScreenPositionRecursive(iX, iY, rVecOut);
	}
}
/*****************************************************************************/
void UIElement::getAllElementsRecursive(TUIElementVector& rVecOut)
{
	rVecOut.push_back(this);

	// Recurse on children
	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;
		pChild->getAllElementsRecursive(rVecOut);
	}

}
/*****************************************************************************/
void UIElement::getSelectedElementsRecursive(TUIElementVector& rVecOut)
{
#ifdef ALLOW_UI_EDITOR
	if(myIsEditorSelected)
	{
		if(std::find(rVecOut.begin(), rVecOut.end(), this) == rVecOut.end())
			rVecOut.push_back(this);

		// Note that we don't add children, even if selected,
		// since they will move with us
		return;
	}

	// Recurse on children
	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;
		pChild->getSelectedElementsRecursive(rVecOut);
	}
#endif
}
/*****************************************************************************/
bool UIElement::getIsEditorSelected()
{
#ifdef ALLOW_UI_EDITOR
	return myIsEditorSelected;
#else
	return false;
#endif
}
/*****************************************************************************/
void UIElement::setIsEditorSelected(bool bValue)
{
#ifdef ALLOW_UI_EDITOR
	myIsEditorSelected = bValue;
#endif
}
/*****************************************************************************/
void UIElement::setInitEditorPos()
{
#ifdef ALLOW_UI_EDITOR
	myInitEditorPos.x = this->getNumProp(PropertyX);
	myInitEditorPos.y = this->getNumProp(PropertyY);
#endif
}
/*****************************************************************************/
void UIElement::getInitEditorPos(SVector2D& svInit)
{
#ifdef ALLOW_UI_EDITOR
	svInit = myInitEditorPos;
#endif
}
/*****************************************************************************/
void UIElement::setAccelImage(AccelImage* pNewImage)
{
	//SCOPED_LOCK(myAccelImageLock);
	SCOPED_MUTEX_LOCK(&myAccelImageLock);
	delete myAccelImage;
	myAccelImage = pNewImage;
}
/*****************************************************************************/
bool UIElement::getAllowValuePropagation(FLOAT_TYPE fNewValue, bool bIsChangingContinuously, UIElement* pOptSourceElem, UIElement* pLinkedToElem)
{
	// This may result in scenes where we cancel the rendering, but this will remember
	// the former value that never got shown as the last one.
	/*
	bool bAllowValuePropagation = true;
	if(myPrevChangedToValue != FLOAT_TYPE_MAX && bIsChangingContinuously && this->doesPropertyExist(PropertyUiObjMinSlidingValueDelta))
	{
		FLOAT_TYPE fDelta = this->getNumProp(PropertyUiObjMinSlidingValueDelta);
		if(fDelta > 0)
			bAllowValuePropagation = fabs(fNewValue - myPrevChangedToValue) >= fDelta;
	}
	if(bAllowValuePropagation && bIsChangingContinuously)
		myPrevChangedToValue = fNewValue;
	if(!bIsChangingContinuously)
		myPrevChangedToValue = FLOAT_TYPE_MAX;

	return bAllowValuePropagation;
	*/
	return true;
}
/*****************************************************************************/
void UIElement::setDoPostponeRendering(bool bValue)
{
	this->setBoolProp(PropertyPostponeRendering, bValue);
	myCachedDoPostponeRendering = bValue;
}
/*****************************************************************************/
void UIElement::setHasNoDefinition(bool bValue)
{
	this->setBoolProp(PropertyUioHasNoDefinition, bValue);
	myCachedHasNoDefinition = bValue;
}
/*****************************************************************************/
UIPlane* UIElement::getUIPlane()
{
	return dynamic_cast<UIPlane*>(this->getParentPlane());
}
/*****************************************************************************/
bool UIElement::handleActionUpParentChain(string& strAction, bool bCallRMBPath, IBaseObject* pData)
{
	UIElement* pHandleElem = this;
	while(pHandleElem)
	{
		bool bResult = false;
		if(bCallRMBPath)
			bResult = pHandleElem->handleRMBAction(strAction, this, pData);
		else
			bResult = pHandleElem->handleAction(strAction, this, pData);
		if(bResult)
			return true;
		pHandleElem = pHandleElem->getParent<UIElement>();
	}

	return false;
}
/*****************************************************************************/
void UIElement::onSizeChangedInternal()
{
	// If we support auto-scroll, we need to set the slider max
	// on every resize
	if(!myCachedDoesSupportAutoScrolling)
		return;

	UISliderElement* pSlider = getRelatedSlider(NULL);
	if(!pSlider)
		return;

	// Otherwise, set the max scroll on this slider
	SVector2D svOwnSize;
	this->getBoxSize(svOwnSize);

	SRect2D srUiBox;
	this->getChildrenBBox(srUiBox);

	bool bIsEnabled = (srUiBox.h > svOwnSize.y);
	pSlider->setIsEnabled(bIsEnabled);
	if(bIsEnabled)
	{
		pSlider->setMinValue(0);
		pSlider->setMaxValue(srUiBox.h - svOwnSize.y);
	}
	else
	{
		pSlider->setMaxValue(0.01);
	}
}
/*****************************************************************************/
UISliderElement* UIElement::getRelatedSlider(UIElement* pOptStartParent)
{
	UIElement* pFinalStartParent = pOptStartParent;
	if(!pOptStartParent)
	{
		// Start two levels up
		pFinalStartParent = this->getParent<UIElement>();
		if(pFinalStartParent && pFinalStartParent->getParent())
			pFinalStartParent = pFinalStartParent->getParent<UIElement>();
	}

	// This isn't necessarily an error - the zoom window, for example,
	// asks for one to see if it exists, because that's optional.
	if(!pFinalStartParent)
		return NULL;

	UISliderElement* pSlider;
	LinkedToSelfElemIterator sli;
	for(sli = linkedToSelfElemIteratorFirst(pFinalStartParent); !sli.isEnd(); sli++)
	{
		pSlider = as<UISliderElement>(sli.getItem());
		if(pSlider)
			return pSlider;
	}

	return NULL;
/*
	// Try to find one in our parent, then grandparent, etc.
	// that's linked to us.
	UIElement* pCurrElem;
	UISliderElement* pSlider;
	int iCurrChild, iNumChildren;

	for(pCurrElem = this->getParent<UIElement>(); pCurrElem != NULL; pCurrElem = pCurrElem->getParent<UIElement>())
	{
		// Note that we only enumerate the top-level children. This
		// means that if we have a slider in a sibling as a child of
		// that sibling, we won't find it.
		iNumChildren = pCurrElem->getNumChildren();
		for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
		{
			pSlider =  as<UISliderElement>(pCurrElem->getChild(iCurrChild));
			if(!pSlider)
				continue;

			if(pSlider->getLinkedToElement<UIElement>() == this)
				return pSlider;
		}
	}

	return NULL;
	*/
}
/*****************************************************************************/
void UIElement::getChildrenBBox(SRect2D& srBBoxOut)
{
	UIElement* pElem;
	SVector2D svCurrPos, svCurrBox;
	srBBoxOut.reset();
	SRect2D srCurrRect;
	int iCurr, iNum = this->getNumChildren();

/*
	if(iNum == 0 && this->getIsVisible())
	{
		this->getLocalPosition(svCurrPos);
		this->getBoxSize(svCurrBox);
		srBBoxOut.setFromCenterAndSize(svCurrPos, svCurrBox);
		return;
	}
*/

	SVector2D svOwnCenter;
	this->getBoxSize(svOwnCenter);
	svOwnCenter *= 0.5;
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pElem = as<UIElement>(this->getChild(iCurr));
		if(!pElem)
			ASSERT_CONTINUE;
		if(!pElem->getIsVisible())
			continue;

		pElem->getLocalPosition(svCurrPos);
		pElem->getBoxSize(svCurrBox);
		// Include no size elems as a point
// 		if(svCurrBox.x <= 0 || svCurrBox.y <= 0)
// 			continue;

		// Make it be based on the top-corner
		svCurrPos += svOwnCenter;
		svCurrPos -= myExtraScrollOffset;

		srCurrRect.setFromCenterAndSize(svCurrPos, svCurrBox);
		if(srBBoxOut.w == 0)
			srBBoxOut = srCurrRect;
		else
			srBBoxOut.includeRect(srCurrRect);
		/*
		pElem->getChildrenBBox(srCurrRect);
		if(srBBoxOut.w == 0)
			srBBoxOut = srCurrRect;
		else
			srBBoxOut.includeRect(srCurrRect);
			*/

	}
}
/*****************************************************************************/
const char* UIElement::getRadioGroupName()
{
	// See if we need to generate our radio button automatically
	if(this->doesPropertyExist(PropertyAutoGenerateRadioGroupNameFromParentAtLevel))
	{
		int iCurrLevel, iNumLevelsUp = getNumProp(PropertyAutoGenerateRadioGroupNameFromParentAtLevel);
		if(iNumLevelsUp > 0)
		{
			UIElement* pParent;
			for(pParent = this, iCurrLevel = 0; pParent && iCurrLevel < iNumLevelsUp; iCurrLevel++)
				pParent = pParent->getParent<UIElement>();

			if(pParent)
			{
				const char* pcsParentName = pParent->getStringProp(PropertyId);
				theCommonString = this->getStringProp(PropertyRadioGroup);
				theCommonString += pcsParentName;
				this->setStringProp(PropertyRadioGroup, theCommonString.c_str());
				this->setNumProp(PropertyAutoGenerateRadioGroupNameFromParentAtLevel, 0);
			}
		}
	}

	return this->getStringProp(PropertyRadioGroup);
}
/*****************************************************************************/
void UIElement::onThreadedThumbnailRenderFinished(const IBaseImage* pThumbnail, IThumbnailProvider* pProvider, int iIndex, const char* pcsOptParentId)
{
	//SCOPED_LOCK(myPendingAnimImageLock);
	SCOPED_MUTEX_LOCK(&myPendingAnimImageLock);
	if(!myPendingAnimImage)
		myPendingAnimImage = getParentWindow()->allocateImage();
		//myPendingAnimImage = new Image;
	else
		myPendingAnimImage->reset();
		//myPendingAnimImage->isValid(false);
	//*myPendingAnimImage = *pThumbnail;
	myPendingAnimImage->setFrom(pThumbnail);

	_ASSERT(myPendingAnimImage);

	ActivityManager::getInstance()->singleValueChanged();
}
/*****************************************************************************/
void UIElement::setOrUpdateAccelImage(const IBaseImage* pImage, bool bOnlySetData)
{
	//SCOPED_LOCK(myAccelImageLock);
	SCOPED_MUTEX_LOCK(&myAccelImageLock);
	if(!myAccelImage)
		myAccelImage = getParentWindow()->allocateAccelImage();
	myAccelImage->updateFrom(pImage, NULL, bOnlySetData);
}
/*****************************************************************************/
bool UIElement::getIsFullyHiddenCumulative()
{
	UIElement* pParent = NULL;
	UIElement* pCurr;
	for(pCurr = this; pCurr; pCurr = pParent)
	{
		if(pCurr->getIsFullyHidden() == true)
			return true;

		pParent = pCurr->getParent<UIElement>();
	}
	return false;
}
/*****************************************************************************/
bool UIElement::getIsVisibleCumulative() const
{
	const UIElement* pParent = NULL;
	const UIElement* pCurr;
	for(pCurr = this; pCurr; pCurr = pParent)
	{
		if(pCurr->getIsVisible() == false)
			return false;

		pParent = pCurr->getParent<UIElement>();
	}

	return true;
}
/*****************************************************************************/
void UIElement::setIsVisible(bool bValue)
{
	myIsVisible = bValue;

	// Now, call on visible changed
	onVisibleChangedCumulativeRecursive(bValue);
}
/*****************************************************************************/
void UIElement::onVisibleChangedCumulativeRecursive(bool bValue)
{
	onVisibleChangedCumulative(bValue);

	UIElement* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = dynamic_cast<UIElement*>(this->getChild(iCurrChild));
		if(!pChild)
			ASSERT_CONTINUE;

		pChild->onVisibleChangedCumulativeRecursive(bValue);
	}

}
/*****************************************************************************/
void UIElement::setFillParentXLessPadding(FLOAT_TYPE fValue)
{
	this->setNumProp(PropertyFillXLess, fValue);
	myCachedFillParentXLessPadding = fValue;
}
/*****************************************************************************/
void UIElement::setFillParentYLessPadding(FLOAT_TYPE fValue)
{
	this->setNumProp(PropertyFillYLess, fValue);
	myCachedFillParentYLessPadding = fValue;
}
/*****************************************************************************/
void UIElement::onDoubleClick(TTouchVector& vecTouches, bool bIgnoreActions)
{
	if(this->doesPropertyExist(PropertyDoubleClickAction) && !bIgnoreActions)
	{
		string strTempDataSourceStorage;
		const char *pcsTargetDataSource = NULL;
		if(this->doesPropertyExist(PropertyTargetDataSource))
		{
			this->getAsString(PropertyTargetDataSource, strTempDataSourceStorage);
			pcsTargetDataSource = strTempDataSourceStorage.c_str();
		}
		if(!pcsTargetDataSource && this->getHaveGenericDataSource())
			pcsTargetDataSource = this->getGenericDataSource();

		string strTempAction = this->getStringProp(PropertyDoubleClickAction);

		// Try to handle this up the chain
		BaseCharPtr rPtr(pcsTargetDataSource);
		if(!handleActionUpParentChain(strTempAction, false, &rPtr))
			this->getUIPlane()->processAction(this, strTempAction, pcsTargetDataSource, PropertyDoubleClickActionValue, PropertyNull, PropertyNull, MouseButtonLeft);
	}
}
/*****************************************************************************/
IUndoItem* UIElement::provideUndoItemForTargetIdentifierRecursive()
{
	IUndoItem* pRes = NULL;
	UIElement* pCurrElem;
	for(pCurrElem = this; !pRes && pCurrElem; pCurrElem = pCurrElem->getParent<UIElement>())
		pRes = pCurrElem->provideUndoItemForTargetIdentifier();

	return pRes;
}
/*****************************************************************************/
void UIElement::invalidateAndUpdateCursor()
{
	invalidateCursorNoUpdate();
	if(myPushedCursorId >= 0)
	{
		getParentWindow()->popCursor(myPushedCursorId);
		myPushedCursorId = getParentWindow()->pushCursor(getOwnCursorType());
		_ASSERT(myPushedCursorId >= 0);
	}
}
/*****************************************************************************/
void UIElement::getFullPath(string& strOut)
{
	strOut = "";
	UIElement* pElem;
	for(pElem = this; pElem; pElem = as<UIElement>(pElem->getParent()))
	{
		if(pElem != this)
			strOut = "/" + strOut;
		strOut = pElem->getStringProp(PropertyId) + strOut;
	}
}
/*****************************************************************************/
UISplitterElement* UIElement::split(DirectionType eDir, int iOverrideChildPos)
{
	return getUIPlane()->splitElement(this, eDir, iOverrideChildPos);
}
/*****************************************************************************/
void UIElement::parentExistingElement(UIElement* pElem, int iOverrideChildPos)
{
	if(!pElem)
		return;
	this->parentExistingNode(pElem, iOverrideChildPos);
	invalidateSizingInfos();
	autoLayoutChildrenRecursive();
}
/*****************************************************************************/
FLOAT_TYPE UIElement::getFillParentYLessPadding() const
{
	return myCachedFillParentYLessPadding; 
#ifdef USE_OWN_FILE_DIALOG
	AHTUGN!
		// While we need to add the top padding here to the fill to account for
		// title space, if we do the below we will cause a gap for tab-based
		// titles, since they have titles, but no offsets. 
		// Ntoe that we have to also shift the innert contents of the child
		// if we're filling, but we don't do this....
#endif
	/*
	FLOAT_TYPE fRes = myCachedFillParentYLessPadding; 

	// Do we need to recurse all the way up?
	if(fRes >= 0.0 && getParent<UIElement>()) 
		fRes += getParent<UIElement>()->getTitleBarHeight()/2.0; 
	
	return fRes; 
	*/
}
/*****************************************************************************/
void UIElement::deleteChildrenDelayed()
{
	UIPlane* pPlane = getUIPlane();
	int iCurr, iNum = this->getNumChildren();
	UIElement* pElem;
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pElem = dynamic_cast<UIElement*>(this->getChild(iCurr));
		if(!pElem)
			ASSERT_CONTINUE;
		//pPlane->deleteElement(pElem);
		pPlane->scheduleForDeletion(pElem);
	}
}
/*****************************************************************************/
void UIElement::resetUiTablesRecursive()
{
	UITableElement* pTable;
	pTable = dynamic_cast<UITableElement*>(this);
	if(pTable)
	{
		pTable->resetCachedCellDef();
		pTable->markDirty();

		// And make sure we immediately recreate them
		// NEW: This is commented out to avoid reloading
		// thumbnails every refresh if they're not visible.
		// If we hit problems reloading tables, uncomment it
		// again.
		//pTable->ensureRowsUpToDate();
	}

	// Go through children
	int iCurr, iNum = this->getNumChildren();
	for(iCurr = 0; iCurr < iNum; iCurr++)
		this->getChild<UIElement>(iCurr)->resetUiTablesRecursive();
}
/*****************************************************************************/
void UIElement::setText(const CHAR_TYPE* pcsText)
{
	// Auto shorten if necessary
	FLOAT_TYPE fMaxShorteningWidth = this->getNumProp(PropertyTextShorteningWidth);
	if(doesPropertyExist(PropertyTextShorteningWidthLessPadding) || doesPropertyExist(PropertyTextShorteningWidthSizeMult))
	{
		SVector2D svNewSize;
		UIElement* pCurrElem;
		for(pCurrElem = this; pCurrElem && svNewSize.x <= FLOAT_EPSILON; pCurrElem = pCurrElem->getParent<UIElement>())
			pCurrElem->getBoxSize(svNewSize);

		if(doesPropertyExist(PropertyTextShorteningWidthLessPadding))
			fMaxShorteningWidth = svNewSize.x - this->getNumProp(PropertyTextShorteningWidthLessPadding)*2.0;
		else
			fMaxShorteningWidth = svNewSize.x*this->getNumProp(PropertyTextShorteningWidthSizeMult);
	}

	if (fMaxShorteningWidth > 0 && pcsText)
	{
		theSharedString = pcsText;
		RenderUtils::shortenString(getParentWindow(), theSharedString, getCachedFont(), getCachedFontSize(), fMaxShorteningWidth, "...");
		this->setStringProp(PropertyText, theSharedString.c_str());
	}
	else
		this->setStringProp(PropertyText, pcsText);

	myIsCachedTextValid = false;
}
/*****************************************************************************/
void UIElement::ensureFlexibleWidthValid()
{
	if(!this->doesPropertyExist(PropertyTextWidthLessPadding))
		return;

	SVector2D svNewSize;
	const UIElement* pCurrElem;
	for(pCurrElem = this; pCurrElem && svNewSize.x <= FLOAT_EPSILON; pCurrElem = pCurrElem->getParent<UIElement>())
		pCurrElem->getBoxSize(svNewSize);

	myTextWidth = svNewSize.x - this->getNumProp(PropertyTextWidthLessPadding)*2.0;
	onTextWidthUpdated();
}
/*****************************************************************************/
void UIElement::getTextAsString(string &strOut, const char* pcsOptFloatConvFormat)
{
	this->getAsString(PropertyText, strOut, pcsOptFloatConvFormat);
}
/*****************************************************************************/
const char* UIElement::getText()
{
	return this->getStringProp(PropertyText);
}
/*****************************************************************************/
FLOAT_TYPE UIElement::getTextAsNumber()
{
	return this->getAsNumber(PropertyText);
}
/*****************************************************************************/
void UIElement::getTextAsStringOrExpression(string &strOut, const char* pcsOptFloatConvFormat)
{
	if(this->getPropertyDataType(PropertyText) == PropertyDataExpression)
		this->getExpressionProp(PropertyText)->getRawExpressionWithPrefix(strOut);
	else
		getTextAsString(strOut, pcsOptFloatConvFormat);
}
/*****************************************************************************/
void UIElement::setTextExt(const CHAR_TYPE* format, ...)
{
	va_list argptr;

	const int iStaticBufferSize = 2048;
	char* pcsFinalPtr = NULL;
	char pcsMessage[iStaticBufferSize];
	char *pcsDynMessage = NULL;

	va_start(argptr, format);
#ifdef WINDOWS
	int iNeededBufferLen = _vscprintf(format, argptr);
#else
	int iNeededBufferLen = vsnprintf(NULL, NULL, format, argptr);
#endif
	va_end(argptr);

	if(iNeededBufferLen < iStaticBufferSize - 1)
	{
		va_start(argptr, format);
		vsprintf(pcsMessage, format, argptr);
		va_end(argptr);
		pcsFinalPtr = pcsMessage;
	}
	else
	{
		pcsDynMessage = new char[iNeededBufferLen + 2];
		va_start(argptr, format);
		vsprintf(pcsDynMessage, format, argptr);
		va_end(argptr);

		pcsFinalPtr = pcsDynMessage;
	}

	this->setText(pcsFinalPtr);

/*
	StringUtils::replaceAllInstances(STR_LIT("%"), STR_LIT("%%"), rItem.myValue);
	gLog(rItem.myValue.c_str());
*/

	if(pcsDynMessage)
		delete[] pcsDynMessage;

}
/*****************************************************************************/
void UIElement::ensureCachedTextValid()
{
	if(myIsCachedTextValid && getAllowTextCaching())
		return;

	if(this->doesPropertyExist(PropertyText))
		this->getTextAsString(myCachedText, DEFAULT_FLOAT_CONV_FORMAT);
	else
		myCachedText = "";
	myIsCachedTextValid = true;
}
/*****************************************************************************/
void UIElement::onPropertyChanged(PropertyType eProp)
{
	// For now, just reset all caches
	this->resetAllCaches(true);

	// If this is one of our sizing/positioning props, 
	// re-adjust to screen:
	if(eProp == PropertyX
	   || eProp == PropertyY
	   || eProp == PropertyWidth
	   || eProp == PropertyHeight
	   || eProp == PropertyScale
	   )
		this->adjustToScreen();
}
/*****************************************************************************/
void UIElement::updateEverything()
{
	this->resetAllCaches(true);
	this->adjustToScreen();

	this->updateDataRecursive();
}
/*****************************************************************************/
};
