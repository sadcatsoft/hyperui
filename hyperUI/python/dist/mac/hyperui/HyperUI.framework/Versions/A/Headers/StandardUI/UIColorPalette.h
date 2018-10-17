#pragma once

#define COLOR_CHANGED_ACTION		"uiaColorChanged"
/*****************************************************************************/
class HYPERUI_API UIColorPalette : public UIElement
{
public:
	DECLARE_STANDARD_UIELEMENT(UIColorPalette, UiElemColorPalette);

	virtual void postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

	virtual void onPressed(TTouchVector& vecTouches);
	virtual void onReleased(TTouchVector& vecTouches, bool bIgnoreActions);
	virtual void onMouseMove(TTouchVector& vecTouches);

	void setBaseColor(const SColor& scolBaseColor);
	void changeModeTo(const char* pcsMode);

	enum AreaType
	{
		AreaNone = 0,
		AreaLarge,
		AreaVert,		
	};

public:

	AreaType getColorFromScreenCoords(const SVector2D& svScreenPoint, AreaType eForcedArea, SColor& scolOut);

	FLOAT_TYPE computeColorAreas(FLOAT_TYPE fOpacity, FLOAT_TYPE fScale, SRect2D& srLargeAreaOut, SRect2D& srVertAreaOut);

	void finishUndo();

private:

	ColorPickerColorGetter myLargeAreaColorGetter;
	ColorPickerColorGetter myVertBarColorGetter;
	AreaType myClickStartArea;

	UNIQUEID_TYPE myCurrUndoBlockId;
};
/*****************************************************************************/
