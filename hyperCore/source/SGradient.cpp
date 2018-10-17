#include "stdafx.h"
#define SAVE_GRADIENT	"grad"

#define PropertyStringSvLastId				"svLastId"

namespace HyperCore
{
/*****************************************************************************/
SGradient::SGradient()
{
#ifdef ALLOW_GRAD_CACHING
	myCachedColorStopsArray = NULL;
	myCachedOpacityStopsArray = NULL;
	myCachedColorArrayLen = 0;
	myCachedOpacityArrayLen = 0;
#endif
	myLastId = -1;
//	setColorAndOpacityStop(0, 1,1,1,1);
}
/*****************************************************************************/
SGradient::SGradient(const SGradient& rOther)
{
#ifdef ALLOW_GRAD_CACHING
	myCachedColorStopsArray = NULL;
	myCachedOpacityStopsArray = NULL;
#endif
	copyFrom(rOther);
}
/*****************************************************************************/
SGradient::~SGradient()
{
	invalidateColorStopsCache();
	invalidateOpacityStopsCache();
}
/*****************************************************************************/
bool SGradient::loadFromString(const char* pcsString)
{
	clear();

	if(!IS_VALID_STRING_AND_NOT_NONE(pcsString))
		return false;

	StringResourceCollection rColl;
	// DO NOT move this to the class definition! On Mac,
	// the headers are compiled by GCC & LLVM separately,
	// and thus will crash if we try to use it that way.
	string strTempCopy;
	strTempCopy = pcsString;
	bool bResult = rColl.loadFromString(strTempCopy);
	if(!bResult || rColl.getNumItems() < 1)
		return false;

	StringResourceItem* pItem = rColl.getItem(0);
	myLastId = pItem->getAsLong(PropertyStringSvLastId);

	SGradientStop rStop;
	StringResourceItem* pChild;
	int iCurr, iNum = pItem->getNumChildren();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pChild = pItem->getChild(iCurr);
		rStop.loadFromItem(*pChild);

		if(rStop.myIsOpacityStop)
			myOpacityStops.insert(rStop);
		else
			myColorStops.insert(rStop);
	}

