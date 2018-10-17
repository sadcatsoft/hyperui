#include "stdafx.h"

#define DRAG_VERT_AUTOSCROLL_HEIGHT		upToScreen(30)
#define DRAG_MAX_AUTOSCROLL_SPEED		upToScreen(300)

#define ANIM_SCROLL_CLOCK_TYPE	ClockMainGame	// ClockUiPrimary

//#define DEBUG_DRAW_CELL_BORDERS

namespace HyperUI
{
#ifndef MAC_BUILD
string UITableElement::theSharedString;
#endif
/*****************************************************************************/
UITableElement::UITableElement(UIPlane* pParentPlane)
	: UIElement(pParentPlane), myElementsMapMemCacher(16) // , myTempGeneratedNames(&myElementsMapMemCacher)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
UITableElement::~UITableElement()
{
	onDeallocated();
}
/*****************************************************************************/
void UITableElement::onDeallocated(void)
{
	clearRows();
	//myTempGeneratedNames.clear();
}
/*****************************************************************************/
void UITableElement::onAllocated(IBaseObject* pData)
{
	UIElement::onAllocated(pData);
	// Special case - never assign this direclty, always do markDirty() elsewhere.
	myIgnoreSlider = false;
	myIsDirty = TableRefreshRecreate;
	myTotalRowWidth = 0;
	myDragAcceptType = AcceptNone;
	myDragHoverTargetCellId = -1;
	myBeginPressPos.set(0,0);
	mySelectedCell = NULL;
	myAutoscrollDir = 0;
	myScrollOffset = TABLE_VERT_PADDING;
	myHasScrolled = false;
	myInitScroll = TABLE_VERT_PADDING;
	myScrollSpeed = 0;
	myAllowScrolling = true;

	myLastCellDef = NULL;

	myMouseTrail.setFadeSpeed(0);
	myMouseTrail.setTrailFreqMult(0);
	myMouseTrail.setMaxPoints(256);

	myCellSpacing = -1;
	myLastMouseElement = NULL;
	myInitPressedCell = NULL;
	myIsCallingFromReleased = false;
}
/*****************************************************************************/
void UITableElement::postInit(void)
{
	UIElement::postInit();
}
/*****************************************************************************/
void UITableElement::render(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	preRender(svScroll, fOpacity, fScale);

	// If we're dirty, recreate the table.
	///if(myIsDirty)
		ensureRowsUpToDate();

	//theSharedString = this->getStringProp(PropertyObjAnim);
	this->getFullTopAnimName(theSharedString);
	renderInternal(theSharedString, svScroll, fOpacity, fScale);
	renderTextInternal(svScroll, fOpacity, fScale, false);

#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif

	// Render the children, too, since a slider might be in there.
	SVector2D svPos;
	FLOAT_TYPE fFinalOpac, fLocScale;
	this->getLocalPosition(svPos, &fFinalOpac, &fLocScale);
	fFinalOpac *= fOpacity;

	renderChildren(svScroll, fOpacity, fScale*fLocScale);

	if(this->getParent())
		svPos *= fScale;

	if(myRows.size() <= 0)
		return;


	// Get widths of all the columns
	SUITableRow* pRow;
	int iCurrCol, iNumCols;
	int iCurrRow, iNumRows = myRows.size();

	int iSpacing = getCellSpacing();

	// Compute total width of the row
	int iCurrRowWidth;
	//int iCurrRowHeight;

	FLOAT_TYPE fSelfW = this->getNumProp(PropertyWidth)*fLocScale*fScale;
	FLOAT_TYPE fSelfH = this->getNumProp(PropertyHeight)*fLocScale*fScale;

	bool bDidBeginStencil = false;
	SVector2D svGlobalTopLeft;
	svGlobalTopLeft.x = svPos.x + svScroll.x - fSelfW/2.0;
	svGlobalTopLeft.y = svPos.y + svScroll.y - fSelfH/2.0;
	if(RenderUtils::isInsideStencil() == false)
	{
		RenderUtils::beginStencilRectangle(getDrawingCache(), svGlobalTopLeft.x, svGlobalTopLeft.y, fSelfW, fSelfH);
		bDidBeginStencil = true;
	}

	SVector2D svInitPos;
	getInitCellPos(svScroll, svPos, fLocScale*fScale, fSelfW, fSelfH, svInitPos);
// 	svInitPos.x = svScroll.x - myTotalRowWidth/2.0*fLocScale*fScale + svPos.x;
// 	svInitPos.y = svScroll.y - fSelfH/2.0 + svPos.y + TABLE_VERT_PADDING;
// 	svInitPos.y -= myScrollOffset;

	FLOAT_TYPE fCellW;
	SVector2D svElemPos;
	int iCurrRowHeight = 0;
	if(iNumRows > 0)
		iCurrRowHeight = myRows[0].myHeight;
	svElemPos.y = svInitPos.y; //  + iCurrRowHeight/2.0*fLocScale*fScale;
	for(iCurrRow = 0; iCurrRow < iNumRows; iCurrRow++)
	{
		iCurrRowWidth = 0;
		iCurrRowHeight = myRows[iCurrRow].myHeight;

		svElemPos.y += iCurrRowHeight/2.0*fScale*fLocScale;

		if(svElemPos.y + iCurrRowHeight/2.0 >= svGlobalTopLeft.y && svElemPos.y - iCurrRowHeight/2.0 <= svGlobalTopLeft.y + fSelfH)
		{
			// Draw only if we're actually visible.
			pRow = &myRows[iCurrRow];
			iNumCols = pRow->myCells.size();
			for(iCurrCol = 0; iCurrCol < iNumCols; iCurrCol++)
			{
				// Draw each cell with proper offsets.
				// Don't we need cell's local scale here, too?
				fCellW = pRow->myCells[iCurrCol]->getNumProp(PropertyWidth)/2.0*fScale*fLocScale;
				svElemPos.x = svInitPos.x + iCurrRowWidth + fCellW;

				// Only if we're visible
				if(svElemPos.x + fCellW/2.0 >= svGlobalTopLeft.x && svElemPos.x - fCellW/2.0 <= svGlobalTopLeft.x + fSelfW)
				{
					pRow->myCells[iCurrCol]->render(svElemPos, fOpacity, fScale);
					this->onCellRendered(pRow->myCells[iCurrCol], svElemPos);
				}

				if(myDragAcceptType != AcceptNone && myDragHoverTargetCellId >= 0 && myDragHoverTargetCellId == pRow->myCells[iCurrCol]->getUniqueCellId())
				{
					// Render the cell drag-and-drop selection
					SRect2D srTempCellRect;
					getScreenCellPos(iCurrCol, iCurrRow, srTempCellRect);
					pRow->myCells[iCurrCol]->renderDragDropHoverState(srTempCellRect, fOpacity, myDragAcceptType);
				}

#ifdef DEBUG_DRAW_CELL_BORDERS
				g_pDrawingCache->flush();
				SRect2D srTemp;
				getScreenCellPos(iCurrCol, iCurrRow, srTemp);
				SColor scolDB(0,0,1,1);
				RenderUtils::drawRectangle(srTemp.x, srTemp.y, srTemp.w, srTemp.h, scolDB, 1.0, true);

/*
				getDrawingCache()->flush();
SColor scolDB(1,1,1,1);
FLOAT_TYPE fTempW = pRow->myCells[iCurrCol]->getNumProp(PropertyObjSizeW);
//FLOAT_TYPE fTempH = pRow->myCells[iCurrCol]->getNumProp(PropertyObjSizeH);
//FLOAT_TYPE fTempH = pRow->myCells[iCurrCol]->getCellHeight();
ITableCellInfoProvider* pProvider = getCellInfoProvider();
int iCurrCellHeight = -1;
if(pProvider)
	iCurrCellHeight = pProvider->getCellHeight(iCurrCol, iCurrRow, pRow->myCells[iCurrCol]);
if(iCurrCellHeight < 0)
	iCurrCellHeight = pRow->myCells[iCurrCol]->getCellHeight();
FLOAT_TYPE fTempH = iCurrCellHeight;

				glDrawRectangle(svElemPos.x - fTempW/2.0, svElemPos.y - fTempH/2.0, fTempW, fTempH, scolDB, 1.0, true);
*/
#endif
				iCurrRowWidth += (pRow->myCells[iCurrCol]->getNumProp(PropertyWidth) + iSpacing)*fLocScale*fScale;
			}
		}
		svElemPos.y += (iCurrRowHeight/2.0 + iSpacing)*fScale*fLocScale;

		if(svElemPos.y - iCurrRowHeight/2.0*fScale*fLocScale > svGlobalTopLeft.y + fSelfH)
			break;
	}

	if(bDidBeginStencil)
		RenderUtils::endStencil(getDrawingCache());

	postRender(svScroll, fOpacity, fScale);
/*
#ifdef WIN32
	if(strcmp(this->getStringProp(PropertyId), "detailsTable")  == 0)
	{
		SRect2D srWindowRect;
		this->getElementRectangle(svScroll, fScale, srWindowRect);

		SColor scolRed(1,0,0,1);
		getDrawingCache()->addRectangle(srWindowRect.x, srWindowRect.y, srWindowRect.w, srWindowRect.h, scolRed, 2);

	}
#endif
	*/
}
/*****************************************************************************/
void UITableElement::clearRows(void)
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif

#ifdef _DEBUG
	if(IS_OF_TYPE("layersTable"))
	{
		int bp = 0;
	}
#endif

