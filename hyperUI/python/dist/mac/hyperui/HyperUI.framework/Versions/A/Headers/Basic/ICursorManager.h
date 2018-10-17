#pragma once

/*****************************************************************************/
struct HYPERUI_API SCursorItem
{
	CursorType myCursor;
	UNIQUEID_TYPE myId;
};
typedef vector < SCursorItem > TCursorItems;
/*****************************************************************************/
class HYPERUI_API ICursorManager
{
public:
	ICursorManager()
	{
		//pushCursor(CursorArrow);
	}

	virtual ~ICursorManager() { }
	virtual void setCursor(CursorType eType) = 0;

	UNIQUEID_TYPE pushCursor(CursorType eType)
	{
		if(eType == CursorLastPlaceholder)
			return -1;

		setCursor(eType);

		SCursorItem rItem;
		rItem.myId = theCursorId;
		rItem.myCursor = eType;
		myCursorStack.push_back(rItem);

		theCursorId++;
		return rItem.myId;
	}

	void popCursor(UNIQUEID_TYPE idCursor)
	{
		if(idCursor < 0)
			return;

		//  The one is the first cursor pushed
		int iStackSize = myCursorStack.size();
		if(iStackSize <= 1)
			ASSERT_RETURN;

		if(myCursorStack[iStackSize - 1].myId != idCursor)
		{
			// Popping off an id that isn't last...
			_ASSERT(0);
		}
		else
		{
			myCursorStack.pop_back();
			setCursor(myCursorStack[iStackSize - 2].myCursor);
		}

	}

	void restoreLastCursor()
	{
		setCursor(myCursorStack[myCursorStack.size() - 1].myCursor);
	}

	void getCursorFilePath(const CHAR_TYPE* pcsFileName, bool bAddScreenScale, STRING_TYPE& strOut)
	{
		STRING_TYPE strResSuffix;
		if(bAddScreenScale)
			ResourceManager::getDensityFileSuffix(strResSuffix);

		getAppStartupPath(strOut);
		strOut += FOLDER_SEP;
		strOut += STR_LIT("cursors");
		strOut += strResSuffix;
		strOut += FOLDER_SEP;
		strOut += pcsFileName;
	}

private:

	static UNIQUEID_TYPE theCursorId;
	TCursorItems myCursorStack;
};
/*****************************************************************************/