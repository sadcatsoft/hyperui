#pragma once

/*****************************************************************************/
class HYPERUI_API UIGradientSliderStop : public UISliderStop
{
public:
	virtual ~UIGradientSliderStop() { }

	virtual void getSelectedAnim(string& strOut)
	{
		if(myIsOpacityStop)
			strOut = "gradControlsPage:4";
		else
			strOut = "gradControlsPage:2";
	}
	virtual void getDeselectedAnim(string& strOut)
	{
		if(myIsOpacityStop)
			strOut = "gradControlsPage:3";
		else
			strOut = "gradControlsPage:1";
	}

	bool myIsOpacityStop;
};
/*****************************************************************************/
enum GradientSelectionType
{
	GradientSelectionNone = 0,
	GradientSelectionOpacity,
	GradientSelectionColor,	
	GradientSelectionMultiple
};

/*****************************************************************************/
class HYPERUI_API UIGradientEdit: public UISlidingStopsEdit, public IUIElementIdentifierCallback
{

public:

	DECLARE_STANDARD_UIELEMENT(UIGradientEdit, UiElemGradientEdit);
	virtual void postInit(void);

	void setGradient(const SGradient& rGrad) { myGradient = rGrad; recreateAllStops(); }
	inline const SGradient* getGradient() const { return &myGradient; }

	virtual void postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

	virtual void onRefreshStoredValueFromUICallback(UIElement* pSourceElem, bool bIsChangingContinuously);
	virtual bool handleAction(string& strAction, UIElement* pSourceElem, IBaseObject* pData);

	virtual bool getIsChildIdentifierEnabled(UIElement* pIdentifierOwnerElem);

protected:

	virtual FLOAT_TYPE getGradAreaStartX() { return myGradientRect.x; }
	virtual FLOAT_TYPE getGradAreaWidth() { return myGradientRect.w; }

	virtual bool getAllowStopAddition() { return true; }
	virtual bool getAllowStopDeletion() { return true; }

	virtual void getInnerTextOffset(SVector2D& svOffsetOut) { svOffsetOut.set(0, -upToScreen(2.0)); }

	virtual bool getStopText(UISliderStop* pStop, string& strOut);
	virtual void removeStopCallback(UISliderStop* pStop);
	virtual void recreateAllStops() { recreateAllFromGradient(myGradient); }
	virtual FLOAT_TYPE getStopPos(UISliderStop* pUiStop);
	virtual void addNewStop(FLOAT_TYPE fPos, const SVector2D svTouchPoint);
	virtual void moveStopTo(UISliderStop* pUiStop, FLOAT_TYPE fNewPos);
	virtual void getStopCenter(UISliderStop* pUiStop, const SVector2D& svStopSize, SVector2D& svCenterOut );
	virtual bool getIsPointInAdditionRegion(const SVector2D& svPoint);
	virtual void drawCustomStopInnerContents(UISliderStop* pStop, const SVector2D& svStopCenter, FLOAT_TYPE fOpacity);

	virtual bool nudgeStopValue(UISliderStop* pStop, FLOAT_TYPE fByNormAmount);
	virtual void refreshStopRelatedUI(UISliderStop* pStopIn, SlidingStopsSelectionType eSelTypeGeneric, bool bIsChangingContinuously);

	virtual void onStopDoubleClicked(UISliderStop* pUiStop, TTouchVector& vecTouches);

	GradientSelectionType getGradientSelectionTypeFromGenericSelection(UISliderStop* pStopIn, SlidingStopsSelectionType eSelTypeGeneric);

	virtual const StringResourceItem* getGradientItemById(const char* pcsId) { return NULL; }

private:
	
	void recreateAllFromGradient(const SGradient& rGrad);
	void setChildTargetIdentifiers();

private:

	SGradient myGradient;
	SRect2D myGradientRect;
};
/*****************************************************************************/
