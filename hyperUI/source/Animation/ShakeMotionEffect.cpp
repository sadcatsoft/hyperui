#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
ShakeMotionEffect::ShakeMotionEffect(FLOAT_TYPE fSecDuration, FLOAT_TYPE fMagnitude)
: IMotionEffect(fSecDuration, fMagnitude)
{

}
/*****************************************************************************/
ShakeMotionEffect::~ShakeMotionEffect()
{

}
/*****************************************************************************/
void ShakeMotionEffect::evaluate(SVector2D& svPosInOut)
{
	if(getTime() % 2 == 0)
	{
		FLOAT_TYPE fRadius = getParm1();
		if(fRadius <= 0)
			fRadius = upToScreen(5.0);

		myPrevPos.x = RANDOM_INT_BETWEEN_INCLUSIVE(0, fRadius) - fRadius/2.0;
		myPrevPos.y = RANDOM_INT_BETWEEN_INCLUSIVE(0, fRadius) - fRadius/2.0;
	}

	svPosInOut.x += myPrevPos.x;
	svPosInOut.y += myPrevPos.y;
}
/*****************************************************************************/
};