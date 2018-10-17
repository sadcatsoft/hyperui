#pragma once

#include "BridgeElementBase.h"

namespace PythonAPI
{
/*****************************************************************************/
template < class BASE_TYPE >
class TBridgeUIElement : public BASE_TYPE, public BridgeElementBase, public HyperUI::ITableCellInfoProvider, public HyperUI::IGridCellInfoProvider
{
public:
	DECLARE_UIELEMENT(TBridgeUIElement);

	virtual bool handleAction(string& strAction, HyperUI::UIElement* pSourceElem, IBaseObject* pData);
	virtual void updateOwnData(HyperUI::SUpdateInfo& rRefreshInfo);
	virtual void onTimerTick(GTIME lGlobalTime);

	// ITableCellInfoProvider
	virtual int getNumRows(HyperUI::UIElement* pCaller);
	virtual int getNumColumns(HyperUI::UIElement* pCaller);
	virtual void updateCell(HyperUI::UITableCellElement* pCell);
	virtual void onPostCreateRows(HyperUI::UITableElement* pTable);

	// IGridCellInfoProvider
	virtual int getNumTotalCells(HyperUI::UIElement* pCaller);
};
/*****************************************************************************/
typedef TBridgeUIElement< HyperUI::UIElement > BridgeUIElement;
typedef TBridgeUIElement< HyperUI::UISplitterElement > BridgeUISplitterElement;
/*****************************************************************************/
};