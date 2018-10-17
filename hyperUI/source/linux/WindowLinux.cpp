#include "stdafx.h"

/*****************************************************************************/
void Window::bringWindowToFront()
{
	glutSetWindow(findWindowInfoByWindow(this)->myGlutWindowIndex);
	glutPopWindow();
}
/*****************************************************************************/
void Window::setTitle(const char* pcsTitle)
{
	SGlutWindowWrapper* pInfo = findWindowInfoByWindow(this);
	if(!pInfo)
		ASSERT_RETURN;

	glutSetWindow(pInfo->myGlutWindowIndex);
	glutSetWindowTitle(pcsTitle);
}
/*****************************************************************************/
void Window::convertWindowCoordsToScreenCoords(const SVector2D& svIn, SVector2D& svOut)
{
	glutSetWindow(findWindowInfoByWindow(this)->myGlutWindowIndex);
	SVector2D svWindowOrigin;
	svWindowOrigin.x = glutGet(GLUT_WINDOW_X);
	svWindowOrigin.y = glutGet(GLUT_WINDOW_Y);

	svOut = svIn + svWindowOrigin;
}
/*****************************************************************************/
void Window::close()
{
	SGlutWindowWrapper* pInfo = findWindowInfoByWindow(pWindow);
	if(!pInfo)
		ASSERT_RETURN;

	glutDestroyWindow(pInfo->myGlutWindowIndex);

	myShouldClose = true;
}
/*****************************************************************************/
void getChosenPixelFormatDescription(string& strOut)
{
	strOut = "N/A on Linux\n";
}
/*****************************************************************************/
