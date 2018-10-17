#pragma once

/*****************************************************************************/
enum MotionEffectType
{
	MotionEffectShake = 0,
	MotionEffectGlow,
};

const char* const g_pcsMotionEffectTokens[] = 
{
	"Shake",
	"Glow"
};
/*****************************************************************************/
class HYPERUI_API IMotionEffect
{
public:
	IMotionEffect(FLOAT_TYPE fSecDuration, FLOAT_TYPE fParm1 = 0, FLOAT_TYPE fParm2 = 0);
	virtual ~IMotionEffect();

	static IMotionEffect* createEffect(MotionEffectType eType, FLOAT_TYPE fSecDuration, FLOAT_TYPE fParm1 = 0, FLOAT_TYPE fParm2 = 0);

	virtual MotionEffectType getType(void) = 0;
	virtual void evaluate(SVector2D& svPosInOut) = 0;
	virtual void evaluateOpacity(FLOAT_TYPE& fOpacityInOut) { }

	GTIME getTime();
	virtual bool isDone();

	inline FLOAT_TYPE getParm1(void) { return myParm1; }
	inline FLOAT_TYPE getParm2(void) { return myParm2; }

	void restart();
	void stop();

private:

	FLOAT_TYPE myParm1, myParm2;
	// Current may chagne depending on whether we're stopped or
	// not, but the scheduled one always stays the same after
	// being set.
	GTIME myCurrentDuration, myStartTime, myScheduledDuration;
};
/*****************************************************************************/