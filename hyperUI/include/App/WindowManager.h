#pragma once

#define TIdWindowMap	map < UNIQUEID_TYPE, ITEM_TYPE* >
/*****************************************************************************/
template < class ITEM_TYPE >
class TWindowManager : public IdMapContainer<ITEM_TYPE>, public GlobalIdGeneratorInternal<TWindowManager < ITEM_TYPE > >
{
public:
	virtual ~TWindowManager();
	static TWindowManager* getInstance();

	void onWindowCreated(ITEM_TYPE* pWindow, bool bIsMainWindow);
	void onWindowDestroyed(ITEM_TYPE* pWindow, bool bDeallocate);

	inline void setLastActiveWindow(ITEM_TYPE* pWindow) { myLastActiveWindow = pWindow; }

	inline ITEM_TYPE* getMainWindow() { _ASSERT(myMainWindow != NULL); return myMainWindow; }
	inline ITEM_TYPE* getLastActiveWindow() { return myLastActiveWindow; }

    inline int getNumItems() const { return IdMapContainer<ITEM_TYPE>::myItems.size(); }

	// Iterator
	class Iterator
	{
	public:
		Iterator(const TWindowManager& rWinManager)
		{
			myWinManager = &rWinManager;
			myIterator = myWinManager->myItems.begin();
		}
		Iterator() { myWinManager = NULL; }

		inline void operator++(int) { myIterator++; }
		inline void operator--(int) { myIterator--; }
		inline void toEnd() { myIterator = myWinManager->myItems.end(); }
		inline bool isEnd() const { return myIterator == myWinManager->myItems.end(); }
		inline bool operator==(const Iterator& other) const { return myIterator == other.myIterator && myWinManager == other.myWinManager; }
		inline bool operator!=(const Iterator& other) const { return ! (*this == other); }
		inline ITEM_TYPE* getWindow() { return myIterator->second; }
	private:
		const TWindowManager *myWinManager;
		typename TIdWindowMap::const_iterator myIterator;
	};

	Iterator windowsBegin() const { Iterator rTempIter(*this); return rTempIter; }

private:
	TWindowManager();

private:
	HYPERUI_API static TWindowManager* theInstance;

	ITEM_TYPE* myMainWindow;
	ITEM_TYPE* myLastActiveWindow;
};
/*****************************************************************************/
#include "WindowManager.hpp"
/*****************************************************************************/
