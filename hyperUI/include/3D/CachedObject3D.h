#pragma once

class CachedClip3D;

//typedef vector < CachedFrame3D* > TCachedFrames3D;
typedef vector < CachedClip3D* > TCachedClips3D;

/********************************************************************************************/
class HYPERUI_API CachedObject3D
{
public:

	CachedObject3D();
	~CachedObject3D();

	bool loadFromItem(ResourceItem* pItem, Window* pWindow);
	void clear(void);

	void getDefaultScale(SVector3D& svOut) { svOut = myDefaultScale; }

	// Note: we can't do currently playing, since this is a definition,
	// not the actual object instance.
	CachedFrame3D* getFrameForProgress(FLOAT_TYPE fProgress, int iClipIndex);

	const char* getTypeName(void) { return myTypeName.c_str(); }

	void getDiffuseMults(CachedPart3D* pPart, SColor& scolOut);
	void getAmbientMults(CachedPart3D* pPart, SColor& scolOut);
	void getSpecularParms(CachedPart3D* pPart, SColor& scolOut, FLOAT_TYPE& fSpecPowerOut);
	TX_MAN_RETURN_TYPE getNormalMapIndex() { return myNormalMapOverride; }
	TX_MAN_RETURN_TYPE getSpecularMapIndex() { return mySpecularMapOverride; }

	CachedClip3D* findClip(const char* pcsName, int* iClipIndexOut);

	int getNumClips(void) { return myClips.size(); }
	CachedClip3D* getClip(int iIndex) { return myClips[iIndex]; }

	void onTimerTick(void);

private:

	TCachedClips3D myClips;

#ifndef MAC_BUILD
	static 
#endif
	string theCommonString, theCommonString2;
	SVector3D myDefaultScale;

	SColor myDiffuseMults, myAmbientMults, mySpecular;
	FLOAT_TYPE mySpecularPower;

	string myTypeName;

	// May override the individual parts
	TX_MAN_RETURN_TYPE myNormalMapOverride;
	TX_MAN_RETURN_TYPE mySpecularMapOverride;


};
/********************************************************************************************/