	return true;
}
/*****************************************************************************/
void SGradient::saveToString(string& strOut) const
{
	strOut = "";
	StringResourceItem rDummy(SAVE_GRADIENT);
	rDummy.setStringProp(STRING_ID_PROP, SAVE_GRADIENT);
	rDummy.setAsLong(PropertyStringSvLastId, myLastId);

	int iCurr;

	StringResourceItem* pChild;
	char pcsBuff[128];
	TGradientStops::const_iterator si;
	iCurr = 0;
	for(si = myColorStops.begin(); si != myColorStops.end(); si++, iCurr++)
	{
		sprintf(pcsBuff, "sc%d", iCurr);
		pChild = rDummy.addChildAndSetId(pcsBuff);
		(*si).saveToItem(*pChild);
	}
	iCurr = 0;
	for(si = myOpacityStops.begin(); si != myOpacityStops.end(); si++, iCurr++)
	{
		sprintf(pcsBuff, "so%d", iCurr);
		pChild = rDummy.addChildAndSetId(pcsBuff);
		(*si).saveToItem(*pChild);
	}

	rDummy.saveToString(strOut);
}
/*****************************************************************************/
void SGradient::setColorAndOpacityStop(FLOAT_TYPE fPos, const SColor& scol)
{
	setColorAndOpacityStop(fPos, scol.r, scol.g, scol.b, scol.alpha);
}
/*****************************************************************************/
void SGradient::changeStopOpacityTo(UNIQUEID_TYPE lId, FLOAT_TYPE fOpacity)
{
	SGradientStop* pStop = internalFindStopById(lId);
	if(!pStop)
		ASSERT_RETURN;
	pStop->myColor.alpha = HyperCore::clampToRange<FLOAT_TYPE>(0, 1, fOpacity);
	invalidateOpacityStopsCache();
}
/*****************************************************************************/
void SGradient::changeStopColorTo(UNIQUEID_TYPE lId, const SColor& scolTemp)
{
	SGradientStop* pStop = internalFindStopById(lId);
	if(!pStop)
		ASSERT_RETURN;
	pStop->myColor.set(HyperCore::clampToRange<FLOAT_TYPE>(0, 1, scolTemp.r), HyperCore::clampToRange<FLOAT_TYPE>(0, 1, scolTemp.g), HyperCore::clampToRange<FLOAT_TYPE>(0, 1, scolTemp.b), 1.0);
	invalidateColorStopsCache();
}
/*****************************************************************************/
void SGradient::changeSecondaryStopColorTo(UNIQUEID_TYPE lId, const SColor& scolTemp)
{
	SGradientStop* pStop = internalFindStopById(lId);
	if(!pStop)
		ASSERT_RETURN;
	pStop->mySecondaryColor.set(HyperCore::clampToRange<FLOAT_TYPE>(0, 1, scolTemp.r), HyperCore::clampToRange<FLOAT_TYPE>(0, 1, scolTemp.g), HyperCore::clampToRange<FLOAT_TYPE>(0, 1, scolTemp.b), 1.0);
	invalidateColorStopsCache();
}
/*****************************************************************************/
UNIQUEID_TYPE SGradient::setColorStop(FLOAT_TYPE fPos, FLOAT_TYPE fR, FLOAT_TYPE fG, FLOAT_TYPE fB)
{
	SGradientStop rFindStop;
	rFindStop.setPos(fPos);
	TGradientStops::iterator si = myColorStops.find(rFindStop);
	SGradientStop rStop;
	if(si != myColorStops.end())
	{
		rStop.myId = (*si).myId;
		myColorStops.erase(si);
	}
	else	
		rStop.myId = getNextId();

	rStop.setPos(fPos);
	rStop.myIsOpacityStop = false;
	rStop.myColor.set(fR, fG, fB, 1.0);
	myColorStops.insert(rStop);

	invalidateColorStopsCache();

	return rStop.myId;
}
/*****************************************************************************/
UNIQUEID_TYPE SGradient::setOpacityStop(FLOAT_TYPE fPos, FLOAT_TYPE fAlpha)
{

	SGradientStop rFindStop;
	rFindStop.setPos(fPos);
	TGradientStops::iterator si = myOpacityStops.find(rFindStop);
	SGradientStop rStop;
	if(si != myOpacityStops.end())
	{
		rStop.myId = (*si).myId;
		myOpacityStops.erase(si);
	}
	else	
		rStop.myId = getNextId();

	rStop.setPos(fPos);
	rStop.myIsOpacityStop = true;
	rStop.myColor.set(-1, -1, -1, fAlpha);
	myOpacityStops.insert(rStop);

	invalidateOpacityStopsCache();

	return rStop.myId;
}
/*****************************************************************************/
void SGradient::setColorAndOpacityStop(FLOAT_TYPE fPos, FLOAT_TYPE fR, FLOAT_TYPE fG, FLOAT_TYPE fB, FLOAT_TYPE fAlpha)
{
	setColorStop(fPos, fR, fG, fB);
	setOpacityStop(fPos, fAlpha);
}
/*****************************************************************************/
inline void interpolateArray(FLOAT_TYPE fPos, const SGradientStop* pSrcArray, int iArrayLen, SColor& scolOut)
{
	const SColor* pColor1;
	FLOAT_TYPE fInterp;
	int iArrayPos;
	int iNext;
	const SGradientStop *pStop;
	const SGradientStop *pStopNext;
	for(iArrayPos = 0; iArrayPos < iArrayLen; iArrayPos++)
	{
		pStop = &pSrcArray[iArrayPos];
		if(fPos > pStop->getPos())
			continue;

		if(iArrayPos == 0)
			scolOut = pStop->myColor;
		else
		{
			// Interpolate
			iNext = iArrayPos - 1;
			pStopNext = &pSrcArray[iNext];
			fInterp = (fPos - pStopNext->getPos())/(pStop->getPos() - pStopNext->getPos());

			if(pStop->myIsSplit)
				pColor1 = &(pStop->mySecondaryColor);
			else
				pColor1 = &(pStop->myColor);
			scolOut = pStopNext->myColor*(1.0 - fInterp) + (*pColor1)*fInterp;
		}
		return;
	}

	if(iArrayLen > 0)
		scolOut = pSrcArray[iArrayLen - 1].myColor;
}

