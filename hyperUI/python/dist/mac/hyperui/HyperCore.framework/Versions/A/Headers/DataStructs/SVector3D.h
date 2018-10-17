#pragma once

/*****************************************************************************/
struct HYPERCORE_API SVector3D
{
	SVector3D()
	{
		x = y = z = 0;
	}

	SVector3D(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fZ)
	{
		x = fX;
		y = fY;
		z = fZ;
	}

	inline void set(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fZ)
	{
		x = fX;
		y = fY;
		z = fZ;
	}

	inline FLOAT_TYPE lengthSquared() const
	{
		return x*x + y*y + z*z;
	}

	inline FLOAT_TYPE length(void) const
	{
		return sqrt(x*x + y*y + z*z);
	}

	inline void projectIsometric(SVector2D& svOut) const
	{
		svOut.x = x;
		FLOAT_TYPE fCos = F_COS_DEG(ISOMETRIC_VERT_ANGLE);
		svOut.y = fCos*y + (1.0 - fCos)*z;
	}


	inline SVector3D operator+(const SVector3D& other) const
	{
		SVector3D svTemp;
		svTemp.x = x + other.x;
		svTemp.y = y + other.y;
		svTemp.z = z + other.z;
		return svTemp;
	}

	inline void operator +=(const SVector3D& src)
	{
		x+=src.x;
		y+=src.y;
		z+=src.z;
	}

	inline void operator -=(const SVector3D& src)
	{
		x-=src.x;
		y-=src.y;
		z-=src.z;
	}

	inline SVector3D operator-(const SVector3D& other) const
	{
		SVector3D svTemp;
		svTemp.x = x - other.x;
		svTemp.y = y - other.y;
		svTemp.z = z - other.z;
		return svTemp;
	}

	inline SVector3D operator*(FLOAT_TYPE fValue) const
	{
		SVector3D svTemp;
		svTemp.x = x*fValue;
		svTemp.y = y*fValue;
		svTemp.z = z*fValue;
		return svTemp;
	}

	inline SVector3D& operator*=(FLOAT_TYPE fVal)
	{
		x *= fVal;
		y *= fVal;
		z *= fVal;
		return *this;
	}

	inline SVector3D cross(const SVector3D& other) const
	{
		SVector3D svRes;
		svRes.set(this->y*other.z - other.y*this->z , this->z*other.x - other.z*this->x , this->x*other.y - other.x*this->y);
		return svRes;
	}

	inline FLOAT_TYPE dot(const SVector3D& other) const
	{
		return x*other.x + y*other.y + z*other.z;
	}


	inline FLOAT_TYPE normalize()
	{
		FLOAT_TYPE length = x*x+y*y+z*z;

		length = sqrt(length);

		if(length < FLOAT_EPSILON)
			return 0.0;

		x /= length;
		y /= length;
		z /= length;
		return length;
	}

	inline FLOAT_TYPE normalizePrecise()
	{
		FLOAT_TYPE length = x*x+y*y+z*z;

		length = F_SQRT(length);

		if(length < FLOAT_EPSILON)
			return 0.0;

		x /= length;
		y /= length;
		z /= length;
		return length;
	}

	inline FLOAT_TYPE operator[](int index) const
	{
		switch(index)
		{
		case 0:return x;
		case 1:return y;
		case 2:return z;
		}
		_ASSERT(0);
		return x;
	}

	// Eeeeevil...
	inline FLOAT_TYPE& operator[](int index)
	{
		switch(index)
		{
		case 0:return x;
		case 1:return y;
		case 2:return z;
		}
		_ASSERT(0);
		return x;
	}

	FLOAT_TYPE x,y,z;
};
/*****************************************************************************/
typedef vector < SVector3D > TPoint3DVector;
typedef TLossyDynamicArray < SVector3D > TSVector3DLossyDynamicArray;
/*****************************************************************************/
