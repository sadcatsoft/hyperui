#pragma once

class CachedSequence;
/*****************************************************************************/
class HYPERUI_API AnimSequenceAddon
{
public:
	AnimSequenceAddon(PropertyType eProperty);
	virtual ~AnimSequenceAddon() { }

	virtual void onAllocated(PropertyType eProperty);

	void getFullBaseAnimName(STRING_TYPE& strOut);

	inline CachedSequence* getBaseAnim() { return myBaseAnimSeq; }
	inline int getBaseAnimFrame() const { return myBaseAnimAtlasFrame; }

	void setBaseAnim(const char* pcsFullAnim);

protected:
	virtual void resetEvalCache();

private:
	CachedSequence* loadAnim(PropertyType eProp, int& iFrameOut);

private:
	CachedSequence* myBaseAnimSeq;
	int myBaseAnimAtlasFrame;
	PropertyType myProperty;
};
/*****************************************************************************/