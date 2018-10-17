#pragma once

class HYPERUI_API UIElement;
class HYPERUI_API UISliderElement;
/*****************************************************************************/
class HYPERUI_API UIElementCallback
{
public:
	virtual ~UIElementCallback() { }

	virtual void onButtonClicked(UIElement *pElem, MouseButtonType eButton, bool bSetSelfAsLastResourceDataSource = false) { }
	virtual void onSliderValueChanged(UISliderElement* pSlider) { }
};
/*****************************************************************************/
