#pragma once

/*****************************************************************************/
class HYPERUI_API UIHorizontalTableElement : public UITableElement
{
public:
	DECLARE_STANDARD_UIELEMENT(UIHorizontalTableElement, UiElemHorizontalTable);

	virtual const char* getCellType(int iColumn, int iRow);

	void ensureColumnVisible(int iCol);

protected:
	virtual bool getAllowScrollFromPoint(const SVector2D& svBeginPoint, FLOAT_TYPE fX, FLOAT_TYPE fY);
	virtual FLOAT_TYPE computeScrollFromPoint(const SVector2D& svBeginPoint, FLOAT_TYPE fX, FLOAT_TYPE fY);
	virtual FLOAT_TYPE computeScrollSpeedFromTrail(const SVector2D& svCurrPoint, FLOAT_TYPE fX, FLOAT_TYPE fY, GTIME lTime, GTIME lCurrPointTime);
	virtual void getInitCellPos(const SVector2D& svScroll, const SVector2D& svPos, FLOAT_TYPE fTotalScale, FLOAT_TYPE fSelfW, FLOAT_TYPE fSelfH, SVector2D& svInitPosOut);
	virtual int getMaxScrollInScrollAxisDirection(const SVector2D& svBoxSize, int iSpacing);
};
/*****************************************************************************/