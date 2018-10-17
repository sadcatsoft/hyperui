#include "stdafx.h"

namespace HyperUI 
{
/*****************************************************************************/
Window* IWindowFactory::allocateWindow(ResourceType eSourceCollection, int iScreenW, int iScreenH, bool bIsMainWindow)
{
	return new Window(iScreenW, iScreenH, bIsMainWindow);
}
/*****************************************************************************/
};