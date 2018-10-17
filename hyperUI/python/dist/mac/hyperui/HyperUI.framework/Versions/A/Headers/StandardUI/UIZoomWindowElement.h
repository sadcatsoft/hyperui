#pragma once

/*****************************************************************************/
class HYPERUI_API UIZoomWindowElement : public UIElement
{
public:

	DECLARE_VIRTUAL_STANDARD_UIELEMENT_NO_DESTRUCTOR_DEF(UIZoomWindowElement, UiElemZoomWindow);

	virtual void render(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

	virtual void onPressed(TTouchVector& vecTouches);
	virtual void onReleased(TTouchVector& vecTouches, bool bIgnoreActions);
	virtual void onMouseLeave(TTouchVector& vecTouches);
	virtual void onMouseMove(TTouchVector& vecTouches);

	virtual void onTimerTick(GTIME lGlobalTime);
	FLOAT_TYPE getZoom(void) const;
	bool getIsAnimatingZoom() const;

	virtual FLOAT_TYPE getMinZoom(void) = 0;
	virtual FLOAT_TYPE getMaxZoom(void) = 0;

	virtual void setZoom(FLOAT_TYPE fZoom);
	void animateZoomTo(FLOAT_TYPE fTargetZoom);

	void panTo(SVector2D& svTargetPan, bool bAnimated, int iEndAction = AnimOverActionNone);
	inline void getPan(SVector2D& svPanOut) const { svPanOut = myPan; }

	void windowToScreen(const SVector2D& svIn, SVector2D& svOut, FLOAT_TYPE fExtraWindowScale = 1.0) const;
	void windowToScreen(const SRect2D& srIn, SRect2D& srOut, FLOAT_TYPE fExtraWindowScale = 1.0) const;
	void screenToWindow(const SVector2D& svIn, SVector2D& svOut) const;

	void scrollOnSlider(UISliderElement* pSlider);

protected:

	inline bool getHasScrolled() { return myHasScrolled; }

	void updateRelatedSlider();

	virtual void onStartShowing(void);

	void endTouch(TTouchVector& vecTouches);

	virtual bool getRenderChildrenInStencilRegion(void) { return true; }

	virtual void renderZoomedContents(const SRect2D& srWindowRect, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale) = 0;
	virtual void getContentSize(SVector2D& svOut) = 0;

	void animateScrollIfNeeded(FLOAT_TYPE fX, FLOAT_TYPE fY);

	virtual void setPan(SVector2D& svNewPan);

	void getMaxPan(SVector2D& svOut, bool bIncludePadding = true);

	virtual FLOAT_TYPE getScrollPixelTolerance(void);

	virtual bool getAllowScrollingX() { return true; }
	virtual bool getAllowScrollingY() { return true; }

	bool getIsAnimvalueScroll(AnimatedValue* pValue) { return myAnimatedPan.doesContain(pValue); }

	bool getIsAtLeftPanLimit();
	bool getIsAtRightPanLimit();

	void setZoomIndepOffset(FLOAT_TYPE fX, FLOAT_TYPE fY) { myExtraZoomIndepOffset.set(fX, fY); }
	
	virtual void onZoomAnimationEnded() { }

	virtual void onPanSet(const SVector2D& svNewPan) { }
	virtual void onZoomSet(FLOAT_TYPE fTargetZoom) { }
	virtual bool getIsPanningLimitless() const { return false; }

	void computeNewPan(FLOAT_TYPE fNewZoom, const SVector2D *pOptScreenNewCenter, FLOAT_TYPE fWindowStartX, FLOAT_TYPE fWindowStartY, bool bPlaceTargetPointUnderMouseCursor, SVector2D& svNewPanOut);
	virtual void screenToTargetFinalSpaceSubclass(const SVector2D& svIn, FLOAT_TYPE fNewZoom, SVector2D& svOut) { _ASSERT(0); }

	virtual void getWindowScrollPadding(SVector2D& svOut) const { svOut.set(0, 0); /* upToScreen(100) */ }

	void clampPanToValidRange(SVector2D& svPanInOut, bool bIncludePadding);

private:

	void setPanInternal(SVector2D& svNewPan, bool bResetAnimation);

private:
	AnimatedValue myZoom;
	SVector2D myPan;
	SVector2D myExtraZoomIndepOffset;

	AnimatedPoint myAnimatedPan;

	FLOAT_TYPE myInitZoom;
	FLOAT_TYPE myInitTouchDistance;
	SVector2D myInitPan, myInitPos;
	SVector2D myZoomMidpoint, myInitMidpointReal; // one in screen coords, the other real.
	FLOAT_TYPE myScrollSpeed;
	SVector2D mySpeedDir;

	int myActiveTouches;
	PathTrail myMouseTrail;
	bool myHasScrolled;

	bool myIgnoreSlider;
};
/*****************************************************************************/