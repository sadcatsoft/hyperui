#include "stdafx.h"

#define SAVE_COLOR_LIST	"colList"

#define PropertySvColStopEnabled			"svColStopEnabled"
#define PropertySvColStopCol				"svColStopCol"

namespace HyperUI
{
/*****************************************************************************/
// SColorListEntry
/*****************************************************************************/
void SColorListEntry::saveToItem(StringResourceItem& rItemOut) const
{
	rItemOut.setBoolProp(PropertySvColStopEnabled, this->myIsEnabled);
	rItemOut.setAsColor(PropertySvColStopCol, myColor);
	//	rItemOut.setAsLong(PropertySaveGradStopId, myId);
}
/*****************************************************************************/
void SColorListEntry::loadFromItem(StringResourceItem& rItemIn)
{
	myIsEnabled = rItemIn.getBoolProp(PropertySvColStopEnabled);
	rItemIn.getAsColor(PropertySvColStopCol, myColor);
	//	myId = rItemIn.getAsLong(PropertySaveGradStopId);
}


/*****************************************************************************/
// SColorList
/*****************************************************************************/
SColorList::SColorList()
{

}
/*****************************************************************************/
bool SColorList::loadFromString(const char* pcsString)
{
	clear();

	if(!IS_VALID_STRING_AND_NOT_NONE(pcsString))
		return false;

	StringResourceCollection rColl;
	mySharedString = pcsString;
	bool bResult = rColl.loadFromString(mySharedString);
	if(!bResult || rColl.getNumItems() < 1)
		return false;

	StringResourceItem* pItem = rColl.getItem(0);
//	myLastId = pItem->getAsLong(PropertySaveLastId);

	SColorListEntry rStop;
	StringResourceItem* pChild;
	int iCurr, iNum = pItem->getNumChildren();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = pItem->getChild(iCurr);
		rStop.loadFromItem(*pChild);
		myColors.push_back(rStop);
	}

	return true;
}
/*****************************************************************************/
void SColorList::saveToString(string& strOut) const
{
	strOut = "";
	StringResourceItem rDummy(SAVE_COLOR_LIST);
	rDummy.setStringProp(STRING_ID_PROP, SAVE_COLOR_LIST);
	//rDummy.setAsLong(PropertySaveLastId, myLastId);

	int iCurr;

	StringResourceItem* pChild;
	char pcsBuff[128];
	TColorListEntries::const_iterator si;
	iCurr = 0;
	for(si = myColors.begin(); si != myColors.end(); si++, iCurr++)
	{
		sprintf(pcsBuff, "c%d", iCurr);
		pChild = rDummy.addChildAndSetId(pcsBuff);
		(*si).saveToItem(*pChild);
	}

	rDummy.saveToString(strOut);
}
/*****************************************************************************/
void SColorList::clear()
{
	myColors.clear();
}
/*****************************************************************************/
void SColorList::toColorVector(TColorVector& vecColors)
{
	vecColors.clear();
	int iCurr, iNum = myColors.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(!myColors[iCurr].myIsEnabled)
			continue;
		vecColors.push_back(myColors[iCurr].myColor);
	}
}
/*****************************************************************************/
};