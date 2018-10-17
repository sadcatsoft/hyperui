#pragma once

/*****************************************************************************/
struct HYPERCORE_API SBBox3D
{
	SBBox3D()
	{
		reset();
	}

	void reset(void)
	{
		myMax.set(-FLOAT_TYPE_MAX, -FLOAT_TYPE_MAX, -FLOAT_TYPE_MAX);
		myMin.set(FLOAT_TYPE_MAX, FLOAT_TYPE_MAX, FLOAT_TYPE_MAX);

	}

	void addPoint(SVector3D& svPt)
	{
		if(svPt.x > myMax.x) myMax.x = svPt.x;
		if(svPt.y > myMax.y) myMax.y = svPt.y;
		if(svPt.z > myMax.z) myMax.z = svPt.z;
		if(svPt.x < myMin.x) myMin.x = svPt.x;
		if(svPt.y < myMin.y) myMin.y = svPt.y;
		if(svPt.z < myMin.z) myMin.z = svPt.z;
	}

	void addBBox(SVector3D &svMin, SVector3D &svMax)
	{
		this->addPoint(svMin);
		this->addPoint(svMax);
	}

	inline void getSize(SVector3D& svSize) const { svSize = myMax - myMin; }

	SVector3D myMin, myMax;
};
/*****************************************************************************/