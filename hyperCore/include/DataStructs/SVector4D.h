#pragma once

/*****************************************************************************/
struct HYPERCORE_API SVector4D
{
	SVector4D()
	{
		x = y = z = w = 0.0;
	}

	SVector4D(FLOAT_TYPE rx, FLOAT_TYPE ry, FLOAT_TYPE rz, FLOAT_TYPE rw)
	{
		x = rx;
		y = ry;
		z = rz;
		w = rw;
	}

	SVector4D(const SVector3D& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
		w = 1.0;
	}

	SVector4D(const SVector4D& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;
	}

	inline void set(FLOAT_TYPE rx, FLOAT_TYPE ry, FLOAT_TYPE rz, FLOAT_TYPE rw)
	{
		x = rx;
		y = ry;
		z = rz;
		w = rw;
	}

	inline void project(SVector3D& svOut) const
	{
		FLOAT_TYPE fRealW;
		if(w == 0)
			fRealW = 1.0;
		else
			fRealW = w;
		svOut.x = x/fRealW;
		svOut.y = y/fRealW;
		svOut.z = z/fRealW;
	}

	FLOAT_TYPE x,y,z,w;
};
/*****************************************************************************/
