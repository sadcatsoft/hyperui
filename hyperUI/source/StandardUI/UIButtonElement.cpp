#include "stdafx.h"

#define BUTTON_PUSH_TIME		0.15
#define NONRADIO_PUSH_TIME		0.15
#define HOVER_FADE_TIME			0.15

//#define DEFAULT_BUTTON_PRESS_ANIM	"uiTouchAnim"
#define DEFAULT_BUTTON_PRESS_ANIM	""

namespace HyperUI
{
#ifndef MAC_BUILD
string UIButtonElement::theSharedString;
#endif
/*****************************************************************************/
UIButtonElement::UIButtonElement(UIPlane* pParentPlane)
	: UIElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void UIButtonElement::onAllocated(IBaseObject* pData)
{
	UIElement::onAllocated(pData);
	myCurrSelEnum = 0;
	myNormalRenderStage = -1;
	myAllowPressing = false;
	myIgnoreWhichMouseButtonIsPressed = false;
	myIsSimulatingClick = false;
}
/*****************************************************************************/
void UIButtonElement::postInit(void)
{
	UIElement::postInit();

#ifdef _DEBUG
	string strDbPath, strDbName;
	this->getFullPath(strDbPath);
	strDbName = strDbPath + " - pushedDownValue";
	myPushedDownValue.setDebugName(strDbName.c_str());
	strDbName = strDbPath + " - hoverAnimValue";
	myHoverAnim.setDebugName(strDbName.c_str());
#endif

	myIsEnum = (this->doesPropertyExist(PropertyUioMainEnum) && this->getEnumPropCount(PropertyUioMainEnum) > 0);

	if(this->doesPropertyExist(PropertyChecked))
		setIsPushed(this->getBoolProp(PropertyChecked), true);
}
/*****************************************************************************/
int UIButtonElement::getSelectedEnumValue(void)
{
	if(this->getEnumPropCount(PropertyUioMainEnum) <= 0)
		return -1;
	else
		return myCurrSelEnum;
}
/*****************************************************************************/
void UIButtonElement::setSelectedEnumValue(int iNewValue)
{
	myCurrSelEnum = iNewValue;
}
/*****************************************************************************/
void UIButtonElement::incrementEnumValue(void)
{
	int iNumEnumValues = this->getEnumPropCount(PropertyUioMainEnum);
	if(iNumEnumValues <= 0)
		return;

	myCurrSelEnum = (myCurrSelEnum + 1)%iNumEnumValues;
}
/*****************************************************************************/
void UIButtonElement::onPressed(TTouchVector& vecTouches)
{
	UIElement::onPressed(vecTouches);

//	if(!this->getIsRadio() || !this->getIsPushed())
	if(!(this->getIsRadio() && this->getIsPushed() && (getParentWindow()->getCurrMouseButtonDown() == MouseButtonLeft || myIgnoreWhichMouseButtonIsPressed)))
	if(getIsEnabled())
	{
		myAllowPressing = true;

		if(getParentWindow()->getCurrMouseButtonDown() == MouseButtonLeft || myIgnoreWhichMouseButtonIsPressed)
		{
			if(!this->doesPropertyExist(PropertyRadioGroup))
				this->setIsPushed(true);
		}
	}
}
/*****************************************************************************/
void UIButtonElement::onMouseEnter(TTouchVector& vecTouches)
{
	UIElement::onMouseEnter(vecTouches);	
	//myAllowPressing = true;
}
/*****************************************************************************/
void UIButtonElement::onMouseLeave(TTouchVector& vecTouches)
{
	UIElement::onMouseLeave(vecTouches);	

	myAllowPressing = false;
	if(getParentWindow()->getCurrMouseButtonDown() == MouseButtonLeft || myIgnoreWhichMouseButtonIsPressed)
	{
		if(!(this->getIsRadio() && this->getIsPushed()))
			if(getIsEnabled())
				this->setIsPushed(false);
	}
}
/*****************************************************************************/
void UIButtonElement::onReleased(TTouchVector& vecTouches, bool bIgnoreActions)
{

	UIElement::onReleased(vecTouches, bIgnoreActions);

	MouseButtonType eMouseButton = getParentWindow()->getCurrMouseButtonDown();

	//if(!this->getIsRadio()) //  || !this->getIsPushed())
	if(!(this->getIsRadio() && this->getIsPushed() && (eMouseButton == MouseButtonLeft || myIgnoreWhichMouseButtonIsPressed)))
	if(getIsEnabled())
	{
		if(myAllowPressing)
		{
			if(eMouseButton == MouseButtonLeft || myIgnoreWhichMouseButtonIsPressed)
			{
				if(getIsEnum())
					incrementEnumValue();
				handleRadioGroup();
				handleTargetElementUpdate(false);
			}

			if(myCallbacks)
				myCallbacks->onButtonClicked(this, eMouseButton);
		}
		else
			this->setIsPushed(false);
	}

	myAllowPressing = false;
}
/*****************************************************************************/
void UIButtonElement::render(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	// See if we want to postpone rendering
	// Bad. There's a copy in UIElement, and anything that overrides render() won't use this...
	if(this->getDoPostponeRendering() && getUIPlane()->getRenderingPass() != RenderingPostopnedElements)
	{
		getUIPlane()->addPostponedElement(this, svScroll, fOpacity, fScale);
		return;
	}


	preRender(svScroll, fOpacity, fScale);

	if(this->getIsEnum())
		theSharedString = this->getEnumPropValue(PropertyUioEnumAnims, this->getSelectedEnumValue());		
	else
		this->getFullTopAnimName(theSharedString);

	SVector2D svAnimOffset, svFinalScroll;
	getTopAnimOffset(svAnimOffset);
	svFinalScroll = svAnimOffset + svScroll;
	
	FLOAT_TYPE fPushedValue  = myPushedDownValue.getValue();
	FLOAT_TYPE fNormalPartOp = fOpacity;
	if(fPushedValue > 0 && this->getBoolProp(PropertyDrawOnlyPressedState))
		fNormalPartOp *= (1.0 - fPushedValue);

	if(fNormalPartOp > 0.0)
		renderInternal(theSharedString, svFinalScroll, fNormalPartOp, fScale);

/*
	if(myNormalRenderStage < 0 && theSharedString.length() > 0 && theSharedString != PROPERTY_NONE)
	{
		// Get our render stage
		int iAtlasFrameDummy;
		CachedSequence* pSeq = getDrawingCache()->getCachedSequence(theSharedString.c_str(), &iAtlasFrameDummy);
		myNormalRenderStage = pSeq->getRenderStage();
	}
*/

	// Render the hover back
	// We need this all the time to make sure we don't skip the activity manager callback.
	FLOAT_TYPE fHoverValue = myHoverAnim.getValue();
	bool bRenderHoverOnTop = this->getBoolProp(PropertyUioRenderHoverOnTop);
	if(!bRenderHoverOnTop && this->getIsEnabled() && this->doesPropertyExist(PropertyHoverImage))
	{
		if(fHoverValue > 0)
		{
			theSharedString = this->getStringProp(PropertyHoverImage);
			renderInternal(theSharedString, svFinalScroll, fOpacity*fHoverValue, fScale);
		}
	}

	renderTextInternal(svScroll, fOpacity, fScale, false);

	// Render the animated button stage if pushed.
	if(fPushedValue > 0.0 && this->doesPropertyExist(PropertySecondaryImage))
	{
		// Get it and render it with the right opacity
		theSharedString = this->getStringProp(PropertySecondaryImage);

//		if(myPushedDownValue.getEndValue() <= FLOAT_EPSILON && theSharedString.find(":") == string::npos)
//			theSharedString += "FadeOut";

		// Set the render stage to be the same as this button
// 		int iAtlasFrameDummy;
// 		CachedSequence* pSeq = getDrawingCache()->getCachedSequence(theSharedString.c_str(), &iAtlasFrameDummy);
// 		if(myNormalRenderStage >= 0 && myNormalRenderStage != pSeq->getRenderStage())
// 			pSeq->setRenderStage(myNormalRenderStage);
		
		renderInternal(theSharedString, svFinalScroll, fOpacity*fPushedValue, fScale);

		// Render text
		renderTextInternal(svScroll, fOpacity*fPushedValue, fScale, true);
	}
	else if(fPushedValue > 0.0)
	{
		// Render the touch anim
		theSharedString = DEFAULT_BUTTON_PRESS_ANIM;
		if(theSharedString.length() > 0)
			renderInternal(theSharedString, svScroll, fOpacity*fPushedValue, fScale);
	}

	if(bRenderHoverOnTop && this->getIsEnabled() && this->doesPropertyExist(PropertyHoverImage))
	{
		FLOAT_TYPE fHoverValue = myHoverAnim.getValue();
		if(fHoverValue > 0)
		{
			theSharedString = this->getStringProp(PropertyHoverImage);
			renderInternal(theSharedString, svFinalScroll, fOpacity*fHoverValue, fScale);
		}
	}

	renderChildren(svScroll, fOpacity, fScale);

	postRender(svScroll, fOpacity, fScale);
}
/*****************************************************************************/
bool UIButtonElement::getIsPushed(void)
{
	// Careful here - we're pushed depending on the transition direction.
	FLOAT_TYPE fValue = myPushedDownValue.getValue();
	if(myPushedDownValue.getEndTime() <= Application::getInstance()->getGlobalTime(ClockUiPrimary))
	{
		if(fValue < FLOAT_EPSILON)
			return false;
		if(fValue >= 1.0 -  FLOAT_EPSILON)
			return true;
	}

	if(myPushedDownValue.getStartValue() < myPushedDownValue.getEndValue())
		return true;
	else
		return false;
}
/*****************************************************************************/
void UIButtonElement::setIsPushed(bool bIsPushed, bool bInstant)
{
	GTIME lTime = Application::getInstance()->getGlobalTime(ClockUiPrimary);
	FLOAT_TYPE fValue = myPushedDownValue.getValue();
	FLOAT_TYPE fTargetValue = 0, fStartValue = 1;

	if(bIsPushed)
	{
		fStartValue = 0.0;
		fTargetValue = 1.0;
	}

	FLOAT_TYPE fTime;
	if(this->doesPropertyExist(PropertyRadioGroup))
	{
		if(fabs(fTargetValue - fValue) < FLOAT_EPSILON)
			return;
		fTime = BUTTON_PUSH_TIME;
	}
	else
	{
		fTime = NONRADIO_PUSH_TIME;
	}

	// Start state animation
	// If we're already in the state we're in, do nothing:
	if(bIsPushed != getIsPushed() && /* !this->getIsRadio() && */
		// This to allow the dragged button to fade out after initially lighting up		
		(!bIsPushed || (bIsPushed && !getParentWindow()->getDragDropManager()->isDragging()))		
		)
	{
 		myPushedDownValue.setAnimation(fStartValue, fTargetValue, fTime, ClockUiPrimary);
		myPushedDownValue.resetCache();
	}

	if(bInstant)
	{
		if(bIsPushed)
			myPushedDownValue.setNonAnimValue(1.0);
		else
			myPushedDownValue.setNonAnimValue(0.0);
	}
}
/*****************************************************************************/
bool UIButtonElement::getIsRadio(void)
{
	return this->doesPropertyExist(PropertyRadioGroup);
}
/*****************************************************************************/
void UIButtonElement::handleRadioGroup()
{
	// Find all buttons from this radio group, start animating the ones we're turning off.
	// Turn self on or off.
	if(this->doesPropertyExist(PropertyRadioGroup))
	{
		getUIPlane()->setPushedForRadioGroup(this->getRadioGroupName(), this, myIsSimulatingClick);
		this->setIsPushed(true, myIsSimulatingClick);
	}
	else
		this->setIsPushed(false, myIsSimulatingClick);
	
}
/*****************************************************************************/
void UIButtonElement::simulateClick(bool bDisableActions)
{
	TTouchVector rDummy;
	UIElementCallback* pPrevCallbacks = myCallbacks;
	if(bDisableActions)
		myCallbacks = NULL;
	myIsSimulatingClick = true;
	myIgnoreWhichMouseButtonIsPressed = true;
	Application::getInstance()->setDisableSound(true);
	this->setIsPushed(false, true);
	this->onPressed(rDummy);
	this->onReleased(rDummy, false);
	Application::getInstance()->setDisableSound(false);
	myIgnoreWhichMouseButtonIsPressed = false;
	myIsSimulatingClick = false;
	if(bDisableActions)
		myCallbacks = pPrevCallbacks;
}
/*****************************************************************************/
void UIButtonElement::onCursorGlideEnter(TTouchVector& vecTouches)
{
	UIElement::onCursorGlideEnter(vecTouches);
	myHoverAnim.changeAnimation(0, 1, HOVER_FADE_TIME, getClockType());
}
/*****************************************************************************/
void UIButtonElement::onCursorGlideLeave(TTouchVector& vecTouches)
{
	UIElement::onCursorGlideLeave(vecTouches);
	myHoverAnim.changeAnimation(1, 0, HOVER_FADE_TIME, getClockType());
}
/*****************************************************************************/
void UIButtonElement::onTimerTick(GTIME lGlobalTime)
{
	UIElement::onTimerTick(lGlobalTime);
// 	myHoverAnim.checkTime(lGlobalTime);
// 	myPushedDownValue.checkTime(lGlobalTime);
}
/*****************************************************************************/
void UIButtonElement::updateOwnData(SUpdateInfo& rRefreshInfo)
{
	// See if we have a target element, and if so, set our value from it.
	if(getHaveValidTargetElement())
		getTargetIdentifier()->refreshUIFromStoredValue(this);
}
/*****************************************************************************/
void UIButtonElement::onFinishHiding()
{
	UIElement::onFinishHiding();
	myPushedDownValue.ensureAnimOverReportedToActivityManager();
	myHoverAnim.ensureAnimOverReportedToActivityManager();
}
/*****************************************************************************/
};