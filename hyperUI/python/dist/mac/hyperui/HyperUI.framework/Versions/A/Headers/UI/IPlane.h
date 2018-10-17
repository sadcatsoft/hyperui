#pragma once

class HYPERUI_API Window;
class HYPERUI_API DrawingCache;
/*****************************************************************************/
enum PlaneType
{
	PlaneGame = 0,
	PlaneUI
};
/*****************************************************************************/
class HYPERUI_API IPlane : public IBaseObject
{
public:
	IPlane(Window* pParentWindow);
	virtual ~IPlane() { }

	inline const Window* getParentWindow() const { return myParentWindow; }
	inline Window* getParentWindow() { return myParentWindow; }

	DrawingCache* getDrawingCache();

private:

	Window* myParentWindow;
};
/*****************************************************************************/
