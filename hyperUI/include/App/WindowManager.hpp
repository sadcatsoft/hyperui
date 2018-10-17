/*****************************************************************************/
template < class ITEM_TYPE >
TWindowManager<ITEM_TYPE>::TWindowManager()
{
	myLastActiveWindow = NULL;
	myMainWindow = NULL;
}
/*****************************************************************************/
template < class ITEM_TYPE >
TWindowManager<ITEM_TYPE>::~TWindowManager()
{

}
/*****************************************************************************/
template < class ITEM_TYPE >
TWindowManager<ITEM_TYPE>* TWindowManager<ITEM_TYPE>::getInstance()
{
	if(!theInstance)
		theInstance = new TWindowManager<ITEM_TYPE>;
	return theInstance;
}
/*****************************************************************************/
template < class ITEM_TYPE >
void TWindowManager<ITEM_TYPE>::onWindowCreated(ITEM_TYPE* pWindow, bool bIsMainWindow)
{
	if(bIsMainWindow)
	{
		_ASSERT(!myMainWindow);
		myMainWindow = pWindow;
	}

	pWindow->setId(this->generateId());
	this->addNewItem(pWindow);
#ifdef _DEBUG
	gLog("Added window to window manager %x this = %x id = %ld num items = %d\n", pWindow, this, pWindow->getId(), this->myItems.size());
#endif
}
/*****************************************************************************/
template < class ITEM_TYPE >
void TWindowManager<ITEM_TYPE>::onWindowDestroyed(ITEM_TYPE* pWindow, bool bDeallocate)
{
	if(!pWindow)
		return;

	if(myMainWindow == pWindow)
		myMainWindow = NULL;
	if(myLastActiveWindow == pWindow)
		myLastActiveWindow = NULL;

#ifdef _DEBUG
	UNIQUEID_TYPE lId = pWindow->getId();
	void* pPtr = pWindow;
#endif
	this->removeByPointer(pWindow, bDeallocate);
#ifdef _DEBUG
	gLog("Removed window from window manager %x this = %x id = %ld num items = %d\n", pPtr, this, lId, this->myItems.size());
#endif
}
/*****************************************************************************/
