#pragma once

/*****************************************************************************/
class HYPERUI_API UICurveEditor : public UIElement, public EventReceiver, public IUIElementIdentifierCallback
{
public:
	DECLARE_STANDARD_UIELEMENT_NO_DESTRUCTOR_DEF(UICurveEditor, UiElemCurveEditor);
	virtual void onDeallocated(void);
	virtual void updateOwnData(SUpdateInfo& rRefreshInfo);
	virtual void postInit();

	virtual void postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

	virtual void receiveEvent(EventType eEventType, IBaseObject* pSender);

	virtual void onSizeChanged() { myArePointsDirty = true; }

	virtual void onPressed(TTouchVector& vecTouches);
	virtual void onReleased(TTouchVector& vecTouches, bool bIgnoreActions);
	virtual void onMouseMove(TTouchVector& vecTouches);
	virtual void onMouseEnter(TTouchVector& vecTouches);
	virtual void onMouseLeave(TTouchVector& vecTouches);

	int getPointAt(const SVector2D& svScreenPoint, bool bIgnoreFirstLast);

	virtual void onRefreshStoredValueFromUICallback(UIElement* pSourceElem, bool bIsChangingContinuously);

	inline const SCubicCurve* getCurve() const { return &myCurve; }
	void setCurve(const SCubicCurve& rCurve) { myCurve.copyFrom(rCurve); myArePointsDirty = true; }

	void resetCurve(bool bTriggerParmChange = true);

private:
	void ensurePointsValid(const SRect2D& srWindowRect);
	void convertPointFromScreenToCurveCoords(const SVector2D& svPointIn, SVector2D& svPointOut);

	void updateUiForPointId(int iId);
	void setChildTargetIdentifiers();

	void finishUndo();

private:

	Mutex myDataLock;

	SCubicCurve myCurve;
	TPointVector myResampledPoints;
	TPointVector myResampledPointsTemp;
	SCubicCurve myCurrSizeCurve;
	bool myArePointsDirty;

	int myDraggedPointId;
	int myLastSelectedPointId;

	SRect2D myLastGraphArea;
	SVector2D myLastGraphScaling;
	SBBox2D myLastCurveBBox;
	SVector2D myLastWindowStart;

	bool myIsRefreshingUI;
	bool myAllowDeletingDraggedPoint;
	bool myIsDraggedPointFirst;

	UNIQUEID_TYPE myCurrUndoBlockId;
};
/*****************************************************************************/
