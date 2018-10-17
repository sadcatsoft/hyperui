#pragma once

/*****************************************************************************/
struct HYPERCORE_API SVector2D
{
	SVector2D()
	{
		x = y = 0.0;
	}

	SVector2D(const SVector2D& other)
	{
		x = other.x;
		y = other.y;
	}

	SVector2D(FLOAT_TYPE dx, FLOAT_TYPE dy)
	{
		x = dx;
		y = dy;
	}	

	inline void set(FLOAT_TYPE dx, FLOAT_TYPE dy)
	{
		x = dx;
		y = dy;
	}	

	inline FLOAT_TYPE minComponent() const 
	{ 
		return min(x, y); 
	}

	inline FLOAT_TYPE maxComponent() const 
	{ 
		return max(x, y); 
	}

	SVector2D project(SVector2D& other, FLOAT_TYPE* fMagnitudeOut) const
	{
		SVector2D svThisNormed;
		svThisNormed = *this;
		svThisNormed.normalize();

		FLOAT_TYPE fDot = this->dot(other);
		svThisNormed *= fDot;

		if(fMagnitudeOut)
			*fMagnitudeOut = fDot;

		return svThisNormed;
	}

	inline SVector2D& operator+=(const SVector2D& other)
	{
		x += other.x;
		y += other.y;
		return *this;
	}

	inline SVector2D& operator/=(const SVector2D& other)
	{
		x /= other.x;
		y /= other.y;
		return *this;
	}

	inline SVector2D& operator*=(const SVector2D& other)
	{
		x *= other.x;
		y *= other.y;
		return *this;
	}

	inline SVector2D& operator-=(const SVector2D& other)
	{
		x -= other.x;
		y -= other.y;
		return *this;
	}

	inline SVector2D& operator*=(FLOAT_TYPE fVal)
	{
		x *= fVal;
		y *= fVal;
		return *this;
	}

	inline SVector2D& operator/=(FLOAT_TYPE fVal)
	{
		x /= fVal;
		y /= fVal;
		return *this;
	}

	inline void getPerpVector(SVector2D& svOut) const
	{
		svOut.x = this->y;
		svOut.y = -this->x;
	}

	inline FLOAT_TYPE normalize()
	{
		FLOAT_TYPE dLen = F_SQRT(x*x + y*y);
		if(dLen < FLOAT_EPSILON)
		{
			x = 0.0;
			y = 0.0;
			return 0;
		}

		x /= dLen;
		y /= dLen;

		return dLen;
	}

	inline FLOAT_TYPE normalizePrecise()
	{
		FLOAT_TYPE dLen = sqrt(x*x + y*y);
		if(dLen < FLOAT_EPSILON)
		{
			//x = 1.0;
			x = 0.0;
			y = 0.0;
			return 0;
		}

		x /= dLen;
		y /= dLen;

		return dLen;
	}

	inline void rotateCCW(FLOAT_TYPE fDegrees)
	{
		SVector2D old_v(x,y);
		FLOAT_TYPE fSin = F_SIN_DEG(-fDegrees);
		FLOAT_TYPE fCos = F_COS_DEG(-fDegrees);
		x = fCos*old_v.x - fSin*old_v.y;
		y = fSin*old_v.x + fCos*old_v.y;
	}

	void rotateCCWPrecise(FLOAT_TYPE fDegrees)
	{
		SVector2D old_v(x,y);
		FLOAT_TYPE rads = -fDegrees/180.0*M_PI;
		FLOAT_TYPE fSin = sin(rads);
		FLOAT_TYPE fCos = cos(rads);
		x = fCos*old_v.x - fSin*old_v.y;
		y = fSin*old_v.x + fCos*old_v.y;
	}


	void rotateCCWCompressY(FLOAT_TYPE fDegrees, FLOAT_TYPE fCompression)
	{
		// Iso view rotation
		SVector2D old_v(x,y);
		FLOAT_TYPE fSin = F_SIN_DEG(-fDegrees)*fCompression;
		FLOAT_TYPE fCos = F_COS_DEG(-fDegrees);
		x = fCos*old_v.x - fSin*old_v.y;
		y = fSin*old_v.x + fCos*old_v.y;	
	}

	void rotateCCWCompressYPrecise(FLOAT_TYPE fDegrees, FLOAT_TYPE fCompression)
	{
		// Iso view rotation
		SVector2D old_v(x,y);
		FLOAT_TYPE rads = -fDegrees/180.0*M_PI;
		FLOAT_TYPE fSin = sin(rads)*fCompression;
		FLOAT_TYPE fCos = cos(rads);
		x = fCos*old_v.x - fSin*old_v.y;
		y = fSin*old_v.x + fCos*old_v.y;	
	}

	inline SVector2D operator*(FLOAT_TYPE dValue) const
	{
		SVector2D svRes;
		svRes.x = x * dValue;
		svRes.y = y * dValue;
		return svRes;
	}

