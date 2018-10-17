#pragma once

class Window;
/*****************************************************************************/
class HYPERUI_API IOffscreenCacher
{
public:
	IOffscreenCacher(Window* pWindow);
	virtual ~IOffscreenCacher();

	inline void enableTextCaching() { myGlobalEnableCaching = true; }
	inline bool getIsCachingEnabled() const { return myGlobalEnableCaching; }

	virtual void createQueued() = 0;

	inline void getGlBitmapSize(const SVector2D& svDimsIn, int& iW, int& iH) const
	{
#if defined(WIN32) || defined(MAC_BUILD)
		iW = forcePowerOfTwo(svDimsIn.x);
		iH = forcePowerOfTwo(svDimsIn.y);
#else	
		iH = forcePowerOfTwo(svDimsIn.x);
		iW = forcePowerOfTwo(svDimsIn.y);
#endif
	}

	void getSrcScreenArea(int iW, int iH, SRect2D& srAreaOut) const;

protected:
	string theSharedString, theSharedString2;

	inline Window* getParentWindow() { return myParentWindow; }

private:

	bool myGlobalEnableCaching;
	Window *myParentWindow;
};
/*****************************************************************************/