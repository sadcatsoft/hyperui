#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
UIHorizontalTableElement::UIHorizontalTableElement(UIPlane* pParentPlane)
	: UITableElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void UIHorizontalTableElement::onAllocated(IBaseObject* pData)
{
	UITableElement::onAllocated(pData);
}
/*****************************************************************************/
const char* UIHorizontalTableElement::getCellType(int iColumn, int iRow)
{
	// If our parent doesn't have one, always return the first entry for our column:
	const char* pcsCellType = UITableElement::getCellType(iColumn, iRow);
	if(IS_VALID_STRING_AND_NOT_NONE(pcsCellType))
		return pcsCellType;

	return this->getEnumPropValue(PropertyCellTypes, 0);
}
/*****************************************************************************/
void UIHorizontalTableElement::ensureColumnVisible(int iCol)
{
	if(getNumExistingRows() <= 0)
		return;

	SRect2D srFirstRow, srOurRow;
	getScreenCellPos(0, 0, srFirstRow);
	getScreenCellPos(iCol, 0, srOurRow);

	SVector2D svOwnSize;
	getBoxSize(svOwnSize);

	FLOAT_TYPE fNeededScroll = srOurRow.x - srFirstRow.x - svOwnSize.x/2.0;
	if(fNeededScroll < 0)
		fNeededScroll = 0;
	setScroll(fNeededScroll + TABLE_VERT_PADDING);
}
/*****************************************************************************/
bool UIHorizontalTableElement::getAllowScrollFromPoint(const SVector2D& svBeginPoint, FLOAT_TYPE fX, FLOAT_TYPE fY)
{
	return fabs(svBeginPoint.x - fX) >= getScrollPixelTolerance()
		// This line makes sure we can drag items out of the table, so it only
		// scrolls if we have a mostly vertical motion
		&& fabs(svBeginPoint.y - fY)/fabs(svBeginPoint.x - fX) <= HOR_VERT_MOTION_RATIO;
}
/*****************************************************************************/
FLOAT_TYPE UIHorizontalTableElement::computeScrollFromPoint(const SVector2D& svBeginPoint, FLOAT_TYPE fX, FLOAT_TYPE fY)
{
	return getInitScroll() - (fX - svBeginPoint.x);
}
/*****************************************************************************/
FLOAT_TYPE UIHorizontalTableElement::computeScrollSpeedFromTrail(const SVector2D& svCurrPoint, FLOAT_TYPE fX, FLOAT_TYPE fY, GTIME lTime, GTIME lCurrPointTime)
{
	return (svCurrPoint.x - fX)/(lTime - lCurrPointTime);
}
/*****************************************************************************/
void UIHorizontalTableElement::getInitCellPos(const SVector2D& svScroll, const SVector2D& svPos, FLOAT_TYPE fTotalScale, FLOAT_TYPE fSelfW, FLOAT_TYPE fSelfH, SVector2D& svInitPosOut)
{
	svInitPosOut.y = svScroll.y - getTotalRowHeight()/2.0*fTotalScale + svPos.y;
	svInitPosOut.x = svScroll.x - fSelfW/2.0 + svPos.x + TABLE_VERT_PADDING;
	svInitPosOut.x -= getScrollOffset();
}
/*****************************************************************************/
int UIHorizontalTableElement::getMaxScrollInScrollAxisDirection(const SVector2D& svBoxSize, int iSpacing)
{
	return getTotalRowWidth() - iSpacing + TABLE_VERT_PADDING*2 - svBoxSize.x;
}
/*****************************************************************************/
};