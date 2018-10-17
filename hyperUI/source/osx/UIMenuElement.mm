#import <Cocoa/Cocoa.h>
#include "stdafx.h"
#include "MacCommon.h"

namespace HyperUI
{
/*****************************************************************************/
void UIMenuElement::onMenuStatusRefreshed()
{
	MainMenuManager::getInstance()->refreshMenuStatus(this);
}
/*****************************************************************************/
void UIMenuElement::onMenuShortcutsChanged()
{
	MainMenuManager::getInstance()->refreshMenuShortcuts(this);
}
/*****************************************************************************/
void UIMenuElement::onMenuChanged()
{
	MainMenuManager::getInstance()->rebuildMenu(this);
}
/*****************************************************************************/
}