inline void interpolateArray(FLOAT_TYPE fPos, const SGradientStop* pSrcArray, int iArrayLen, FLOAT_TYPE& fOpacityOut)
{
	const SColor* pColor1;
	FLOAT_TYPE fInterp;
	int iArrayPos;
	int iNext;
	const SGradientStop *pStop;
	const SGradientStop *pStopNext;
	for(iArrayPos = 0; iArrayPos < iArrayLen; iArrayPos++)
	{
		pStop = &pSrcArray[iArrayPos];
		if(fPos > pStop->getPos())
			continue;

		if(iArrayPos == 0)
			fOpacityOut = pStop->myColor.alpha;
		else
		{
			// Interpolate
			iNext = iArrayPos - 1;
			pStopNext = &pSrcArray[iNext];
			fInterp = (fPos - pStopNext->getPos())/(pStop->getPos() - pStopNext->getPos());

			if(pStop->myIsSplit)
				pColor1 = &(pStop->mySecondaryColor);
			else
				pColor1 = &(pStop->myColor);
			fOpacityOut = pStopNext->myColor.alpha*(1.0 - fInterp) + pColor1->alpha*fInterp;
		}
		return;
	}

	if(iArrayLen > 0)
		fOpacityOut = pSrcArray[iArrayLen - 1].myColor.alpha;
}