	// No! Go through every row and deallocate the pointers
	clearSavedStateInfo();
	myBaseStateInfos.clear();
	mySavedSelectedRowId = -1;

	bool bDoRowSelection = !this->getBoolProp(PropertyPerCellSelection);
	bool bIsMultiSelect = this->getBoolProp(PropertyAllowMultipleSelection);

	bool bAskCellsForSelection = this->getAskCellsForSelection();

	UIPlane* pUIPlane = getUIPlane();

	int iCurrRow, iNumRows = myRows.size();
	int iCurrCol, iNumCols;
	for(iCurrRow = 0; iCurrRow < iNumRows; iCurrRow++)
	{
		iNumCols = myRows[iCurrRow].myCells.size();
		for(iCurrCol = 0; iCurrCol < iNumCols; iCurrCol++)
		{
			// Save state first
			myRows[iCurrRow].myCells[iCurrCol]->saveStateInto(this);

			// Remove from the map
			pUIPlane->removeFromNamesMap(myRows[iCurrRow].myCells[iCurrCol]);

			// See if we ask for selection. I think this is deprecated,
			// since all selection info should really be stored in
			// the backend, not UI elements.
			if(!bAskCellsForSelection)
			{
				SBaseCellStateInfo rBaseInfo;
				rBaseInfo.myId = myRows[iCurrRow].myCells[iCurrCol]->getUniqueCellId();
				if(rBaseInfo.myId >= 0)
				{
					if(bDoRowSelection)
						rBaseInfo.myIsSelected = myRows[iCurrRow].myIsSelected;
					else
					{
						_ASSERT(!bIsMultiSelect);
						rBaseInfo.myIsSelected = (mySelectedCell == myRows[iCurrRow].myCells[iCurrCol]);
					}
					myBaseStateInfos[rBaseInfo.myId] = rBaseInfo;

					if(iCurrRow == mySelectedRow && iCurrCol == 0)
						mySavedSelectedRowId = rBaseInfo.myId;
				}
			}

			getParentWindow()->getDragDropManager()->cancelDragFor(myRows[iCurrRow].myCells[iCurrCol]);
			delete myRows[iCurrRow].myCells[iCurrCol];
		}
		myRows[iCurrRow].myCells.clear();
	}
	clearBasic();

	// Also special case
	myIsDirty = TableRefreshRecreate;
}
/*****************************************************************************/
void UITableElement::clearBasic()
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif

	myRows.clear();

	myPressedCell = NULL;
	mySelectedCell = NULL;
	mySelectedRow = myPressedRow = -1;

	myLastMouseElement = NULL;
	myInitPressedCell = NULL;
	myOnReleaseFinalChild = NULL;
}
/*****************************************************************************/
const char* UITableElement::getCellType(int iColumn, int iRow)
{
	const char* pcsCellType = NULL;
	ITableCellInfoProvider* pInfoProvider = getCellInfoProvider();
	if(pInfoProvider)
		pcsCellType = pInfoProvider->getCellType(iColumn, iRow, this);

	if(IS_VALID_STRING_AND_NOT_NONE(pcsCellType))
		return pcsCellType;
	else
		return this->getEnumPropValue(PropertyCellTypes, iColumn);
}
/*****************************************************************************/
UITableCellElement* UITableElement::createCell(int iColumn, int iRow)
{
	const char *pcsType = getCellType(iColumn, iRow);
	UIPlane* pUIPlane = this->getUIPlane();

	if(myLastCellDefType != pcsType)
	{
		myLastCellDef = UIElement::getDefinitionBlindSearch(getUIPlane()->getInitCollectionType(), pcsType);
		myLastCellDefType = pcsType;
	}

	// We can be in any one of our collections, including nodes.
	// Note that if we crash below with a NULL pointer, it is because the element definition is not in one of
	// the below collections (such as ResourceNodeDefinitions). However, if we add one here, we must
	// rewrite some code in UIPlane and UIElement that also explicitly only has three collections now.
	// And make it generic, finally.
//	ResourceType eSupportedCollections[] = { ResourceUIElements, ResourceUIElements, ResourceSepWindowUIElements, ResourceLastPlaceholder };
	int iCurrColl;
	ResourceCollection* pUIElemsColl;
	UITableCellElement* pRes = NULL;
	//for(iCurrColl = 0; eSupportedCollections[iCurrColl] != ResourceLastPlaceholder && !pRes; iCurrColl++)
	{
		//pUIElemsColl = ResourceManager::getInstance()->getCollection(eSupportedCollections[iCurrColl]);
		pUIElemsColl = ResourceManager::getInstance()->getCollection(getUIPlane()->getInitCollectionType());
		pRes = pUIElemsColl->createItemOfType<UITableCellElement>(pcsType, pUIPlane, NULL, pUIPlane->getAllocator());
	}

	pRes->setCachedOwnDef(myLastCellDef, true);

	//myTempGeneratedNames.clear();
	pRes->setUniqueName(this->getUIPlane(), true); // , myTempGeneratedNames

	onCellCreated(iColumn, iRow, pRes);

	pRes->setParent(this);
	pRes->defineLayersOnChildren(getStringProp(PropertyLayer), 1);

	getUIPlane()->insertIntoNamesMap(pRes, false);

	pRes->show(true, NULL);

	return pRes;
}
/*****************************************************************************/
void UITableElement::onStartShowing(void)
{
	UIElement::onStartShowing();
	myScrollOffset = TABLE_VERT_PADDING;
	updateRelatedSlider();
}
/*****************************************************************************/
void UITableElement::updateOwnData(SUpdateInfo& rRefreshInfo)
{
	UIElement::updateOwnData(rRefreshInfo);

	// This is wrong.
	//int iNumColumns = this->getNumProp(PropertyUiObjTableNumColumns);

	if(rRefreshInfo.myMarkTableDirty)
		markDirty(TableRefreshRecreate);


	/*
	// Refresh its cells
	int iCurrCol;
	int iCurrRow, iNumRows = this->getNumRows();
	UITableCellElement* pCurrCell;
	for(iCurrRow = 0; iCurrRow < iNumRows; iCurrRow++)
	{
		for(iCurrCol = 0; iCurrCol < iNumColumns; iCurrCol++)
		{
			pCurrCell = getCell(iCurrCol, iCurrRow);
			if(pCurrCell)
				pCurrCell->refreshData(rRefreshInfo);
		}
	}
	 */
}
/*****************************************************************************/
void UITableElement::createRows()
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif

	int iNumOverrideColumns, iNumColumns;
	int iNumFixedColumns = this->getNumColumns();

	bool bDoRowSelection = !this->getBoolProp(PropertyPerCellSelection);
	bool bIsMultiSelect = this->getBoolProp(PropertyAllowMultipleSelection);

	bool bAskCellsForSelection = this->getAskCellsForSelection();

	onPreCreateRows();

	// const char* pcsSelfLayer = this->getStringProp(PropertyUiObjLayerName);
	ResourceItem* pTopParent = this;
	while(pTopParent->getParent())
		pTopParent = pTopParent->getParent();
	const char* pcsSelfLayer = pTopParent->getStringProp(PropertyLayer);

	// Get the number of rows we have
	int iCurrCol;
	int iCurrRow, iNumRows = this->getNumRows();
	UITableCellElement* pCurrCell;
	TBaseCellStateInfos::iterator mi;
	int iCellIndexCount = 0;
	for(iCurrRow = 0; iCurrRow < iNumRows; iCurrRow++)
	{
		iNumOverrideColumns = this->getNumColumnsForRow(iCurrRow);
		if(iNumOverrideColumns > 0)
			iNumColumns = iNumOverrideColumns;
		else
			iNumColumns = iNumFixedColumns;

		SUITableRow rDummyRow;
		for(iCurrCol = 0; iCurrCol < iNumColumns; iCurrCol++)
		{
			pCurrCell = createCell(iCurrCol, iCurrRow);

			pCurrCell->setRowCol(iCurrCol, iCurrRow, iCellIndexCount);
			pCurrCell->updateDataRecursive();
			pCurrCell->setStringProp(PropertyLayer, pcsSelfLayer);

			// See if there's saved basic info
			if(bAskCellsForSelection)
			{
				bool bIsCellSelected = pCurrCell->getIsSelectedFromContents();
				if(bDoRowSelection)
				{
					if(iCurrCol == 0)
						rDummyRow.myIsSelected = bIsCellSelected;
				}
				else
				{
					_ASSERT(!bIsMultiSelect);
					if(bIsCellSelected)
						mySelectedCell = pCurrCell;
				}

				if(iCurrCol == 0 && mySavedSelectedRowId >= 0 && mySavedSelectedRowId == pCurrCell->getUniqueCellId())
					mySelectedRow = iCurrRow;
			}
			else
			{
				mi = myBaseStateInfos.find(pCurrCell->getUniqueCellId());
				if(mi != myBaseStateInfos.end())
				{
					// Restore info
					if(bDoRowSelection)
					{
						if(iCurrCol == 0)
							rDummyRow.myIsSelected = mi->second.myIsSelected;
					}
					else
					{
						_ASSERT(!bIsMultiSelect);
						if(mi->second.myIsSelected)
							mySelectedCell = pCurrCell;
					}

					if(iCurrCol == 0 && mySavedSelectedRowId >= 0 && mySavedSelectedRowId == pCurrCell->getUniqueCellId())
						mySelectedRow = iCurrRow;

				}
			}
			rDummyRow.myCells.push_back(pCurrCell);
			iCellIndexCount++;
		}

		myRows.push_back(rDummyRow);
		if(rDummyRow.myIsSelected)
			setIsRowSelected(iCurrRow, true, false);

		// If we crash here during debugging, it's probably because we hit an assert before this,
		// and then the previous drawing call didn't finish creating rows when the next one comes
		// in, still sees that the table is dirty, and starts reconstructing it again in what, in
		// the debugger, looks like a separate thread. Ignore this crash and comment out any 
		// asserts triggered before this to see if there's a real problem with the code.

		// Refresh all cells again if selection changed
		for(iCurrCol = 0; iCurrCol < iNumColumns; iCurrCol++)
		{
			pCurrCell = myRows[iCurrRow].myCells[iCurrCol];
			pCurrCell->updateDataRecursive();
		}
	}

	updateCellRelatedTableInfo(true, true);

	myIsDirty = TableRefreshClean;

	onPostCreateRows();
}
/*****************************************************************************/
void UITableElement::updateCellRelatedTableInfo(bool bCallAdjustToScreen, bool bCallSelectedCell)
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif

	// Compute total width of the row
	int iSpacing = getCellSpacing();
	int iNumRows = myRows.size();
	myTotalRowWidth = 0;

	if(mySelectedCell && bCallSelectedCell)
	{
		mySelectedCell->setIsSelected(true, false);
		mySelectedCell->updateDataRecursive();
	}

	// Now, we need to adjust our size:
	if(bCallAdjustToScreen)
		this->adjustToScreen();

	recomputeStoredTableDims();

	updateRelatedSlider();

	clearSavedStateInfo();
	myBaseStateInfos.clear();
}
/*****************************************************************************/
UIElement* UITableElement::getFinalCellElementAt(FLOAT_TYPE fX, FLOAT_TYPE fY, bool bGlidingOnly)
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif

	UITableCellElement* pCell = this->getCell(fX, fY);
	if(pCell)
	{
		SVector2D svScroll;
		SVector2D svPoint(fX, fY);
		return pCell->getChildAtRecursiveInternal(svPoint, svScroll, false, false, bGlidingOnly);
	}
	else
		return this;
}
/*****************************************************************************/
UIElement* UITableElement::getChildAtRecursiveInternal( const SVector2D& svPoint, const SVector2D& svScroll, bool bFirstNonPassthrough, bool bIgnoreChildren, bool bGlidingOnly )
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif

	// Table will have no children listed,  but check anyway:
	UIElement* pTempFound = UIElement::getChildAtRecursiveInternal(svPoint, svScroll, bFirstNonPassthrough, bIgnoreChildren, bGlidingOnly);

	// NOTE: The below would have gotten a particular cell or its child element
	// as a result. However, if this is enabled here, we no longer can scroll,
	// since instead of the table being returned, we get a cell or its child
	// returned, which know nothing about scrolling, and the table never gets
	// its mouse events.
	// Note, however, that this is ok if we're just gliding over the table...
	if(bGlidingOnly)
	{
		if(pTempFound != this)
			return pTempFound;

		// Otherwise, see if we actually have a child:
		UITableCellElement* pCell = this->getCell(svPoint.x, svPoint.y);
		if(pCell)
		{
			return pCell->getChildAtRecursiveInternal(svPoint, svScroll, bFirstNonPassthrough, bIgnoreChildren, bGlidingOnly);
		}
		else
			return this;
	}
	else
		return pTempFound;
}
/*****************************************************************************/
UIElement* UITableElement::getChildAtRecursive(FLOAT_TYPE fX, FLOAT_TYPE fY, bool bFirstNonPassthrough, bool bGlidingOnly)
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif

	UIElement* pCell = this->getCell(fX, fY);
	if(pCell)
	{
		return pCell->getChildAtRecursive(fX, fY, bFirstNonPassthrough, bGlidingOnly);
//		SVector2D svScreenCoords(fX, fY), svScroll;
//		return pCell->getChildAtRecursiveInternal(svScreenCoords, svScroll);
	}
	else
		return NULL;
}
/*****************************************************************************/
void UITableElement::getScreenCellPos(int iCol, int iRow, SRect2D& srRectOut)
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif

	if(myRows.size() <= 0)
		return;

	if(iRow >= myRows.size())
		return;
	/*
	// We must get our total offset from parent
	ResourceItem* pTopParent = this;
	UIElement* pUIElem;
	SVector2D svPos, svAccumPos;
	FLOAT_TYPE fFinalOpac;
	FLOAT_TYPE fLocScale, fScale = 1.0;
	UITableElement* pTable;
	while(pTopParent)
	{
		pUIElem = dynamic_cast<UIElement*>(pTopParent);
		pUIElem->getLocalPosition(svPos, &fFinalOpac, &fLocScale);
		if(pUIElem->getParent())
			svAccumPos += svPos*fLocScale;
		else
			svAccumPos += svPos;

		pTable = as<UITableElement>(pUIElem);
		if(pTable)
			svAccumPos.y -= pTable->getScrollOffset()*fLocScale;

		fScale *= fLocScale;
		pTopParent = pTopParent->getParent();
	}

	FLOAT_TYPE fInitTopY = svAccumPos.y - this->getNumProp(PropertyObjSizeH)/2.0*fScale + TABLE_VERT_PADDING; // + svPos.y;
	FLOAT_TYPE fInitLeftX = svAccumPos.x - myTotalRowWidth/2.0*fScale; // + svPos.x;
	///fInitTopY -= myScrollOffset;
	*/

	// This is shifted for some raisin....
	FLOAT_TYPE fScale = 1.0;
	SVector2D svAccumPos;
	this->getGlobalPosition(svAccumPos, NULL, &fScale);

	SVector2D svInitPos;
	getInitCellPos(svAccumPos, SVector2D::Zero, fScale, this->getNumProp(PropertyWidth), this->getNumProp(PropertyHeight), svInitPos);
