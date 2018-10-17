#pragma once

/*****************************************************************************/
class HYPERUI_API UIButtonElement : public UIElement
{
public:

	DECLARE_STANDARD_UIELEMENT(UIButtonElement, UiElemButton);

	int getSelectedEnumValue(void);
	void setSelectedEnumValue(int iNewValue);
	void incrementEnumValue(void);

	virtual void onPressed(TTouchVector& vecTouches);
	virtual void onMouseLeave(TTouchVector& vecTouches);
	virtual void onMouseEnter(TTouchVector& vecTouches);
	virtual void onReleased(TTouchVector& vecTouches, bool bIgnoreActions);

	virtual void onCursorGlideEnter(TTouchVector& vecTouches);
	virtual void onCursorGlideLeave(TTouchVector& vecTouches);


	inline bool getIsEnum(void) const { return myIsEnum; }
	void setIsPushed(bool bIsPushed, bool bInstant = false);
	bool getIsPushed(void);
	virtual void postInit(void);
	bool getIsRadio(void);

	virtual void onTimerTick(GTIME lGlobalTime);
	
	virtual void render(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);
	void simulateClick(bool bDisableActions = false);

protected:

	virtual void updateOwnData(SUpdateInfo& rRefreshInfo);
	virtual void onFinishHiding();

	void handleRadioGroup();

protected:
	bool myAllowPressing;

private:
	int myCurrSelEnum;

	// "pressed" in UIElem is only true as long as the finger is held down;
	// this is 1.0 if the button is pushed down, either as a user clicking on it
	// (after the click has been completed!) or as a member of the radio buttons
	// group.
	AnimatedValue myPushedDownValue;

#ifndef MAC_BUILD
	static 
#endif
	string theSharedString;

	bool myIsEnum;
	int myNormalRenderStage;
	bool myIgnoreWhichMouseButtonIsPressed;
	bool myIsSimulatingClick;
	AnimatedValue myHoverAnim;
};
/*****************************************************************************/
