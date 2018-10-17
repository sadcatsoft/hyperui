#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
IMotionEffect::IMotionEffect(FLOAT_TYPE fSecDuration, FLOAT_TYPE fParm1, FLOAT_TYPE fParm2)
{
	myParm1 = fParm1;
	myParm2 = fParm2;

	myCurrentDuration = Application::secondsToTicks(fSecDuration);
	myScheduledDuration = myCurrentDuration;

	restart();
}
/*****************************************************************************/
IMotionEffect::~IMotionEffect()
{

}
/*****************************************************************************/
void IMotionEffect::restart()
{
	myStartTime = getTime();
	myCurrentDuration = myScheduledDuration;
}
/*****************************************************************************/
void IMotionEffect::stop()
{
	myStartTime = 0;
	myCurrentDuration = 0;
}
/*****************************************************************************/
bool IMotionEffect::isDone()
{
	if(myStartTime + myCurrentDuration < getTime())
		return true;
	else
		return false;
}
/*****************************************************************************/
GTIME IMotionEffect::getTime()
{	
	return Application::getInstance()->getGlobalTime(ClockUiPrimary);
}
/*****************************************************************************/
IMotionEffect* IMotionEffect::createEffect(MotionEffectType eType, FLOAT_TYPE fSecDuration, FLOAT_TYPE fParm1, FLOAT_TYPE fParm2)
{
	IMotionEffect* pRes = NULL;
	if(eType == MotionEffectShake)
		pRes = new ShakeMotionEffect(fSecDuration, fParm1);
	else if(eType == MotionEffectGlow)
		pRes = new GlowMotionEffect(fSecDuration, fParm1);
	else
	{
		// Unknown effect
		_ASSERT(0);
	}

	return pRes;
}
/*****************************************************************************/
};