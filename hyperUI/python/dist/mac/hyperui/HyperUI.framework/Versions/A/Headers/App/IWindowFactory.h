#pragma once

/*****************************************************************************/
class HYPERUI_API IWindowFactory
{
public:
	virtual ~IWindowFactory() { }
	virtual Window* allocateWindow(ResourceType eSourceCollection, int iScreenW, int iScreenH, bool bIsMainWindow);
};
/*****************************************************************************/