// 	svInitPos.x = svAccumPos.x - myTotalRowWidth/2.0*fScale; // + svPos.x;
// 	svInitPos.y = svAccumPos.y - this->getNumProp(PropertyObjSizeH)/2.0*fScale + TABLE_VERT_PADDING; // + svPos.y;
// 	svInitPos.y -= myScrollOffset;

	int iSpacing = getCellSpacing();

//	int iCurrRowHeight = myRows[iRow].myHeight;

	SVector2D svElemPos;
//	svElemPos.y = fInitTopY + iCurrRowHeight/2.0*fScale;
	//svElemPos.y += (iCurrRowHeight + iSpacing)*iRow;
	int iTempRow;
	svElemPos.y = svInitPos.y;
	for(iTempRow = 0; iTempRow < iRow; iTempRow++)
		svElemPos.y += (myRows[iTempRow].myHeight + iSpacing)*fScale;
	svElemPos.y += (myRows[iRow].myHeight/2.0 + iSpacing)*fScale;

	int iCurrRowHeight = myRows[iRow].myHeight;

	SUITableRow* pRow;
	int iCurrRowWidth = 0;
	pRow = &myRows[iRow];
	int iCurrCol, iNumCols = pRow->myCells.size();
	for(iCurrCol = 0; iCurrCol < iNumCols; iCurrCol++)
	{
		svElemPos.x = svInitPos.x + iCurrRowWidth + pRow->myCells[iCurrCol]->getNumProp(PropertyWidth)/2.0*fScale;
		if(iCurrCol == iCol)
		{
			srRectOut.w = pRow->myCells[iCol]->getNumProp(PropertyWidth);
			srRectOut.h = iCurrRowHeight*fScale;
			srRectOut.x = svElemPos.x - srRectOut.w/2.0;
			srRectOut.y = svElemPos.y - iCurrRowHeight/2.0;
			break;
		}
		iCurrRowWidth += (pRow->myCells[iCurrCol]->getNumProp(PropertyWidth) + iSpacing)*fScale;
	}


}
/*****************************************************************************/
UITableCellElement* UITableElement::getCell(FLOAT_TYPE fX, FLOAT_TYPE fY)
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif

	SRect2D srCellRect;
	SVector2D svPos(fX, fY);
	int iRow, iNumRows = myRows.size();
	int iCol, iNumCols;
	SUITableRow* pRow;
	for(iRow = 0; iRow < iNumRows; iRow++)
	{
		pRow = &myRows[iRow];
		iNumCols = pRow->myCells.size();
		for(iCol = 0; iCol < iNumCols; iCol++)
		{
			this->getScreenCellPos(iCol, iRow, srCellRect);
			if(srCellRect.doesContain(svPos))
			{
				return pRow->myCells[iCol];
			}
		}
	}
	return NULL;
}
/*****************************************************************************/
void UITableElement::onPressed(TTouchVector& vecTouches)
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif

	FLOAT_TYPE fX = vecTouches[0].myPoint.x;
	FLOAT_TYPE fY = vecTouches[0].myPoint.y;

	UIElement::onPressed(vecTouches);
	myInitScroll = myScrollOffset;
	myBeginPressPos.set(fX, fY);	
	myPressTime = Application::getInstance()->getGlobalTime(ANIM_SCROLL_CLOCK_TYPE);
	myLatestMouseMove = myBeginPressPos;
	myPressedCell = this->getCell(fX, fY);
	if(myPressedCell)
		myPressedRow = myPressedCell->getRow();
	else
		myPressedRow = -1;
	myHasScrolled = false;
	myScrollSpeed = 0;
	myAutoscrollDir = 0;

	myAllowScrolling = true;
	myMouseTrail.clear();

	// Figure our when to pass to release to child elements, so that
	// buttons within cells could be pressed.
	UITableCellElement* pCell = getCell(fX, fY);
	myLastMouseElement = NULL;
	myInitPressedCell = pCell;
	if(pCell)
	{
		// Subtract from the world space (screen) position
		SVector2D svRelPos(fX, fY);

		// Now, get the final child. He-he. Final.
		UIElement* pFinalChild = pCell->getChildAtRecursive(svRelPos.x, svRelPos.y, false, false);
		if(pFinalChild)
		{
			// The vector we pass in has wrong coords (screen), but we currently
			// don't look at it anyway. I bet, though, someday I'll come back to
			// this with a big d'oh! on my lips.
			pFinalChild->onPressed(vecTouches);
			myLastMouseElement = pFinalChild;
		}

		// See if we even allow scrolling. That is, if the mose is over the
		// target element (which is not necessarily the final child), we don't allow
		// scrolling.
		UIElement* pTriggerChild = this->getCellTargetDragTriggerChild(pCell);
		if(pTriggerChild)
		{
			SRect2D srCellRect;
			pTriggerChild->getGlobalRectangle(srCellRect);
			if(srCellRect.doesContain(svRelPos))
			{
				myAllowScrolling = false;
			}
		}
	}
}
/*****************************************************************************/
void UITableElement::onReleased(TTouchVector& vecTouches, bool bIgnoreActions)
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif

	FLOAT_TYPE fX = vecTouches[0].myPoint.x;
	FLOAT_TYPE fY = vecTouches[0].myPoint.y;

	myIsCallingFromReleased = true;
	onMouseMove(vecTouches);
	myIsCallingFromReleased = false;

	bool bIsRMB = getParentWindow()->getCurrMouseButtonDown() == MouseButtonRight;
	bool bIsDragging = getParentWindow()->getDragDropManager()->isDragging();

	UNIQUEID_TYPE idSelectionUndoBlock = -1;

	myOnReleaseFinalChild = NULL;
	if(!bIsDragging)
	{
		UITableCellElement* pCell = getCell(fX, fY);

		if(pCell)
			myOnReleaseFinalChild = pCell->getChildAtRecursive(fX, fY, false, false);

		if(!myHasScrolled && !dynamic_cast<UIButtonElement*>(myOnReleaseFinalChild))
		{
			UITableCellElement* pReleasedCell = this->getCell(fX, fY);
			bool bAllowSelection = this->getBoolProp(PropertyAllowSelection);
			bool bDoRowSelection = !this->getBoolProp(PropertyPerCellSelection);
			bool bAllowMultiSelect = this->getBoolProp(PropertyAllowMultipleSelection);

			bool bIsCtrlKeyDown = KeyManager::getInstance()->getIsModifierKeyDown(SilentKeyControl);
			bool bIsShiftKeyDown = KeyManager::getInstance()->getIsModifierKeyDown(SilentKeyShift);

			// Note that we added not changing selection here when we were doing the brush stroke
			// table, since RMBing opens the menu to rename it. However, it also disables selecting
			// and RMBed layer in the layers table, which is bad.
			bool bAllowChangingSelection = !bIgnoreActions; // && !bIsRMB;
			if(myOnReleaseFinalChild && !myOnReleaseFinalChild->getAllowChangingTableSelection(fX, fY))
				bAllowChangingSelection = false;

			int iRowToUseForShiftSelectionStart = mySelectedRow;
			if(iRowToUseForShiftSelectionStart < 0)
			{
				// We haven't clicked on anything yet. See if we can find a selected row.
				int iInnerRow;
				for(iInnerRow = 0; iInnerRow < myRows.size(); iInnerRow++)
				{
					if(myRows[iInnerRow].myIsSelected)
					{
						iRowToUseForShiftSelectionStart = iInnerRow;
						break;
					}
				}
			}

			// WTF is this doing here???? We should not have code like this here!
			if(bAllowChangingSelection && UndoManager::canAcceptNewUndoNow())
				idSelectionUndoBlock = UndoManager::addContainerBlockItem("Layer Selection", getParentWindow(), NULL, PROPERTY_NONE);

			if(bDoRowSelection && bAllowChangingSelection)
			{
				// Row-based selection
				int iSelectedRow = -1;
				if(pReleasedCell)
					iSelectedRow = pReleasedCell->getRow();

				if(bIsRMB)
				{
					// We are RMBing.
					// This means we want to only change selection if
					// the current row clicked is not selected
					if(iSelectedRow >= 0 && this->canSelectRow(iSelectedRow))
					{
						if(!myRows[iSelectedRow].myIsSelected)
						{
							if(!bIsCtrlKeyDown)
								deselectAllRows(true);
							setIsRowSelected(iSelectedRow, true, true);
							mySelectedRow = iSelectedRow;
						}
					}
				}
				else
				{
					if(iSelectedRow >= 0 && this->canSelectRow(iSelectedRow))
					{
						if(!bIsCtrlKeyDown || !bAllowMultiSelect)
						{
							// Clear previous selection. Note that if we have multiselect,
							// we must mark all rows as deselected
							if(bAllowMultiSelect)
							{
								// We only deselect if we're not clicking on an already selected row

								// But what if the shift/ctrl isn't up, and we're clicking on the sel row?
								// Deselect anyway
								//if(iSelectedRow < 0 || (iSelectedRow >= 0 && !myRows[iSelectedRow].myIsSelected))
								if(iSelectedRow < 0 || (iSelectedRow >= 0))
									deselectAllRows(true);
							}
							else
							{
								// Deselect the current row
								if(mySelectedRow >= 0)
									setIsRowSelected(mySelectedRow, false, true);
							}
						}

						if(bAllowSelection && iSelectedRow == myPressedRow && iSelectedRow >= 0)
						{
							if(bIsShiftKeyDown && iRowToUseForShiftSelectionStart >= 0 && bAllowMultiSelect)
							{
								// Select all rows from the last selected row
								// to the clicked row
								int iStartRow = min(iRowToUseForShiftSelectionStart, iSelectedRow);
								int iEndRow = max(iRowToUseForShiftSelectionStart, iSelectedRow);
								int iInnerRow;
								for(iInnerRow = iStartRow; iInnerRow <= iEndRow; iInnerRow++)
									setIsRowSelected(iInnerRow, true, true);
							}
							else
							{
								bool bNewIsSelected = true;
								if(bIsCtrlKeyDown)
									bNewIsSelected = !myRows[iSelectedRow].myIsSelected;
								setIsRowSelected(iSelectedRow, bNewIsSelected, true);
							}
						}
					}
					else if(iSelectedRow < 0)
						deselectAllRows(true);

					mySelectedRow = iSelectedRow;
				}


			}
			else if(bAllowChangingSelection)
			{

				// Not supported yet
				_ASSERT(!bAllowMultiSelect);

				// Cell-based selection
				if(mySelectedCell)
				{
					mySelectedCell->setIsSelected(false, true);
					mySelectedCell->updateDataRecursive();
				}

				if(bAllowSelection && pReleasedCell == myPressedCell && myPressedCell)
				{
					// Set its selection
					myPressedCell->setIsSelected(true, true);
					mySelectedCell = myPressedCell;
					mySelectedCell->updateDataRecursive();
				}
			}
		}
		else if(myHasScrolled)
			animateScrollIfNeeded(fX, fY);

		// Figure out when to pass to release to child elements, so that
		// buttons within cells could be pressed.
		if(pCell)
		{
			myOnReleaseFinalChild = pCell->getChildAtRecursive(fX, fY, false, false);
			bool bDidPressCellItself = (myOnReleaseFinalChild == pCell);
			UIElement* pCalledReleaseOn = NULL;
			if(myOnReleaseFinalChild)
			{
				// The vector we pass in has wrong coords (screen), but we currently
				// don't look at it anyway. I bet, though, someday I'll come back to
				// this with a big d'oh! on my lips.
				myOnReleaseFinalChild->onReleased(vecTouches, bIgnoreActions);
				pCalledReleaseOn = myOnReleaseFinalChild;
			}

			if(myOnReleaseFinalChild != myLastMouseElement && myLastMouseElement
				&& !myLastMouseElement->canReleaseFocus())
				myOnReleaseFinalChild = myLastMouseElement;
			if(myOnReleaseFinalChild)
			{
				// This is here to avoid calling onReleased() twice on
				// an element if it hasn't changed.
				if(myOnReleaseFinalChild != pCalledReleaseOn)
					myOnReleaseFinalChild->onReleased(vecTouches, bIgnoreActions);
				myOnReleaseFinalChild->onMouseLeave(vecTouches);
			}
			else
			{
				// Kill focus.
				getParentWindow()->setFocusElement(NULL);
			}

			// Now, do RMB action if any.
			if(bIsRMB && bDidPressCellItself && pCell->doesPropertyExist(PropertyRmbAction))
				getUIPlane()->onButtonClicked(pCell, getParentWindow()->getCurrMouseButtonDown());
		} // end if have a cell
	} // end if not dragging

	myLastMouseElement = NULL;
 	myInitPressedCell = NULL;
	myAutoscrollDir = 0;

	UIElement::onReleased(vecTouches, bIgnoreActions);

	if(idSelectionUndoBlock >= 0)
		UndoManager::endUndoBlockInCurrentManager(idSelectionUndoBlock, getParentWindow(), NULL);

	myHasScrolled = false;
}
/*****************************************************************************/
void UITableElement::onMouseMove(TTouchVector& vecTouches)
{
	UIElement::onMouseMove(vecTouches);

#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif

	FLOAT_TYPE fX = vecTouches[0].myPoint.x;
	FLOAT_TYPE fY = vecTouches[0].myPoint.y;

	if(getIsMousePressed())
	{
		GTIME lTime = Application::getInstance()->getGlobalTime(ANIM_SCROLL_CLOCK_TYPE);
		myMouseTrail.addPoint(fX, fY, lTime, true);

		myLatestMouseMove.set(fX, fY);

		if( (lTime - myPressTime) % (GTIME)SCROLLER_LOOKBACK_TICKS == 0)
		{
			myPressTime = Application::getInstance()->getGlobalTime(ANIM_SCROLL_CLOCK_TYPE);
			myLastLookbackPos = myLatestMouseMove;
		}
	}

	if(getIsMousePressed() && myAllowScrolling && getAllowScrollFromPoint(myBeginPressPos, fX, fY))
	{
		myHasScrolled = true;
		setScroll(computeScrollFromPoint(myBeginPressPos, fX, fY)); // , false);
	}

	// Now, if we're dragging something and we're close to the top or bottom of the table,
	// scroll.
	myAutoscrollDir = 0;
	if(getParentWindow()->getDragDropManager()->isDragging() && vecTouches.size() > 0)
	{
		SVector2D svPoint = vecTouches[0].myPoint;
		SRect2D srOwnRect;

		this->getGlobalRectangle(srOwnRect);

#ifdef _DEBUG
		//string strTemp;
		//getNiceNumber(Application::ticksToSeconds(Application::getInstance()->getGlobalTime(ClockUiPrimary)), 2, false, strTemp);
		//strTemp = "MouseMove " + strTemp + "\n";
		//OutputDebugString(strTemp.c_str());

		char pcsBuff[1024];
		sprintf(pcsBuff,  "Point: (%.0f, %.0f)  y_bounds: (%.0f, %.0f)\n", svPoint.x, svPoint.y, srOwnRect.y, srOwnRect.y + srOwnRect.h);
		gLog(pcsBuff);
#endif

		if(svPoint.y >= srOwnRect.y && svPoint.y <= srOwnRect.y + DRAG_VERT_AUTOSCROLL_HEIGHT)
			myAutoscrollDir = (svPoint.y - (srOwnRect.y + DRAG_VERT_AUTOSCROLL_HEIGHT))/DRAG_VERT_AUTOSCROLL_HEIGHT;
		else if(svPoint.y >= srOwnRect.y + srOwnRect.h  - DRAG_VERT_AUTOSCROLL_HEIGHT && svPoint.y <= srOwnRect.y  + srOwnRect.h)
			myAutoscrollDir = (svPoint.y - (srOwnRect.y + srOwnRect.h - DRAG_VERT_AUTOSCROLL_HEIGHT))/DRAG_VERT_AUTOSCROLL_HEIGHT;
	}


	UITableCellElement* pCell = getCell(fX, fY);
	if(pCell && !myIsCallingFromReleased)
	{
#ifndef INITIATE_DRAG_AND_DROP_ON_MOUSE_LEAVE
		if(!myHasScrolled && myInitPressedCell && getIsMousePressed() && vecTouches.size() > 0
			&& getParentWindow()->getDragDropManager()->isDragging() == false)
		{
			SVector2D svDist, svPressedPos;
			getPressedPos(svPressedPos);
			svDist = svPressedPos - vecTouches[0].myPoint;
			if(svDist.lengthSquared() > DRAG_DROP_MIN_DIST*DRAG_DROP_MIN_DIST)
			{
				UIElement* pTriggerChild = this->getCellTargetDragTriggerChild(myInitPressedCell);
				if(pTriggerChild)
				{
					// See what we actually pressed...
					UIElement* pInnerCellElement = myInitPressedCell->getChildAtRecursive(myBeginPressPos.x, myBeginPressPos.y, false, false);

					// Now, go march up to us to try and find the first elem that allows drag and drop
					// to start...
					UIElement* pTempElem = pInnerCellElement;
					while(pTempElem && pTempElem != myInitPressedCell && !pTempElem->allowDragDropStart(vecTouches[0].myPoint) && pTempElem != pTriggerChild)
						pTempElem = pTempElem->getParent<UIElement>();

					UIElement* pElemToDrag = NULL;
					if(pTempElem && pTempElem != myInitPressedCell)
					{
						if(pTempElem == pTriggerChild)
							pElemToDrag = myInitPressedCell;
						else
							pElemToDrag = pTempElem;
					}

					if(pElemToDrag && pElemToDrag->allowDragDropStart(vecTouches[0].myPoint))
					{
						// Tell the D&D manager we're being dragged.
						getParentWindow()->getDragDropManager()->beginDrag(pElemToDrag);
					}
				}
			}
		}
#endif

		if(!myHasScrolled)
		{


		UIElement* pElement = pCell->getChildAtRecursive(fX, fY, false, false);
		if(pElement != myLastMouseElement && (!myLastMouseElement || myLastMouseElement->canReleaseFocus()))
		{
			if(myLastMouseElement)
				myLastMouseElement->onMouseLeave(vecTouches);
			if(pElement)
			{
				pElement->onMouseEnter(vecTouches);
			}
			myLastMouseElement = pElement;
		}

		if(myLastMouseElement)
			myLastMouseElement->onMouseMove(vecTouches);
		}
	}
}
/*****************************************************************************/
void UITableElement::onMouseLeave(TTouchVector& vecTouches)
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif

	FLOAT_TYPE fX = vecTouches[0].myPoint.x;
	FLOAT_TYPE fY = vecTouches[0].myPoint.y;

	if(myInitPressedCell && !getParentWindow()->getDragDropManager()->isDragging() &&
		myInitPressedCell->allowDragDropStart(vecTouches[0].myPoint))
	{
		getParentWindow()->getDragDropManager()->beginDrag(myInitPressedCell);
	}

	if(myLastMouseElement)
		myLastMouseElement->onMouseLeave(vecTouches);

	onMouseMove(vecTouches);

	if(myHasScrolled && getIsMousePressed())
		animateScrollIfNeeded(fX, fY);

	myDragAcceptType = AcceptNone;
	myDragHoverTargetCellId = -1;

	UIElement::onMouseLeave(vecTouches);
	myInitPressedCell = NULL;
	myHasScrolled = false;
	myAutoscrollDir = 0;
}
/*****************************************************************************/
int UITableElement::getMaxScroll(void)
{
	SVector2D svBoxSize;
	this->getBoxSize(svBoxSize);
	int iSpacing = getCellSpacing();

	int iMaxScroll = getMaxScrollInScrollAxisDirection(svBoxSize, iSpacing);

	if(iMaxScroll < TABLE_VERT_PADDING*2)
		iMaxScroll = TABLE_VERT_PADDING*2;

	return iMaxScroll;
}
/*****************************************************************************/
void UITableElement::setScroll( FLOAT_TYPE fScroll, bool bUseActualContentLimits)
{
	if(!getAllowScrolling())
		return;

	if(myScrollOffset == fScroll)
		return;

	myScrollOffset = fScroll;

	// See if we're not exceeding max scroll
	int iMaxScroll = getMaxScroll();

	if(bUseActualContentLimits)
	{
		if(myScrollOffset < TABLE_VERT_PADDING)
			myScrollOffset = TABLE_VERT_PADDING;

		if(myScrollOffset > iMaxScroll - TABLE_VERT_PADDING)
			myScrollOffset = iMaxScroll - TABLE_VERT_PADDING;
	}
	else
	{
		if(myScrollOffset < 0)
			myScrollOffset = 0;

		if(myScrollOffset > iMaxScroll)
			myScrollOffset = iMaxScroll;
	}

	if(!myIgnoreSlider)
		updateRelatedSlider();
}
/*****************************************************************************/
void UITableElement::onTimerTick(GTIME lGlobalTime)
{
	// This is necessary for multiple reasons, but also because if we're closing
	// a tab that was previously moved to a different pane, we will call 
	// adjustToScreen(), which will call recompute stored table dims, which will
	// use stale info, but midway (say, at render) that info will be cleared,
	// and we'll be left with a crash.
	// AHTUNG! This causes random crashes when rendering thumbnails on startup
	// that we can't repro!
	//ensureRowsUpToDate();

	UIElement::onTimerTick(lGlobalTime);

	myScrollSpeed *= (1.0 - SCROLL_SPEED_DECAY);

	FLOAT_TYPE fAdditionalScroll = 0.0;
	if(!getIsMousePressed())
	{
		if(myScrollOffset < TABLE_VERT_PADDING)
		{
			// Apply a force
			fAdditionalScroll = ((FLOAT_TYPE)(TABLE_VERT_PADDING - myScrollOffset))/PADDING_SCROLL_RETURN_RATE;
		}
		else if(myScrollOffset > getMaxScroll() - TABLE_VERT_PADDING)
		{
			fAdditionalScroll = ((FLOAT_TYPE)(getMaxScroll()  - TABLE_VERT_PADDING - myScrollOffset))/PADDING_SCROLL_RETURN_RATE;
		}

		bool bIgnoreSliderLocal = false;
		if(getRelatedTableSlider())
		{
			// See if we're animating the slider
			if(getRelatedTableSlider()->getIsSliderBeingAnimated())
				bIgnoreSliderLocal = true;
		}

		if(bIgnoreSliderLocal)
			myIgnoreSlider = true;
		setScroll(myScrollOffset + myScrollSpeed + fAdditionalScroll); // , true);
		if(bIgnoreSliderLocal)
			myIgnoreSlider = false;
	}
	else
	{
		myIgnoreSlider = true;
		setScroll(myScrollOffset + myScrollSpeed + fAdditionalScroll);
		myIgnoreSlider = false;
	}

	if(fabs(myAutoscrollDir) > FLOAT_EPSILON)
	{
		setScroll(myScrollOffset + myAutoscrollDir*DRAG_MAX_AUTOSCROLL_SPEED/GAME_FRAMERATE, true); // , true);
	}

#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif

	// Guess what? Call self on children...
	int iCol, iNumCols;
	int iRow, iNumRows = myRows.size();
	SUITableRow* pRow;
	for(iRow = 0; iRow < iNumRows; iRow++)
	{
		pRow = &myRows[iRow];
		iNumCols = pRow->myCells.size();
		for(iCol = 0; iCol < iNumCols; iCol++)
		{
			if(pRow->myCells[iCol])
				pRow->myCells[iCol]->onTimerTick(lGlobalTime);
		}
	}

}
/*****************************************************************************/
void UITableElement::animateScrollIfNeeded(FLOAT_TYPE fX, FLOAT_TYPE fY)
{
	GTIME lTime = Application::getInstance()->getGlobalTime(ANIM_SCROLL_CLOCK_TYPE);

	// Find where the mouse was a little while ago
	int iCurrPoint, iNumPoints = myMouseTrail.getNumPoints();
	for(iCurrPoint = iNumPoints - 1; iCurrPoint >= 0 ; iCurrPoint--)
	{
#ifdef _DEBUG
		GTIME rTestTime = myMouseTrail.getPointTime(iCurrPoint);
#endif
		if( myMouseTrail.getPointTime(iCurrPoint) <= lTime - SCROLLER_LOOKBACK_TICKS )
			break;
	}

	if(iCurrPoint == iNumPoints - 1)
		return;

	if(iCurrPoint < 0)
		return;
//		iCurrPoint = 0;

	SVector2D svPoint;
	myMouseTrail.getPoint(iCurrPoint, svPoint);
	myScrollSpeed = computeScrollSpeedFromTrail(svPoint, fX, fY, lTime, myMouseTrail.getPointTime(iCurrPoint));
	//myScrollSpeed = (svPoint.y - fY)/(lTime - myMouseTrail.getPointTime(iCurrPoint));

/*
	GTIME lDiff;
	if(lTime - myPressTime < SCROLLER_LOOKBACK_TICKS)
		lDiff = lTime - myPressTime;
	else
		lDiff = SCROLLER_LOOKBACK_TICKS;

	// Compute our speed
	myScrollSpeed = (myLastLookbackPos.y - fY)/lDiff;
*/
}
/*****************************************************************************/
void UITableElement::setIsRowSelected(int iRow, bool bSelected, bool bIsComingFromUI)
{
	if(iRow >= 0 && iRow < (int)myRows.size())
	{
		if(myRows[iRow].myIsSelected != bSelected)
		{
			myRows[iRow].myIsSelected = bSelected;
			onRowSelected(iRow, bSelected, bIsComingFromUI);
		}
	}
	else
	{
		_ASSERT(0);
	}
}
/*****************************************************************************/
int UITableElement::getSelectedRow(void)
{
	// Note that it makes no sense to ask for a selected row in a multi-select table.
	// Eva.
	_ASSERT(!this->getBoolProp(PropertyAllowMultipleSelection) || !getAreMultipleRowsSelected());
	//_ASSERT(mySelectedRow < 0 || myRows[mySelectedRow].myIsSelected);
	return mySelectedRow;
}
/*****************************************************************************/
UITableCellElement* UITableElement::getCell(int iCol, int iRow, bool bCreateIfNeeded)
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif

	if(bCreateIfNeeded) //  && myIsDirty)
		ensureRowsUpToDate();

	if(iRow < 0 || iRow >= (int)myRows.size())
		return NULL;

	SUITableRow* pRow = &myRows[iRow];
	if(iCol < 0 || iCol >= (int)pRow->myCells.size())
		return  NULL;
	else
		return pRow->myCells[iCol];
}
/*****************************************************************************/
bool UITableElement::getIsRowSelected(int iRow)
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif
	if(iRow >= 0 && iRow < (int)myRows.size())
		return myRows[iRow].myIsSelected;
	else
	{
		// No assert, since we may be just creating it. Returns false.
		//_ASSERT(0);
		return false;
	}
}
/*****************************************************************************/
void UITableElement::markDirty(TableRefreshType eDirtyType)
{
	// Maximum dirtiness already
	_ASSERT(eDirtyType != TableRefreshClean);
	if(myIsDirty == TableRefreshRecreate)
		return;

	myIsDirty = eDirtyType;
}
/*****************************************************************************/
int UITableElement::getCellSpacing(void)
{
	if(myCellSpacing < 0)
		myCellSpacing = this->getNumProp(PropertyCellSpacing);
	return myCellSpacing;
}
/*****************************************************************************/
void UITableElement::adjustCellsSize(int iOptWidth, int iOptHeight)
{
	SUITableRow* pRow;
	UITableCellElement* pCell;
	int iCol, iNumCols;
	int iRow, iNumRows = myRows.size();
	for(iRow = 0; iRow < iNumRows; iRow++)
	{
		pRow = &myRows[iRow];
		iNumCols = pRow->myCells.size();
		for(iCol = 0; iCol < iNumCols; iCol++)
		{
			if(pRow->myCells[iCol])
			{
				pCell = pRow->myCells[iCol];

				if(iOptWidth > 0 && iOptHeight > 0)
				{
					pCell->setNumProp(PropertyWidth, iOptWidth);
					pCell->setNumProp(PropertyHeight, iOptHeight);
					pCell->resetEvalCache(true);
				}

				pCell->adjustToScreen();
			}
		}
	}
	recomputeStoredTableDims();
}
/*****************************************************************************/
void UITableElement::adjustToScreen()
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif
	UIElement::adjustToScreen();
	this->adjustCellsSize();
}
/*****************************************************************************/
void UITableElement::recomputeStoredTableDims(void)
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif
	int iSpacing = getCellSpacing();
	SUITableRow* pRow;
	int iNumCols;
	int iNumRows = myRows.size();
	myTotalRowWidth = 0;