	inline SVector2D operator/(FLOAT_TYPE dValue) const
	{
		SVector2D svRes;
		svRes.x = x / dValue;
		svRes.y = y / dValue;
		return svRes;
	}

	inline void castToInt()
	{
		x = (int)x;
		y = (int)y;
	}

	inline void roundToInt()
	{
		x = HyperCore::roundToInt(x);
		y = HyperCore::roundToInt(y);
	}

	inline void clampToRange(FLOAT_TYPE fMin, FLOAT_TYPE fMax)
	{
		x = HyperCore::clampToRange<FLOAT_TYPE>(fMin, fMax, x);
		y = HyperCore::clampToRange<FLOAT_TYPE>(fMin, fMax, y);
	}

	inline SVector2D operator+(const SVector2D& other) const
	{
		SVector2D svRes;
		svRes.x = x + other.x;
		svRes.y = y + other.y;
		return svRes;
	}

	inline SVector2D operator/(const SVector2D& other) const
	{
		SVector2D svRes;
		svRes.x = x / other.x;
		svRes.y = y / other.y;
		return svRes;
	}

	inline SVector2D operator*(const SVector2D& other) const
	{
		SVector2D svRes;
		svRes.x = x * other.x;
		svRes.y = y * other.y;
		return svRes;
	}

	inline SVector2D operator-(const SVector2D& other) const
	{
		SVector2D svRes;
		svRes.x = x - other.x;
		svRes.y = y - other.y;
		return svRes;
	}	

	inline FLOAT_TYPE operator[](int index) const
	{
		if(index == 0) 
			return x;
		else
			return y;
	}

	// NOTE: This assumes the vector is normalized.
	inline FLOAT_TYPE getAngleFromPositiveX() const
	{
		FLOAT_TYPE dAngle = ::F_ATAN2(y*-1, x)/M_PI*180.0;
		if(dAngle < 0)
			dAngle += 360.0;
		return dAngle;

	}

	inline FLOAT_TYPE getAngleFromPositiveXAccurate() const
	{
		FLOAT_TYPE dAngle = atan2(y*-1, x)/M_PI*180.0;
		if(dAngle < 0)
			dAngle += 360.0;
		return dAngle;
	}

	inline FLOAT_TYPE lengthSquared() const	
	{ 
		return x*x+y*y; 
	}

	inline FLOAT_TYPE length() const 
	{ 
		return F_SQRT(x*x+y*y); 
	}

	inline FLOAT_TYPE dot(const SVector2D& other) const 
	{ 
		return x*other.x + y*other.y; 
	}

	inline FLOAT_TYPE cross(const SVector2D& other) const 
	{ 
		return y*other.x - x*other.y; 
	}

	inline FLOAT_TYPE perpdot(const SVector2D& svOther) const 
	{ 
		return x*svOther.y - y*svOther.x; 
	}

	inline FLOAT_TYPE getAngleFrom(const SVector2D& svOther) const 
	{ 
		return ::F_ATAN2(this->perpdot(svOther), this->dot(svOther))/M_PI*180.0; 
	}

	inline FLOAT_TYPE getAngleFromAccurate(const SVector2D& svOther) const 
	{ 
		return atan2(this->perpdot(svOther), this->dot(svOther))/M_PI*180.0; 
	}

	inline void interpolateFrom(const SVector2D& svPoint1, const SVector2D& svPoint2, FLOAT_TYPE fInterpFromPoint1)
	{
		x = HyperCore::interpolateFloat(svPoint1.x, svPoint2.x, fInterpFromPoint1);
		y = HyperCore::interpolateFloat(svPoint1.y, svPoint2.y, fInterpFromPoint1);
	}

	inline SVector2D& getPoint() 
	{ 
		return *this; 
	}

	inline const SVector2D& getPoint() const 
	{ 
		return *this; 
	}

	inline BezierPointCurveType getPointType() const 
	{ 
		return BezierPointCurveSharp; 
	}

	inline bool operator==(const SVector2D& svOther) const 
	{ 
		return x == svOther.x && y == svOther.y; 
	}

	inline bool operator!=(const SVector2D& svOther) const 
	{ 
		return x != svOther.x || y != svOther.y; 
	}

	inline void upToScreen()
	{
		this->x = HyperCore::upToScreen(x);
		this->y = HyperCore::upToScreen(y);
	}

	bool isOnLineSegment(const SVector2D& svStart, const SVector2D& svEnd, FLOAT_TYPE fTolerance, SVector2D* pExactPointOut, FLOAT_TYPE fHorTolerance = FLOAT_EPSILON) const;

	static const SVector2D Zero;

	FLOAT_TYPE x,y;
};
/*****************************************************************************/
#ifdef WIN32
//HYPERCORE_EXTERN template class HYPERCORE_API std::vector < HyperCore::SVector2D >;
#endif
typedef vector < SVector2D > TPointVector;
typedef TLossyDynamicArray < SVector2D > TSVector2DLossyDynamicArray;
/*****************************************************************************/
