#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
IPlaneObject::IPlaneObject(IPlane* pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void IPlaneObject::onAllocated(IPlane* pParentPlane)
{
	_ASSERT(pParentPlane != NULL);
	myParentPlane = pParentPlane;
	myTopAnimSeq = NULL;
	myTopAnimOffset.set(0,0);
}
/*****************************************************************************/
void IPlaneObject::onDeallocated()
{

}
/*****************************************************************************/
TextureManager* IPlaneObject::getTextureManager()
{
	return const_cast<TextureManager*>(const_cast<const IPlaneObject*>(this)->getTextureManager());
}
/*****************************************************************************/
const Window* IPlaneObject::getParentWindow() const
{	
	const IPlane *pParentPlane = getParentPlane();
	if(!pParentPlane)
		ASSERT_RETURN_NULL;
	return pParentPlane->getParentWindow();
}
/*****************************************************************************/
Window* IPlaneObject::getParentWindow()
{	
	IPlane* pParentPlane = getParentPlane();
	if(!pParentPlane)
		return NULL;
	return pParentPlane->getParentWindow();
}
/*****************************************************************************/
DrawingCache* IPlaneObject::getDrawingCache()
{
	Window* pWindow = getParentWindow();
	if(!pWindow)
		ASSERT_RETURN_NULL;
	return pWindow->getDrawingCache();
}
/*****************************************************************************/
const TextureManager* IPlaneObject::getTextureManager() const
{
	const Window* pWindow = getParentWindow();
	if(!pWindow)
		ASSERT_RETURN_NULL;
	return pWindow->getTextureManager();
}
/*****************************************************************************/
void IPlaneObject::resetEvalCache(bool bRecursive)
{

	myCachedBoxSize.set(-1, -1);
	myCachedBoxOffset.set(FLOAT_TYPE_MAX, FLOAT_TYPE_MAX);

	myTopAnimSeq = loadAnim(PropertyImage, myTopAnimAtlasFrame);
	myTopAnimOffset.x = this->getNumProp(PropertyObjAnimOffsetX);
	myTopAnimOffset.y = this->getNumProp(PropertyObjAnimOffsetY);

	if(!bRecursive)
		return;

	IPlaneObject* pChild;
	int iCurrChild, iNumChildren = this->getNumChildren();
	for(iCurrChild = 0; iCurrChild < iNumChildren; iCurrChild++)
	{
		pChild = FAST_CAST<IPlaneObject*>(this->getChild(iCurrChild));
		pChild->resetEvalCache(true);
	}
}
/*****************************************************************************/
void IPlaneObject::getBoxOffset(SVector2D& svOut)
{
	if(myCachedBoxOffset.x == FLOAT_TYPE_MAX)
	{
		myCachedBoxOffset.x = this->getNumProp(PropertyBoxOffsetX);
		myCachedBoxOffset.y = this->getNumProp(PropertyBoxOffsetY);
	}

	svOut = myCachedBoxOffset;
}
/********************************************************************************************/
void IPlaneObject::getBoxSize(SVector2D& svOut) const
{
	if(myCachedBoxSize.x < 0)
	{
		myCachedBoxSize.x = this->getNumProp(PropertyWidth);
		myCachedBoxSize.y = this->getNumProp(PropertyHeight);
	}
	svOut = myCachedBoxSize;
}
/********************************************************************************************/
void IPlaneObject::getFullTopAnimName(STRING_TYPE& strOut)
{
	//ENSURE_VARS_CACHED;
	if(!myTopAnimSeq)
	{
		strOut = "";
		return;
	}

	strOut = myTopAnimSeq->getName();
	if(myTopAnimAtlasFrame >= 0)
	{
		char pcsBuff[32];
		sprintf(pcsBuff, ":%d", myTopAnimAtlasFrame + 1);
		strOut += pcsBuff;
	}
}
/*****************************************************************************/
void IPlaneObject::setTopAnim(const char* pcsFullAnim)
{
	this->setStringProp(PropertyImage, pcsFullAnim);
	myTopAnimSeq = getDrawingCache()->getCachedSequence(pcsFullAnim, &myTopAnimAtlasFrame);
	onTopAnimChanged();
}
/*****************************************************************************/
void IPlaneObject::getTopAnimNameNoFrameNum(STRING_TYPE& strOut)
{
	//ENSURE_VARS_CACHED;
	if(!myTopAnimSeq)
		strOut = EMPTY_STRING;
	else
		strOut = myTopAnimSeq->getName();
}
/*****************************************************************************/
void IPlaneObject::onTopAnimChanged()
{
	if(!myTopAnimSeq)
		return;

	STRING_TYPE strTemp = SkinManager::getInstance()->mapOrigNameToSkinned(myTopAnimSeq->getName());
	if(strTemp != myTopAnimSeq->getName())
	{
		int iDummyFrame;
		myTopAnimSeq = getDrawingCache()->getCachedSequence(strTemp.c_str(), &iDummyFrame);
	}
}
/*****************************************************************************/
CachedSequence* IPlaneObject::loadAnim(PropertyType eProp, int& iFrameOut)
{
	iFrameOut = -1;
	CachedSequence* pRes = NULL;
	STRING_TYPE strName = this->getStringProp(eProp);
	if(IS_VALID_STRING_AND_NOT_NONE(strName.c_str()))
		pRes = getDrawingCache()->getCachedSequence(strName.c_str(), &iFrameOut);

	return pRes;
}
/*****************************************************************************/
};