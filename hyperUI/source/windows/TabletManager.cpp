#include "stdafx.h"
#include <windows/TabletHeaders.h>
#include <windows/TabletManager.h>

namespace HyperUI
{
extern FLOAT_TYPE g_fMaxPenPressure;
TabletManager* TabletManager::theInstance = NULL;
/*****************************************************************************/
TabletManager::TabletManager()
{
	myTabletContext = NULL;
	// Not the same as { 0 }...
	//memset(&myLogContext, 0, sizeof(LOGCONTEXTA));
	myDoHaveTablet = false;
}
/*****************************************************************************/
TabletManager::~TabletManager()
{

}
/*****************************************************************************/
TabletManager* TabletManager::getInstance()
{
	if(!theInstance)
		theInstance = new TabletManager;
	return theInstance;
}
/*****************************************************************************/
void TabletManager::initialize(HWND hWnd, bool bLoadDLL)
{
	bool bDllInitialized = false;
	if(bLoadDLL)
	{
		if(LoadWintab())
		{
			if(gpWTInfoA(0, 0, NULL))
				bDllInitialized = true;
			ELSE_ASSERT;
		}
	}
	else
		// Assume initalized.
		bDllInitialized = true;

	if(bDllInitialized)
		myTabletContext = tabletInit(hWnd);
}
/*****************************************************************************/
void TabletManager::shutdown(bool bUnloadDLL)
{
	if (myTabletContext)
		gpWTClose(myTabletContext);
	if(bUnloadDLL)
		UnloadWintab( );
}
/*****************************************************************************/
HCTX TabletManager::tabletInit(HWND hWnd)
{
	HCTX hctx = NULL;
	UINT wDevice = 0;
	UINT wExtX = 0;
	UINT wExtY = 0;
	UINT wWTInfoRetVal = 0;
	AXIS TabletX = {0};
	AXIS TabletY = {0};

	LOGCONTEXTA rLogContext = { 0 };

	if(!gpWTInfoA)
		return NULL;

	// Set option to move system cursor before getting default system context.
	rLogContext.lcOptions |= CXO_SYSTEM;

	// Open default system context so that we can get tablet data
	// in screen coordinates (not tablet coordinates).
	wWTInfoRetVal = gpWTInfoA(WTI_DEFSYSCTX, 0, &rLogContext);
	_ASSERT( wWTInfoRetVal == sizeof( LOGCONTEXTA ) );

	_ASSERT( rLogContext.lcOptions & CXO_SYSTEM );

	// modify the digitizing region
	sprintf(rLogContext.lcName, "HyperUI Digitizing");

	// We process WT_PACKET (CXO_MESSAGES) messages.
	rLogContext.lcOptions |= CXO_MESSAGES;

	// What data items we want to be included in the tablet packets
	rLogContext.lcPktData = PACKETDATA;

	// Which packet items should show change in value since the last
	// packet (referred to as 'relative' data) and which items
	// should be 'absolute'.
	rLogContext.lcPktMode = PACKETMODE;

	// This bitfield determines whether or not this context will receive
	// a packet when a value for each packet field changes.  This is not
	// supported by the Intuos Wintab.  Your context will always receive
	// packets, even if there has been no change in the data.
	rLogContext.lcMoveMask = PACKETDATA;

	// Which buttons events will be handled by this context.  lcBtnMask
	// is a bitfield with one bit per button.
	rLogContext.lcBtnUpMask = rLogContext.lcBtnDnMask;

	// Set the entire tablet as active
	wWTInfoRetVal = gpWTInfoA( WTI_DEVICES + 0, DVC_X, &TabletX );
	_ASSERT( wWTInfoRetVal == sizeof( AXIS ) );

	wWTInfoRetVal = gpWTInfoA( WTI_DEVICES, DVC_Y, &TabletY );
	_ASSERT( wWTInfoRetVal == sizeof( AXIS ) );
	/*
	myLogContext.lcInOrgX = 0;
	myLogContext.lcInOrgY = 0;
	myLogContext.lcInExtX = TabletX.axMax;
	myLogContext.lcInExtY = TabletY.axMax;
	*/
	// Guarantee the output coordinate space to be in screen coordinates.  
	rLogContext.lcOutOrgX = GetSystemMetrics( SM_XVIRTUALSCREEN );
	rLogContext.lcOutOrgY = GetSystemMetrics( SM_YVIRTUALSCREEN );
	rLogContext.lcOutExtX = GetSystemMetrics( SM_CXVIRTUALSCREEN ); //SM_CXSCREEN );

	// In Wintab, the tablet origin is lower left.  Move origin to upper left
	// so that it coincides with screen origin.
	rLogContext.lcOutExtY = -GetSystemMetrics( SM_CYVIRTUALSCREEN );	//SM_CYSCREEN );

	// Leave the system origin and extents as received:
	// lcSysOrgX, lcSysOrgY, lcSysExtX, lcSysExtY

	AXIS rPressure;
	bool pressureSupport = gpWTInfoA(WTI_DEVICES, DVC_NPRESSURE, &rPressure) != 0;
	if (pressureSupport)
		g_fMaxPenPressure = rPressure.axMax;
	else
		g_fMaxPenPressure = 1024;

	AXIS rOrientation[3];
	bool bHaveTiltSupport = gpWTInfoA(WTI_DEVICES, DVC_ORIENTATION, &rOrientation);
	// Does support azimuth & altitude
	if (rOrientation[0].axResolution && rOrientation[1].axResolution) 
	{
		FLOAT_TYPE fAziFactor = (FLOAT_TYPE)(rOrientation[0].axResolution)/(2.0*M_PI);  
		FLOAT_TYPE fAltFactor = (FLOAT_TYPE)(rOrientation[1].axResolution)/1000.0; 
		FLOAT_TYPE fAltAdjust = (double)rOrientation[1].axMax/fAltFactor;

		/*
		wintab.h defines .orAltitude
		as a UINT but documents .orAltitude
		as positive for upward angles
		and negative for downward angles.
		WACOM uses negative altitude values to
		show that the pen is inverted;
		therefore we cast .orAltitude as an
		(int) and then use the absolute value.
		*/
/*
		ZAngle = (int)m_ortNew.orAltitude;
		ZAngle2 = m_altAdjust - (double)abs(ZAngle)/m_altFactor;
		// adjust azimuth 
		Thata = m_ortNew.orAzimuth;
		Thata2 = (double)Thata/m_aziFactor;
		// get the length of the diagnal to draw 
		m_Z1Angle.x = (int)(ZAngle2*sin(Thata2));
		m_Z1Angle.y = (int)(ZAngle2*cos(Thata2));*/
	}
	else
		bHaveTiltSupport = false;

	unsigned int iNumCursors;
	gpWTInfoA(WTI_INTERFACE, IFC_NCURSORS, &iNumCursors);

	myCursors.clear();
	
	int iCursor;
	UINT mode;
	UINT capabilities;
	DWORD physid;
	char pcsCurName[2048];
	for(iCursor = 0; iCursor < iNumCursors; iCursor++)
	{
		gpWTInfoA(WTI_CURSORS + iCursor, CSR_PHYSID, &physid);
		// Tablet reports non-existant cursors...
		if(physid == 0)
			continue;

		gpWTInfoA(WTI_CURSORS + iCursor, CSR_NAME, &pcsCurName);
		gpWTInfoA(WTI_CURSORS + iCursor, CSR_CAPABILITIES, &capabilities);
		gpWTInfoA(WTI_CURSORS + iCursor, CSR_MODE, &mode);

		STableCursorInfo rInfo;
		if((capabilities & CRC_INVERT) != 0)
			rInfo.myType = TabletCursorErase;
		else
			rInfo.myType = TabletCursorDraw;
		rInfo.myHardwareIndex = iCursor;
		myCursors[iCursor] = rInfo;
	}
	

// 	char cNumButtons;
// 	gpWTInfoA(WTI_CURSORS, CSR_BUTTONS, &cNumButtons);
// 
// 	int bp2 = 0;

// 	char pcsButtonNames[2048];
// 	gpWTInfoA(WTI_CURSORS, CSR_BTNNAMES, &pcsButtonNames);

	int bp3 = 0;
	// See cursors
	//gpWTInfoA(WTI_CURSORS

	// open the region
	// The Wintab spec says we must open the context disabled if we are 
	// using cursor masks.  
	hctx = gpWTOpenA( hWnd, &rLogContext, FALSE );

	//WacomTrace("HCTX: %i\n", hctx);

	myDoHaveTablet = true;
	return hctx;

}
/*****************************************************************************/
void TabletManager::enableContext()
{
	if(!myTabletContext) 
		return;

	// 		gpWTEnable(g_pTabletContext, GET_WM_ACTIVATE_STATE(wParam, lParam));
	// 		if (g_pTabletContext && GET_WM_ACTIVATE_STATE(wParam, lParam))
	// 			gpWTOverlap(g_pTabletContext, TRUE);

	int iRes;
	iRes = gpWTEnable(myTabletContext, true);
	// We can hit this after closing a second window
	// for unknown reasons
	//_ASSERT(iRes != 0);
	if (myTabletContext)
	{
		iRes = gpWTOverlap(myTabletContext, TRUE);
		// We can hit this after closing a second window
		// for unknown reasons
		//_ASSERT(iRes != 0);
	}
}
/*****************************************************************************/
const STableCursorInfo* TabletManager::getCursorInfo(int iHardwareIndex) const
{
	TTabletCursorsMap::const_iterator mi = myCursors.find(iHardwareIndex);
	if(mi == myCursors.end())
		return NULL;
	else
		return &mi->second;
}
/*****************************************************************************/
};