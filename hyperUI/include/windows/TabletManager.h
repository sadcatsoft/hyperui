#pragma once

namespace HyperUI
{
/*****************************************************************************/
struct HYPERUI_API STableCursorInfo
{
	TabletCursorType myType;
	int myHardwareIndex;
};
typedef map < int, STableCursorInfo > TTabletCursorsMap;
/*****************************************************************************/
class HYPERUI_API TabletManager
{
public:
	~TabletManager();
	static TabletManager* getInstance();

	void initialize(HWND hWnd, bool bLoadDLL);
	void enableContext();
	void shutdown(bool bUnloadDLL);

	const STableCursorInfo* getCursorInfo(int iHardwareIndex) const;

private:
	TabletManager();
	HCTX tabletInit(HWND hWnd);

private:

	static TabletManager* theInstance;

	HCTX myTabletContext;
	bool myDoHaveTablet;

	TTabletCursorsMap myCursors;
};
/*****************************************************************************/
};