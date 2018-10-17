#pragma once

class HYPERUI_API UndoManager;
class HYPERUI_API UndoBlock;
typedef vector < UndoBlock* > TUndoBlocks;
typedef list < UndoBlock* > TUndoBlocksList;
/*****************************************************************************/
class HYPERUI_API IUndoItem
{
public:
	virtual ~IUndoItem() { }

	virtual void undo(IBaseObject* pContext, UIElement* pElement) = 0;
	virtual void redo(IBaseObject* pContext, UIElement* pElement) = 0;

	virtual const char* getType() = 0;
	virtual void generateDescriptiveString(string& strOut) { strOut = ""; }

	virtual void onBeforeDelete(IBaseObject* pOptParentContext) { }

	virtual ResourceItem* saveToItem(ResourceItem& rItem) 
	{ 	
		ResourceItem* pChild = rItem.addChild("undoBlock"); 
		pChild->setStringProp(PropertyId, this->getType()); 
		string strTemp;
		this->generateDescriptiveString(strTemp);
		if(strTemp.length() > 0)
			pChild->setStringProp(PropertyText, strTemp.c_str());
		return pChild; 
	}
};
typedef vector < IUndoItem* > TUndoItems;
/*****************************************************************************/
// An undo block is also an undo item, because we need the right sequence of
// blocks and items while undoing.
class HYPERUI_API UndoBlock : public IUndoItem, public GlobalIdGeneratorInternal< UndoBlock >
{
public:
	UndoBlock(UndoBlock* pParent, UndoManager* pParentManager, const char* pcsBlockTitle);
	virtual ~UndoBlock();
	virtual void onBeforeDelete(IBaseObject* pOptParentContext);

	virtual void undo(IBaseObject* pContext, UIElement* pElem);
	virtual void redo(IBaseObject* pContext, UIElement* pElem);
	virtual const char* getType() { return "__undoBlock__"; }

	bool isEmpty();
	bool isOpen() { return myIsOpen; }
	bool wasAutoclosed() { return myWasAutoclosed; }

	UndoBlock* getLastOpenBlock();
	UndoBlock* getLastClosedUndoBlock();

	void reopen() { _ASSERT(!myIsOpen); myIsOpen = true; }
	void close() { _ASSERT(myIsOpen || myWasAutoclosed); myIsOpen = false; }
	void autoClose() { _ASSERT(myIsOpen); myWasAutoclosed = true; myIsOpen = false; }

	void addItem(IUndoItem* pItem);

	UndoBlock* getParentBlock() { return myParentBlock; }
	UndoManager* getParentManager() { return myParentManager; }

	void cancelBlock(UndoBlock* pChildBlock);

	bool canContinue(GTIME lTime, const char* pcsUndoType);
	virtual ResourceItem* saveToItem(ResourceItem& rItem);

	inline const char* getTitle() { return myTitle.c_str(); }

	virtual void generateDescriptiveString(string& strOut) { strOut = myTitle; }

protected:

	IUndoItem* getLastItem();

private:

	TUndoItems myUndoItems;

	bool myIsOpen;
	string myTitle;

	bool myWasAutoclosed;

	UndoBlock* myParentBlock;
	UndoManager* myParentManager;

	GTIME myCreationTime;
};
/*****************************************************************************/