#ifdef _DEBUG
	if(IS_OF_TYPE("layersTable"))
	{
		int bp = 0;
	}
#endif

	int iCurrCol;

	if(iNumRows > 0)
	{
		ITableCellInfoProvider* pProvider = getCellInfoProvider();

		pRow = &myRows[0];
		iNumCols = pRow->myCells.size();
		for(iCurrCol = 0; iCurrCol < iNumCols; iCurrCol++)
			myTotalRowWidth += pRow->myCells[iCurrCol]->getNumProp(PropertyWidth) + iSpacing;

		int iRow;
		int iCurrCellHeight, iCellMaxHeight;
		for(iRow = 0; iRow < iNumRows; iRow++)
		{
			pRow = &myRows[iRow];
			iNumCols = pRow->myCells.size();
			iCellMaxHeight = 0;
			for(iCurrCol = 0; iCurrCol < iNumCols; iCurrCol++)
			{
				iCurrCellHeight = -1;
				if(pProvider)
					iCurrCellHeight = pProvider->getCellHeight(iCurrCol, iRow, pRow->myCells[iCurrCol]);
				if(iCurrCellHeight < 0)
					iCurrCellHeight = pRow->myCells[iCurrCol]->getCellHeight();
				if(iCurrCellHeight < 0)
					iCurrCellHeight = pRow->myCells[iCurrCol]->getNumProp(PropertyHeight);
				else
				{
					// We got the cell size from an override. Make sure we set it on a cell.
					pRow->myCells[iCurrCol]->setNumProp(PropertyHeight, iCurrCellHeight);
					pRow->myCells[iCurrCol]->resetEvalCache(false);
				}

				if(iCurrCellHeight > iCellMaxHeight)
					iCellMaxHeight = iCurrCellHeight;
			}

			pRow->myHeight = iCellMaxHeight;
		}

		// Spacing only applies between cells, not between cells and borders.
		myTotalRowWidth -= iSpacing;
	}

