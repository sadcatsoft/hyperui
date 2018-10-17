#pragma once

class CachedClip3D;

#define MAX_SHORTLIST_CLIPS		2

// Meant to be used with a GameObject to blend clips together
/*****************************************************************************/
struct HYPERUI_API SPlayingClipInfo
{
	SPlayingClipInfo()
	{
		myLastValue = 0;
		mySpeedMult = 1.0;
	}

	CachedClip3D* myClip;
	AnimatedValue myValue;
	int myClipIndex;
	
	// Used for pausing
	FLOAT_TYPE myLastValue;
	FLOAT_TYPE mySpeedMult;
};
typedef vector < SPlayingClipInfo > TPlayingClips;
/*****************************************************************************/
class HYPERUI_API Clip3DPlayer
{
public:
	Clip3DPlayer();
	~Clip3DPlayer();

	void init(CachedObject3D* pObj3D);

	void addClip(const char* pcsClipName, bool bLooping, bool bStartPlaying, FLOAT_TYPE fSpeedMult = 1.0);
	void clearPlaying(void);

	int getNumPlaying(void);
	bool getReachedCycleEnd();
	FLOAT_TYPE getProgress(void);
	int getClipIndex(void);

	void onTimerTick(void);

	void pause(void);
	void unpause(void);
	void resetToBeginning();
	bool getIsPaused() { return myIsPaused; }

	void deallocateAllPlaying();

private:

	bool myIsPaused;
	bool myIsUsingShortList;

	SPlayingClipInfo myShortList[MAX_SHORTLIST_CLIPS];
	int myNumShortListClips;

	TPlayingClips* myLongList;

	CachedObject3D *myObject3D;
};
/*****************************************************************************/