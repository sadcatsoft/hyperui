#pragma once

/*****************************************************************************/
class HYPERUI_API IActivityManagerCallback
{
public:
	~IActivityManagerCallback() { }

	virtual bool getIsActive() = 0;
};
/*****************************************************************************/
class HYPERUI_API ActivityManager
{
public:
	virtual ~ActivityManager();
	static ActivityManager* getInstance();

	inline void singleValueChanged() { myHaveSingleValueChange = true; myExtraRenderTicksCounter = 2; }

	void beginAnimation(const char* pcsOptDebugTag = NULL);
	void endAnimation(const char* pcsOptDebugTag = NULL);

	inline bool getIsActive() { return getIsActiveNoExtraDrawsCounter() || myExtraRenderTicksCounter > 0; }
	bool getIsActiveNoExtraDrawsCounter();
	inline void onScreenRedrawn() { myHaveSingleValueChange = false; if(myExtraRenderTicksCounter > 0) myExtraRenderTicksCounter--; }

	void setCallback(IActivityManagerCallback* pCallback) { myCallback = pCallback; }

protected:
	ActivityManager();

private:

	static ActivityManager* theInstance;

	bool myHaveSingleValueChange;
	int myAnimationsCounter;

	int myExtraRenderTicksCounter;

	Mutex myLock;

	IActivityManagerCallback* myCallback;
};
/*****************************************************************************/