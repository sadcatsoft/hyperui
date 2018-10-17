#pragma once

typedef vector < CachedFrame3D* > TCachedFrames3D;

/********************************************************************************************/
class HYPERUI_API CachedClip3D
{
public:

	CachedClip3D();
	~CachedClip3D();

	void setLoadInfo(bool bLoadFromClip, const char* pcsNameToken, Window* pWindow, bool bAutoCenter, bool bFlipU, bool bFlipV, ResourceItem* pClipItem, SVector3D& svDefaultScale, CachedClip3D* pMainClip, bool bFlipAllTris);
	void clear(void);

	void getBBox(SBBox3D& bbox_out);

	CachedFrame3D* getFrameForProgress(FLOAT_TYPE fProgress);
	FLOAT_TYPE getDuration(void);
	const char* getTypeName(void);

	int getDataMemUsage(void);
	int getNumFrames(void);
	CachedFrame3D* getFrame(int iIndex);

	// We can use this to force-load some models, such as simple OBJ files.
	void ensureIsLoaded(void);

	GTIME getLastUseTime(void) { return myLastUseTime; }
	void unloadData(void);

	bool getIsLoaded(void) { return myIsLoaded; }

protected:

	void centerOnOrigin(void);

	// These are protected because we load on demand automatically. The idea is to set all loaded info first, then load/unload as necessary.
	// Note that we only apply the default scale when loading an obj. For clips, we apply them when the .clp file is being converted.
	bool loadFromObjFile(const char* pcsNameToken, Window* pWindow, bool bAutoCenter, bool bFlipU, bool bFlipV, ResourceItem* pClipItem, SVector3D& svDefaultScale, bool bFlipAllTris);
	bool loadFromClipFile(const char* pcsNameToken, Window* pWindow, bool bAutoCenter, bool bFlipU, bool bFlipV, ResourceItem* pClipItem, CachedClip3D* pMainClip);

private:

	TCachedFrames3D myFrames;

#ifndef MAC_BUILD
	static 
#endif
	string theCommonString, theCommonString2;

	// When adding anything here, make sure it's copied in CachedClip3D::loadFromClipFile();
	// Also, these are now used for delayed load.
	FLOAT_TYPE myDuration;
	string myName;
	string myFileName;
	SVector3D myDefaultScale;
	bool myAutoCenter, myFlipU, myFlipV, myFlipAllTris;
	bool myLoadFromClip;
	Window* myWindow;
	ResourceItem* myClipItem;
	CachedClip3D* myMainClip;

	bool myIsLoaded;

	// In universal clock.
	GTIME myLastUseTime;
};
/********************************************************************************************/