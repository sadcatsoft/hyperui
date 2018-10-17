#pragma once

class UIPlane;
/*****************************************************************************/
struct HYPERUI_API SUIDragRegion
{
	SRect2D myRect;
	SideType mySide;
	string myAnim;
	string myHoverAnim;
	bool myIsGlobal;
};
typedef vector < SUIDragRegion > TUIDragRegions;
/*****************************************************************************/
class HYPERUI_API UIDragDropManager
{
public:
	UIDragDropManager();

	void render(UIElement* pElem, SVector2D svPos);

	void recomputeDragRegions(UIPlane* pUIPlane, const SRect2D& srNewRect);

	SUIDragRegion* getRegionAt(const SVector2D& svScreenPos);

protected:

	int getRegionIndexAt(const SVector2D& svScreenPos);

private:
	TUIDragRegions myDragRegions;
	SRect2D myLastDragRegion;
};
/*****************************************************************************/
