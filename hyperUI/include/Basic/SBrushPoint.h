#pragma once

/********************************************************************************************/
struct SBrushPoint
{
	SBrushPoint() 
	{ 
		myPressure = 1.0; 
		myCumulativeStrokeLength = 0.0; 
		myOpacityJitterMult = 0.0; 
		mySizeJitterMult = 0.0;
		myScatterJitter = 0.0;
		myScatterCountJitter = 0.0;
		myScatterRandSeed = 0;
		myHueJitterMult = 0.0;
		mySatJitterMult = 0.0;
		myBriJitterMult = 0.0;
		myHoldingStrength = 0.0;
	}

	inline SVector2D& getPoint() { return myPoint; }
	inline const SVector2D& getPoint() const { return myPoint; }

	inline void interpolateFrom(const SBrushPoint& svPoint1, const SBrushPoint& svPoint2, FLOAT_TYPE fInterpFromPoint1)
	{
		fInterpFromPoint1 = HyperCore::clampToRange<FLOAT_TYPE>(0, 1, fInterpFromPoint1);
		myPoint.interpolateFrom(svPoint1.getPoint(), svPoint2.getPoint(), fInterpFromPoint1);
		//myPressure = svPoint1.myPressure*(1.0 - fInterpFromPoint1) + svPoint2.myPressure*fInterpFromPoint1;

		myPressure = HyperCore::interpolateFloat(svPoint1.myPressure, svPoint2.myPressure, fInterpFromPoint1);
		myOpacityJitterMult = HyperCore::interpolateFloat(svPoint1.myOpacityJitterMult, svPoint2.myOpacityJitterMult, fInterpFromPoint1);
		mySizeJitterMult = HyperCore::interpolateFloat(svPoint1.mySizeJitterMult, svPoint2.mySizeJitterMult, fInterpFromPoint1);
		myScatterJitter = HyperCore::interpolateFloat(svPoint1.myScatterJitter, svPoint2.myScatterJitter, fInterpFromPoint1);
		myScatterCountJitter = HyperCore::interpolateFloat(svPoint1.myScatterCountJitter, svPoint2.myScatterCountJitter, fInterpFromPoint1);
		myScatterRandSeed = HyperCore::interpolateFloat(svPoint1.myScatterRandSeed, svPoint2.myScatterRandSeed, fInterpFromPoint1);

		myHueJitterMult = HyperCore::interpolateFloat(svPoint1.myHueJitterMult, svPoint2.myHueJitterMult, fInterpFromPoint1);
		mySatJitterMult = HyperCore::interpolateFloat(svPoint1.mySatJitterMult, svPoint2.mySatJitterMult, fInterpFromPoint1);
		myBriJitterMult = HyperCore::interpolateFloat(svPoint1.myBriJitterMult, svPoint2.myBriJitterMult, fInterpFromPoint1);

		myHoldingStrength = HyperCore::interpolateFloat(svPoint1.myHoldingStrength, svPoint2.myHoldingStrength, fInterpFromPoint1);

		myCumulativeStrokeLength = HyperCore::interpolateFloat(svPoint1.myCumulativeStrokeLength, svPoint2.myCumulativeStrokeLength, fInterpFromPoint1);
	}

	inline void setPoint(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fPressure) { myPoint.set(fX, fY); myPressure = fPressure; }
	inline FLOAT_TYPE getPressure() const { return myPressure; }

	inline BezierPointCurveType getPointType() const { return BezierPointCurveSharp; }

	FLOAT_TYPE myScatterJitter, myScatterCountJitter;
	int myScatterRandSeed;

	FLOAT_TYPE myOpacityJitterMult, mySizeJitterMult;
	FLOAT_TYPE myHueJitterMult, mySatJitterMult, myBriJitterMult;

	// Total stroke length at this point, in pixels
	FLOAT_TYPE myCumulativeStrokeLength;

	// Strength that increases if the user holds the pressed mouse over
	// a specific point and doesn't move. Starts at 0.
	FLOAT_TYPE myHoldingStrength;

private:

	SVector2D myPoint;
	// Between [0, 1]
	FLOAT_TYPE myPressure;
};
typedef vector < SBrushPoint > TBrushPointVector;
/********************************************************************************************/
