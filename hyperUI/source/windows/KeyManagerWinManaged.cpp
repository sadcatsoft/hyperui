#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
bool KeyManager::getIsCapsLockOn()
{
#ifdef DIRECTX_PIPELINE
	_ASSERT(0);
	DXSTAGE3
		return false;
#else
	return System::Windows::Forms::Control::IsKeyLocked(System::Windows::Forms::Keys::CapsLock);
#endif
}
/*****************************************************************************/
void KeyManager::beginKeyboardFocus(UIElement* pElement)
{

}
/*****************************************************************************/
void KeyManager::endKeyboardFocus()
{

}
/*****************************************************************************/
}