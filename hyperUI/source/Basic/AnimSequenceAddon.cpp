#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
AnimSequenceAddon::AnimSequenceAddon(PropertyType eProperty)
{
	AnimSequenceAddon::onAllocated(eProperty);
}
/*****************************************************************************/
void AnimSequenceAddon::onAllocated(PropertyType eProperty)
{
	myProperty = eProperty;
	myBaseAnimSeq = NULL;
	myBaseAnimAtlasFrame = -1;
}
/*****************************************************************************/
void AnimSequenceAddon::getFullBaseAnimName(STRING_TYPE& strOut)
{
	//ENSURE_VARS_CACHED;
	if(!myBaseAnimSeq)
	{
		strOut = "";
		return;
	}
	strOut = myBaseAnimSeq->getName();
	if(myBaseAnimAtlasFrame >= 0)
	{
		char pcsBuff[32];
		sprintf(pcsBuff, ":%d", myBaseAnimAtlasFrame + 1);
		strOut += pcsBuff;
	}
}
/*****************************************************************************/
void AnimSequenceAddon::resetEvalCache()
{
	myBaseAnimSeq = loadAnim(myProperty, myBaseAnimAtlasFrame);
}
/*****************************************************************************/
CachedSequence* AnimSequenceAddon::loadAnim(PropertyType eProp, int& iFrameOut)
{
	iFrameOut = -1;
	CachedSequence* pRes = NULL;
	STRING_TYPE strName = dynamic_cast<const ResourceItem*>(this)->getStringProp(eProp);
	if(IS_VALID_STRING_AND_NOT_NONE(strName.c_str()))
		pRes = dynamic_cast<IPlaneObject*>(this)->getDrawingCache()->getCachedSequence(strName.c_str(), &iFrameOut);

	return pRes;
}
/*****************************************************************************/
void AnimSequenceAddon::setBaseAnim(const char* pcsFullAnim)
{
	dynamic_cast<ResourceItem*>(this)->setStringProp(myProperty, pcsFullAnim);
	myBaseAnimSeq = dynamic_cast<IPlaneObject*>(this)->getDrawingCache()->getCachedSequence(pcsFullAnim, &myBaseAnimAtlasFrame);
}
/*****************************************************************************/
};