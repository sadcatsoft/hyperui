#include "stdafx.h"

#define CUT_ELIPSIS		"..."
#define HOR_SCROLL_AMOUNT	upToScreen(4.0)

namespace HyperUI
{
/*****************************************************************************/
UITextFieldElement::UITextFieldElement(UIPlane* pParentPlane)
	: UIElement(pParentPlane)
{
	myMeasuredString = NULL;
	onAllocated(pParentPlane);
}
/*****************************************************************************/
UITextFieldElement::~UITextFieldElement()
{
	onDeallocated();
}
/*****************************************************************************/
void UITextFieldElement::onAllocated(IBaseObject* pData)
{
	myMeasuredString = NULL;
	UIElement::onAllocated(pData);

	clearSelection();

	myDisableTextUpdate = false;
	myCutStartPosition = 0;
	myDidLockMouseCursor = false;
	myCursorPos = -1;
	myIsEnteringText = false;
	myNumMinValue = myNumMaxValue = FLOAT_TYPE_MAX;
	myCurrUndoBlockId = -1;
	//myNumValue = FLOAT_TYPE_MAX;
	//myNumValue.set(FLOAT_TYPE_MAX, UnitLastPlaceholder);
	// Be default, we always use the scene defined units
	myNumValue.set(FLOAT_TYPE_MAX, getParentWindow()->getDisplayUnits());
	myCursorAnim.setNonAnimValue(0.0);
}
/*****************************************************************************/
void UITextFieldElement::onDeallocated()
{
	UIElement::onDeallocated();
	delete myMeasuredString;
	myMeasuredString = NULL;
}
/*****************************************************************************/
void UITextFieldElement::postInit(void)
{
	myMeasuredString = new SMeasuredString(getParentWindow(), OpenGLStringMeasurer::getInstance());

    UIElement::postInit();

    // See if we're numeric:
    if(this->getIsNumeric())
	{
		// See if we have min/max values:
		if(this->doesPropertyExist(PropertyMin))
		{
			this->getAsString(PropertyMin, theCommonString);
			getParentWindow()->substituteVariables(theCommonString);
			FLOAT_TYPE fVal = StringUtils::convertStringToNumber(theCommonString);
// 			FLOAT_TYPE fVal = FLOAT_TYPE_MAX;
// 			if(pScene)
// 				fVal = pScene->evaluateNumVariable(theCommonString.c_str());
			if(fVal == FLOAT_TYPE_MAX)
				setNumericMinValue(this->getNumProp(PropertyMin));
			else
				setNumericMinValue(fVal);
		}

		if(this->doesPropertyExist(PropertyMax))
		{
			this->getAsString(PropertyMax, theCommonString);
			getParentWindow()->substituteVariables(theCommonString);
			FLOAT_TYPE fVal = StringUtils::convertStringToNumber(theCommonString);
// 			FLOAT_TYPE fVal = FLOAT_TYPE_MAX;
// 			if(pScene)
// 				fVal = pScene->evaluateNumVariable(theCommonString.c_str());
			if(fVal == FLOAT_TYPE_MAX)
				setNumericMaxValue(this->getNumProp(PropertyMax));
			else
				setNumericMaxValue(fVal);
		}

		if(this->doesPropertyExist(PropertyText))
		{
			this->getTextAsString(theCommonString2);

			// This will auto set it to 0, unitless if not valid string.
			myNumValue.setFromString(theCommonString2.c_str());
		}
		else
			myNumValue.set(0, myNumValue.getUnits());
		clampNumericToBounds();
    }

	// Recompute the shortened string, as well:
	recomputeShortenedString();
}
/*****************************************************************************/
void UITextFieldElement::onPressed(TTouchVector& vecTouches)
{
	UIElement::onPressed(vecTouches);
	clearSelection();
	myDidLockMouseCursor = false;
	myMouseDragStartCharPos = -1;
	if(vecTouches.size() > 0)
		myMouseDragStartCharPos = getCharPosFromScreenPoint(vecTouches[0].myPoint, true);
}
/*****************************************************************************/
void UITextFieldElement::onMouseMove(TTouchVector& vecTouches)
{
	UIElement::onMouseMove(vecTouches);
	if(vecTouches.size() > 0 && !getParentWindow()->getDragDropManager()->isDragging())
	{
		int iMouseEndCharPos = getCharPosFromScreenPoint(vecTouches[0].myPoint, true);
		clearSelection();
		if(iMouseEndCharPos != myMouseDragStartCharPos)
		{
			mySelectionStartPos = min(myMouseDragStartCharPos, iMouseEndCharPos);
			mySelectionEndPos = max(myMouseDragStartCharPos, iMouseEndCharPos);

			if(!myDidLockMouseCursor)
				getUIPlane()->lockMouseCursor(this);
		}
	}
}
/*****************************************************************************/
void UITextFieldElement::onReleased(TTouchVector& vecTouches, bool bIgnoreActions)
{
	UIElement::onReleased(vecTouches, bIgnoreActions);

	if(myDidLockMouseCursor)
		getUIPlane()->unlockMouseCursor();
	myDidLockMouseCursor = false;

	// If we're protected, erase all contents:
	if(this->getBoolProp(PropertyTextIsProtected))
		this->setText("");

	// Set the cursor to our pos:
	if(vecTouches.size() > 0)
		setCursorPosFromScreenPoint(vecTouches[0].myPoint);
}
/*****************************************************************************/
void UITextFieldElement::onTimerTick(GTIME lGlobalTime)
{
	UIElement::onTimerTick(lGlobalTime);
	//myCursorAnim.checkTime(lGlobalTime);
}
/*****************************************************************************/
FLOAT_TYPE UITextFieldElement::getCursorOpacity(void)
{
	// Draw the cursor only if we're being edited.
	if(getParentWindow()->getFocusElement() == this && !getIsReadOnly() && !(getIsMousePressed() && getHaveSelection() && mySelectionStartPos != mySelectionEndPos) )
		return myCursorAnim.getValue();
	else
		return 0.0;
}
/*****************************************************************************/
bool UITextFieldElement::onTextPreRender(string& strText, SColor& scolText, int &iCursorPosOut)
{
	bool bIsProtected = this->getBoolProp(PropertyTextIsProtected);

	// See if we're numeric and if we don't currently have focus
	if(myNumValue.getRawValue() != FLOAT_TYPE_MAX && getParentWindow()->getFocusElement() != this)
		formatNumber(myNumValue, this->getBoolProp(PropertyUioDisplayUnits), strText);

	iCursorPosOut = myCursorPos;
	
	// Set shortened text, adjust cursor pos.
	if(myCutStartPosition > 0)
	{
		iCursorPosOut = myCursorPos - (myCutStartPosition + strlen(CUT_ELIPSIS));
		strText = myShortenedString;
	}

	// Protect if necessary
	if(bIsProtected)
		protectString(strText);

/*
	// See if we need to shorten the visual string to fit. Don't do this for numerics.
	if(!getIsNumeric())
	{
		SVector2D svTextOffset, svBoxSize, svTextDims;
		FLOAT_TYPE fTextWidth;
		this->getBoxSize(svBoxSize);
		this->getTextOffset(svTextOffset);
		// Measure the text
		RenderUtils::measureText(strText.c_str(), getCachedFont(), 0, svTextDims);
		FLOAT_TYPE fActualPadding = svBoxSize.x - (svBoxSize.x/2.0 - svTextOffset.x);
		fTextWidth = svBoxSize.x - fActualPadding*2.0;
		if(svTextDims.x > fTextWidth)
		{
			// Shorten it
			int iCutBy = RenderUtils::shortenStringFromTheFront(strText, getCachedFont(), fTextWidth, "...", true);
 			iCursorPosOut = myCursorPos - iCutBy;
		}
	}
	*/
	// Allow shadow if no selection.
	return mySelectionEndPos < 0;
}
/*****************************************************************************/
void UITextFieldElement::changeValueTo(FLOAT_TYPE fAmount, UIElement* pOptSourceElem, bool bAnimate, bool bIsChangingContinuously)
{
	if(!myIsCallingChangeValue)
	{
		// Change *our* value first...
		if(myNumValue.getRawValue() != FLOAT_TYPE_MAX)
		{
			myNumValue.setRawValue(fAmount);
			clampNumericToBounds();

			// Actually replace the text, too, since if we're typing into us,
			// the text will not be replaced in text pre-render above.
			formatNumber(myNumValue, this->getBoolProp(PropertyUioDisplayUnits), theCommonString);
			setText(theCommonString.c_str());
		}
		else
		{
			_ASSERT(0);
		}

	}

	// ...then call the callbacks which may use it...
	UIElement::changeValueTo(fAmount, pOptSourceElem, bAnimate, bIsChangingContinuously);
}
/*****************************************************************************/
bool UITextFieldElement::getIsNumeric()
{
	bool bRes = false;
	if(this->doesPropertyExist(PropertyTextFormat))
	{
		const char* pcsFormat = this->getStringProp(PropertyTextFormat);
		if(strstr(pcsFormat, "d") || strstr(pcsFormat, "f") || strstr(pcsFormat, "g"))
			bRes = true;
	}

	return bRes;
}
/*****************************************************************************/
void UITextFieldElement::setText(const char* pcsText)
{
//	gLog("Updating uitext %x new val %s\n", this, pcsText);

	bool bCallBaseClass = true;
	const char* pcsFinalText = pcsText;
	if(getIsNumeric() && !myIsEnteringText)
	{
		SUnitNumber rTempNum(pcsText);
		if(StringUtils::isANumber(pcsText, true) || rTempNum.getUnits() != UnitLastPlaceholder)
		{
			// If we're told not to display units, we should not be
			// affecting the units of the number we're storing, otherwise
			// they'll be reset to unitless every time.
			if(this->getBoolProp(PropertyUioDisplayUnits))
				myNumValue.setFromString(pcsText);
			else
			{
				// This preserves the units.
				// But we could have overridden the value with our units.
				// Parse the units into a new number, and see if they are
				// valid.
				if(rTempNum.getUnits() == UnitLastPlaceholder) // && rTempNum.getUnits() != myNumValue.getUnits())
					myNumValue.setRawValue(atof(pcsText));
				else
				{
					// The units are changing. Notify the dependent, if any?
					bool bDoAllowNewUnits = false;
					UIElement* pLinkedElem = this->getLinkedToElement();
					if(pLinkedElem)
						bDoAllowNewUnits = pLinkedElem->onUnitsChanging(rTempNum.getUnits());
					if(!bDoAllowNewUnits && myNumValue.getUnits() != UnitLastPlaceholder)
						myNumValue = rTempNum.convertTo(myNumValue.getUnits(), FLOAT_TYPE_MAX, getParentWindow()->getCurrentDpi());
					else
						myNumValue = rTempNum;
				}
			}
			clampNumericToBounds();

			// NOTE that we deliberately don't reset the
			// text here to the clamped value, since
			// otherwise the control wouldn't let you type
			// anything larger than the value. We reset the
			// text in onLostFocus().
		}
		else
			// Don't change the value
			bCallBaseClass = false;
	}

	if(bCallBaseClass)
		UIElement::setText(pcsText);

	if(myMeasuredString)
		myMeasuredString->markDirty();

	ensureMeasuredStringUpdated();

	// Recompute the shortened string, as well:
	recomputeShortenedString();

	updateLinkedSlider();
}
/*****************************************************************************/
FLOAT_TYPE UITextFieldElement::getNumericValue(UnitType eUnits, FLOAT_TYPE fDpi) 
{ 
	_ASSERT(getIsNumeric()); 
	if(eUnits == UnitLastPlaceholder)
		return myNumValue.getRawValue();
	else
	{
		FLOAT_TYPE fFinalDpi = fDpi;
		if(fFinalDpi == FLOAT_TYPE_MAX)
			fFinalDpi = getParentWindow()->getCurrentDpi();
		return myNumValue.getConvertedValue(eUnits, FLOAT_TYPE_MAX, fFinalDpi);
	}
	//return myNumValue.getConvertedValue(PARM_STORAGE_UNITS, FLOAT_TYPE_MAX, g_pMainEngine->getCurrentDpi());
}
/*****************************************************************************
void UITextFieldElement::handleTargetElementUpdate()
{
	Node* pNode;
	StringResourceProperty* pParm = getTargetParm(pNode);
	if(!pParm)
		return;

	if(getIsNumeric())
		pParm->setNum(myNumValue.getConvertedValue(PARM_STORAGE_UNITS, FLOAT_TYPE_MAX, g_pMainEngine->getCurrentDpi()));
	else
		pParm->setString(this->getUiText());

	pNode->markCacheDirty();
	pNode->onParmChanged(pParm->getPropertyName());
}
/*****************************************************************************/
void UITextFieldElement::onGainedFocus()
{
	myCursorAnim.setAnimation(getMinCursorOpacity(), 1.0, 0.45, ClockUiPrimary, AnimOverActionPlayReverseForever, NULL);

	// Save undo...
	// if(myTargetElementToUpdate >= 0 )
	// Now we can update non-elem targeted portions.
	//if(myTargetElementToUpdate >= 0 || g_pMainEngine->getOverrideUndoManager())
	//if(g_pMainEngine->getCurrentUndoManager())
	//if(UndoManager::canAcceptUIUndoNow())
	if(UndoManager::canAcceptNewUndoNow())
	{
		_ASSERT(myCurrUndoBlockId < 0);
		//UndoItemParmChange* pNewUndo = new UndoItemParmChange(g_pMainEngine->getCurrentSceneNodeById(myTargetElementToUpdate), myTargetElementParmName.c_str());
		myCurrUndoBlockId = UndoManager::addUndoItemToCurrentManager(getUndoStringForSelfChange(), createUndoItemForSelfChange(), true, getParentWindow(), NULL);
	}

	// Set our cursor pos
	if(myCursorPos < 0)
		setCursorToEnd();

	myIsEnteringText = true;
}
/*****************************************************************************/
void UITextFieldElement::onLostFocus(bool bHasCancelled)
{
	myCursorAnim.setNonAnimValue(0.0);
	myCursorPos = -1;
	getMeasuredString()->setRawCursorIndex(myCursorPos);

	if(myCurrUndoBlockId >= 0)
	{
		UndoManager::endUndoBlockInCurrentManager(myCurrUndoBlockId, getParentWindow(), NULL);
		myCurrUndoBlockId = -1;
	}

	myIsEnteringText = false;

	updateText();

	if(this->doesPropertyExist(PropertyLostFocusAction))
	{
		theCommonString = this->getStringProp(PropertyLostFocusAction);
		if(!handleActionUpParentChain(theCommonString, false))
			getUIPlane()->processAction(this, theCommonString, this->getGenericDataSource(), PropertyLostFocusActionValue, PropertyNull, PropertyNull, MouseButtonLeft);
	}

	clearSelection();
}
/*****************************************************************************/
void UITextFieldElement::updateText()
{
	if(myDisableTextUpdate)
		return;

	// Update the UI text
	if(getIsNumeric())
	{
		// Because we're not entering the text, this reformats it.
		getTextAsString(theCommonString3);		
		UITextFieldElement::setText(theCommonString3.c_str());

		// And this changes it to a corrected state to reflect our
		formatNumber(myNumValue, this->getBoolProp(PropertyUioDisplayUnits), theCommonString);
		UITextFieldElement::setText(theCommonString.c_str());
	}

	// Note that here we update the target elements after UI text because if
	// one of them wants the new value and asks for getNumericValue(), we will
	// have returned an old value because the above wouldn't have updated it yet...
	handleTargetElementUpdate(false);
	updateLinkedElement(true, false);

	if(this->doesPropertyExist(PropertyTextChangedAction))
	{
		theCommonString = this->getStringProp(PropertyTextChangedAction);
		handleActionUpParentChain(theCommonString, false);
	}

}
/*****************************************************************************/
void UITextFieldElement::updateOwnData(SUpdateInfo& rRefreshInfo)
{
	// See if we have a target element, and if so, set our value from it.
	if(getHaveValidTargetElement())
		getTargetIdentifier()->refreshUIFromStoredValue(this);

	recomputeShortenedString();
	updateLinkedSlider();
/*
	// I don't think we should be doing this here... if we're an element of some
	// other control (such as color) we will basically be resetting ourselves to
	// the default definition every refresh. If something isn't setting
	// the default text, we should be doing it elsewhere.
	else 
	{
		// See if we're a variable, and eval it again.
		ResourceItem* pOwnDef = getOwnDefinition();
		if(pOwnDef && pOwnDef->doesPropertyExist(PropertyUiObjText))
		{
			Scene* pScene = g_pMainEngine->getLastActiveScene();
			pOwnDef->getUiTextAsString(theCommonString2);
			g_pMainEngine->substituteVariables(theCommonString2, NULL, pScene);
// 			FLOAT_TYPE fVal = FLOAT_TYPE_MAX;
// 			if(pScene)
// 				fVal = pScene->evaluateNumVariable(theCommonString2.c_str());
			FLOAT_TYPE fVal = stringToNumber(theCommonString2);
			if(fVal != FLOAT_TYPE_MAX)
			{
				myNumValue.set(fVal, UnitLastPlaceholder);
				numberToString(fVal, theCommonString);
				UIElement::setUiText(theCommonString.c_str());
			}
		}

	}
	*/

	// I don't think this is supposed to be there on refresh... since this
	// guy has the potential to update the original parm, and it may do so
	// with the default value above...
	//updateLinkedElement(true);
}
/*****************************************************************************/
void UITextFieldElement::clampNumericToBounds()
{
	if(!getIsNumeric())
		ASSERT_RETURN;

	if(myNumMinValue != FLOAT_TYPE_MAX && myNumValue.getRawValue() < myNumMinValue)
		myNumValue.setRawValue(myNumMinValue);
	if(myNumMaxValue != FLOAT_TYPE_MAX && myNumValue.getRawValue() > myNumMaxValue)
		myNumValue.setRawValue(myNumMaxValue);
}
/*****************************************************************************/
bool UITextFieldElement::canAccept(int iKey)
{
	if(iKey >= SilentKeyLArrow)
		return false;

	return  true;
#if 0
	//char cKey = (char)iKey;

	/*
	if(getIsNumeric())
		return isdigit(cKey) || cKey == '.' || cKey == '-';
	else */
	// For now, we accept everything, and deal with it later.
	{
		if(isalnum(cKey))
			return true;

		if(isspace(cKey) || cKey == '.' || cKey == '_' || cKey == '\\' || cKey == ':' ||
			cKey == '(' || cKey == ')' || cKey == '-' || cKey == ';')
			return true;
		
		return false;
	}
#endif
}
/*****************************************************************************/
bool UITextFieldElement::onTextInput(int iKey)
{
	if(getIsReadOnly())
		return false;

	// See if we can accept this key
	if(!canAccept(iKey))
		return false;

	if(iKey == SilentKeyCarriageReturn)
		iKey = '\n';

	deleteSelectedText();

	// Otherwise, insert it where the cursor is
	char pcsBuff[2] = { (char)iKey, 0 };
	this->getTextAsString(theCommonString);
	if(KeyManager::getInstance()->getIsCapsLockOn())
		pcsBuff[0] = toupper(pcsBuff[0]);

	int iPrevMappedCursorPos = getMeasuredString()->adjustCursorIndexToRaw(myCursorPos);
	theCommonString.insert(iPrevMappedCursorPos, pcsBuff);
	iPrevMappedCursorPos++;
	this->setText(theCommonString.c_str());
	myCursorPos = getMeasuredString()->adjustCursorIndexFromRaw(iPrevMappedCursorPos);
	getMeasuredString()->setRawCursorIndex(myCursorPos);

	// See if we need to update the scrolling
	if(iKey == '\n' && getAllowTextScrolling())
	{
		SRect2D srOwnRect;
		getGlobalRectangle(srOwnRect);
		srOwnRect.enlargeAllSidesBy(SCISSOR_RECT_PADDING);

		SVector2D svRelPos;
		getRelativeCursorPos(svRelPos);
		screenToScrolledSpace(svRelPos, svRelPos);

		if(svRelPos.y >= srOwnRect.h)
		{
			FLOAT_TYPE fLineHeight = getMeasuredString()->getMeasuredLineHeight(NULL);
			myTextScroll.y += fLineHeight;
		}
	}

	ensureScrollToMakeCursorVisible();

	if(this->doesPropertyExist(PropertyTextChangedAction))
	{
		theCommonString = this->getStringProp(PropertyTextChangedAction);
		handleActionUpParentChain(theCommonString, false);
	}
	onTextContentsChangedByTyping();
	return true;
}
/*****************************************************************************/
bool UITextFieldElement::onKeyUp(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl)
{
	bool bDidTextChange = false;
	bool bHandled = false;
	if(iKey == SilentKeyUpArrow || iKey == SilentKeyDownArrow)
	{
		// Attempt to increment using up-down controls
		UIElement* pUpElem = this->getElementLinkedToSelfWithMaxPropValue(PropertyActionValue);
		if(pUpElem)
		{
			FLOAT_TYPE fAmount = pUpElem->getNumProp(PropertyActionValue);
			if(iKey == SilentKeyDownArrow)
				fAmount *= -1.0;
			_ASSERT(myIsEnteringText);
			incrementCurrentUiTextBy(fAmount);
			// So now we need to see what we have, convert that to the current number
			// (since it may be united), increment, and convert back to text.
			//this->changeValueTo(fAmount + this->getNumericValue(), NULL, true);
			bHandled = true;
		}
	}
	else if(iKey == SilentKeyLArrow)
	{
		int iOldPos = myCursorPos;
		if(!bShift)
			clearSelection();
		bool bWasAtStart = myCursorPos == 0;
		myCursorPos--;
		if(myCursorPos < 0)
			myCursorPos = 0;
		getMeasuredString()->setRawCursorIndex(myCursorPos);

/*
		if(getAllowTextScrolling() && !bWasAtStart)
		{
			SVector2D svRelPos;
			getRelativeCursorPos(svRelPos);
			screenToScrolledSpace(svRelPos, svRelPos);
			SRect2D srOwnRect;
			getGlobalRectangle(srOwnRect);
			srOwnRect.enlargeAllSidesBy(SCISSOR_RECT_PADDING);
			if(svRelPos.x - myTextScroll.x <= HOR_SCROLL_AMOUNT)
				myTextScroll.x -= HOR_SCROLL_AMOUNT;
			if(myTextScroll.x < 0)
				myTextScroll.x = 0;
			if(svRelPos.x >= srOwnRect.w)
			{
				// We went to the prev line:
				FLOAT_TYPE fLineHeight = getMeasuredString()->getMeasuredLineHeight(NULL);
				SMeasuredLine* pLine = getMeasuredString()->getLineForPos(myCursorPos);			
				SVector2D svEndLinePos;
				getMeasuredString()->getCursorPos(pLine->myEndChar, svEndLinePos, false);
				myTextScroll.x = svEndLinePos.x - srOwnRect.w + SCISSOR_RECT_PADDING*2 + HOR_SCROLL_AMOUNT*2;
				myTextScroll.y -= fLineHeight;
				if(myTextScroll.y < 0)
					myTextScroll.y = 0;
			}

			updateLinkedSlider();
		}*/

		ensureScrollToMakeCursorVisible();

		if(bShift && myCursorPos != iOldPos)
			manageSelectionWhenCursorMovesLeft(iOldPos);

		onCursorPosChangedByTyping();
		bHandled = true;
	}
	else if(iKey == SilentKeyRArrow)
	{
		int iOldPos = myCursorPos;
		if(!bShift)
			clearSelection();

// 		this->getUiTextAsString(theCommonString);
// 		bool bWasAtEnd = (myCursorPos == theCommonString.length());
		int iNumWrappedChars = getMeasuredString()->getNumWrappedChars();
		bool bWasAtEnd = (myCursorPos == iNumWrappedChars);
		myCursorPos++;
		if (myCursorPos > iNumWrappedChars)
			myCursorPos = iNumWrappedChars;
		getMeasuredString()->setRawCursorIndex(myCursorPos);

/*
		// Scroll
		if(getAllowTextScrolling() && !bWasAtEnd)
		{
			SVector2D svRelPos;
			getRelativeCursorPos(svRelPos);
			screenToScrolledSpace(svRelPos, svRelPos);
			SRect2D srOwnRect;
			getGlobalRectangle(srOwnRect);
			srOwnRect.enlargeAllSidesBy(SCISSOR_RECT_PADDING);
			if(svRelPos.x >= srOwnRect.w)
				myTextScroll.x += HOR_SCROLL_AMOUNT;
			if(svRelPos.x <= FLOAT_EPSILON)
			{
				// We went to the next line
				myTextScroll.x = 0;
				FLOAT_TYPE fLineHeight = getMeasuredString()->getMeasuredLineHeight(NULL);
				myTextScroll.y += fLineHeight;
			}
			updateLinkedSlider();
		}
*/

		ensureScrollToMakeCursorVisible();

		
		if(bShift && myCursorPos != iOldPos)
			manageSelectionWhenCursorMovesRight(iOldPos);

		onCursorPosChangedByTyping();
		bHandled = true;
	}
	else if((iKey == SilentKeyBackspace || iKey == SilentKeyDelete) && !getIsReadOnly())
	{
		this->getTextAsString(theCommonString);
		int iPrevMappedCursorPos = getMeasuredString()->adjustCursorIndexToRaw(myCursorPos);

		if(mySelectionStartPos >= 0)
		{
			deleteSelectedText();
			bDidTextChange = true;
			onTextContentsChangedByTyping();
		}
		else if(iKey == SilentKeyBackspace && theCommonString.length() > 0 && iPrevMappedCursorPos > 0)
		{
			theCommonString2 = "";
			if(iPrevMappedCursorPos > 1)
				theCommonString2 += theCommonString.substr(0, iPrevMappedCursorPos - 1);
			if(iPrevMappedCursorPos < theCommonString.length())
				theCommonString2 += theCommonString.substr(iPrevMappedCursorPos);

			// Erase the letter just before the cursor
			this->setText(theCommonString2.c_str());
			iPrevMappedCursorPos--;
			myCursorPos = getMeasuredString()->adjustCursorIndexFromRaw(iPrevMappedCursorPos);
			getMeasuredString()->setRawCursorIndex(myCursorPos);
			bDidTextChange = true;
			onTextContentsChangedByTyping();
		}
		else if(iKey == SilentKeyDelete && theCommonString.length() > 0 && myCursorPos < theCommonString.length())
		{
			// Erase the letter just after the cursor
			theCommonString2 = "";
			if(iPrevMappedCursorPos > 0)
				theCommonString2 += theCommonString.substr(0, iPrevMappedCursorPos);
			if(iPrevMappedCursorPos < theCommonString.length())
				theCommonString2 += theCommonString.substr(iPrevMappedCursorPos + 1);

			this->setText(theCommonString2.c_str());
			myCursorPos = getMeasuredString()->adjustCursorIndexFromRaw(iPrevMappedCursorPos);
			getMeasuredString()->setRawCursorIndex(myCursorPos);
			bDidTextChange = true;
			onTextContentsChangedByTyping();
		}

		ensureScrollToMakeCursorVisible();
		bHandled = true;
	}
	else if(iKey == SilentKeyHome)
	{
		// Beginning of the line
		myCursorPos = getMeasuredString()->setToStartOfLine(myCursorPos);
		ensureScrollToMakeCursorVisible();
		bHandled = true;
	}
	else if(iKey == SilentKeyEnd)
	{
		// End of line
		myCursorPos = getMeasuredString()->setToEndOfLine(myCursorPos);
		ensureScrollToMakeCursorVisible();
		bHandled = true;
	}
	else if((char)iKey == 'v' && bControl && !getIsReadOnly())
	{
		// See if we have valid text in the pasteboard, and
		// change us:
		if(ClipboardManager::getDoHaveTextInClipboard())
		{
			deleteSelectedText();
			int iPrevMappedCursorPos = getMeasuredString()->adjustCursorIndexToRaw(myCursorPos);
			ClipboardManager::getClipboardText(theCommonString2);
			this->getTextAsString(theCommonString);
			if(iPrevMappedCursorPos >= 0 && iPrevMappedCursorPos <= theCommonString.length())
			{
				theCommonString.insert(iPrevMappedCursorPos, theCommonString2);
				iPrevMappedCursorPos += theCommonString2.length();
				this->setText(theCommonString.c_str());

				myCursorPos = getMeasuredString()->adjustCursorIndexFromRaw(iPrevMappedCursorPos);
				getMeasuredString()->setRawCursorIndex(myCursorPos);

				// TEMP hack: since we no support multiline editing,
				// kill focus if we detect lines pasted.
				if(theCommonString2.find('\n') != string::npos)
					getParentWindow()->setFocusElement(NULL);
			}
			bHandled = true;
			bDidTextChange = true;
			onTextContentsChangedByTyping();
			ensureScrollToMakeCursorVisible();
		}
	}
	else if(((char)iKey == 'x' || (char)iKey == 'c') && bControl)
	{
		this->getTextAsString(theCommonString);
		int iSelectionStartPos = getMeasuredString()->adjustCursorIndexToRaw(mySelectionStartPos);
		int iSelectionEndPos = getMeasuredString()->adjustCursorIndexToRaw(mySelectionEndPos);

		if(iSelectionStartPos >= 0)
		{
			// Copy selection only
			theCommonString = theCommonString.substr(iSelectionStartPos, iSelectionEndPos - iSelectionStartPos);
		}

		if(!this->getBoolProp(PropertyTextIsProtected))
			ClipboardManager::setClipboardText(theCommonString.c_str());

		if((char)iKey == 'x' && !getIsReadOnly())
		{
			// Cut the text
			if (iSelectionStartPos >= 0)
				deleteSelectedText();
			else
			{
				theCommonString = "";
				setText(theCommonString.c_str());
				myCursorPos = 0;
				getMeasuredString()->setRawCursorIndex(myCursorPos);
			}
			bDidTextChange = true;
			onTextContentsChangedByTyping();
			ensureScrollToMakeCursorVisible();
		}
		bHandled = true;
	}
	else if((char)iKey == 'a' && bControl)
	{
		selectAll();
	}

	if(bHandled && bDidTextChange)
	{
		if(this->doesPropertyExist(PropertyTextChangedAction))
		{
			theCommonString = this->getStringProp(PropertyTextChangedAction);
			handleActionUpParentChain(theCommonString, false);
		}
	}

	return bHandled;

}
/*****************************************************************************/
void UITextFieldElement::updateLinkedElement(bool bAnimate, bool bIsChangingContinuously)
{
	// Propagate...?
	// Make sure we're not calling self twice...
	myIsCallingChangeValue = true;
	UIElement* pLinkedElem = getLinkedToElement();
	if(pLinkedElem && getIsNumeric())
		pLinkedElem->changeValueTo(getNumericValue(UnitLastPlaceholder), this, bAnimate, bIsChangingContinuously);
	myIsCallingChangeValue = false;
}
/*****************************************************************************/
void UITextFieldElement::switchToUnits(UnitType eNewUnits, FLOAT_TYPE fDpi)
{
	// We need to convert the value to our
	if(myNumValue.getUnits() == UnitLastPlaceholder)
		myNumValue.assignUnits(eNewUnits);
	else
		myNumValue = myNumValue.convertTo(eNewUnits, FLOAT_TYPE_MAX, fDpi);
	formatNumber(myNumValue, this->getBoolProp(PropertyUioDisplayUnits), theCommonString);
	setText(theCommonString.c_str());
}
/*****************************************************************************/
void UITextFieldElement::incrementCurrentUiTextBy(FLOAT_TYPE fAmount)
{
	// So now we need to see what we have, convert that to the current number
	// (since it may be united), increment, and convert back to text.

	// First, see if what we have now is a simple number.
	getTextAsString(theCommonString3);		
	const char* pcsUiText = theCommonString3.c_str();
	//const char* pcsUiText = this->getUiText();
	SUnitNumber rNewNumber(pcsUiText);
	if(StringUtils::isANumber(pcsUiText, true) || strlen(pcsUiText) == 0)
	{
		// This is simple:
		FLOAT_TYPE fNewVal = atof(pcsUiText) + fAmount;
		rNewNumber.set(fNewVal, UnitLastPlaceholder);
	}
	else if(rNewNumber.getUnits() != UnitLastPlaceholder)
	{
		// This is a united number:
		FLOAT_TYPE fNewVal = rNewNumber.getRawValue() + fAmount;
		rNewNumber.setRawValue(fNewVal);
	}
	formatNumber(rNewNumber, true, theCommonString);
	setText(theCommonString.c_str());
}
/*****************************************************************************/
void UITextFieldElement::setNumericValue(FLOAT_TYPE fValue, UnitType eUnits, bool bIsChangingContinuously)
{
	myNumValue.set(fValue, eUnits);
	changeValueTo(fValue, NULL, false, bIsChangingContinuously);
}
/*****************************************************************************/
void UITextFieldElement::getRelativeCursorPos(SVector2D& svOut)
{
	svOut.set(0, 0);
	ensureMeasuredStringUpdated();

	if(myCursorPos < 0)
		return;
	myMeasuredString->getCursorPos(myCursorPos, svOut);

	// What we need to do is first measure the cut portion, then the unadjuted cur pos,
	// then subtract and add elipsis len:
	if(myCutStartPosition > 0)
	{
		SVector2D svCutSize;
		myMeasuredString->getCursorPos(myCutStartPosition, svCutSize);
		svOut.x = svOut.x - svCutSize.x + myElipsisDims.x;
	}

	scrolledToScreenSpace(svOut, svOut);
}
/*****************************************************************************/
void UITextFieldElement::renderSelection(const SVector2D& svTextPos, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale, bool bXorMode)
{
	if(mySelectionStartPos < 0)
		return;

	// We have to render line-by-line
	SColor scolSel("0f9cff");
	scolSel.alpha = fOpacity;

	int iStartLine = 0, iEndLine = 0;
	SMeasuredLine *pLineInfo;

	pLineInfo = myMeasuredString->getLineForPos(mySelectionStartPos + myCutStartPosition);
	if(pLineInfo)
		iStartLine = pLineInfo->myLineNum;
	pLineInfo = myMeasuredString->getLineForPos(mySelectionEndPos);
	if(pLineInfo)
		iEndLine = pLineInfo->myLineNum;

	DrawingCache* pDrawingCache = getDrawingCache();
	pDrawingCache->flush();
	if(bXorMode)
		GraphicsUtils::enableXorMode();

	FLOAT_TYPE fLineHeight = myMeasuredString->getMeasuredLineHeight(NULL)*fScale;

	FLOAT_TYPE fCursorVertShift = 0;
	if (getTextVertAlign() == VertAlignTop)
		fCursorVertShift += fLineHeight/2.0;
	else if(getTextVertAlign() == VertAlignBottom)
		fCursorVertShift -= fLineHeight/2.0;

	SVector2D svStart, svEnd;
	int iLineStartChar, iLineEndChar;
	int iLine;
	for(iLine = iStartLine; iLine <= iEndLine; iLine++)
	{
		pLineInfo = myMeasuredString->getLineInfo(iLine);

		if(!pLineInfo)
			ASSERT_CONTINUE;

		if(iLine == iStartLine)
			iLineStartChar = mySelectionStartPos + myCutStartPosition;
		else
			iLineStartChar = pLineInfo->myStartChar;

		if(iLine == iEndLine)
			iLineEndChar = mySelectionEndPos;
		else
			iLineEndChar = pLineInfo->myEndChar;

		// Get the positions:
		myMeasuredString->getCursorPos(iLineStartChar, svStart, false);
		myMeasuredString->getCursorPos(iLineEndChar, svEnd, false);

		if(myCutStartPosition > 0)
		{
			svStart.x = svStart.x - myCutPortionSize.x + myElipsisDims.x;
			svEnd.x = svEnd.x - myCutPortionSize.x + myElipsisDims.x;
		}

		scrolledToScreenSpace(svStart, svStart);
		scrolledToScreenSpace(svEnd, svEnd);

		// Add rectangle
		pDrawingCache->addRectangle(svTextPos.x + svStart.x*fScale, svTextPos.y + svStart.y*fScale - fLineHeight/2.0 + fCursorVertShift, (svEnd.x - svStart.x)*fScale, fLineHeight, scolSel);
	}

	pDrawingCache->flush();
	if(bXorMode)
		GraphicsUtils::disableXorMode();
}
/*****************************************************************************/
void UITextFieldElement::deleteSelectedText()
{
	if(mySelectionStartPos < 0)
		return;

	this->getTextAsString(theCommonString2);

	int iSelectionStartPos = getMeasuredString()->adjustCursorIndexToRaw(mySelectionStartPos);
	int iSelectionEndPos = getMeasuredString()->adjustCursorIndexToRaw(mySelectionEndPos);

	int iNewCursorPos = 0;
	int iLen = theCommonString2.size();
	if(iSelectionStartPos > iLen || iSelectionEndPos > iLen)
	{
		_ASSERT(0);
		iNewCursorPos = iLen;
	}
	else
	{
		theCommonString2 = theCommonString2.substr(0, iSelectionStartPos) + theCommonString2.substr(iSelectionEndPos);
		iNewCursorPos = iSelectionStartPos;
	}

	myCursorPos = getMeasuredString()->adjustCursorIndexFromRaw(iNewCursorPos);

	setText(theCommonString2.c_str());
	getMeasuredString()->setRawCursorIndex(myCursorPos);
	clearSelection();
}
/*****************************************************************************/
void UITextFieldElement::onDoubleClick(TTouchVector& vecTouches, bool bIgnoreActions)
{
	UIElement::onDoubleClick(vecTouches, bIgnoreActions);
	selectAll();
}
/*****************************************************************************/
void UITextFieldElement::selectAll()
{
	// Select all
	int iLength = getMeasuredString()->getNumWrappedChars();
	if (iLength > 0)
	{
		mySelectionStartPos = 0;
		mySelectionEndPos = iLength;
	}

}
/*****************************************************************************/
int UITextFieldElement::getCharPosFromScreenPoint(const SVector2D& svScreenPoint, bool bClampToTextLength)
{
	// This ensures we have fully set refreshed info
	SVector2D svDummy;
	getRelativeCursorPos(svDummy);

	SVector2D svLastTextPos;
	getLastRenderedTextPos(svLastTextPos);

	FLOAT_TYPE fVertAlignmentOffset = 0;
	VertAlignType eVertAlign = getTextVertAlign();
	// Note that this may still be wrong for multiline that's non-top aligned...
	if(eVertAlign != VertAlignTop)
	{
		FLOAT_TYPE fLineHeight = getMeasuredString()->getMeasuredLineHeight(NULL);
		if(eVertAlign == VertAlignCenter)
			fVertAlignmentOffset = -fLineHeight/2.0;
		else if(eVertAlign == VertAlignBottom)
			fVertAlignmentOffset = -fLineHeight;
	}

	SVector2D svRelPos = svScreenPoint - svLastTextPos;
	svRelPos.y -= fVertAlignmentOffset;

	// Now, take into account any shortening we may have done:
	if(myCutStartPosition > 0)
		svRelPos.x = svRelPos.x - myElipsisDims.x + myCutPortionSize.x;

	screenToScrolledSpace(svRelPos, svRelPos);

	return myMeasuredString->getCursorPosForRelativePoint(true, true, false, svRelPos);
}
/*****************************************************************************/
void UITextFieldElement::manageSelectionWhenCursorMovesLeft(int iOldPos)
{
	if(mySelectionEndPos >= 0)
	{
		if(myCursorPos < mySelectionStartPos)
			mySelectionStartPos = myCursorPos;
		else if(myCursorPos < mySelectionEndPos)
			mySelectionEndPos = myCursorPos;
		if(mySelectionStartPos > mySelectionEndPos)
			HyperCore::swapValues<int>(mySelectionStartPos, mySelectionEndPos);
	}
	else
	{
		// Start the selection
		mySelectionStartPos = myCursorPos;
		mySelectionEndPos = iOldPos;
	}
	if(mySelectionEndPos == mySelectionStartPos)
		clearSelection();
}
/*****************************************************************************/
void UITextFieldElement::manageSelectionWhenCursorMovesRight(int iOldPos)
{
	if(mySelectionEndPos >= 0)
	{
		if(myCursorPos > mySelectionEndPos)
			mySelectionEndPos = myCursorPos;
		else if(myCursorPos > mySelectionStartPos)
			mySelectionStartPos = myCursorPos;
		if(mySelectionStartPos > mySelectionEndPos)
			HyperCore::swapValues<int>(mySelectionStartPos, mySelectionEndPos);
	}
	else
	{
		// Start the selection
		mySelectionStartPos = iOldPos;
		mySelectionEndPos = myCursorPos;
	}

	if(mySelectionEndPos == mySelectionStartPos)
		clearSelection();
}
/*****************************************************************************/
void UITextFieldElement::setCursorPosFromScreenPoint(const SVector2D& svPointIn)
{
	//SVector2D svPoint;
	// NO - this is done in the getCharPosFromScreenPoint
	//screenToScrolledSpace(svPointIn, svPoint);
	int iMouseEndCharPos = getCharPosFromScreenPoint(svPointIn, true);
	//this->getUiTextAsString(theCommonString);
	int iNumChars = getMeasuredString()->getNumWrappedChars();
	// Change our cursor
	if (iMouseEndCharPos >= 0 && iMouseEndCharPos < iNumChars)
		myCursorPos = iMouseEndCharPos;
	else
		myCursorPos = iNumChars;
	getMeasuredString()->setRawCursorIndex(myCursorPos);
	onCursorPosChangedByTyping();
}
/*****************************************************************************/
void UITextFieldElement::recomputeShortenedString()
{
	if(!getAllowTextShortening())
		return;

	if(getIsNumeric())
		return;
	
	SVector2D svTextOffset, svBoxSize, svTextDims;
	FLOAT_TYPE fTextWidth;
	this->getBoxSize(svBoxSize);
	this->getTextOffset(svTextOffset);

	RenderUtils::measureText(getParentWindow(), CUT_ELIPSIS, getCachedFont(), getCachedFontSize(), 0, myElipsisDims);

	// Measure the text
	getTextAsString(myShortenedString);
	RenderUtils::measureText(getParentWindow(), myShortenedString.c_str(), getCachedFont(), getCachedFontSize(), 0, svTextDims);
	FLOAT_TYPE fActualPadding = svBoxSize.x - (svBoxSize.x/2.0 - svTextOffset.x);
	fTextWidth = svBoxSize.x - fActualPadding*2.0;
	if(svTextDims.x > fTextWidth && getAllowTextCutting())
	{
		// Shorten it
		string strTempCopy = myShortenedString;
		myCutStartPosition = RenderUtils::shortenStringFromTheFront(getParentWindow(), myShortenedString, getCachedFont(), getCachedFontSize(), fTextWidth, CUT_ELIPSIS, true);
		strTempCopy = strTempCopy.substr(0, myCutStartPosition);
		RenderUtils::measureText(getParentWindow(), strTempCopy.c_str(), getCachedFont(), getCachedFontSize(),  0, myCutPortionSize);
	}
	else
		myCutStartPosition = 0;
}
/*****************************************************************************/
void UITextFieldElement::scrolledToScreenSpace(const SVector2D& svPointIn, SVector2D& svPointOut)
{
	svPointOut = svPointIn - myTextScroll;
}
/*****************************************************************************/
void UITextFieldElement::screenToScrolledSpace(const SVector2D& svPointIn, SVector2D& svPointOut)
{
	svPointOut = svPointIn + myTextScroll;
}
/*****************************************************************************/
void UITextFieldElement::getScrolledTextPosition(SVector2D& svInOut) const
{
	svInOut -= myTextScroll;
}
/*****************************************************************************/
bool UITextFieldElement::getIsReadOnly() const
{
	return this->getBoolProp(PropertyIsReadOnly);
}
/*****************************************************************************/
UISliderElement* UITextFieldElement::getRelatedSlider()
{
	// By default, try to find our child by type
	UISliderElement* pRes = this->getChildByClass<UISliderElement>();
	if(!pRes)
	{
		// Try to find a slider in the immediate parent which has this as its related
		// tables
		UIElement* pParent = this->getParent<UIElement>();
		if(pParent)
		{
			UISliderElement* pTemp = pParent->getChildByClass<UISliderElement>(false);
			if(pTemp && pTemp->getLinkedToElement<UITextFieldElement>() == this)
				pRes = pTemp;
		}
	}

	return pRes;
}
/*****************************************************************************/
void UITextFieldElement::updateLinkedSlider()
{
	UISliderElement* pSlider = getRelatedSlider();
	if(!pSlider)
		return;

	pSlider->setMinValue(0.0);

	FLOAT_TYPE fMaxValue = getMaxTextScroll();

	if(fMaxValue <= 0.0)
	{
		pSlider->setMaxValue(1.0);
		pSlider->setValue(0.0);
		pSlider->setIsEnabled(false);
	}
	else
	{
		pSlider->setIsEnabled(true);
		pSlider->setMaxValue(fMaxValue);
		pSlider->setValue(myTextScroll.y);
	}
	
}
/*****************************************************************************/
void UITextFieldElement::scrollOnSlider(UISliderElement* pSlider)
{
	myTextScroll.y = pSlider->getValue();
}
/*****************************************************************************/
bool UITextFieldElement::getAllowValuePropagation(FLOAT_TYPE fNewValue, bool bIsChangingContinuously, UIElement* pOptSourceElem, UIElement* pLinkedToElem)
{
	// For tables, we don't propagate the value if the source elem is our slider.
	// Otherwise, we may start marking nodes dirty...
	if(pOptSourceElem && getRelatedSlider() == pOptSourceElem)
		return false;
	else
		return UIElement::getAllowValuePropagation(fNewValue, bIsChangingContinuously, pOptSourceElem, pLinkedToElem);
}
/*****************************************************************************/
FLOAT_TYPE UITextFieldElement::getMaxTextScroll()
{
	ensureMeasuredStringUpdated();

	SRect2D srOwnRect;
	getGlobalRectangle(srOwnRect);
	srOwnRect.enlargeAllSidesBy(SCISSOR_RECT_PADDING);

	int iNumLines = myMeasuredString->getNumLines();
	FLOAT_TYPE fLineHeight = 0;
	if(iNumLines > 0)
		fLineHeight = myMeasuredString->getMeasuredLineHeight(NULL);
	
	FLOAT_TYPE fMaxScroll = (iNumLines - 0)*fLineHeight - srOwnRect.h;
	if(fMaxScroll < 0)
		fMaxScroll = 0.0;
	return fMaxScroll;
}
/*****************************************************************************/
void UITextFieldElement::ensureMeasuredStringUpdated()
{
	if(!myMeasuredString || !myMeasuredString->isDirty())
		return;

	// Reset all:
	myMeasuredString->setFont(getFontForMeasuredString());
	this->getTextAsString(theCommonString);
	if(this->getBoolProp(PropertyTextIsProtected))
		protectString(theCommonString);
	myMeasuredString->setText(theCommonString.c_str());
}
/*****************************************************************************/
void UITextFieldElement::setCursorToEnd()
{
	//this->getUiTextAsString(theCommonString);
	//myCursorPos = theCommonString.length();
	myCursorPos = getMeasuredString()->getNumWrappedChars();
	getMeasuredString()->setRawCursorIndex(myCursorPos);
}
/*****************************************************************************/
void UITextFieldElement::scrollToEnd()
{
	FLOAT_TYPE fMaxValue = getMaxTextScroll();
	if(fMaxValue > 0.0)
		myTextScroll.y = fMaxValue;
	else
		myTextScroll.y = 0.0;
	updateLinkedSlider();
}
/*****************************************************************************/
void UITextFieldElement::updateNumericValue(FLOAT_TYPE fValue)
{
	myNumValue.setRawValue(fValue);
}
/*****************************************************************************/
void UITextFieldElement::onSizeChanged()
{
	UIElement::onSizeChanged();
	if(myMeasuredString)
		myMeasuredString->markDirty();
	recomputeShortenedString();
}
/*****************************************************************************/
void UITextFieldElement::ensureScrollToMakeCursorVisible()
{
	if(!getAllowTextScrolling())
		return;

	SRect2D srOwnRect;
	getGlobalRectangle(srOwnRect);
	srOwnRect.enlargeAllSidesBy(SCISSOR_RECT_PADDING);

	FLOAT_TYPE fPaddingX = this->getNumProp(PropertyTextOffsetX);
	FLOAT_TYPE fPaddingY = this->getNumProp(PropertyTextOffsetY);
	srOwnRect.enlargeHorVertBy(-fPaddingX, -fPaddingY);

	SVector2D svRelPos;
	getRelativeCursorPos(svRelPos);
	SVector2D svOrigRelPos(svRelPos);
	screenToScrolledSpace(svRelPos, svRelPos);

	FLOAT_TYPE fLineHeight = getMeasuredString()->getMeasuredLineHeight(NULL);

	if(svOrigRelPos.x <= 0.0)
	{
		int iHScrollMult = (svRelPos.x/HOR_SCROLL_AMOUNT) + 0.99;
		myTextScroll.x = iHScrollMult*HOR_SCROLL_AMOUNT;
	}

	if(svOrigRelPos.x > srOwnRect.w)
	{
		int iHScrollMult = ((svRelPos.x - srOwnRect.w)/HOR_SCROLL_AMOUNT) + 0.99;
		_ASSERT(iHScrollMult > 0);
		myTextScroll.x = iHScrollMult*HOR_SCROLL_AMOUNT;
	}

	if(svOrigRelPos.y <= 0.0)
	{
		int iVScrollMult = (svRelPos.y/fLineHeight) + 0.99;
		myTextScroll.y = iVScrollMult*fLineHeight;
	}

	if(svOrigRelPos.y > srOwnRect.h - fLineHeight)
	{
		int iVScrollMult = ((svRelPos.y - srOwnRect.h + fLineHeight)/fLineHeight) + 0.99;
		_ASSERT(iVScrollMult > 0);
		myTextScroll.y = iVScrollMult*fLineHeight;
	}


	updateLinkedSlider();
/*
		// Scroll
		if (getAllowTextScrolling() && !bWasAtEnd)
		{
			SVector2D svRelPos;
			getRelativeCursorPos(svRelPos);
			screenToScrolledSpace(svRelPos, svRelPos);
			SRect2D srOwnRect;
			getGlobalRectangle(srOwnRect);
			srOwnRect.enlargeAllSidesBy(SCISSOR_RECT_PADDING);
			if (svRelPos.x >= srOwnRect.w)
				myTextScroll.x += HOR_SCROLL_AMOUNT;
			if (svRelPos.x <= FLOAT_EPSILON)
			{
				// We went to the next line
				myTextScroll.x = 0;
				FLOAT_TYPE fLineHeight = getMeasuredString()->getMeasuredLineHeight(NULL);
				myTextScroll.y += fLineHeight;
			}
		}

*/

}
/*****************************************************************************/
void UITextFieldElement::protectString(string& strTextInOut)
{
	// Replace all but the last char with the * char.
	char *pcsChars = const_cast<char*>(strTextInOut.c_str());
	int iCurrChar, iNum = strTextInOut.length();

	// If editing, replace all but last one, otherwise, 
	// replace all.
	if(getParentWindow()->getFocusElement() == (UIElement*)this)
		iNum--;

	for(iCurrChar = 0; iCurrChar < iNum; iCurrChar++)
	pcsChars[iCurrChar] = '*';

}
/*****************************************************************************/
};