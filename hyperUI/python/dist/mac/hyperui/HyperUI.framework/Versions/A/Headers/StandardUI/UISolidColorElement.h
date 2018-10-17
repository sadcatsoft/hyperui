#pragma once

/*****************************************************************************/
class HYPERUI_API UISolidColorElement : public UIElement
{
public:
	DECLARE_STANDARD_UIELEMENT(UISolidColorElement, UiElemSolidColor);
	virtual void render(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);
};
/*****************************************************************************/