#ifdef _DEBUG
	if(IS_OF_TYPE("layersTable"))
	{
		int bp = 0;
	}
#endif
}
/*****************************************************************************/
void UITableElement::scrollOnSlider(UISliderElement* pSlider)
{
	FLOAT_TYPE fSliderValue = pSlider->getValue();
	// Assume this is [0, 1] value.
	int iMaxScroll = getMaxScroll() - TABLE_VERT_PADDING*2;

	FLOAT_TYPE fActualScroll = fSliderValue*(FLOAT_TYPE)iMaxScroll + TABLE_VERT_PADDING;
	setScroll(fActualScroll); // , true);
}
/*****************************************************************************/
void UITableElement::updateRelatedSlider(void)
{
	// Find one.
	//UISliderElement* pSlider = this->getChildByClass<UISliderElement>();
	UISliderElement* pSlider = getRelatedTableSlider();
	if(!pSlider)
		return;

	// Otherwise, keep the slider between 0 and 1, and adjust our scroll.
	int iMaxScroll = getMaxScroll() - TABLE_VERT_PADDING*2;

	FLOAT_TYPE fSliderVal;
	if(iMaxScroll <= 0)
	{
		pSlider->setIsEnabled(false);
		fSliderVal = 0;
	}
	else
	{
		pSlider->setIsEnabled(true);
		fSliderVal = (myScrollOffset - TABLE_VERT_PADDING)/(FLOAT_TYPE)iMaxScroll;
	}

	if(fSliderVal < 0)
		fSliderVal = 0;
	else if(fSliderVal > 1.0)
		fSliderVal = 1.0;

	pSlider->setValue(fSliderVal);
}
/*****************************************************************************/
UISliderElement* UITableElement::getRelatedTableSlider(void)
{
	// By default, try to find our child by type
	UISliderElement* pRes = this->getChildByClass<UISliderElement>();
	if(!pRes)
	{
		// Try to find a slider in the immediate parent which has this as its related
		// tables
		UIElement* pParent = this->getParent<UIElement>();
		if(pParent)
		{
			UISliderElement* pTemp = pParent->getChildByClass<UISliderElement>(false);
			if(pTemp && pTemp->getLinkedToElement<UITableElement>() == this)
				pRes = pTemp;
		}
	}

	return pRes;
}
/*****************************************************************************/
bool UITableElement::getDoTouchesLookLikeScrolling()
{
	return myHasScrolled;
}
/*****************************************************************************/
void UITableElement::setPushedForRadioGroupRecursive(const char* pcsGroup, UIButtonElement* pException, bool bInstant)
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif
	if(myRows.size() <= 0)
		return;

    int iCurrCol;
    int iCurrRow, iNumRows = myRows.size(); // this->getNumRows();
    UITableCellElement* pCurrCell;
    int iNumColumns;
    for(iCurrRow = 0; iCurrRow < iNumRows; iCurrRow++)
    {
		iNumColumns = myRows[iCurrRow].myCells.size();
		for(iCurrCol = 0; iCurrCol < iNumColumns; iCurrCol++)
		{
			pCurrCell = getCell(iCurrCol, iCurrRow);
			if(pCurrCell)
				pCurrCell->setPushedForRadioGroupRecursive(pcsGroup, pException, bInstant);
		}
    }

}
/*****************************************************************************/
void UITableElement::deselectAllRows(bool bIsComingFromUI)
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif
	int iRow, iNumRows = myRows.size();
	for(iRow = 0; iRow < iNumRows; iRow++)
	{
		myRows[iRow].myIsSelected = false;
		onRowSelected(iRow, false, bIsComingFromUI);
	}
}
/*****************************************************************************/
int UITableElement::getTotalRowHeight()
{
#ifdef ENABLE_TABLE_LOCK
	// Get the row heights with spacing
	SCOPED_LOCK(myLock);
#endif
	int iTotalHeight = 0;
	int iRow, iNumRows = myRows.size();
	int iSpacing = getCellSpacing();
	for(iRow = 0; iRow < iNumRows; iRow++)
		iTotalHeight += myRows[iRow].myHeight + iSpacing;

	return iTotalHeight;
}
/*****************************************************************************/
int UITableElement::getLastSelectedRow()
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif
	int iRow, iNumRows = myRows.size();
	for(iRow = iNumRows - 1; iRow >= 0; iRow--)
	{
		if(myRows[iRow].myIsSelected)
			return iRow;
	}

	return -1;
}
/*****************************************************************************/
bool UITableElement::getIsAnyRowSelected()
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif
	int iRow, iNumRows = myRows.size();
	for(iRow = 0; iRow < iNumRows; iRow++)
	{
		if(myRows[iRow].myIsSelected)
			return true;
	}

	return false;
}
/*****************************************************************************/
bool UITableElement::getAreMultipleRowsSelected()
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif
	int iSelCount = 0;
	int iRow, iNumRows = myRows.size();
	for(iRow = 0; iRow < iNumRows; iRow++)
	{
		if(myRows[iRow].myIsSelected)
			iSelCount++;
		if(iSelCount > 1)
			return true;
	}

	return false;
}
/*****************************************************************************/
void UITableElement::ensureRowsUpToDate()
{
	if(myIsDirty == TableRefreshRecreate)
	{
#ifdef ENABLE_TABLE_LOCK
		SCOPED_LOCK(myLock);
#endif
		this->clearRows();
		this->createRows();
	}
	else if(myIsDirty == TableRefreshRefreshDataOnly)
	{
		updateCellRelatedTableInfo(true, false);
		updateCellData();
		myIsDirty = TableRefreshClean;
	}

	_ASSERT(myIsDirty == TableRefreshClean);
}
/*****************************************************************************/
UIElement* UITableElement::getCellTargetDragTriggerChild(UITableCellElement* pCell)
{
	if(!pCell->doesPropertyExist(PropertyDragTriggerChild))
		return NULL;

	return pCell->getChildById(pCell->getStringProp(PropertyDragTriggerChild));
}
/*****************************************************************************/
void UITableElement::onDragDropHover(UIElement* pOther, SVector2D& svScreenPos)
{
	// We're going to get this on the entire table, not each cell. For now, see
	// if this is our cell being dragged or not:

#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif
	// Ignore non-cells
	UITableCellElement* pAsCell = dynamic_cast<UITableCellElement*>(pOther);
	if(!pAsCell)
		return;

	// Ignore foreign cells
	if(pAsCell->getParent<UITableElement>() != this)
		return;

	// Otherwise, find a cell at this location, call us on that
	AcceptType eNewAcceptType = AcceptNone;
	UNIQUEID_TYPE idTargetCell = -1;
	UITableCellElement* pCell = getCell(svScreenPos.x,	svScreenPos.y);
	if(pCell)
	{
		bool bIsFirst = pCell->getIsFirstChild();
		bool bIsLast = pCell->getIsLastChild();

		// See if the cell can contain the dragged cell
		bool bCanContain = pCell->getCanContain(pOther);

		// Insert before or after another cell
		SRect2D srCellRectOut;
		getScreenCellPos(pCell->getCol(), pCell->getRow(), srCellRectOut);

		// If we're the first or last contianer, make our top
		// or bottom third paste before/after us.
		if(bCanContain && bIsFirst && svScreenPos.y < srCellRectOut.y + srCellRectOut.h/3.0)
		{
			idTargetCell = pCell->getUniqueCellId();
			eNewAcceptType = AcceptBefore;
		}
		else if(bCanContain && bIsLast && svScreenPos.y > srCellRectOut.y + srCellRectOut.h - srCellRectOut.h/3.0)
		{
			idTargetCell = pCell->getUniqueCellId();
			eNewAcceptType = AcceptAfter;
		}

		// Special case: if we're the very last cell globally, insert at the end
		if(bCanContain && (pCell->getRow() == myRows.size() - 1) && svScreenPos.y > srCellRectOut.y + srCellRectOut.h - srCellRectOut.h/5.0)
		{
			idTargetCell = pCell->getUniqueCellId();
			eNewAcceptType = AcceptLastGlobal;
		}

		if(bCanContain && eNewAcceptType == AcceptNone)
		{
			// The cell can contain the dragged object. Examples are folders,
			// which can contain other stuff.
			idTargetCell = pCell->getUniqueCellId();
			eNewAcceptType = AcceptContain;
		}
		else if(eNewAcceptType == AcceptNone)
		{
			// See if we're above or below the cell midline
			idTargetCell = pCell->getUniqueCellId();
			if(svScreenPos.y < srCellRectOut.y + srCellRectOut.h/2.0)
			{
				// Insert before
				//moveCell(pAsCell, pCell, true);
				eNewAcceptType = AcceptBefore;
			}
			else
			{
				// Insert after
				//moveCell(pAsCell, pCell, false);
				eNewAcceptType = AcceptAfter;
			}
		}
		//pCell->onDragDropHover(pOther, svScreenPos);
	}

	myDragAcceptType = eNewAcceptType;
	myDragHoverTargetCellId = idTargetCell;
}
/*****************************************************************************/
bool UITableElement::receiveDragDrop(UIElement* pOther, SVector2D& svScreenPos)
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif
	if(myDragAcceptType == AcceptNone && as<UITableCellElement>(pOther))
		return false;

	// Otherwise, find a cell at this location, call us on that
	UITableCellElement* pCell = getCell(svScreenPos.x,	svScreenPos.y);
	bool bRes = false;

	//_ASSERT(pCell->getUniqueCellId() == myDragHoverTargetCellId);

	// Otherwise, do something useful (subclass), refresh, and reset the drag
	// thingies.
	bool bDoCopy = KeyManager::getInstance()->getIsModifierKeyDown(SilentKeyAlt);
	if(pCell && (pCell != pOther || bDoCopy) )
		bRes = onTableReceiveDragDrop(pCell, pOther, svScreenPos, myDragAcceptType, bDoCopy);
	if(bRes)
		this->markDirty(TableRefreshRecreate);

	clearDragDropState();
	return bRes;
}
/*****************************************************************************/
void UITableElement::clearDragDropState()
{
	myDragAcceptType = AcceptNone;
	myDragHoverTargetCellId = -1;
	myAutoscrollDir = 0;
}
/*****************************************************************************/
int UITableElement::getNumExistingColumns(int iRow)
{
	if(iRow >= 0 && iRow < myRows.size())
		return myRows[iRow].myCells.size();

	return 0;
}
/*****************************************************************************/
bool UITableElement::onMouseWheel(FLOAT_TYPE fDelta)
{
	// Scroll. Negative because of our offset
	setScroll(myScrollOffset - fDelta*MOUSEWHEEL_SCROLL, true);
	return true;
}
/*****************************************************************************/
ITableCellInfoProvider* UITableElement::getCellInfoProvider()
{
	ITableCellInfoProvider* pProvider;
	UIElement* pCurr = this;
	while(pCurr)
	{
		if(pProvider = dynamic_cast<ITableCellInfoProvider*>(pCurr))
			return pProvider;
		pCurr = pCurr->getParent<UIElement>();
	}

	return NULL;
}
/*****************************************************************************/
int UITableElement::getNumRows()
{
	int iNumRows = -1;
	ITableCellInfoProvider* pProvider = getCellInfoProvider();
	if(pProvider)
		iNumRows = pProvider->getNumRows(this);
	
	if(iNumRows >= 0)
		return iNumRows;

	return 1;
}
/*****************************************************************************/
int UITableElement::getNumColumns()
{
	int iNumColumns = -1;
	ITableCellInfoProvider* pProvider = getCellInfoProvider();
	if(pProvider)
		iNumColumns = pProvider->getNumColumns(this);

	if(iNumColumns >= 0)
		return iNumColumns;

	return this->getNumProp(PropertyNumColumns);
}
/*****************************************************************************/
void UITableElement::onPostCreateRows()
{
	ITableCellInfoProvider* pProvider = getCellInfoProvider();
	if(pProvider)
		pProvider->onPostCreateRows(this);
}
/*****************************************************************************/
UIElement* UITableElement::getCellWithPropValue(PropertyType eProp, const char* pcsValue, bool bIncludeChildren)
{
	if(!pcsValue)
		return NULL;

	UIElement* pChildRes;
	UITableCellElement* pCell;
	CellsIterator ci;
	for(ci = cellsFirst(); !ci.isEnd(); ci++)
	{
		pCell = ci.getCell();
		if(pCell->doesPropertyExist(eProp) && IS_STRING_EQUAL(pCell->getStringProp(eProp), pcsValue))
			return pCell;

		// Otherwise, look through children
		if(bIncludeChildren)
		{
			pChildRes = as<UIElement>(pCell->getChildByPropertyValue(eProp, pcsValue, true));
			if(pChildRes)
				return pChildRes;
		}
	}

	return NULL;
}
/*****************************************************************************/
void UITableElement::selectRow(int iRow, bool bComingFromUI)
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif
	ensureRowsUpToDate();
	if(iRow >= 0 && iRow < myRows.size())
	{
		deselectAllRows(true);
		setIsRowSelected(iRow, true, bComingFromUI);
		mySelectedRow = iRow;
	}
}
/*****************************************************************************/
void UITableElement::selectCell(int iCol, int iRow, bool bComingFromUI)
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif
	ensureRowsUpToDate();
	deselectAllCells(bComingFromUI);

	UITableCellElement* pCell = this->getCell(iCol, iRow);
	if(pCell)
	{
		pCell->setIsSelected(true, bComingFromUI);
	}
	ELSE_ASSERT;
}
/*****************************************************************************/
void UITableElement::deselectAllCells(bool bIsComingFromUI)
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif
	int iCell, iNumCells;
	int iCurrRow, iNumRows = myRows.size();
	SUITableRow * pRow;
	UITableCellElement* pCell;
	for(iCurrRow = 0; iCurrRow < iNumRows; iCurrRow++)
	{
		pRow = &myRows[iCurrRow];
		iNumCells = pRow->myCells.size();
		for(iCell = 0; iCell < iNumCells; iCell++)
		{
			pCell = pRow->myCells[iCell];
			pCell->setIsSelected(false, bIsComingFromUI);
		}
	}
}
/*****************************************************************************/
void UITableElement::onRowSelected(int iRow, bool bIsSelected, bool bIsComingFromUI)
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif
	UITableCellElement* pCell = myRows[iRow].myCells[0];
	// This was changed because we use bIsComingFromUI to distinguish between internally setting
	// selected rows from data and the user actually clicking on the cell; if this causes problems,
	// change other code so it works correctly. We should not have a cell selected internally but not
	// show its selected elem.
	//if(bIsComingFromUI && pCell && pCell->doesPropertyExist(PropertyUiObjTableCellSelectionElem))
	if(pCell && pCell->doesPropertyExist(PropertyCellSelectionElem))
	{
		UIElement *pSelectionElem = pCell->getElementAtPath(pCell->getStringProp(PropertyCellSelectionElem));
		if(pSelectionElem)
			pSelectionElem->setIsVisible(bIsSelected);	
		ELSE_ASSERT;
	}

	onRowSelectedSubclass(iRow, bIsSelected, bIsComingFromUI);

	// This is legacy, because it doesn't distinguish between selecting and deselecting the row.
	// Send an action, too, in case we don't want to inherit
	theSharedString = UIA_TABLE_ROW_SELECTION_CHANGED;
	BaseInt rSelRow(iRow);
	handleActionUpParentChain(theSharedString, false, &rSelRow);

	// Try to use these instead
	if(bIsSelected)
		theSharedString = UIA_TABLE_ROW_SELECTED;
	else
		theSharedString = UIA_TABLE_ROW_DESELECTED;
	ResourceItem rItem;
	rItem.setNumProp(PropertyRow, iRow);
	rItem.setBoolProp(PropertyIsComingFromUI, bIsComingFromUI);
	handleActionUpParentChain(theSharedString, false, &rItem);
}
/*****************************************************************************/
void UITableElement::updateCellData()
{
#ifdef ENABLE_TABLE_LOCK
	SCOPED_LOCK(myLock);
#endif
	UITableCellElement* pCellElement;
	SUITableRow *pRow;
	int iRow, iNumRows = myRows.size();
	int iCell, iNumCells;
	for(iRow = 0; iRow < iNumRows; iRow++)
	{
		pRow = &myRows[iRow];
		iNumCells = pRow->myCells.size();
		for(iCell = 0; iCell < iNumCells; iCell++)
		{
			pCellElement = pRow->myCells[iCell];
			pCellElement->updateDataRecursive();
		}
	}
}
/*****************************************************************************/
void UITableElement::ensureRowVisible(int iRow)
{
	if(myRows.size() <= 0)
		return;

	SRect2D srFirstRow, srOurRow;
	getScreenCellPos(0, 0, srFirstRow);
	getScreenCellPos(0, iRow, srOurRow);

	SVector2D svOwnSize;
	getBoxSize(svOwnSize);

	FLOAT_TYPE fNeededScroll = srOurRow.y - srFirstRow.y - svOwnSize.y/2.0;
	if(fNeededScroll < 0)
		fNeededScroll = 0;
	setScroll(fNeededScroll + TABLE_VERT_PADDING);
}
/*****************************************************************************/
bool UITableElement::getAllowValuePropagation(FLOAT_TYPE fNewValue, bool bIsChangingContinuously, UIElement* pOptSourceElem, UIElement* pLinkedToElem)
{
	// For tables, we don't propagate the value if the source elem is our slider.
	// Otherwise, we may start marking nodes dirty...
	if(pOptSourceElem && getRelatedTableSlider() == pOptSourceElem)
		return false;
	else
		return UIElement::getAllowValuePropagation(fNewValue, bIsChangingContinuously, pOptSourceElem, pLinkedToElem);
}
/*****************************************************************************/
bool UITableElement::getAllowScrollFromPoint(const SVector2D& svBeginPoint, FLOAT_TYPE fX, FLOAT_TYPE fY)
{
	return fabs(svBeginPoint.y - fY) >= getScrollPixelTolerance()
		// This line makes sure we can drag items out of the table, so it only
		// scrolls if we have a mostly vertical motion
		&& fabs(svBeginPoint.x - fX)/fabs(svBeginPoint.y - fY) <= HOR_VERT_MOTION_RATIO;
}
/*****************************************************************************/
FLOAT_TYPE UITableElement::computeScrollFromPoint(const SVector2D& svBeginPoint, FLOAT_TYPE fX, FLOAT_TYPE fY)
{
	return myInitScroll - (fY - svBeginPoint.y);
}
/*****************************************************************************/
FLOAT_TYPE UITableElement::computeScrollSpeedFromTrail(const SVector2D& svCurrPoint, FLOAT_TYPE fX, FLOAT_TYPE fY, GTIME lTime, GTIME lCurrPointTime)
{
	return (svCurrPoint.y - fY)/(lTime - lCurrPointTime);
}
/*****************************************************************************/
void UITableElement::getInitCellPos(const SVector2D& svScroll, const SVector2D& svPos, FLOAT_TYPE fTotalScale, FLOAT_TYPE fSelfW, FLOAT_TYPE fSelfH, SVector2D& svInitPosOut)
{
	svInitPosOut.x = svScroll.x - myTotalRowWidth/2.0*fTotalScale + svPos.x;
	svInitPosOut.y = svScroll.y - fSelfH/2.0 + svPos.y + TABLE_VERT_PADDING;
	svInitPosOut.y -= myScrollOffset;
}
/*****************************************************************************/
int UITableElement::getMaxScrollInScrollAxisDirection(const SVector2D& svBoxSize, int iSpacing)
{
	return getTotalRowHeight() - iSpacing + TABLE_VERT_PADDING*2 - svBoxSize.y;
}
/*****************************************************************************/
};
