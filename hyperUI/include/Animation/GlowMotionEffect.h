#pragma once

/*****************************************************************************/
class HYPERUI_API GlowMotionEffect : public IMotionEffect
{
public:
	GlowMotionEffect(FLOAT_TYPE fSecDuration, FLOAT_TYPE fMagnitude = 0);
	virtual ~GlowMotionEffect();

	virtual void evaluate(SVector2D& svPosInOut) { }
	virtual void evaluateOpacity(FLOAT_TYPE& fOpacityInOut);

	MotionEffectType getType() { return MotionEffectGlow; }

	virtual bool isDone() { return false; }
private:

	AnimatedValue myOpacity;
};
/*****************************************************************************/
