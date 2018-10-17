#include "stdafx.h"
#include "CustomWindow.h"

/*****************************************************************************/
CustomWindow::CustomWindow(int iScreenW, int iScreenH, bool bIsMainWindow) : Window(iScreenW, iScreenH, bIsMainWindow)
{
}
/*****************************************************************************/
CustomWindow::~CustomWindow()
{

}
/*****************************************************************************/
bool CustomWindow::processActionSubclass(UIElement* pElem, const string& strAction, const char* pcsTargetDataSource, 
										 PropertyType eActionValue1Prop, PropertyType eActionValue2Prop, 
										 PropertyType eActionValue3Prop, MouseButtonType eButton)
{
	bool bDidHandle = false;

	if(strAction == "customAction")
	{
		const char* pcsActionValue = pElem->getStringProp(eActionValue1Prop);
		pElem->setText(pcsActionValue);
		pElem->setNumProp(PropertyWidth, upToScreen(150));
		pElem->getParent<UIElement>()->updateEverything();
		bDidHandle = true;
	}

	return bDidHandle;
}
/*****************************************************************************/
