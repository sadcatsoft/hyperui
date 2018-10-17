#pragma once

#ifdef _DEBUG
#define DEBUG_UNDO_STACK
#endif

class HYPERUI_API UIElement;
class HYPERUI_API UndoManager;
/*****************************************************************************/
class HYPERUI_API IUndoManagerCallback
{
public:
	virtual ~IUndoManagerCallback() { }
	virtual UndoManager* getCurrentUndoManager(IBaseObject* pOptContext, Window* pOptWindow) = 0;
};
/*****************************************************************************/
class HYPERUI_API UndoManager : public IBaseObject
{
public:
	explicit UndoManager(IBaseObject* pParentContext);
	explicit UndoManager(UIElement* pParentElem);
	virtual ~UndoManager();

	void clear();

	bool canUndo();
	bool canRedo();

	void undo();
	void redo();

	UNIQUEID_TYPE getCurrentlyPointedToBlockId();

	void cancelUndoBlock(UNIQUEID_TYPE idBlock);

	// Helper functions
	static UNIQUEID_TYPE addUndoItemToCurrentManager(const char* pcsBlockDesc, IUndoItem* pItem, bool bAllowContinuation, Window* pWindow, IBaseObject* pContext);
	static UNIQUEID_TYPE addUndoItemToCurrentManagerFromPointer(const char* pcsBlockDesc, IUndoItem*& pItem, bool bAllowContinuation, Window* pWindow, IBaseObject* pContext);
	static void addUndoItemToCurrentBlock(const char* pcsItemDesc, IUndoItem* pItem, IBaseObject* pContext);
	static void addUndoItemToCurrentBlockFromPointer(const char* pcsItemDesc, IUndoItem*& pItem, IBaseObject* pContext);
	// Pass in PROPERTY_NONE as pcsContainerSubId to always create a new block
	static UNIQUEID_TYPE addContainerBlockItem(const char* pcsBlockDesc, Window* pWindow, IBaseObject* pContext, const char* pcsContainerSubId = PROPERTY_NONE);
	static UNIQUEID_TYPE addContainerBlockItemToManager(UndoManager *pManager, const char* pcsBlockDesc, const char* pcsContainerSubId = PROPERTY_NONE);
	static void endUndoBlockInCurrentManager(UNIQUEID_TYPE idBlock, Window* pWindow, IBaseObject* pContext, UndoManager* pOptOverrideManager = NULL);

	// This returns true if any undo can be accepted.
	static bool canAcceptNewUndoNow(IBaseObject* pOptContext = NULL, Window* pOptWindow = NULL);

	void disableNewUndos() { myDisableAddingNewUndosStackCounter++; }
	void enableNewUndos() { myDisableAddingNewUndosStackCounter--; _ASSERT(myDisableAddingNewUndosStackCounter >= 0); }
	bool getAreNewUndosEnabled() { return myDisableAddingNewUndosStackCounter <= 0; }

	IBaseObject* getParentContext() { return myParentContext; }

	void saveToItem(ResourceItem& rItem);

	static void setCallback(IUndoManagerCallback* pCallback, bool bOnlyIfNotSet);

#ifdef DEBUG_UNDO_STACK
	void saveStackToFile(const char* pcsFilename);
#endif

	static UndoManager* getCurrentUndoManager(IBaseObject* pOptContext = NULL, Window* pOptWindow = NULL);

protected:

	virtual void onBeforeUndo(IBaseObject* pContext, UIElement* pParentElem) { }
	virtual void onAfterUndo(IBaseObject* pContext, UIElement* pParentElem, const CHAR_TYPE* pcsDescription) { }
	virtual void onBeforeRedo(IBaseObject* pContext, UIElement* pParentElem) { }
	virtual void onAfterRedo(IBaseObject* pContext, UIElement* pParentElem, const CHAR_TYPE* pcsDescription) { }

private:

	UndoBlock* getLastUndoBlock();
	UndoBlock* getLastOpenUndoBlock();
	void clearAllUndosStartingAt(TUndoBlocksList::iterator& it);

	// Do not use these, if possible - they create an empty undo block, but they don't
	// check to see if they could continue the previous block (since they can't, without
	// knowing which undo item is being added). Instead, always provide at least one undo item
	// so we can check.
	UndoBlock* beginUndoBlock(const char* pcsBlockDesc, bool bAddContainerBlock, const char* pcsContainerId);
	void endUndoBlock(UNIQUEID_TYPE idBlock);

	UndoBlock* getUndoBlockFor(const char* pcsBlockDesc, const char* pcsUndoItemType, bool bAllowContinuation, bool bCreateToolBlockIfNeeded);

	void trimToMaxDepth();

private:

	static IUndoManagerCallback* theCallback;

	// Either a scene or an element must be set. With a scene, we can
	// undo the nodes, with an element, we can undo disconnected UI values
	// such as in dialogs.
	IBaseObject* myParentContext;
	UIElement* myParentElem;

	TUndoBlocksList myTopLevelUndos;
	
	// The iterator pointing to the position just one *after* the currently
	// last undo. So for empty stack, it'll point at end(), for one with
	// U1, U2, U3 it'll point to end(), then after one undo to U3, meaning
	// the next undo to undo (ha!) is U2 (another ha!). Etc.
	TUndoBlocksList::iterator myInsertPositionUndoIterator;

	int myDisableAddingNewUndosStackCounter;

	string myCommonString;
};
/*****************************************************************************/
// A block that automatically begins and ends the undo block. Useful for one-time
// stack operations.
class HYPERUI_API AutoUndoBlock
{
public:
	explicit AutoUndoBlock(const char* pcsBlockDesc, IUndoItem* pItem, Window* pWindow, IBaseObject* pContext, bool bAllowContinuation = true)  { if(pItem) myBlockId = UndoManager::addUndoItemToCurrentManager(pcsBlockDesc, pItem, bAllowContinuation, pWindow, pContext); else myBlockId = -1; myInitWindow = pWindow; myInitContext = pContext; }
	AutoUndoBlock(const char* pcsBlockDesc, IUndoItem*& pItem, Window* pWindow, IBaseObject* pContext, bool bAllowContinuation = true)  { if(pItem) myBlockId = UndoManager::addUndoItemToCurrentManager(pcsBlockDesc, pItem, bAllowContinuation, pWindow, pContext); else myBlockId = -1; myInitWindow = pWindow; myInitContext = pContext; }
	~AutoUndoBlock() { if(myBlockId >= 0) UndoManager::endUndoBlockInCurrentManager(myBlockId, myInitWindow, myInitContext); }

private:
	int myBlockId;
	Window* myInitWindow;
	IBaseObject* myInitContext;
};
/*****************************************************************************/
class HYPERUI_API AutoUndoContainerBlock
{
public:
	AutoUndoContainerBlock();
	explicit AutoUndoContainerBlock(const char* pcsBlockDesc, Window* pWindow, IBaseObject* pContext, const char* pcsContainerSubId = PROPERTY_NONE);
	explicit AutoUndoContainerBlock(const char* pcsBlockDesc, UndoManager* pManager, const char* pcsContainerSubId = PROPERTY_NONE);

	virtual ~AutoUndoContainerBlock() { if(myBlockId >= 0) UndoManager::endUndoBlockInCurrentManager(myBlockId, myInitWindow, myInitContext, myOverrideManager); }

protected:
	int myBlockId;
	Window* myInitWindow;
	IBaseObject* myInitContext;
	UndoManager* myOverrideManager;
};
/*****************************************************************************/
typedef list < UndoManager* > TUndoManagersList;
/*****************************************************************************/