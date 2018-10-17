#import <Cocoa/Cocoa.h>
#include "stdafx.h"
#include "MacCommon.h"

namespace HyperUI
{
/*****************************************************************************/
bool ClipboardManager::getDoHaveTextInClipboard()
{
	NSPasteboard *pBoard = [NSPasteboard pasteboardWithName:NSGeneralPboard];
	NSData* pData = [pBoard dataForType:NSPasteboardTypeString];
	return pData != NULL;
}
/*****************************************************************************/
void ClipboardManager::getClipboardText(string& strTextOut)
{
	NSPasteboard *pBoard = [NSPasteboard pasteboardWithName:NSGeneralPboard];
	NSString* pString = [pBoard stringForType:NSPasteboardTypeString];
	strTextOut = getCStringSafe(pString);
}
/*****************************************************************************/
void ClipboardManager::setClipboardText(const char* pcsText)
{
	NSPasteboard *pBoard = [NSPasteboard pasteboardWithName:NSGeneralPboard];
	NSString *pStr = [NSString stringWithCString:pcsText];
	[pBoard clearContents];
	[pBoard setString:pStr forType:NSPasteboardTypeString];
}
/*****************************************************************************/
}
