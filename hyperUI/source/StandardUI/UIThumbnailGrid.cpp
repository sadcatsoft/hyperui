#include "stdafx.h"

#define USE_THREADED_THUMBNAILS

#define THUMBNAIL_RECEPTOR_TAG		"thumbnail_receptor"
#define THUMB_INNER_PADDING			upToScreen(2.0)

namespace HyperUI
{
/*****************************************************************************/
UIThumbnailGrid::UIThumbnailGrid(UIPlane* pParentPlane)
	: UIGrid(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void UIThumbnailGrid::onAllocated(IBaseObject* pData)
{
	UIGrid::onAllocated(pData);
	myProvider = NULL;
}
/*****************************************************************************/
UIThumbnailGrid::~UIThumbnailGrid()
{
	onDeallocated();
}
/*****************************************************************************/
void UIThumbnailGrid::onDeallocated(void)
{
	UIGrid::onDeallocated();
	clearProvider();
}
/*****************************************************************************/
int UIThumbnailGrid::getNumTotalCells()
{
	if(myProvider)
		return myProvider->getNumThumbnails();
	else
		return 0;
}
/*****************************************************************************/
void UIThumbnailGrid::onPostCreateRows()
{
	UIGrid::onPostCreateRows();
	ensureThumbnailsCreated();
}
/*****************************************************************************/
void UIThumbnailGrid::clearProvider()
{
	if(myProvider && myDoOwnProvider)
		myProvider->deleteSelf();
	myProvider = NULL;
}
/*****************************************************************************/
void UIThumbnailGrid::ensureThumbnailsCreated()
{
// We can't do this because we don't adjust to size on making the tab visible...
//	if(!this->getIsVisibleCumulative() || this->getIsFullyHiddenCumulative())
	//	return;

	SVector2D svCellSize(this->getNumProp(PropertyCellWidth), this->getNumProp(PropertyCellHeight));

	FLOAT_TYPE fThumbnailPadding = -1; 
	if(myProvider)
		fThumbnailPadding = myProvider->getThumbnailPadding();
	if(fThumbnailPadding < 0)
		fThumbnailPadding = THUMB_INNER_PADDING;

	svCellSize.x -= fThumbnailPadding*2.0;
	svCellSize.y -= fThumbnailPadding*2.0;

	// Now, set the thumbnails
	UITableElement::CellsIterator ci;
	UITableCellElement* pCell;
#ifndef USE_THREADED_THUMBNAILS
	AccelImage* pAccelImage;
#endif
	const char* pcsTargetData;
	UIElement *pThumbElem;
	int iCellIndex;
	for(ci = this->cellsFirst(), iCellIndex = 0; !ci.isEnd(); ci++, iCellIndex++)
	{
		pCell = ci.getCell();

		pThumbElem = pCell->findChildWithTag(THUMBNAIL_RECEPTOR_TAG, true);

		pcsTargetData = NULL;
#ifdef USE_THREADED_THUMBNAILS
		pThumbElem->setAccelImage(NULL);
		if(myProvider)
		{
			myProvider->queueThreadedThumbnail(iCellIndex, svCellSize, pThumbElem);
			pcsTargetData = myProvider->getTargetData(iCellIndex);
		}
#else
		// Now, force an image onto it
		if(myProvider)
		{
			pAccelImage = myProvider->getThumbnail(iCellIndex, svCellSize);
			pcsTargetData = myProvider->getTargetData(iCellIndex);
		}
		else
			pAccelImage = NULL;
		pThumbElem->setAccelImage(pAccelImage);
#endif
		if(pcsTargetData)
			pCell->setStringProp(PropertyTargetDataSource, pcsTargetData);
	}
}
/*****************************************************************************/
void UIThumbnailGrid::setProvider(IThumbnailProvider* pProvider, bool bDoOwn)
{
	clearProvider();
	myProvider = pProvider; 
	myDoOwnProvider = bDoOwn;
}
/*****************************************************************************/
};