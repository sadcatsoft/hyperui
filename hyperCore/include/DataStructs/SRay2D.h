#pragma once

/*****************************************************************************/
struct HYPERCORE_API SRay2D
{
	SRay2D()
	{

	}

	SRay2D(const SVector2D& svStart)
	{
		myStart = svStart;
		myDir.set(1, 0);
	}

	SRay2D(const SVector2D& svStart, const SVector2D& svDir)
	{
		myStart = svStart;
		myDir = svDir;	
	}

	void set(const SVector2D& svStart, const SVector2D& svDir)
	{
		myStart = svStart;
		myDir = svDir;	
	}

	void rotateCCW(FLOAT_TYPE fAngle)
	{
		myDir.rotateCCW(fAngle);
	}

	FLOAT_TYPE normalize()
	{
		return myDir.normalize();
	}

	inline FLOAT_TYPE doesIntersectLineSegment(const SVector2D &pLine2Start, const SVector2D &pLine2End, FLOAT_TYPE *dSecondLineIntersect, FLOAT_TYPE *dSecondLineLength, FLOAT_TYPE fTolerance = FLOAT_EPSILON) const
	{
		// Assume ray dir is normalized
		SVector2D lineDir = pLine2End - pLine2Start;
		FLOAT_TYPE dLineLength = lineDir.normalize();
		return this->doesIntersectLineSegment(pLine2Start, lineDir, dLineLength, dSecondLineIntersect, dSecondLineLength, fTolerance);
	}

	FLOAT_TYPE doesIntersectLineSegment(const SVector2D &pLine2Start, const SVector2D &pLine2Dir, FLOAT_TYPE fLine2Len, FLOAT_TYPE *dSecondLineIntersect, FLOAT_TYPE *dSecondLineLength, FLOAT_TYPE fTolerance = FLOAT_EPSILON) const
	{
		SVector2D intersect;
		FLOAT_TYPE s,t;

		if(dSecondLineLength!=NULL)
			*dSecondLineLength = fLine2Len;

		if(dSecondLineIntersect!=NULL)
			*dSecondLineIntersect = FLOAT_TYPE_MAX;
		
		if( fabs(fabs(this->myDir.dot(pLine2Dir)) - 1.0) < FLOAT_EPSILON)
			return FLOAT_TYPE_MAX;

		if(pLine2Dir.y == 0.0 && pLine2Dir.x == 0.0)
			return FLOAT_TYPE_MAX;

		FLOAT_TYPE numerator, denom;

		// For numerical stability, take the larger component
		if(pLine2Dir.y > pLine2Dir.x)
		{
			denom = pLine2Dir.x * this->myDir.y - this->myDir.x*pLine2Dir.y;
			if(denom == 0.0)
				return FLOAT_TYPE_MAX;
			else
			{
				numerator = pLine2Dir.y*(this->myStart.x - pLine2Start.x) - pLine2Dir.x*(this->myStart.y - pLine2Start.y);

				// See if the intersect lies within the other line segment
				t = numerator/denom;
				intersect = this->myStart + this->myDir*t;
				s = (intersect - pLine2Start).dot(pLine2Dir);
				//			if(s>=0.0 && s<=fLine2Len)
				if(s>-fTolerance && s<=fLine2Len+fTolerance)
				{
					if(dSecondLineIntersect!=NULL)
						*dSecondLineIntersect = s;
					return t;
				}
				else
					return FLOAT_TYPE_MAX;
			}
		}
		else
		{
			denom = pLine2Dir.y*this->myDir.x - this->myDir.y*pLine2Dir.x;
			if(denom == 0.0)
				return FLOAT_TYPE_MAX;
			else
			{
				numerator = pLine2Dir.x*(this->myStart.y - pLine2Start.y) - pLine2Dir.y*(this->myStart.x - pLine2Start.x);
				t = numerator/denom;
				intersect = this->myStart + this->myDir*t;
				s = (intersect - pLine2Start).dot(pLine2Dir);

				if(s>-fTolerance && s<=fLine2Len+fTolerance)
				{
					if(dSecondLineIntersect!=NULL)
						*dSecondLineIntersect = s;
					return t;
				}
				else
					return FLOAT_TYPE_MAX;
			}
		}

		return FLOAT_TYPE_MAX;
	}

	bool doesIntersectEllipse(const SVector2D& svEllipseCenter, const SVector2D& svEllipseRadii, FLOAT_TYPE& fDist1Out, FLOAT_TYPE& fDist2Out) const
	{
		// The equations below solve for ellipse at the origin. Make it so.
		SVector2D svMovedRayStart = this->myStart - svEllipseCenter;

		SVector2D svRadSquared(svEllipseRadii.x*svEllipseRadii.x, svEllipseRadii.y*svEllipseRadii.y);

		FLOAT_TYPE fA = svRadSquared.y*this->myDir.x*this->myDir.x + this->myDir.y*this->myDir.y*svRadSquared.x;
		FLOAT_TYPE fB = 2.0*(svMovedRayStart.x*this->myDir.x*svRadSquared.y + svMovedRayStart.y*this->myDir.y*svRadSquared.x);
		FLOAT_TYPE fC = svRadSquared.y*svMovedRayStart.x*svMovedRayStart.x + svRadSquared.x*svMovedRayStart.y*svMovedRayStart.y - svRadSquared.x*svRadSquared.y;

		FLOAT_TYPE fATimesTwo = 2.0*fA;

		FLOAT_TYPE fDiscr = fB*fB - 2.0*fATimesTwo*fC;
		fDist1Out = fDist2Out = FLOAT_TYPE_MAX;
		if(fDiscr < 0)
			return false;

		fDiscr = F_SQRT(fDiscr);
		fDist1Out = (-fB + fDiscr)/(fATimesTwo);
		fDist2Out = (-fB - fDiscr)/(fATimesTwo);

		return true;
	}

	SVector2D myStart, myDir;
};
/*****************************************************************************/