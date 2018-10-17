#import <Cocoa/Cocoa.h>
#include "stdafx.h"
#include "MacCommon.h"

namespace HyperUI
{
/*****************************************************************************/
void Window::bringWindowToFront()
{
	NSResponder* pController = findFormByWindow(this);
	if(!pController)
		ASSERT_RETURN;
	NSWindow *pNativeWindow = [pController getNativeWindow];
	[pNativeWindow makeKeyAndOrderFront:nil];
}
/*****************************************************************************/
void Window::setTitle(const char* pcsTitle)
{
	NSResponder* pController = findFormByWindow(this);
	if(!pController)
		ASSERT_RETURN;
	NSWindow *pNativeWindow = [pController getNativeWindow];

	[pNativeWindow setTitle:[NSString stringWithCString:pcsTitle]];
}
/*****************************************************************************/
void Window::convertWindowCoordsToScreenCoords(const SVector2D& svLocalCoords, SVector2D& svScreenCoordsOut)
{
	svScreenCoordsOut = svLocalCoords;
	NSResponder* pController = findFormByWindow(this);
	if(!pController)
		ASSERT_RETURN;
	NSWindow *pNativeWindow = [pController getNativeWindow];

	NSRect nsRect;
	nsRect.origin.x = svLocalCoords.x;
	nsRect.origin.y = svLocalCoords.y;
	nsRect.size.width = nsRect.size.height = 1;
	if([pNativeWindow respondsToSelector:@selector(convertRectToScreen:)])
		nsRect = [pNativeWindow convertRectToScreen:nsRect];
	else
	{
		NSPoint pt;
		pt.x = nsRect.origin.x;
		pt.y = nsRect.origin.y;
		nsRect.origin = [pNativeWindow convertBaseToScreen:pt];
	}

	svScreenCoordsOut.x = nsRect.origin.x;
	svScreenCoordsOut.y = nsRect.origin.y;
/*
	// Now, flip the Y coord... idiots...
	NSScreen* pWindowScreen = [pNativeWindow screen];
	NSRect nsScreenRect = [pWindowScreen visibleFrame];

	svScreenCoordsOut.y = nsScreenRect.size.height - svScreenCoordsOut.y - 1;
    int bp = 0;
    */
}
/*****************************************************************************/
void Window::close()
{
	// This means we've already sent the message
	gLog("gCloseWindow\n");
	if(this->getShouldClose())
	{
		gLog("gCloseWindow - should close is false\n");
		return;
	}

	NSResponder* pController = findFormByWindow(this);
	if(!pController)
	{
		gLog("gCloseWindow - controller not found\n");
		ASSERT_RETURN;
	}
	NSWindow *pNativeWindow = [pController getNativeWindow];

	gLog("gCloseWindow - pNativeWindow is valid = %s\n", pNativeWindow ? "YES" : "NO");

    if(pNativeWindow.parentWindow)
        [pNativeWindow.parentWindow removeChildWindow:pNativeWindow];
    [pNativeWindow performSelector:@selector(performClose:) withObject:nil afterDelay:0.0];

    gLog("gCloseWindow done\n");

	myShouldClose = true;
}
/*****************************************************************************/
void getChosenPixelFormatDescription(string& strOut)
{
	strOut = "N/A on OS X\n";
}
/*****************************************************************************/
#ifdef OSX_BUILD
class MacCursorManager : public ICursorManager
{
public:

	MacCursorManager() 
	{
		memset(myCursors, 0, sizeof(NSCursor*)*CursorLastPlaceholder);
        pushCursor(CursorArrow);
	}

	virtual ~MacCursorManager() { }
	
	virtual void setCursor(CursorType eType)
	{
		if(eType == CursorLastPlaceholder)
			return;
		_ASSERT(eType != CursorInvalidUnset);

		STATIC_ASSERT(CursorLastPlaceholder == 17);

		SIntVector2D siTip;
		string strCursorName;

		if(!myCursors[eType])
		{
			// Load the cursor
			if(eType == CursorArrow)
				myCursors[eType] = [NSCursor arrowCursor];
			else if(eType == CursorWait)
			{
				_ASSERT(0);
				myCursors[eType] = [NSCursor arrowCursor];
			}
			else if(eType == CursorHorSize)
				myCursors[eType] = [NSCursor resizeLeftRightCursor];
			else if(eType == CursorVertSize)
				myCursors[eType] = [NSCursor resizeUpDownCursor];
			else if(eType == CursorForbidden)
			{
				_ASSERT(0);
				myCursors[eType] = [NSCursor arrowCursor];
			}
			else if(eType == CursorCrosshair)
				myCursors[eType] = [NSCursor crosshairCursor];
			else if(eType == CursorIBeam)
				myCursors[eType] = [NSCursor IBeamCursor];
			else if(eType == CursorSizeTopLeftCorner 
				|| eType == CursorSizeBottomRightCorner)
				myCursors[eType] = [NSCursor pointingHandCursor];
			else if(eType == CursorSizeTopRightCorner
				|| eType == CursorSizeBottomLeftCorner)
				myCursors[eType] = [NSCursor pointingHandCursor];
			else if(eType == CursorEyeDropper)
			{
				strCursorName = "cursor_eyedropper";
				siTip.set(2, 19);
			}
			else if(eType == CursorPan)
			{
				strCursorName = "cursor_pan";
				siTip.set(10, 12);
			}
			else if(eType == CursorFill)
			{
				strCursorName = "cursor_fill";
				siTip.set(1, 18);
			}
			else if(eType == CursorTarget)
			{
				strCursorName = "cursor_target";
				siTip.set(15, 15);
			}
			else if(eType == CursorMagicWand)
			{
				strCursorName = "cursor_mwand";
				siTip.set(2, 1);
			}
			else if(eType == CursorZoom)
			{
				strCursorName = "cursor_zoom";
				siTip.set(1, 1);
			}
			ELSE_ASSERT;

			// Now, if we have the custom file cursor, load it:
			if(strCursorName.length() > 0)
			{
				// Scale the tip if necessary
				int iDensityMult = getScreenDensityScalingFactor();
				siTip.x *= iDensityMult;
				siTip.y *= iDensityMult;

				strCursorName += ".png";

				// Now, construct a full file name. Note that it may or may not exist:
				string strFullFileName;
				this->getCursorFilePath(strCursorName.c_str(), true, strFullFileName);
				if(!FileUtils::doesFileExist(strFullFileName.c_str(), FileSourcePackageDir))
					this->getCursorFilePath(strCursorName.c_str(), false, strFullFileName);

				myCursors[eType] = [[NSCursor alloc] initWithImage:[NSImage imageNamed:[NSString stringWithCString:strFullFileName.c_str()]] hotSpot:NSMakePoint(siTip.x, siTip.y)];
			}
		}

		[myCursors[eType] set];
	}

private:

	NSCursor* myCursors[CursorLastPlaceholder];
};
#endif

ICursorManager* Window::allocateCursorManager()
{
	return new MacCursorManager();
}
/*****************************************************************************/
void Window::deallocateCursorManager(ICursorManager* pCursorManager)
{
	delete pCursorManager;
}
/*****************************************************************************/
}
