#pragma once

/*****************************************************************************/
struct HYPERCORE_API SQuad2D
{
	inline void rotateAbout(const SVector2D& svCenter, FLOAT_TYPE fDegAngle)
	{
		SVector2D svTemp;
		int iCurr;
		for(iCurr = 0; iCurr < 4; iCurr++)
		{
			svTemp = myPoints[iCurr] - svCenter;
			svTemp.rotateCCW(fDegAngle);
			myPoints[iCurr] = svTemp + svCenter;
		}
	}

	inline void textureFrom(const SRect2D& srRect)
	{
		int iCurr;
		for(iCurr = 0; iCurr < 4; iCurr++)
		{
			myU[iCurr] = (myPoints[iCurr].x - srRect.x)/srRect.w;
			myV[iCurr] = (myPoints[iCurr].y - srRect.y)/srRect.h;
		}
	}

	inline void getCentroid(SVector2D &svOut)
	{
		svOut = (myPoints[0] + myPoints[1] + myPoints[2] + myPoints[3])*0.25;
	}

	inline void makeVerticesRelativeTo(FLOAT_TYPE fX, FLOAT_TYPE fY)
	{
		int iCurr;
		for(iCurr = 0; iCurr < 4; iCurr++)
		{
			myPoints[iCurr].x -= fX;
			myPoints[iCurr].y -= fY;
		}
	}

	inline void makeUVsAbsolute(const SUVSet& otherUVs)
	{
		FLOAT_TYPE fDiffU = otherUVs.myEndX - otherUVs.myStartX;
		FLOAT_TYPE fDiffV = otherUVs.myEndY - otherUVs.myStartY;
		int iCurr;
		for(iCurr = 0; iCurr < 4; iCurr++)
		{
			myU[iCurr] = myU[iCurr]*fDiffU + otherUVs.myStartX;
			myV[iCurr] = myV[iCurr]*fDiffV + otherUVs.myStartY;
		}		
	}

	SVector2D myPoints[4];
	FLOAT_TYPE myU[4];
	FLOAT_TYPE myV[4];
};
/*****************************************************************************/
typedef vector < SQuad2D > TQuadVector;
/*****************************************************************************/
