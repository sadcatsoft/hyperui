#include "stdafx.h"

/*****************************************************************************/
class CustomWindow : public Window
{
public:
	CustomWindow(int iScreenW, int iScreenH, bool bIsMainWindow = false);
	virtual ~CustomWindow();

	virtual bool processActionSubclass(UIElement* pElem, const string& strAction, const char* pcsTargetDataSource, PropertyType eActionValue1Prop, PropertyType eActionValue2Prop, PropertyType eActionValue3Prop, MouseButtonType eButton);
};
/*****************************************************************************/
