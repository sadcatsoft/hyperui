#pragma once

/*****************************************************************************/
class HYPERUI_API ClipboardManager
{
public:

	// Text methods
	static bool getDoHaveTextInClipboard();
	static void getClipboardText(string& strTextOut);
	static void setClipboardText(const char* pcsText);
};
/*****************************************************************************/