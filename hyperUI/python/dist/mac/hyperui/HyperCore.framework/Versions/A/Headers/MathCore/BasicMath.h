#pragma once

#ifdef USE_FAST_MATH_ROUTINES
	#define F_SQRT			HyperCore::fastSqrt
	#define F_COS			HyperCore::fastCos
	#define F_SIN			HyperCore::fastSin
	#define F_ATAN2			HyperCore::fastAtan2
	#define F_COS_DEG		HyperCore::fastCosDeg
	#define F_SIN_DEG		HyperCore::fastSinDeg
#else
	#define F_SQRT			sqrt
	#define F_COS			cos
	#define F_SIN			sin
	#define F_ATAN2			atan2
	#define F_COS_DEG(x)	cos((x)/180.0*M_PI)
	#define F_SIN_DEG(x)	sin((x)/180.0*M_PI)
#endif

// This is here because for rotating large images, we can be
// noticeably faster using lookup tables, yet we need very high
// precisions.
#define SIN_COS_STEP_DEG			(FLOAT_TYPE)0.05f
#define SIN_COS_STEP				(FLOAT_TYPE)(SIN_COS_STEP_DEG)/180.0f*(FLOAT_TYPE)M_PI

/*****************************************************************************/
class BasicMath 
{ 
public:
	static void initialize(); 

