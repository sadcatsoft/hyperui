#import <Cocoa/Cocoa.h>
#include "stdafx.h"
#include "MacCommon.h"

namespace HyperUI
{
/*****************************************************************************/
bool KeyManager::getIsCapsLockOn()
{
#ifdef IOS_BUILD
    return false;
#else
	return (NSAlphaShiftKeyMask & [NSEvent modifierFlags]) != 0;
#endif
}
/*****************************************************************************/
void KeyManager::beginKeyboardFocus(UIElement* pElement)
{
#ifndef MAC_BUILD
	[g_pMainView beginKeyboardFocus:pElement];
#endif
}
/*****************************************************************************/
void KeyManager::endKeyboardFocus()
{
#ifndef MAC_BUILD
	[g_pMainView endKeyboardFocus];
#endif
}
/*****************************************************************************/
}
