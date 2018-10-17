#include "stdafx.h"
#include <gtk/gtk.h>
#include "GlutWindow.h"

/*****************************************************************************/
GtkClipboard* getClipboardWithText()
{
	GtkClipboard *clipboard;
	clipboard = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
	if(gtk_clipboard_wait_is_text_available(clipboard))
		return clipboard;

	clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	if(gtk_clipboard_wait_is_text_available(clipboard))
		return clipboard;

	return NULL;
}
/*****************************************************************************/
bool ClipboardManager::getDoHaveTextInClipboard()
{
	return getClipboardWithText() != NULL;
}
/*****************************************************************************/
void ClipboardManager::getClipboardText(string& strTextOut)
{
	strTextOut = "";
	GtkClipboard* pTextClipboard = getClipboardWithText();
	if(!pTextClipboard)
		return;

	gchar* pcsText = gtk_clipboard_wait_for_text(pTextClipboard);
	if(pcsText)
		strTextOut = pcsText;

	g_free(pcsText);
}
/*****************************************************************************/
void ClipboardManager::setClipboardText(const char* pcsText)
{
	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_PRIMARY), pcsText, strlen(pcsText));
	gtk_clipboard_store(gtk_clipboard_get(GDK_SELECTION_PRIMARY));

	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), pcsText, strlen(pcsText));
	gtk_clipboard_store(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD));
}
/*****************************************************************************/
