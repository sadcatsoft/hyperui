#pragma once

class UIColorSwatch;

#define COLOR_SWATCH_CLICK_ACTION		"uiaColorSwatchClicked"
/*****************************************************************************/
class HYPERUI_API IColorChangedCallback
{
public:
	virtual ~IColorChangedCallback() { }
	virtual void onColorChanged(const SColor& scolNewColor, UIColorSwatch* pElem) = 0;
};
/*****************************************************************************/
class HYPERUI_API UIColorSwatch : public UIElement
{
public:

	enum ClickActionType
	{
		ClickActionOpenColorPicker = 0,
		ClickActionExecuteUiAction,
		ClickActionUpdateColorPickerIfOpened,
	};

	DECLARE_STANDARD_UIELEMENT(UIColorSwatch, UiElemColorSwatch);

	inline void setClickAction(ClickActionType eAction) { myClickAction = eAction; }

	virtual void onReleased(TTouchVector& vecTouches, bool bIgnoreActions);
	virtual void changeValueTo(FLOAT_TYPE fAmount, UIElement* pOptSourceElem, bool bAnimate, bool bIsChangingContinuously);
	//virtual void handleTargetElementUpdate();

	inline void getColor(SColor& scolOut) const { scolOut = myColor; }
	void setColor(const SColor& scolIn);

	inline void setColorCallback(IColorChangedCallback* pCallback) { myColorCallback = pCallback; }

	void changeColorFromUI(const SColor& scolNew, bool bIsChangingContinuously);

	void simulateClick();

protected:

	virtual void updateOwnData(SUpdateInfo& rRefreshInfo);

	virtual void render(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

	virtual void onColorChanged() { }

	void updateLinkedElements(bool bIsChangingContinuously);

protected:

	SColor myColor;
	IColorChangedCallback* myColorCallback;

	ClickActionType myClickAction;
};
/*****************************************************************************/