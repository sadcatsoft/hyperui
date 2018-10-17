#pragma once

enum SliderPartType
{
	SliderPartNone = 0,
	SliderPartLess,
	SliderPartMore,
	SliderPartMain,
	SliderPartKnob
};
/*****************************************************************************/
class HYPERUI_API UISliderElement : public UIElement, public AnimSequenceAddon
{
public:

	DECLARE_STANDARD_UIELEMENT(UISliderElement, UiElemSlider);

	virtual void render(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

	virtual void postInit(void);
	virtual void resetEvalCache(bool bRecursive);

	void setMinValue(FLOAT_TYPE fValue);
	void setMaxValue(FLOAT_TYPE fValue);
	void setValue(FLOAT_TYPE fValue);
	void setButtonStep(FLOAT_TYPE fValue);
	FLOAT_TYPE getMaxValue() { return myMaxValue; }

	virtual void onPressed(TTouchVector& vecTouches);
	virtual void onMouseLeave(TTouchVector& vecTouches);
	virtual void onMouseEnter(TTouchVector& vecTouches);
	virtual void onMouseMove(TTouchVector& vecTouches);
	virtual void onReleased(TTouchVector& vecTouches, bool bIgnoreActions);

	FLOAT_TYPE getValue(void);

	virtual void onTimerTick(GTIME lGlobalTime);

	bool getIsSliderBeingAnimated(void);
	virtual void changeValueTo(FLOAT_TYPE fValue, UIElement* pOptSourceElem, bool bAnimate, bool bIsChangingContinuously);

	virtual bool onMouseWheel(FLOAT_TYPE fDelta);
	inline bool getIsVertical() const { return myIsVertical; }

protected:

	SliderPartType getClickedPart(int iX, int iY, FLOAT_TYPE* fKnobPosOut);

	void setValueSafe(FLOAT_TYPE fValue, bool bAnimated, UIElement* pOptChangeSourceElem, bool bIsChangingContinuously);
	void finishUndo();

	FLOAT_TYPE percProgressToValue(FLOAT_TYPE fPercProgress);
	FLOAT_TYPE valueToPercProgress(FLOAT_TYPE fValue);

	virtual bool getAllowValuePropagation(FLOAT_TYPE fNewValue, bool bIsChangingContinuously, UIElement* pOptSourceElem, UIElement* pLinkedToElem);

private:

	void renderSliderInternal(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

protected:

	FLOAT_TYPE myMinValue, myMaxValue, myButtonValueStep;
	AnimatedValue myCurrValue;
	FLOAT_TYPE myDragStartValue;
	bool myAllowPressing;
	SliderPartType myActiveSliderPart;

	string theLocalSharedString, theLocalSharedString2;

	SVector2D myKnobSize;
	SVector2D myButtonSize;
	string myFullSliderAnim;
	string myLessButtonAnim, myMoreButtonAnim;
	string myValueDisplayAnim;

	FLOAT_TYPE myValueDisplayYOffset;
	FLOAT_TYPE myValueDisplayMult;

	bool myIsSizable;
	bool myIsVertical;
	bool myIsInCallback;
	bool myStartInMiddle;

	AnimatedValue myValueDisplayOpacity;

	UNIQUEID_TYPE myCurrUndoBlockId;
};
/*****************************************************************************/