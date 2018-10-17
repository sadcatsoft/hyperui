#include "stdafx.h"
#include "WinManaged.h"

#include <windows/WinIncludes.h>

namespace HyperUI
{
/*****************************************************************************/
const char* const g_pcsWinCursorFiles[CursorLastPlaceholder + 1] = 
{
	PROPERTY_NONE,
	PROPERTY_NONE,
	PROPERTY_NONE,
	PROPERTY_NONE,
	PROPERTY_NONE,
	PROPERTY_NONE,
	PROPERTY_NONE,
	"eyedropper.cur",
	"pan.cur",
	"fill.cur",
	"target.cur",
	"magicwand.cur",
	"zoom.cur",
	0
};
/*****************************************************************************/
class WinCursorManager : public ICursorManager
{
public:
	WinCursorManager()
	{
		memset(myCursors, 0, sizeof(HCURSOR)*CursorLastPlaceholder);
		pushCursor(CursorArrow);
	}

	virtual ~WinCursorManager() { }

	virtual void setCursor(CursorType eType)
	{
		if(eType == CursorLastPlaceholder)
			return;
		_ASSERT(eType != CursorInvalidUnset);

		STATIC_ASSERT(CursorLastPlaceholder == 17);

		if(!myCursors[eType])
		{
			// Load the cursor
			// Use LoadCursorFromFile for custom ones.
			if(eType == CursorArrow)
				myCursors[eType] = LoadCursor(NULL, IDC_ARROW);
			else if(eType == CursorWait)
				myCursors[eType] = LoadCursor(NULL, IDC_WAIT);
			else if(eType == CursorHorSize)
				myCursors[eType] = LoadCursor(NULL, IDC_SIZEWE);
			else if(eType == CursorVertSize)
				myCursors[eType] = LoadCursor(NULL, IDC_SIZENS);
			else if(eType == CursorForbidden)
				myCursors[eType] = LoadCursor(NULL, IDC_NO);
			else if(eType == CursorCrosshair)
				myCursors[eType] = LoadCursor(NULL, IDC_CROSS);
			else if(eType == CursorIBeam)
				myCursors[eType] = LoadCursor(NULL, IDC_IBEAM);
			else if(eType == CursorSizeTopLeftCorner 
				|| eType == CursorSizeBottomRightCorner)
				myCursors[eType] = LoadCursor(NULL, IDC_SIZENWSE);
			else if(eType == CursorSizeTopRightCorner
				|| eType == CursorSizeBottomLeftCorner)
				myCursors[eType] = LoadCursor(NULL, IDC_SIZENESW);
			else if(eType == CursorEyeDropper
				|| eType == CursorPan
				|| eType == CursorFill
				|| eType == CursorTarget
				|| eType == CursorMagicWand
				|| eType == CursorZoom)
			{
				string strCursorPath;
				this->getCursorFilePath(g_pcsWinCursorFiles[eType], false, strCursorPath);
				wchar_t* pcsConvName = StringUtils::asciiToWide(strCursorPath.c_str());

				myCursors[eType] = LoadCursorFromFile(pcsConvName);
				StringUtils::freeWideText(pcsConvName);
			}
			ELSE_ASSERT;

			_ASSERT(myCursors[eType]);
		}
		/*
		static bool bHad7 = false;
		static CursorType eOldType = CursorLastPlaceholder;
		gLog("Setting cursor %d\n", (int)eType);
		if(bHad7 && eOldType == CursorCrosshair && eType == CursorArrow)
		{
			int bp =0;
		}
		eOldType = eType;	
		if(eType == CursorEyeDropper)
			bHad7 = true;
		*/
		// Set the cursor
		SetCursor(myCursors[eType]);		
	}

private:
	HCURSOR myCursors[CursorLastPlaceholder];
};
/*****************************************************************************/
ICursorManager* Window::allocateCursorManager()
{
	return new WinCursorManager;
}
/*****************************************************************************/
void Window::deallocateCursorManager(ICursorManager* pCursorManager)
{
	delete pCursorManager;
}
/*****************************************************************************/
void Window::close()
{
	myHasCalledCloseAlready = false;
	myShouldClose = true;
}
/*****************************************************************************/
void Window::bringWindowToFront()
{
	IBaseForm^ pBaseForm = findFormByWindow(this);
	if(pBaseForm)
		pBaseForm->BringToFront();
	ELSE_ASSERT_RETURN;
}
/*****************************************************************************/
void Window::setTitle(const char* pcsTitle)
{
	IBaseForm^ pBaseForm = findFormByWindow(this);
	if(!pBaseForm)
		ASSERT_RETURN;
	pBaseForm->Text = gcnew System::String(pcsTitle);
}
/*****************************************************************************/
void Window::convertWindowCoordsToScreenCoords(const SVector2D& svLocalCoords, SVector2D& svScreenCoordsOut)
{
	svScreenCoordsOut = svLocalCoords;
	IBaseForm^ pBaseForm = findFormByWindow(this);
	if(!pBaseForm)	
		ASSERT_RETURN;

	// Now, get the window origin
	System::Drawing::Point rPoint(svLocalCoords.x, svLocalCoords.y);
	rPoint = pBaseForm->PointToScreen(rPoint);
	svScreenCoordsOut.x = rPoint.X;
	svScreenCoordsOut.y = rPoint.Y;
}
/*****************************************************************************/
};