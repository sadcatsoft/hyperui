#include "stdafx.h"

#define DEFAULT_CLOCK_TYPE		ClockMainGame

namespace HyperUI
{
/*****************************************************************************/
Clip3DPlayer::Clip3DPlayer()
{
	myIsPaused = false;
	myLongList = NULL;
	clearPlaying();
}
/*****************************************************************************/
Clip3DPlayer::~Clip3DPlayer()
{
	clearPlaying();
	delete myLongList;
	myLongList = NULL;
}
/*****************************************************************************/
void Clip3DPlayer::init(CachedObject3D* pObj3D)
{
	myObject3D = pObj3D;
	// Clear to avoid keeping clips from other objects
	clearPlaying();
}
/*****************************************************************************/
void Clip3DPlayer::addClip(const char* pcsClipName, bool bLooping, bool bStartPlaying, FLOAT_TYPE fSpeedMult)
{
	myIsPaused = !bStartPlaying;
	SPlayingClipInfo rDummyInfo;

	// Find the right clip from our object
	rDummyInfo.myClip = myObject3D->findClip(pcsClipName, &rDummyInfo.myClipIndex);
	rDummyInfo.mySpeedMult = fSpeedMult;

	// Set the duration
	AnimOverActionType eOverAction = AnimOverActionNone;
	if(bLooping)
		eOverAction = AnimOverActionReplayForever;
	rDummyInfo.myValue.setAnimation(0, 1, rDummyInfo.myClip->getDuration()*fSpeedMult, DEFAULT_CLOCK_TYPE, eOverAction);

	bool bAddToLong = false;
	if(myIsUsingShortList)
	{
		if(myNumShortListClips >= MAX_SHORTLIST_CLIPS)
		{
			// Copy the short to the long
			if(!myLongList)
				myLongList = new TPlayingClips;
			
			int iCurr;		
			for(iCurr = 0; iCurr < myNumShortListClips; iCurr++)
				myLongList->push_back(myShortList[iCurr]);
			bAddToLong = true;
			myIsUsingShortList = false;
		}
		else
		{
			myShortList[myNumShortListClips] = rDummyInfo;
			myNumShortListClips++;
		}
	}

	if(bAddToLong)
	{
		if(!myLongList)
			myLongList = new TPlayingClips;
		myLongList->push_back(rDummyInfo);
	}
}
/*****************************************************************************/
void Clip3DPlayer::clearPlaying(void)
{
	myIsPaused = false;
	myNumShortListClips = 0;
	if(myLongList)
		myLongList->clear();
	myIsUsingShortList = true;
}
/*****************************************************************************/
int Clip3DPlayer::getNumPlaying(void)
{
	if(myIsUsingShortList)
		return myNumShortListClips;
	else
	{
		_ASSERT(myLongList);
		return myLongList->size();
	}
}
/*****************************************************************************/
int Clip3DPlayer::getClipIndex(void)
{
	// Only valid when playing a single clip
	int iNumPlaying = getNumPlaying();
	_ASSERT(iNumPlaying <= 1);
	if(iNumPlaying <= 0)
		return 0.0;
	else
	{
		GTIME lTime = Application::getInstance()->getGlobalTime(DEFAULT_CLOCK_TYPE);
		return myShortList[0].myClipIndex;
	}
}
/*****************************************************************************/
FLOAT_TYPE Clip3DPlayer::getProgress(void)
{
	// Only valid when playing a single clip
	int iNumPlaying = getNumPlaying();
	_ASSERT(iNumPlaying <= 1);
	if(iNumPlaying <= 0)
		return 0.0;
	else
	{
		return myShortList[0].myLastValue;
//		GTIME lTime = Application::getInstance()->getGlobalTime(DEFAULT_CLOCK_TYPE);
//		return myShortList[0].myValue.getValue(lTime);
	}

}
/*****************************************************************************/
void Clip3DPlayer::onTimerTick(void)
{
	if(myIsPaused)
		return;

	GTIME lTime = Application::getInstance()->getGlobalTime(DEFAULT_CLOCK_TYPE);
	int iCurr;
	if(myIsUsingShortList)
	{
		int iRemoved = 0;
		for(iCurr = myNumShortListClips - 1; iCurr >= 0; iCurr--)
		{
			//myShortList[iCurr].myValue.checkTime(lTime);
			myShortList[iCurr].myLastValue = myShortList[iCurr].myValue.getValue();
			if(myShortList[iCurr].myValue.getIsAnimating(true) == false)
			{
				// We're done, remove us.
				myShortList[iCurr].myClip = NULL;
				iRemoved++;
			}
		}

		// Now actually overwrite any removed clips.
		int iCurr2;
		for(iCurr = 0; iCurr < myNumShortListClips; iCurr++)
		{
			if(myShortList[iCurr].myClip)
				continue;
			for(iCurr2 = iCurr + 1; iCurr2 < myNumShortListClips; iCurr2++)
			{
				myShortList[iCurr2 - 1] = myShortList[iCurr2];
			}
			iCurr--;
		}

		myNumShortListClips -= iRemoved;
	}
	else
	{
		_ASSERT(myLongList);
		int iNum = myLongList->size();
		for(iCurr = iNum - 1; iCurr >= 0; iCurr-- )
		{
			//(*myLongList)[iCurr].myValue.checkTime(lTime);
			if((*myLongList)[iCurr].myValue.getIsAnimating(true) == false)
			{
				myLongList->erase(myLongList->begin() + iCurr);
			}
		}

		
		if(myLongList->size() < MAX_SHORTLIST_CLIPS)
		{
			// Migrate them back?
			myNumShortListClips = myLongList->size();
			for(iCurr = 0; iCurr < myNumShortListClips; iCurr++)
			{
				myShortList[iCurr] = (*myLongList)[iCurr];
			}
			myLongList->clear();
			myIsUsingShortList = true;
		}
	}
}
/*****************************************************************************/
void Clip3DPlayer::pause(void)
{
	myIsPaused = true;
}
/*****************************************************************************/
void Clip3DPlayer::unpause(void)
{
	myIsPaused = false;
}
/*****************************************************************************/
bool Clip3DPlayer::getReachedCycleEnd()
{
	bool bHaveAnimating = false;
	//GTIME lTime = Application::getInstance()->getGlobalTime(DEFAULT_CLOCK_TYPE);
	int iCurr;
	if(myIsUsingShortList)
	{
		int iRemoved = 0;
		for(iCurr = myNumShortListClips - 1; iCurr >= 0; iCurr--)
		{
			bHaveAnimating |= myShortList[iCurr].myValue.getIsAnimating(false);
			if(bHaveAnimating)
				return false;
		}
	}
	else
	{
		_ASSERT(myLongList);
		int iNum = myLongList->size();
		for(iCurr = iNum - 1; iCurr >= 0; iCurr-- )
		{
			bHaveAnimating |= (*myLongList)[iCurr].myValue.getIsAnimating(false);
			if(bHaveAnimating)
				return false;
		}
	}

	return true;
}
/*****************************************************************************/
void Clip3DPlayer::resetToBeginning()
{
	bool bHaveAnimating = false;
	GTIME lTime = Application::getInstance()->getGlobalTime(DEFAULT_CLOCK_TYPE);
	int iCurr;
	if(myIsUsingShortList)
	{
		int iRemoved = 0;
		for(iCurr = myNumShortListClips - 1; iCurr >= 0; iCurr--)
		{			
			myShortList[iCurr].myLastValue = 0;
			myShortList[iCurr].myValue.setAnimation(0, 1, myShortList[iCurr].myClip->getDuration()*myShortList[iCurr].mySpeedMult, DEFAULT_CLOCK_TYPE, myShortList[iCurr].myValue.getAnimOverAction(), NULL);
		}
	}
	else
	{
		_ASSERT(myLongList);
		int iNum = myLongList->size();
		for(iCurr = iNum - 1; iCurr >= 0; iCurr-- )
		{
			(*myLongList)[iCurr].myLastValue = 0;
			(*myLongList)[iCurr].myValue.setAnimation(0, 1, (*myLongList)[iCurr].myClip->getDuration()*(*myLongList)[iCurr].mySpeedMult, DEFAULT_CLOCK_TYPE, (*myLongList)[iCurr].myValue.getAnimOverAction(), NULL);
		}
	}


}
/*****************************************************************************/
void Clip3DPlayer::deallocateAllPlaying()
{
	int iCurr;
	if(myIsUsingShortList)
	{
		int iRemoved = 0;
		for(iCurr = myNumShortListClips - 1; iCurr >= 0; iCurr--)
		{
			myShortList[iCurr].myClip->unloadData();
		}
	}
	else
	{
		_ASSERT(myLongList);
		int iNum = myLongList->size();
		for(iCurr = iNum - 1; iCurr >= 0; iCurr-- )
		{
			(*myLongList)[iCurr].myClip->unloadData();
		}
	}

	clearPlaying();
}
/*****************************************************************************/
};