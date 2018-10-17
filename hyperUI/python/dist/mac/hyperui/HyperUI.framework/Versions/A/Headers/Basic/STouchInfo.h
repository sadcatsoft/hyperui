#pragma once

/*****************************************************************************/
struct HYPERUI_API STouchInfo
{
	STouchInfo() { myTouchId = NULL; }
	// Note that this is an id, you can't use
	// it as an actual pointer.
	SVector2D myPoint;
	void* myTouchId;
};
/*****************************************************************************/
typedef vector < STouchInfo > TTouchVector;
typedef map < void*, STouchInfo > TTouchMap;
/*****************************************************************************/