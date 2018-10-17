#pragma once

/*****************************************************************************/
struct HYPERCORE_API SOBB
{
	SOBB() 
	{ 
		myAngle = 0; 
	}
	
	bool doesIntersect(const SOBB& rOther) const
	{
		// Compute the bases and the extents
		SVector2D akA[2], akB[2];
		akA[0].set(1,0);
		akA[0].rotateCCW(myAngle);
		akA[1].set(akA[0].y, -akA[0].x);
		
		akB[0].set(1,0);
		akB[0].rotateCCW(rOther.myAngle);
		akB[1].set(akB[0].y, -akB[0].x);
		
		FLOAT_TYPE afEA[2] = { mySize.x*0.5f, mySize.y*0.5f };
		FLOAT_TYPE afEB[2] = { rOther.mySize.x/2.0f, rOther.mySize.y/2.0f };
		
		// compute difference of box centers, D = C1-C0
		SVector2D kD = rOther.myCenter - myCenter;
		
		FLOAT_TYPE aafAbsAdB[2][2], fAbsAdD, fRSum;
		
		// axis C0+t*A0
		aafAbsAdB[0][0] = fabs(akA[0].dot(akB[0]));
		aafAbsAdB[0][1] = fabs(akA[0].dot(akB[1]));
		fAbsAdD = fabs(akA[0].dot(kD));
		fRSum = afEA[0] + afEB[0]*aafAbsAdB[0][0] + afEB[1]*aafAbsAdB[0][1];
		if (fAbsAdD > fRSum)
			return false;
		
		// axis C0+t*A1
		aafAbsAdB[1][0] = fabs(akA[1].dot(akB[0]));
		aafAbsAdB[1][1] = fabs(akA[1].dot(akB[1]));
		fAbsAdD = fabs(akA[1].dot(kD));
		fRSum = afEA[1] + afEB[0]*aafAbsAdB[1][0] + afEB[1]*aafAbsAdB[1][1];
		if (fAbsAdD > fRSum)
			return false;
		
		// axis C0+t*B0
		fAbsAdD = fabs(akB[0].dot(kD));
		fRSum = afEB[0] + afEA[0]*aafAbsAdB[0][0] + afEA[1]*aafAbsAdB[1][0];
		if (fAbsAdD > fRSum)
			return false;
		
		// axis C0+t*B1
		fAbsAdD = fabs(akB[1].dot(kD));
		fRSum = afEB[1] + afEA[0]*aafAbsAdB[0][1] + afEA[1]*aafAbsAdB[1][1];
		if (fAbsAdD > fRSum)
			return false;
		
		return true;
	}

	bool doesIntersect(const SVector2D& svStartIn, const SVector2D& svDirIn, FLOAT_TYPE& tOut1, FLOAT_TYPE& tOut2) const;
	
	SVector2D myCenter;
	SVector2D mySize;
	FLOAT_TYPE myAngle;
};
/*****************************************************************************/
