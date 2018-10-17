#pragma once

/*****************************************************************************/
class HYPERUI_API ShakeMotionEffect : public IMotionEffect
{
public:
	ShakeMotionEffect(FLOAT_TYPE fSecDuration, FLOAT_TYPE fMagnitude = 0);
	virtual ~ShakeMotionEffect();

	virtual void evaluate(SVector2D& svPosInOut);
	MotionEffectType getType() { return MotionEffectShake; }

private:

	SVector2D myPrevPos;
};
/*****************************************************************************/
