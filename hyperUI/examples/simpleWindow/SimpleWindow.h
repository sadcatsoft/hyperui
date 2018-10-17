#pragma once

/*****************************************************************************/
class SimpleWindow : public Window
{
public:
	SimpleWindow(int iScreenW, int iScreenH, bool bIsMainWindow = false);
	virtual ~SimpleWindow();

	virtual void onInitialized();

	virtual bool processActionSubclass(UIElement* pElem, const string& strAction, const char* pcsTargetDataSource, PropertyType eActionValue1Prop, PropertyType eActionValue2Prop, PropertyType eActionValue3Prop, MouseButtonType eButton);

	virtual bool onKeyUpSubclass(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl);
	virtual bool onKeyDownBeforeUISubclass(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl, bool bIsKeyARepeat);

private:
	int myCounter;
};
/*****************************************************************************/
