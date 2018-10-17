#pragma once

/*****************************************************************************/
struct HYPERCORE_API SBBox2D
{
	SBBox2D()
	{
		reset();
	}

	inline void reset()
	{
		myMax.set(-FLOAT_TYPE_MAX, -FLOAT_TYPE_MAX);
		myMin.set(FLOAT_TYPE_MAX, FLOAT_TYPE_MAX);
	}

	inline void addPoint(const SVector2D& svPt)
	{
		if(svPt.x > myMax.x) myMax.x = svPt.x;
		if(svPt.y > myMax.y) myMax.y = svPt.y;
		if(svPt.x < myMin.x) myMin.x = svPt.x;
		if(svPt.y < myMin.y) myMin.y = svPt.y;
	}

	inline void addPoint(FLOAT_TYPE fX, FLOAT_TYPE fY)
	{
		if(fX > myMax.x) myMax.x = fX;
		if(fY > myMax.y) myMax.y = fY;
		if(fX < myMin.x) myMin.x = fX;
		if(fY < myMin.y) myMin.y = fY;
	}

	inline void addRect(const SRect2D& srRect)
	{
		addPoint(srRect.origin());
		addPoint(srRect.farCorner());
	}

	void addBBox(const SVector2D &svMin, const SVector2D &svMax)
	{
		this->addPoint(svMin);
		this->addPoint(svMax);
	}

	void addPoints(const TPointVector& rPoints)
	{
		int iCurr, iNum = rPoints.size();
		SVector2D svPt;
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
			svPt = rPoints[iCurr];
			if(svPt.x > myMax.x) myMax.x = svPt.x;
			if(svPt.y > myMax.y) myMax.y = svPt.y;
			if(svPt.x < myMin.x) myMin.x = svPt.x;
			if(svPt.y < myMin.y) myMin.y = svPt.y;
		}
	}

	inline void getSize(SVector2D& svSize) const 
	{ 
		svSize = myMax - myMin; 
	}

	inline void getCenter(SVector2D& svCenter) const 
	{ 
		svCenter = (myMax + myMin)*0.5; 
	}

	SVector2D myMin, myMax;
};
/*****************************************************************************/