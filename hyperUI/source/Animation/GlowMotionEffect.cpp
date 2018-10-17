#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
GlowMotionEffect::GlowMotionEffect(FLOAT_TYPE fSecDuration, FLOAT_TYPE fMagnitude)
: IMotionEffect(fSecDuration, fMagnitude)
{
	myOpacity.setAnimation(0, 0.5, fSecDuration, ClockUiPrimary, AnimOverActionPlayReverseForever);
}
/*****************************************************************************/
GlowMotionEffect::~GlowMotionEffect()
{

}
/*****************************************************************************/
void GlowMotionEffect::evaluateOpacity(FLOAT_TYPE& fOpacityInOut)
{
	fOpacityInOut *= myOpacity.getValue();
}
/*****************************************************************************/
};