	HYPERCORE_API static FLOAT_TYPE *theSinLookupTable;
	HYPERCORE_API static FLOAT_TYPE *theCosLookupTable;
	HYPERCORE_API static FLOAT_TYPE *theSinDegLookupTable;
	HYPERCORE_API static FLOAT_TYPE *theCosDegLookupTable;
	HYPERCORE_API static int theSinCosDegArrayLength;
	HYPERCORE_API static int theSinCosRadArrayLength;
};
/*****************************************************************************/
inline FLOAT_TYPE fastSqrt(FLOAT_TYPE x)
{
	FLOAT_TYPE xhalf = 0.5f*x;
	int i = *(int*)&x; // get bits for floating value
	i = 0x5f375a86- (i>>1); // gives initial guess y0
	x = *(FLOAT_TYPE*)&i; // convert bits back to float
	x = x*(1.5f-xhalf*x*x); // Newton step, repeating increases accuracy
	return 1.0f/x;
}
/*****************************************************************************/
inline FLOAT_TYPE sanitizeDegAngle(FLOAT_TYPE x )
{
	while(x < 0.0f)
		x += 360.0f;
	while(x > 360.0f)
		x -= 360.0f;

	return x;
}
/*****************************************************************************/
inline FLOAT_TYPE fastSinDeg(FLOAT_TYPE x)
{
	FLOAT_TYPE fStep = SIN_COS_STEP_DEG;
	while(x < 0.0f)
		x += 360.0f;
	while(x > 360.0f)
		x -= 360.0f;

	int iIndex = (int)(x/fStep);
	_ASSERT(iIndex >= 0 && iIndex < BasicMath::theSinCosDegArrayLength);
	return BasicMath::theSinDegLookupTable[iIndex];
}
/*****************************************************************************/
inline FLOAT_TYPE fastSin(FLOAT_TYPE x)
{
	FLOAT_TYPE fStep = SIN_COS_STEP;
	while(x < 0.0f)
		x += (FLOAT_TYPE)M_PI*2.0f;
	while(x > (FLOAT_TYPE)M_PI*2.0f)
		x -= (FLOAT_TYPE)M_PI*2.0f;

	int iIndex = (int)(x/fStep);
	_ASSERT(iIndex >= 0 && iIndex < BasicMath::theSinCosRadArrayLength);
	return BasicMath::theSinLookupTable[iIndex];

}
/*****************************************************************************/
inline FLOAT_TYPE fastCosDeg(FLOAT_TYPE x)
{
	FLOAT_TYPE fStep = SIN_COS_STEP_DEG;
	while(x < 0.0f)
		x += 360.0f;
	while(x > 360.0f)
		x -= 360.0f;

	int iIndex = (int)(x/fStep);
	_ASSERT(iIndex >= 0 && iIndex < BasicMath::theSinCosDegArrayLength);
	return BasicMath::theCosDegLookupTable[iIndex];
}
/*****************************************************************************/
inline FLOAT_TYPE fastCos(FLOAT_TYPE x)
{
	FLOAT_TYPE fStep = SIN_COS_STEP;
	while(x < 0.0f)
		x += (FLOAT_TYPE)M_PI*2.0f;
	while(x > (FLOAT_TYPE)M_PI*2.0f)
		x -= (FLOAT_TYPE)M_PI*2.0f;

	int iIndex = (int)(x/fStep);
	_ASSERT(iIndex >= 0 && iIndex < BasicMath::theSinCosRadArrayLength);
	return BasicMath::theCosLookupTable[iIndex];
}
/*****************************************************************************/
inline FLOAT_TYPE fastAtan2(FLOAT_TYPE y, FLOAT_TYPE x)
{
	FLOAT_TYPE angle;
	FLOAT_TYPE coeff_1 = (FLOAT_TYPE)M_PI/4.0f;
	FLOAT_TYPE coeff_2 = 3.0f*coeff_1;
	FLOAT_TYPE abs_y = fabsf(y) + 1e-10f;      // kludge to prevent 0/0 condition
	if(x >= 0)
	{
		FLOAT_TYPE r = (x - abs_y)/(x + abs_y);
		angle = coeff_1 - coeff_1*r;
	}
	else
	{
		FLOAT_TYPE r = (x + abs_y)/(abs_y - x);
		angle = coeff_2 - coeff_1*r;
	}

	if (y < 0)
		// negate if in quad III or IV
		return -angle;
	else
		return angle;
}
/*****************************************************************************/
inline FLOAT_TYPE computeLanczos(FLOAT_TYPE x, int order)
{
	if(x == 0)
		return 1.0;

	if(x > order || x < -order)
		return 0.0;

	return (FLOAT_TYPE)(order*sin(M_PI*x)*sin(M_PI*x/(double)order)/((M_PI*x)*(M_PI*x)));
}
/*****************************************************************************/
template < class S >
inline int getSign(S elem) 
{ 
	return (((elem) < 0) ? -1 : 1); 
}
/*****************************************************************************/
inline FLOAT_TYPE mantissa(FLOAT_TYPE fNum)
{ 
	return fNum - ((FLOAT_TYPE)((int)fNum)); 
}
/*****************************************************************************/
template < class TYPE >
inline TYPE interpolateValue(TYPE fValue1, TYPE fValue2, FLOAT_TYPE fInterp)
{
	_ASSERT(fInterp >= 0 && fInterp <= 1.0);
	return (fValue2 - fValue1)*fInterp + fValue1;
}
/*****************************************************************************/
inline FLOAT_TYPE interpolateFloat(FLOAT_TYPE fValue1, FLOAT_TYPE fValue2, FLOAT_TYPE fInterp)
{
	_ASSERT(fInterp >= 0 && fInterp <= 1.0);
	return (fValue2 - fValue1)*fInterp + fValue1;
}
/*****************************************************************************/
template < class TYPE >
inline TYPE clampToRange(TYPE fMin, TYPE fMax, TYPE fValue)
{
	if(fValue > fMax)
		return fMax;
	else if(fValue < fMin)
		return fMin;
	else
		return fValue;
}
/*****************************************************************************/
template < class TYPE >
inline void swapValues(TYPE& tLeft, TYPE& tRight)
{
	TYPE tTemp = tLeft;
	tLeft = tRight;
	tRight = tTemp;
}
/*****************************************************************************/
inline FLOAT_TYPE roundToInt(FLOAT_TYPE fValue) 
{ 
	return fValue >= 0.0f ? (int)(fValue + 0.5f) : (int)(fValue - 0.5f); 
}
/*****************************************************************************/
inline FLOAT_TYPE hypotFast(FLOAT_TYPE fX, FLOAT_TYPE fY)
{
	return F_SQRT(fX*fX + fY*fY);
}
/*****************************************************************************/
template < class TYPE > 
inline bool isNaN(TYPE fNumber) { return fNumber != fNumber; }
/*****************************************************************************/
HYPERCORE_API int forcePowerOfTwo(FLOAT_TYPE fValue);
HYPERCORE_API void normalizeProbList(FLOAT_TYPE* fProbs, int iNum, bool bInvert, FLOAT_TYPE fMinProb);
HYPERCORE_API void normalizeProbList(TFloatVector& rProbsIn, bool bInvert = false, FLOAT_TYPE fMinProb = 0.0);
// I.e. not infinity and not a NaN
HYPERCORE_API bool isValidNumber(FLOAT_TYPE fValue);
/*****************************************************************************/
