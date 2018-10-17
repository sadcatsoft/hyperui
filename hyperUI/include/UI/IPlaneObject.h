#pragma once

class HYPERUI_API CachedSequence;
/*****************************************************************************/
class HYPERUI_API IPlaneObject : public ResourceItem, public AnimationOverCallback
{
public:
	IPlaneObject(IPlane* pParentPlane);
	virtual ~IPlaneObject() { }
	virtual void onAllocated(IPlane* pParentPlane);
	virtual void onDeallocated();

	inline IPlane* getParentPlane() { return myParentPlane; }
	inline const IPlane* getParentPlane() const { return myParentPlane; }
	template < class TYPE> inline TYPE* getParentPlane() { return dynamic_cast<TYPE*>(myParentPlane); }

	const Window* getParentWindow() const;
	Window* getParentWindow();
	template < class TYPE> inline TYPE* getParentWindow() { return dynamic_cast<TYPE*>(myParentPlane->getParentWindow()); }

	const TextureManager* getTextureManager() const;
	TextureManager* getTextureManager();
	DrawingCache* getDrawingCache();

	void getBoxSize(SVector2D& svOut) const;
	void getBoxOffset(SVector2D& svOut);

	void setTopAnim(const char* pcsFullAnim);
	inline CachedSequence* getTopAnim() { return myTopAnimSeq; }
	inline int getTopAnimFrame() const { return myTopAnimAtlasFrame; }

	void getTopAnimNameNoFrameNum(STRING_TYPE& strOut);
	void getFullTopAnimName(STRING_TYPE& strOut);

	virtual void animationOver(AnimatedValue *pAValue, string* pData) { }

	inline void getTopAnimOffset(SVector2D& svOut) const { svOut = myTopAnimOffset; }

protected:

	virtual void resetEvalCache(bool bRecursive);
	void onTopAnimChanged();
	CachedSequence* loadAnim(PropertyType eProp, int& iFrameOut);

private:

	IPlane* myParentPlane;

	SVector2D myCachedBoxOffset;
	mutable SVector2D myCachedBoxSize;

	CachedSequence* myTopAnimSeq;
	int myTopAnimAtlasFrame;
	SVector2D myTopAnimOffset;
};
/*****************************************************************************/