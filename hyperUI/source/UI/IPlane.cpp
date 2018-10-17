#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
IPlane::IPlane(Window* pParentWindow)
{
	myParentWindow = pParentWindow;
}
/*****************************************************************************/
DrawingCache* IPlane::getDrawingCache()
{
	if(!getParentWindow())
		ASSERT_RETURN_NULL;
	return getParentWindow()->getDrawingCache();
}
/********************************************************************************************/
};