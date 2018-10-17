#include "stdafx.h"
#include "SimpleWindow.h"

/*****************************************************************************/
SimpleWindow::SimpleWindow(int iScreenW, int iScreenH, bool bIsMainWindow) : Window(iScreenW, iScreenH, bIsMainWindow)
{
	myCounter = 0;
}
/*****************************************************************************/
SimpleWindow::~SimpleWindow()
{

}
/*****************************************************************************/
void SimpleWindow::onInitialized()
{
	getUIPlane()->showUI("firstLayer");
}
/*****************************************************************************/
bool SimpleWindow::processActionSubclass(UIElement* pElem, const string& strAction, const char* pcsTargetDataSource, 
										 PropertyType eActionValue1Prop, PropertyType eActionValue2Prop, 
										 PropertyType eActionValue3Prop, MouseButtonType eButton)
{
	bool bDidHandle = false;

	if(strAction == "incrementGlobalCounter")
	{
		myCounter++;

		STRING_TYPE strNewText;
		StringUtils::numberToString(myCounter, strNewText);
		strNewText = "PRESSED #" + strNewText;
		pElem->setText(strNewText.c_str());

		pElem->setNumProp(PropertyWidth, upToScreen(200));
		pElem->getParent<UIElement>()->updateEverything();

		this->getUIPlane()->getElementById("exampleElement")->setTextExt("The %s button has been pressed last", pElem->getId());

		bDidHandle = true;
	}

	return bDidHandle;
}
/*****************************************************************************/
bool SimpleWindow::onKeyUpSubclass(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl)
{
	gLog("Key Up: %c\n", (char)iKey);
	return false;
}
/*****************************************************************************/
bool SimpleWindow::onKeyDownBeforeUISubclass(int iKey, bool bControl, bool bAlt, bool bShift, bool bMacActualControl, bool bIsKeyARepeat)
{
	gLog("Key Down: %c is repeat: %s\n", (char)iKey, bIsKeyARepeat ? "Yes" : "No");
	return false;
}
/*****************************************************************************/