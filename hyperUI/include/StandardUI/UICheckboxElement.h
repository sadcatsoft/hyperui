#pragma once

/*****************************************************************************/
class HYPERUI_API UICheckboxElement : public UIButtonElement
{
public:

	DECLARE_STANDARD_UIELEMENT(UICheckboxElement, UiElemCheckbox);

	virtual void onPressed(TTouchVector& vecTouches);
	virtual void onReleased(TTouchVector& vecTouches, bool bIgnoreActions);
	virtual void onMouseLeave(TTouchVector& vecTouches);

	//virtual void handleTargetElementUpdate();

	bool getIsEnum(void);
};
/*****************************************************************************/