void SGradient::interpFor(FLOAT_TYPE fPos, bool bIsDoingColor, SColor& scolOut) const
{
	// Walk our set until we find a grad stop just behind it.
	//scolOut.set(1,1,1,1);
// 	if(vecStops.size() <= 0)
// 		return;

	const SColor* pColor1;
	FLOAT_TYPE fInterp;

	if(!myCachedColorStopsArray)
		ensureColorStopsCacheValid();
	if(!myCachedOpacityStopsArray)
		ensureOpacityStopsCacheValid();
#ifdef ALLOW_GRAD_CACHING
	//bool bIsDoingColor = (&vecStops) == &myColorStops;
	const SGradientStop* pSrcArray = bIsDoingColor ? myCachedColorStopsArray : myCachedOpacityStopsArray;
	int iArrayLen = bIsDoingColor ? myCachedColorArrayLen : myCachedOpacityArrayLen;

	if(bIsDoingColor)
		interpolateArray(fPos, pSrcArray, iArrayLen, scolOut);
	else
		interpolateArray(fPos, pSrcArray, iArrayLen, scolOut.alpha);

	/*
	int iArrayPos;
	int iNext;
	const SGradientStop *pStop;
	const SGradientStop *pStopNext;
	for(iArrayPos = 0; iArrayPos < iArrayLen; iArrayPos++)
	{
		pStop = &pSrcArray[iArrayPos];
		if(fPos > pStop->getPos())
			continue;

		if(iArrayPos == 0)
			scolOut = pStop->myColor;
		else
		{
			// Interpolate
			iNext = iArrayPos - 1;
			pStopNext = &pSrcArray[iNext];
			fInterp = (fPos - pStopNext->getPos())/(pStop->getPos() - pStopNext->getPos());

			if(pStop->myIsSplit)
				pColor1 = &(pStop->mySecondaryColor);
			else
				pColor1 = &(pStop->myColor);

			scolOut = pStopNext->myColor*(1.0 - fInterp) + (*pColor1)*fInterp;
			scolOut.alpha = pStopNext->myColor.alpha*(1.0 - fInterp) + pColor1->alpha*fInterp;
		}
		return;
	}
	*/
#else
	TGradientStops::const_iterator si, si_next;
	for(si = vecStops.begin(); si != vecStops.end(); si++)
	{
		if(fPos <= (*si).getPos())
		{
			if(si == vecStops.begin())
				scolOut = (*si).myColor;
			else
			{
				// Interpolate
				si_next = si--;
				fInterp = (fPos - (*si_next).getPos())/((*si).getPos() - (*si_next).getPos());

				if((*si).myIsSplit)
					pColor1 = &((*si).mySecondaryColor);
				else
					pColor1 = &((*si).myColor);

				scolOut = (*si_next).myColor*(1.0 - fInterp) + (*pColor1)*fInterp;
				scolOut.alpha = (*si_next).myColor.alpha*(1.0 - fInterp) + pColor1->alpha*fInterp;
			}
			return;
		}
	}

	// Set to the last one
	si = vecStops.end();
	si--;
	scolOut = (*si).myColor;
#endif
}
/*****************************************************************************/
void SGradient::getColorAndOpacityFor(FLOAT_TYPE fPos, SColor& scolOut) const
{
//	SColor scolTempOp;
	scolOut.set(1,1,1,1);
	interpFor(fPos, true, scolOut);
	interpFor(fPos, false, scolOut); // scolTempOp);

	//scolOut.alpha = scolTempOp.alpha;
}
/*****************************************************************************/
void SGradient::clear(void)
{
	myLastId = -1;
	myColorStops.clear();
	myOpacityStops.clear();

	invalidateColorStopsCache();
	invalidateOpacityStopsCache();
}
/*****************************************************************************/
SGradient::StopsIterator SGradient::colorStopsBegin() const
{
	StopsIterator rTempIter(*this, true); 
	return rTempIter; 
}
/*****************************************************************************/
SGradient::StopsIterator SGradient::opacityStopsBegin() const
{
	StopsIterator rTempIter(*this, false); 
	return rTempIter; 
}
/*****************************************************************************/
void SGradient::getSingleStopsArray(TGradientStops& rStopsOut) const
{
	rStopsOut = myColorStops;

	// Now, go and insert all the opacity stops
	SColor scolTemp;
	TGradientStops::iterator fi;
	TGradientStops::const_iterator si;
	SGradientStop* pStop;
	SColor scolTempOp;

	// Go over every stop and duplicate the ones with split colors
	for(fi = rStopsOut.begin(); fi != rStopsOut.end();)
	{
		pStop = const_cast<SGradientStop*>(&(*fi));
		if(pStop->myIsSplit)
		{
			pStop->myIsSplit = false;
			
			SGradientStop rStop;
			getColorAndOpacityFor(pStop->getPos(), rStop.myColor);
			rStop.setPos(pStop->getPos() + (1.0/SGRAD_MULT_FACTOR));
			rStop.myIsOpacityStop = false;
			rStop.myId = pStop->myId;
			rStop.myColor = pStop->mySecondaryColor;
			rStopsOut.insert(rStop);
			fi = rStopsOut.begin();
		}
		else
			fi++;
	}

	// First, go over every color stop and ask for opacity
	for(fi = rStopsOut.begin(); fi != rStopsOut.end(); fi++)
	{
		interpFor((*fi).getPos(), false, scolTempOp);
		pStop = const_cast<SGradientStop*>(&(*fi));
		pStop->myColor.alpha = scolTempOp.alpha;
	}

	for(si = myOpacityStops.begin(); si != myOpacityStops.end(); si++)
	{
		fi = rStopsOut.find(*si);
		if(fi == rStopsOut.end())
		{
			// No such stop. Interpolate the color and insert it.
			SGradientStop rStop;
			getColorAndOpacityFor((*si).getPos(), rStop.myColor);
			rStop.setPos((*si).getPos());
			rStop.myIsOpacityStop = false;
			rStop.myId = (*si).myId;
			rStopsOut.insert(rStop);
		}
		else
		{
			// This stop exists as a color. Set its opacity.
			pStop = const_cast<SGradientStop*>(&(*fi));
			pStop->myColor.alpha = (*si).myColor.alpha;
		}
	}
}
/*****************************************************************************/
SGradientStop* SGradient::internalFindStopById(UNIQUEID_TYPE lId)
{
	TGradientStops::iterator si;
	for(si = myColorStops.begin(); si != myColorStops.end(); si++)
	{
		if((*si).myId == lId)
			return const_cast<SGradientStop*>(&(*si));
	}

	for(si = myOpacityStops.begin(); si != myOpacityStops.end(); si++)
	{
		if((*si).myId == lId)
			return const_cast<SGradientStop*>(&(*si));
	}

	return NULL;
}
/*****************************************************************************/
const SGradientStop* SGradient::findStopById(UNIQUEID_TYPE lId) const
{
	TGradientStops::const_iterator si;
	for(si = myColorStops.begin(); si != myColorStops.end(); si++)
	{
		if((*si).myId == lId)
			return &(*si);
	}

	for(si = myOpacityStops.begin(); si != myOpacityStops.end(); si++)
	{
		if((*si).myId == lId)
			return &(*si);
	}

	return NULL;
}
/*****************************************************************************/
void SGradient::removeStop(UNIQUEID_TYPE lId, SGradientStop* pOptStopCopyOut)
{
	TGradientStops::iterator si;
	for(si = myColorStops.begin(); si != myColorStops.end(); si++)
	{
		if((*si).myId == lId)
		{
			if(pOptStopCopyOut)
				*pOptStopCopyOut = *si;
			myColorStops.erase(si);
			invalidateColorStopsCache();
			return;
		}
	}

	for(si = myOpacityStops.begin(); si != myOpacityStops.end(); si++)
	{
		if((*si).myId == lId)
		{
			if(pOptStopCopyOut)
				*pOptStopCopyOut = *si;
			myOpacityStops.erase(si);
			invalidateOpacityStopsCache();
			return;
		}
	}
}
/*****************************************************************************/
void SGradient::moveStopTo(UNIQUEID_TYPE lId, FLOAT_TYPE fNewPos)
{
	fNewPos = HyperCore::clampToRange<FLOAT_TYPE>(0.0, 1.0, fNewPos);

	TGradientStops* pSetPtr = findStopById(lId)->myIsOpacityStop ? &myOpacityStops : &myColorStops;

	// We should never allow the stops to be removed here if one
	// falls exactly onto another, because we could be dragging a stop - 
	// and we can't let it disappear. So instead what we do is
	// snap it to the nearest percent.
	SGradientStop rFindStop;
	rFindStop.setPos(fNewPos);
	TGradientStops::iterator si = pSetPtr->find(rFindStop);
	while(si != pSetPtr->end() && (*si).myId != lId)
	{

		// Set the new pos
		fNewPos = ((int)(fNewPos*100.0) - 1)/100.0;
		if(fNewPos < 0.0)
			break;
		rFindStop.setPos(fNewPos);
		si = pSetPtr->find(rFindStop);
	}
	while(si != pSetPtr->end() && (*si).myId != lId)
	{
		// Set the new pos
		fNewPos = ((int)(fNewPos*100.0) + 1)/100.0;
		if(fNewPos > 1.0)
			break;
		rFindStop.setPos(fNewPos);
		si = pSetPtr->find(rFindStop);
	}

	if(si != pSetPtr->end() && (*si).myId != lId)
		ASSERT_RETURN;
	
	SGradientStop rCopy;
	removeStop(lId, &rCopy);
	rCopy.setPos(fNewPos);
	rCopy.myId = lId;
	pSetPtr->insert(rCopy);

	if(pSetPtr == &myOpacityStops)
		invalidateOpacityStopsCache();
	else
		invalidateColorStopsCache();

// 	if(rCopy.myIsOpacityStop)
// 		myOpacityStops.insert(rCopy);
// 	else
// 		myColorStops.insert(rCopy);
}
/*****************************************************************************/
void SGradient::mirror(SideType eSide, bool bDoOpacity)
{
	if(eSide != SideLeft && eSide != SideRight)
		ASSERT_RETURN;

	// Now, take the things on the "from" side and mirror
	// them on the right. Pay special attention to things 
	// exactly in the middle.
	TGradientStops* pSetPtr = bDoOpacity ? &myOpacityStops : &myColorStops;

	// First, delete everything on one side
	TGradientStops::iterator si;
	TGradientStops::iterator si_prev;
	bool bRemove;
	for(si = pSetPtr->begin(); si != pSetPtr->end(); )
	{
		bRemove = false;
		if(eSide == SideLeft && (*si).getPos() > 0.5)
			bRemove = true;
		else if(eSide == SideRight && (*si).getPos() < 0.5)
			bRemove = true;

		if(bRemove)
		{
			pSetPtr->erase(si);
			si = pSetPtr->begin();
		}
		else
			si++;
	}

	// Now, copy the rest mirroring their positions
	TGradientStops rNewStops;
	SGradientStop rStop;
	SColor scolTemp;
	for(si = pSetPtr->begin(); si != pSetPtr->end(); si++)
	{
		// Do not mirror the middle guy.
		if( fabs((*si).getPos() - 0.50) < FLOAT_EPSILON)
			continue;

		rStop = *si;
		rStop.setPos(1.0 - rStop.getPos());
		rStop.myId = getNextId();
		if(!bDoOpacity && rStop.myIsSplit)
		{
			scolTemp = rStop.mySecondaryColor;
			rStop.mySecondaryColor = rStop.myColor;
			rStop.myColor = scolTemp;
		}
		rNewStops.insert(rStop);
	}

	pSetPtr->insert(rNewStops.begin(), rNewStops.end());

	invalidateColorStopsCache();
	invalidateOpacityStopsCache();

	/* 
	// NOOOOO. Here we're copying only one half of the graident
	if(!bDoOpacity)
	{
		// We must also flip any two-sided stops.
		SGradientStop* pStop;
		SColor scolTemp;
		for(si = pSetPtr->begin(); si != pSetPtr->end(); si++)
		{
			pStop = const_cast<SGradientStop*>(&(*si));
			if(pStop->myIsSplit)
			{
				scolTemp = pStop->mySecondaryColor;
				pStop->mySecondaryColor = pStop->myColor;
				pStop->myColor = scolTemp;
			}
		}
	}
	*/
}
/*****************************************************************************/
void SGradient::setIsStopSplit(UNIQUEID_TYPE lId, bool bIsSplit)
{
	SGradientStop* pStop = internalFindStopById(lId);
	if(!pStop)
		ASSERT_RETURN;
	pStop->myIsSplit = bIsSplit;
	invalidateOpacityStopsCache();
	invalidateColorStopsCache();
}
/*****************************************************************************/
void SGradient::swapKeyColors(UNIQUEID_TYPE lId)
{
	SGradientStop* pStop = internalFindStopById(lId);
	if(!pStop)
		ASSERT_RETURN;

	SColor scolTemp = pStop->mySecondaryColor;
	pStop->mySecondaryColor = pStop->myColor;
	pStop->myColor = scolTemp;

	invalidateOpacityStopsCache();
	invalidateColorStopsCache();
}
/*****************************************************************************/
void SGradient::invalidateColorStopsCache()
{
#ifdef ALLOW_GRAD_CACHING
	SCOPED_MUTEX_LOCK(&myCacheLock);
	if(myCachedColorStopsArray)
		delete[] myCachedColorStopsArray;
	myCachedColorStopsArray = NULL;
	myCachedColorArrayLen = 0;
#endif
}
/*****************************************************************************/
void SGradient::invalidateOpacityStopsCache()
{
#ifdef ALLOW_GRAD_CACHING
	SCOPED_MUTEX_LOCK(&myCacheLock);
	if(myCachedOpacityStopsArray)
		delete[] myCachedOpacityStopsArray;
	myCachedOpacityStopsArray = NULL;
	myCachedOpacityArrayLen = 0;
#endif
}
/*****************************************************************************/
void SGradient::ensureColorStopsCacheValid() const
{
#ifdef ALLOW_GRAD_CACHING

	// Cache being non-null means we're up to date.
	if(myCachedColorStopsArray)
		return;

	SCOPED_MUTEX_LOCK(&myCacheLock);
	if(myCachedColorStopsArray)
		return;

	_ASSERT(myColorStops.size() > 0);
	myCachedColorArrayLen = myColorStops.size();
	SGradientStop *pTempPtr = new SGradientStop[myCachedColorArrayLen];
	convertSetToArray(myColorStops, pTempPtr);
	myCachedColorStopsArray = pTempPtr;
#endif
}
/*****************************************************************************/
void SGradient::ensureOpacityStopsCacheValid() const
{
#ifdef ALLOW_GRAD_CACHING
	// Cache being non-null means we're up to date.
	if(myCachedOpacityStopsArray)
		return;

	SCOPED_MUTEX_LOCK(&myCacheLock);
	if(myCachedOpacityStopsArray)
		return;

	_ASSERT(myOpacityStops.size() > 0);
	myCachedOpacityArrayLen = myOpacityStops.size();
	SGradientStop *pTempPtr = new SGradientStop[myCachedOpacityArrayLen];
	convertSetToArray(myOpacityStops, pTempPtr);
	myCachedOpacityStopsArray = pTempPtr;
#endif
}
/*****************************************************************************/
void SGradient::convertSetToArray(const TGradientStops& setStops, SGradientStop* pArrayOut) const
{
	int iArrayPos;
	TGradientStops::const_iterator si;
	for(si = setStops.begin(), iArrayPos = 0; si != setStops.end(); si++, iArrayPos++)
		pArrayOut[iArrayPos] = *si;
}
/*****************************************************************************/
void SGradient::copyFrom(const SGradient& rOther)
{
	clear();

	myLastId = rOther.myLastId;

	myColorStops = rOther.myColorStops;
	myOpacityStops = rOther.myOpacityStops;

	invalidateOpacityStopsCache();
	invalidateColorStopsCache();
}
/*****************************************************************************/
};