#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
UndoBlock::UndoBlock(UndoBlock* pParent, UndoManager* pParentManager, const char* pcsBlockTitle) 
{ 
// 	if(pParentManager && pParentManager && pcsBlockTitle)
// 	{

	_ASSERT(pParentManager);
	myParentManager = pParentManager;
	myParentBlock = pParent;
	myWasAutoclosed = false;
	
	myCreationTime = Application::getInstance()->getGlobalTime(ClockUniversal);

	_ASSERT(pcsBlockTitle);
	if(pcsBlockTitle)
		myTitle = pcsBlockTitle;
	else
		myTitle = "Unnamed";
	//myChildren = NULL; 
	myIsOpen = true;
//	}
}
/*****************************************************************************/
UndoBlock::~UndoBlock() 
{
	int iCurr, iNum = myUndoItems.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
		delete myUndoItems[iCurr];
	myUndoItems.clear();
/*
	if(myChildren) 
	{
		int iCurr, iNum = myChildren->size();
		for(iCurr = 0; iCurr < iNum; iCurr++)
			delete (*myChildren)[iCurr];
		delete myChildren; 
	}
	myChildren = NULL; 
	*/
}
/*****************************************************************************/
bool UndoBlock::isEmpty()
{
	if(myUndoItems.size() > 0)
		return false;

	UndoBlock* pBlock;
	int iChild, iNumChildren = myUndoItems.size();
	for(iChild = 0; iChild < iNumChildren; iChild++)
	{
		pBlock = dynamic_cast<UndoBlock*>(myUndoItems[iChild]);
		if(pBlock && !pBlock->isEmpty())
			return false;
	}

/*
	// Otherwise, look in all child blocks
	if(myChildren)
	{
		int iChild, iNumChildren = myChildren->size();
		for(iChild = 0; iChild < iNumChildren; iChild++)
		{
			if(!(*myChildren)[iChild]->isEmpty())
				return false;
		}
	}
*/

	return true;
}
/*****************************************************************************/
UndoBlock* UndoBlock::getLastClosedUndoBlock()
{
	UndoBlock* pBlock, *pLastBlock;
	int iChild, iNumChildren = myUndoItems.size();
	for(iChild = iNumChildren - 1; iChild >= 0; iChild--)
	{
		pBlock = dynamic_cast<UndoBlock*>(myUndoItems[iChild]);
		if(pBlock)
		{
			pLastBlock = pBlock->getLastClosedUndoBlock();
			if(pLastBlock)
				return pLastBlock;
		}
	}

	return this;
}
/*****************************************************************************/
UndoBlock* UndoBlock::getLastOpenBlock()
{
	// First, try to find one in children, then try ourselves
	UndoBlock* pBlock, *pLastOpenBlock;
	int iChild, iNumChildren = myUndoItems.size();
	for(iChild = 0; iChild < iNumChildren; iChild++)
	{
		pBlock = dynamic_cast<UndoBlock*>(myUndoItems[iChild]);
		if(pBlock)
		{
			pLastOpenBlock = pBlock->getLastOpenBlock();
			if(pLastOpenBlock)
				return pLastOpenBlock;
		}
	}

/*
	if(myChildren)
	{
		int iChild, iNumChildren = myChildren->size();
		for(iChild = 0; iChild < iNumChildren; iChild++)
		{
			pCurrBlock = (*myChildren)[iChild]->getLastOpenBlock();
			if(pCurrBlock)
				return pCurrBlock;
		}
	}
*/
	if(myIsOpen)
		return this;
	else
		return NULL;
}
/*****************************************************************************/
void UndoBlock::cancelBlock(UndoBlock* pChildBlock)
{
	TUndoItems::iterator fi = find(myUndoItems.begin(), myUndoItems.end(), dynamic_cast<IUndoItem*>(pChildBlock));
	myUndoItems.erase(fi);
	delete pChildBlock;
	/*
	_ASSERT(myChildren);
	if(!myChildren)
		return;

	TUndoBlocks::iterator fi = find(myChildren->begin(), myChildren->end(), pChildBlock);
	myChildren->erase(fi);
	delete pChildBlock;
	*/
}
/*****************************************************************************/
void UndoBlock::undo(IBaseObject* pContext, UIElement* pElem)
{
	// Undo all the items in turn...
	int iChild, iNumChildren = myUndoItems.size();
	for(iChild = iNumChildren - 1; iChild >= 0; iChild--)
	{
		myUndoItems[iChild]->undo(pContext, pElem);
	}
}
/*****************************************************************************/
void UndoBlock::redo(IBaseObject* pContext, UIElement* pElem)
{
	int iChild, iNumChildren = myUndoItems.size();
	for(iChild = 0; iChild < iNumChildren; iChild++)
	{
		myUndoItems[iChild]->redo(pContext, pElem);
	}
}
/*****************************************************************************/
void UndoBlock::addItem(IUndoItem* pItem)
{
	myUndoItems.push_back(pItem);
}
/*****************************************************************************/
IUndoItem* UndoBlock::getLastItem()
{
	int iNumItems = myUndoItems.size();
	if(iNumItems == 0)
		return NULL;
	else
		return myUndoItems[iNumItems - 1];
}
/*****************************************************************************/
bool UndoBlock::canContinue(GTIME lTime, const char* pcsUndoType)
{
	if(lTime - myCreationTime >= Application::secondsToTicks(1.0))
		return false;

	// See if we're the same type
	IUndoItem* pLastItem = getLastItem();
	if(!pLastItem)
		return false;

	if(strcmp(pLastItem->getType(), pcsUndoType) != 0)
		return false;

	return true;
}
/*****************************************************************************/
ResourceItem* UndoBlock::saveToItem(ResourceItem& rItem)
{
	// Add a block for self
	ResourceItem* pChild = IUndoItem::saveToItem(rItem);
	pChild->setAsLong(PropertyId, getId());
	pChild->setBoolProp(PropertySvDoorOpenness, myIsOpen);

	// Go over all children
	TUndoItems::iterator li;
	for(li = myUndoItems.begin(); li != myUndoItems.end(); li++)
		(*li)->saveToItem(*pChild);

	return pChild;
}
/*****************************************************************************/
void UndoBlock::onBeforeDelete(IBaseObject* pOptParentContext)
{
	// Go over all children
	TUndoItems::iterator li;
	for(li = myUndoItems.begin(); li != myUndoItems.end(); li++)
		(*li)->onBeforeDelete(pOptParentContext);
}
/*****************************************************************************/
template < class ID_CLASS >
UNIQUEID_TYPE GlobalIdGeneratorInternal<ID_CLASS>::myGeneratorLastId = 0;
    

};
