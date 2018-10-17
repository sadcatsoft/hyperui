#pragma once

#define DEFAULT_TOOLTIP_DELAY		0.75

class Window;
/*****************************************************************************/
class HYPERUI_API TooltipManager
{
public:

	TooltipManager(Window* pWindow);
	~TooltipManager();

	void onTimerTick(GTIME lTime);
	bool getIsShowingTooltip(void);

	void render(void);

	void onMouseDown(void);
	void onMouseUp(void);
	
	void setIsEnabled(bool bValue);
	
	inline void resetTooltipDelay() { myTooltipDelay = -1; }
	FLOAT_TYPE getTooltipDelay();

private:

	void killTooltip(void);

private:

	SVector2D myTooltipStartPosition;
	SVector2D myPrevMousePos;
	GTIME myPrevPosSetTime;

	AnimatedValue myTooltipAnim;
	string myTooltipText;

	SVector2D myTextSize;

	bool myIsShowingTooltip, myIsEnabled;

	Window* myParentWindow;

	FLOAT_TYPE myTooltipDelay;
};
/*****************************************************************************/