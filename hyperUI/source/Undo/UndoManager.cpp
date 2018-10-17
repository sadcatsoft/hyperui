#include "stdafx.h"

#define DB_INSERTION_POINTER	"-------------------------------INSERTION_POINTER_HERE-------------------------------"


namespace HyperUI
{
IUndoManagerCallback* UndoManager::theCallback = NULL;

/*****************************************************************************/
// AutoUndoContainerBlock
/*****************************************************************************/
AutoUndoContainerBlock::AutoUndoContainerBlock(const char* pcsBlockDesc, UndoManager* pManager, const char* pcsContainerSubId) 
{ 
	myInitWindow = NULL;
	myInitContext = NULL;
	myOverrideManager = pManager; 
	myBlockId = UndoManager::addContainerBlockItemToManager(pManager, pcsBlockDesc, pcsContainerSubId); 
}
AutoUndoContainerBlock::AutoUndoContainerBlock(const char* pcsBlockDesc, Window* pWindow, IBaseObject* pContext, const char* pcsContainerSubId) 
{ 
	myInitWindow = pWindow; 
	myInitContext = pContext; 
	myOverrideManager = NULL; 
	myBlockId = UndoManager::addContainerBlockItem(pcsBlockDesc, pWindow, pContext, pcsContainerSubId); 
}
/*****************************************************************************/
AutoUndoContainerBlock::AutoUndoContainerBlock()
{
	myInitWindow = NULL;
	myInitContext = NULL;
	myOverrideManager = NULL;
	myBlockId = -1;
}
/*****************************************************************************/
UndoManager::UndoManager(IBaseObject* pParentContext)
{
	_ASSERT(pParentContext);
	myParentContext = pParentContext;
	myParentElem = NULL;

	myInsertPositionUndoIterator = myTopLevelUndos.end();
	myDisableAddingNewUndosStackCounter = 0;
}
/*****************************************************************************/
UndoManager::UndoManager(UIElement* pParentElem)
{
	_ASSERT(pParentElem);
	myParentElem = pParentElem;
	myParentContext = NULL;

	myInsertPositionUndoIterator = myTopLevelUndos.end();
	myDisableAddingNewUndosStackCounter = 0;
}
/*****************************************************************************/
UndoManager::~UndoManager()
{
	clear();
}
/*****************************************************************************/
UndoBlock* UndoManager::getLastUndoBlock()
{
	UndoBlock* pRes = NULL;
	if(myTopLevelUndos.size() > 0)
	{
		TUndoBlocksList::iterator li = myTopLevelUndos.end();
		li--;
		pRes = (*li)->getLastClosedUndoBlock();
	}

	return pRes;
}
/*****************************************************************************/
UndoBlock* UndoManager::getLastOpenUndoBlock()
{
	UndoBlock* pRes = NULL;
	if(myTopLevelUndos.size() > 0)
	{
		TUndoBlocksList::iterator li = myTopLevelUndos.end();
		li--;
		pRes = (*li)->getLastOpenBlock();
	}

	return pRes;
}
/*****************************************************************************/
UndoBlock* UndoManager::beginUndoBlock(const char* pcsBlockDesc, bool bAddContainerBlock, const char* pcsContainerId)
{
	// We need to find the last hierarchical block
	UndoBlock* pFoundParent = getLastOpenUndoBlock();

	// If we're inserting and we're not at the end of the queue...

	UndoBlock* pNew = NULL;
	if(bAddContainerBlock)
		pNew = new UndoContainerBlock(pFoundParent, this, pcsBlockDesc, pcsContainerId);
	else
		pNew = new UndoBlock(pFoundParent, this, pcsBlockDesc);
	if(pFoundParent)
		pFoundParent->addItem(pNew);
	else
	{
		clearAllUndosStartingAt(myInsertPositionUndoIterator);
		// No assert because the iterator will simply be invalid now,
		// but not point to th end.
		//_ASSERT(myInsertPositionUndoIterator == myTopLevelUndos.end());
		myTopLevelUndos.push_back(pNew);
		trimToMaxDepth();
		myInsertPositionUndoIterator = myTopLevelUndos.end();
	}
	return pNew;
}
/*****************************************************************************/
void UndoManager::endUndoBlock(UNIQUEID_TYPE idBlock)
{
#ifdef _DEBUG
	//saveStackToFile("db_end_undo_block.txt");
#endif
	UndoBlock* pLastOpen  = getLastOpenUndoBlock();
	UndoBlock* pLastBlock = getLastUndoBlock();

	// Now, it may happen here so that we are skipping closing one (child) block,
	// and instead are immediately closing its upper parent. This is fine - 
	// otherwise we would have had to structure our undo open/close in precise order,
	// which is likely to lead to messier, more complex, and buggier code. Instead,
	// we take care of this here.
	UndoBlock* pFinalBlockToClose = NULL;
	for(pFinalBlockToClose = pLastOpen; pFinalBlockToClose && pFinalBlockToClose->getId() != idBlock; pFinalBlockToClose = pFinalBlockToClose->getParentBlock())
		;

	// We may, in the future, fail here again if we've already closed the top parent block (correctly so)
	// and are trying to close an inner child.
	_ASSERT(
		(pFinalBlockToClose && pFinalBlockToClose->getId() == idBlock && pFinalBlockToClose->isOpen())
		|| (!pFinalBlockToClose && pLastBlock && pLastBlock->wasAutoclosed())
		);
	if(pFinalBlockToClose)
		pFinalBlockToClose->close();
}
/*****************************************************************************/
void UndoManager::cancelUndoBlock(UNIQUEID_TYPE idBlock)
{
	UndoBlock* pLastOpen  = getLastOpenUndoBlock();
	_ASSERT(pLastOpen && pLastOpen->getId() == idBlock && pLastOpen->isOpen());
	
	UndoBlock* pParent = pLastOpen->getParentBlock();
	if(!pParent)
	{
		// Top-level block:
		TUndoBlocksList::iterator fi = find(myTopLevelUndos.begin(), myTopLevelUndos.end(), pLastOpen);
		_ASSERT(fi != myTopLevelUndos.end());
		if(fi != myTopLevelUndos.end())
		{
			myTopLevelUndos.erase(fi);
			pLastOpen->onBeforeDelete(myParentContext);
			delete pLastOpen;
		}
	}
	else
	{
		pParent->cancelBlock(pLastOpen);
	}
}
/*****************************************************************************/
void UndoManager::clearAllUndosStartingAt(TUndoBlocksList::iterator& it)
{
	if(it == myTopLevelUndos.end())
		return;

	TUndoBlocksList::iterator di;
	for(di = it; di != myTopLevelUndos.end(); di++)
	{
		(*di)->onBeforeDelete(myParentContext);
		delete (*di);
	}
	
	myTopLevelUndos.erase(it, myTopLevelUndos.end());
}
/*****************************************************************************/
void UndoManager::trimToMaxDepth()
{
	int iMaxDepth = SettingsCollection::getInstance()->getSettingsItem()->getNumProp(PropertySetMaxUndoDepth);
	if(iMaxDepth <= 0)
	{
		// Set to some decent default value
		iMaxDepth = 100;
	}

	int iNum = myTopLevelUndos.size();
	if(iNum <= iMaxDepth)
		return;

	// Otherwise, start at the front
	int iTrimmed, iNumToTrim = iNum - iMaxDepth;
	_ASSERT(iNumToTrim > 0);
	TUndoBlocksList::iterator di;
	for(iTrimmed = 0, di = myTopLevelUndos.begin(); iTrimmed < iNumToTrim; iTrimmed++, di++)
	{
		_ASSERT(di != myInsertPositionUndoIterator);
		(*di)->onBeforeDelete(myParentContext);
		delete (*di);
	}

	myTopLevelUndos.erase(myTopLevelUndos.begin(), di);
}
/*****************************************************************************/
bool UndoManager::canUndo()
{
	return myTopLevelUndos.size() > 0 && myInsertPositionUndoIterator != myTopLevelUndos.begin();
}
/*****************************************************************************/
bool UndoManager::canRedo()
{
	return myTopLevelUndos.size() > 0 && myInsertPositionUndoIterator != myTopLevelUndos.end();
}
/*****************************************************************************/
void UndoManager::undo()
{
	// Now, migrate one back, get the undo block pointer, and undo it.
	_ASSERT(canUndo());
	if(!canUndo())
		return;

#ifdef DEBUG_UNDO_STACK
	saveStackToFile("dbur_before_undo.txt");
#endif

	this->onBeforeUndo(myParentContext, myParentElem);

	//SCOPED_SCENE_CANCEL_RENDERING_AND_WRITE_LOCK(myParentContext);
	this->disableNewUndos();

	// Note that we may be undoing something in the middle of an open block...
	// And it's perfectly legal. We could be undoing a number change while
	// the focus is on the UIText elem, which mean it still hasn't closed
	// the last undo block. So we auto-close, but mark it as such so we don't
	// assert later on but still check for invalid nesting.
	UndoBlock* pLastOpeUndoBlock = getLastOpenUndoBlock();
	if(pLastOpeUndoBlock)
		pLastOpeUndoBlock->autoClose();

	myInsertPositionUndoIterator--;
	(*myInsertPositionUndoIterator)->undo(myParentContext, myParentElem);

	const char* pcsTitle = (*myInsertPositionUndoIterator)->getTitle();
	myCommonString = "Undoing ";
	myCommonString += pcsTitle;

	this->enableNewUndos();
	EventManager::getInstance()->sendEvent(EventUndoPerformed, this);

#ifdef DEBUG_UNDO_STACK
	saveStackToFile("dbur_after_undo.txt");
#endif

	this->onAfterUndo(myParentContext, myParentElem, myCommonString.c_str());
}
/*****************************************************************************/
void UndoManager::redo()
{
	_ASSERT(canRedo());
	if(!canRedo())
		return;

#ifdef DEBUG_UNDO_STACK
	saveStackToFile("dbur_before_redo.txt");
#endif

	this->onBeforeRedo(myParentContext, myParentElem);
	//SCOPED_SCENE_CANCEL_RENDERING_AND_WRITE_LOCK(myParentContext);
	this->disableNewUndos();

	(*myInsertPositionUndoIterator)->redo(myParentContext, myParentElem);
	
	const char* pcsTitle = (*myInsertPositionUndoIterator)->getTitle();
	myCommonString = "Redoing ";
	myCommonString += pcsTitle;

	myInsertPositionUndoIterator++;

	this->enableNewUndos();

	EventManager::getInstance()->sendEvent(EventRedoPerformed, this);

#ifdef DEBUG_UNDO_STACK
	saveStackToFile("dbur_after_redo.txt");
#endif

	this->onAfterRedo(myParentContext, myParentElem, myCommonString.c_str());
}
/*****************************************************************************/
UndoBlock* UndoManager::getUndoBlockFor(const char* pcsBlockDesc, const char* pcsUndoItemType, bool bAllowContinuation, bool bCreateContainerBlockIfNeeded)
{
	_ASSERT(this->getAreNewUndosEnabled());

	// If we allow continuation, see if we can continue the last block
	UndoBlock* pResBlock = NULL;
	UndoBlock* pLastUndoBlock = getLastUndoBlock();
	if(bAllowContinuation && pLastUndoBlock && !IS_STRING_EQUAL(pcsUndoItemType, PROPERTY_NONE) ) //  && strcmp(pLastUndoBlock->getType(), pcsBlockType) == 0)
	{
		// See if the time allows		
		GTIME lNow = Application::getInstance()->getGlobalTime(ClockUniversal);
		if(pLastUndoBlock->canContinue(lNow, pcsUndoItemType))
		{
			pResBlock = pLastUndoBlock;
			pResBlock->reopen();
		}
	}

	if(!pResBlock)
		pResBlock = beginUndoBlock(pcsBlockDesc, bCreateContainerBlockIfNeeded, pcsUndoItemType);

	return pResBlock;
}
/*****************************************************************************/
UNIQUEID_TYPE UndoManager::addContainerBlockItem(const char* pcsBlockDesc, Window* pWindow, IBaseObject* pContext, const char* pcsContainerSubId)
{
	UndoManager* pCurrUndoManager = getCurrentUndoManager(pContext, pWindow);
	if(!pCurrUndoManager)
	{
		_ASSERT(0);
		return -1;
	}
	if(!pCurrUndoManager->getAreNewUndosEnabled())
		return -1;

	return addContainerBlockItemToManager(pCurrUndoManager, pcsBlockDesc, pcsContainerSubId);
}
/*****************************************************************************/
UNIQUEID_TYPE UndoManager::addContainerBlockItemToManager(UndoManager *pManager, const char* pcsBlockDesc, const char* pcsContainerSubId)
{
	if(!pManager)
	{
		_ASSERT(0);
		return -1;
	}
	if(!pManager->getAreNewUndosEnabled())
		return -1;
	UndoBlock* pCurrUndoBlock = pManager->getUndoBlockFor(pcsBlockDesc, pcsContainerSubId, true, true);
	EventManager::getInstance()->sendEvent(EventUndoBlockAdded, pCurrUndoBlock);
	return pCurrUndoBlock->getId();
}
/*****************************************************************************/
void UndoManager::addUndoItemToCurrentBlock(const char* pcsItemDesc, IUndoItem* pItem, IBaseObject* pContext)
{
	IUndoItem* pPtr = pItem;
	addUndoItemToCurrentBlockFromPointer(pcsItemDesc, pPtr, pContext);
}
/*****************************************************************************/
void UndoManager::addUndoItemToCurrentBlockFromPointer(const char* pcsItemDesc, IUndoItem*& pItem, IBaseObject* pContext)
{
	if(!pItem)
		return;

	// Note that for undos related to nodes, we *always* have to have a valid scene.
	UndoManager* pCurrUndoManager = getCurrentUndoManager(pContext);

	if(!pCurrUndoManager)
	{
		pItem->onBeforeDelete(pContext);
		delete pItem;
		pItem = NULL;
		ASSERT_RETURN;
	}

	if(!pCurrUndoManager->getAreNewUndosEnabled())
	{
		pItem->onBeforeDelete(pContext);
		delete pItem;
		pItem = NULL;
		return;
	}

	UndoBlock* pOpenBlock = pCurrUndoManager->getLastOpenUndoBlock();
	// If you broke here, it's likely you need a top-level undo for
	// whatever the overall operation you are doing. Or you need
	// to disable undos completely.
	//
	// We can't auto add it here if we have no open block because then
	// there would be no way of actually closing it.
	if(!pOpenBlock)
	{
		pItem->onBeforeDelete(pContext);
		delete pItem;
		pItem = NULL;
		ASSERT_RETURN;
	}

	pOpenBlock->addItem(pItem);
}
/*****************************************************************************/
UNIQUEID_TYPE UndoManager::addUndoItemToCurrentManager(const char* pcsBlockDesc, IUndoItem* pItem, bool bAllowContinuation, Window* pWindow, IBaseObject* pContext)
{
	IUndoItem* pPtr = pItem;
	return UndoManager::addUndoItemToCurrentManagerFromPointer(pcsBlockDesc, pPtr, bAllowContinuation, pWindow, pContext);
}
/*****************************************************************************/
UNIQUEID_TYPE UndoManager::addUndoItemToCurrentManagerFromPointer(const char* pcsBlockDesc, IUndoItem*& pItem, bool bAllowContinuation, Window* pWindow, IBaseObject* pContext)
{
	if(!pItem)
		return -1;

	UndoManager* pCurrUndoManager = getCurrentUndoManager(pContext, pWindow);
	if(!pCurrUndoManager)
	{
		pItem->onBeforeDelete(NULL);
		delete pItem;
		pItem = NULL;

		_ASSERT(0);
		return -1;
	}

	if(!pCurrUndoManager->getAreNewUndosEnabled())
	{
		pItem->onBeforeDelete(pCurrUndoManager->getParentContext());
		delete pItem;
		pItem = NULL;
		return -1;
	}

	UndoBlock* pCurrUndoBlock = pCurrUndoManager->getUndoBlockFor(pcsBlockDesc, pItem->getType(), bAllowContinuation, false);
	pCurrUndoBlock->addItem(pItem);

	EventManager::getInstance()->sendEvent(EventUndoBlockAdded, pCurrUndoBlock);

	return pCurrUndoBlock->getId();
}
/*****************************************************************************/
void UndoManager::endUndoBlockInCurrentManager(UNIQUEID_TYPE idBlock, Window* pWindow, IBaseObject* pContext, UndoManager* pOptOverrideManager)
{
	_ASSERT(idBlock >= 0);
	UndoManager* pCurrUndoManager = pOptOverrideManager;
	if(!pCurrUndoManager)
		pCurrUndoManager = getCurrentUndoManager(pContext, pWindow);
	if(!pCurrUndoManager)
		ASSERT_RETURN;

	pCurrUndoManager->endUndoBlock(idBlock);
}
/*****************************************************************************/
bool UndoManager::canAcceptNewUndoNow(IBaseObject* pOptContext, Window* pOptWindow)
{
	// No undos for viewer!
#ifdef IMAGE_VIEWER_MODE
	return false;
#endif

	UndoManager* pCurrUndoManager = getCurrentUndoManager(pOptContext, pOptWindow);
	if(!pCurrUndoManager)
		return false;
	else
		return pCurrUndoManager->getAreNewUndosEnabled();
}
/*****************************************************************************/
void UndoManager::saveToItem(ResourceItem& rItem)
{
	TUndoBlocksList::iterator li;
	for(li = myTopLevelUndos.begin(); li != myTopLevelUndos.end(); li++)
	{
#ifdef DEBUG_UNDO_STACK
		if(li == myInsertPositionUndoIterator)
			rItem.addChild("insPtr")->setStringProp(PropertyId, DB_INSERTION_POINTER);
#endif
		(*li)->saveToItem(rItem);
	}
}
/*****************************************************************************/
void UndoManager::setCallback(IUndoManagerCallback* pCallback, bool bOnlyIfNotSet)
{
	if(bOnlyIfNotSet && theCallback)
		return;

	delete theCallback; 
	theCallback = pCallback;
}
/*****************************************************************************/
#ifdef DEBUG_UNDO_STACK
void UndoManager::saveStackToFile(const char* pcsFilename)
{
	ResourceItem rItem;
	rItem.setTag("top");
	rItem.setStringProp(PropertyId, "top");
	saveToItem(rItem);
	string strTemp;
	rItem.saveToString(strTemp);

	if(myInsertPositionUndoIterator == myTopLevelUndos.end())
	{
		strTemp += "\n";
		strTemp += DB_INSERTION_POINTER;
		strTemp += "\n";
	}

	FileUtils::saveToFile(pcsFilename, strTemp.c_str(), true);
}
#endif
/*****************************************************************************/
UNIQUEID_TYPE UndoManager::getCurrentlyPointedToBlockId()
{
	if(!canUndo())
		return -1;
	else
	{
		TUndoBlocksList::iterator it2 = myInsertPositionUndoIterator;
		it2--;
		return (*it2)->getId();
	}
}
/*****************************************************************************/
void UndoManager::clear()
{
	TUndoBlocksList::iterator di;
	for(di = myTopLevelUndos.begin(); di != myTopLevelUndos.end(); di++)
		(*di)->onBeforeDelete(myParentContext);

	clearAndDeleteContainer(myTopLevelUndos);
	myInsertPositionUndoIterator = myTopLevelUndos.end();
}
/*****************************************************************************/
UndoManager* UndoManager::getCurrentUndoManager(IBaseObject* pOptContext, Window* pOptWindow)
{
	if(theCallback)
		return theCallback->getCurrentUndoManager(pOptContext, pOptWindow);

	UndoManager* pRes = NULL;
	Window* pParentWindow = pOptWindow;
	if(!pParentWindow)
		pParentWindow = dynamic_cast<Window*>(WindowManager::getInstance()->getLastActiveWindow());
	if(pParentWindow)
		pRes = pParentWindow->getLastOverrideUndoManager();

	return pRes;
}
/*****************************************************************************/
template < class ITEM_TYPE >
HYPERUI_API TWindowManager<ITEM_TYPE>* TWindowManager<ITEM_TYPE>::theInstance = NULL;
};
