#pragma once

class HYPERUI_API UIElement;
/*****************************************************************************/
class HYPERUI_API ICustomDragRenderer
{
public:
	virtual ~ICustomDragRenderer() { }

	virtual void render(UIElement* pElem, SVector2D svPos) = 0;
};
/*****************************************************************************/
typedef map < string, ICustomDragRenderer* > TStringCustomRendererMap;
/*****************************************************************************/