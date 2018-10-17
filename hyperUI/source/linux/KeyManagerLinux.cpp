#include "stdafx.h"

/*****************************************************************************/
bool KeyManager::getIsCapsLockOn()
{
	return KeyManager::getInstance()->getIsModifierKeyDown(SilentKeyShift, false);
}
/*****************************************************************************/
