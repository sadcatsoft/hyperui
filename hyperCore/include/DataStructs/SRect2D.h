#pragma once

/*****************************************************************************/
template < class T >
struct HYPERCORE_API TRect2D
{
	TRect2D()
	{
		reset();
	}

	TRect2D(T dX, T dY, T dW, T dH)
	{
		x = dX;
		y = dY;
		w = dW;
		h = dH;
	}
	
	inline void reset() 
	{ 
		x = y = w = h = 0.0; 
	}

	inline const SVector2D origin() const 
	{ 
		return SVector2D(x, y); 
	}

	inline const SVector2D size() const 
	{ 
		return SVector2D(w, h); 
	}

	inline const SVector2D center() const 
	{ 
		return SVector2D(x + w/2.0, y + h/2.0); 
	}

	inline const SVector2D farCorner() const 
	{ 
		return SVector2D(x + w, y + h); 
	}

	inline void set(T dX, T dY, T dW, T dH)
	{
		x = dX;
		y = dY;
		w = dW;
		h = dH;
	}

	inline void ensureWidthHeightPositive()
	{
		if(w < 0)
		{
			x += w;
			w *= -1.0;
		}

		if(h < 0)
		{
			y += h;
			h *= -1.0;
		}
	}

	inline void ensureStartsAtZero()
	{
		if(x < 0)
		{
			w += x;
			x = 0;
		}
		
		if(y < 0)
		{
			h += y;
			y = 0;
		}
	}

	inline void multiplyBy(T fValue)
	{
		x *= fValue;
		y *= fValue;
		w *= fValue;
		h *= fValue;
	}

	inline void roundToInt()
	{
		x = HyperCore::roundToInt(x);
		y = HyperCore::roundToInt(y);
		w = HyperCore::roundToInt(w);
		h = HyperCore::roundToInt(h);
	}

	inline void castToInt()
	{
		x = int(x);
		y = int(y);
		w = int(w);
		h = int(h);
	}

	inline void castOriginRoundSize()
	{
		x = int(x);
		y = int(y);
		w = HyperCore::roundToInt(w);
		h = HyperCore::roundToInt(h);
	}

	inline void castSizeRoundOrigin()
	{
		x = HyperCore::roundToInt(x);
		y = HyperCore::roundToInt(y);
		w = int(w);
		h = int(h);
	}

	inline void setToPixelBoundaries(bool bMinimize = false)
	{
		FLOAT_TYPE fDiff;
		fDiff = HyperCore::mantissa(x);
		x = (int)x;
		w += fDiff;
		fDiff = HyperCore::mantissa(y);
		y = (int)y;
		h += fDiff;

		if(bMinimize)
		{
			w = (int)w;
			h = (int)h;
		}
		else
		{
			if(HyperCore::mantissa(w) > FLOAT_EPSILON*2.0)
				w = (int)(w + 1.0);

			if(HyperCore::mantissa(h) > FLOAT_EPSILON*2.0)
				h = (int)(h + 1.0);
		}
	}

	inline T operator[](int index) const
	{
		switch(index)
		{
		case 0:return x;
		case 1:return y;
		case 2:return w;
		case 3:return h;
		}
		_ASSERT(0);
		return x;
	}

	// Eeeeevil...
	inline T& operator[](int index)
	{
		switch(index)
		{
		case 0:return x;
		case 1:return y;
		case 2:return w;
		case 3:return h;
		}
		_ASSERT(0);
		return x;
	}

	inline void scaleFromCenterByPercentDims(FLOAT_TYPE fValue)
	{
		enlargeHorVertBy(w*fValue/2.0, h*fValue/2.0);
	}

	inline void scaleAroundCenter(FLOAT_TYPE fValue)
	{
		FLOAT_TYPE fNewW = w*fValue;
		FLOAT_TYPE fNewH = h*fValue;

		x -= (fNewW - w)/2.0;
		y -= (fNewH - h)/2.0;
		w = fNewW;
		h = fNewH;
	}

	inline void setCenter(FLOAT_TYPE fX, FLOAT_TYPE fY)
	{
		x = fX - w/2.0;
		y = fY - h/2.0;
	}
	inline void enlargeAllSidesBy(FLOAT_TYPE fValue)
	{
		x -= fValue;
		y -= fValue;
		w += fValue*2.0;
		h += fValue*2.0;
	}

	inline void enlargeHorVertBy(FLOAT_TYPE fHor, FLOAT_TYPE fVert)
	{
		x -= fHor;
		y -= fVert;
		w += fHor*2.0;
		h += fVert*2.0;
	}

	inline void shrinkAllSidesBy(FLOAT_TYPE fValue)
	{
		x += fValue;
		y += fValue;
		w -= fValue*2.0;
		h -= fValue*2.0;
	}

	inline void shrinkHorVertBy(FLOAT_TYPE fHor, FLOAT_TYPE fVert)
	{
		x += fHor;
		y += fVert;
		w -= fHor*2.0;
		h -= fVert*2.0;
	}

	inline void setFromCenterAndSize(const SVector2D& svCenter, const SVector2D& svSize)
	{
		x = svCenter.x - svSize.x/2.0;
		y = svCenter.y - svSize.y/2.0;
		w = svSize.x;
		h = svSize.y;
	}

	inline bool doesContain(const TRect2D<T> & srOther) const
	{
		T fEndX = x + w;
		T fEndY = y + h;

		T fOtherEndX = srOther.x + srOther.w;
		T fOtherEndY = srOther.y + srOther.h;

		if(srOther.x >= x && srOther.y >= y &&
			fOtherEndX <= fEndX && fOtherEndY <= fEndY)
			return true;
		else
			return false;
	}

	void getRotatedBBox(FLOAT_TYPE fDegAngle, TRect2D<T>& srBBoxOut) const
	{
		SVector2D svPoints[4];
		svPoints[0].set(x, y);
		svPoints[1].set(x + w, y);
		svPoints[2].set(x + w, y + h);
		svPoints[3].set(x, y + h);
		int iCurr;

		srBBoxOut.set(0, 0, 0, 0);
		for(iCurr = 0; iCurr < 4; iCurr++)
		{
			svPoints[iCurr].rotateCCW(fDegAngle);
			if(iCurr == 0)
				srBBoxOut.set(svPoints[iCurr].x, svPoints[iCurr].y, 0, 0);
			else
				srBBoxOut.includePoint(svPoints[iCurr]);
		}
	}

	void getTransformedBBox(const SMatrix2D& smMatrix, TRect2D<T>& srBBoxOut) const
	{
		SVector2D svPoints[4];
		svPoints[0].set(x, y);
		svPoints[1].set(x + w, y);
		svPoints[2].set(x + w, y + h);
		svPoints[3].set(x, y + h);
		int iCurr;

		srBBoxOut.set(0, 0, 0, 0);
		for(iCurr = 0; iCurr < 4; iCurr++)
		{
			svPoints[iCurr] = smMatrix*svPoints[iCurr];
			if(iCurr == 0)
				srBBoxOut.set(svPoints[iCurr].x, svPoints[iCurr].y, 0, 0);
			else
				srBBoxOut.includePoint(svPoints[iCurr]);
		}
	}

	void getSideNormal(const SVector2D& svPoint, SVector2D& svNormalOut, bool bIsInside) const 
	{
		SVector2D svRayStart(x + w/2.0, y + h/2.0);
		SVector2D svRayDir(svPoint.x - svRayStart.x, svPoint.y - svRayStart.y);
		svRayDir.normalize();
		
		FLOAT_TYPE dT1Out, dT2Out;
		int iSide1IdxOut, iSide2IdxOut;
		
		SRay2D svRay(svRayStart, svRayDir);
		if(!doesIntersect(svRay, dT1Out, dT2Out, &iSide1IdxOut, &iSide2IdxOut))
			return;

		int iFinalSideIdx;
		if(dT1Out > 0 && dT2Out > 0)
		{
			if(iSide1IdxOut >= 0)
				iFinalSideIdx = iSide1IdxOut;
			else
				iFinalSideIdx = iSide2IdxOut;
		}
		else if(dT1Out > 0)
			iFinalSideIdx = iSide1IdxOut;
		else
			iFinalSideIdx = iSide2IdxOut;
		switch(iFinalSideIdx)
		{
			case 0:svNormalOut.set(0, -1);break;
			case 1:svNormalOut.set(1, 0);break;
			case 2:svNormalOut.set(0, 1);break;
			case 3:svNormalOut.set(-1, 0);break;
			default:_ASSERT(0);svNormalOut.set(0,0);
		}

		if(bIsInside)
			svNormalOut *= -1;
	}

	inline bool doesIntersect(const SCircle& sCircle, FLOAT_TYPE& fAmount) const
	{
		if(sCircle.myCenter.x - sCircle.myRadius > x + w ||
			sCircle.myCenter.y - sCircle.myRadius > y + h ||
			sCircle.myCenter.x + sCircle.myRadius < x ||
			sCircle.myCenter.y + sCircle.myRadius < y)		
		{
			fAmount = 0;
			return false;
		}

		// Figure out how much we intersect by
		T x_far, y_far, temp;
		x_far = x + w;
		y_far = y + h;

		fAmount = 9999;

		temp = x_far - (sCircle.myCenter.x - sCircle.myRadius);
		if(temp > 0 && temp < fAmount)
			fAmount = temp;

		temp = y_far - (sCircle.myCenter.y - sCircle.myRadius);
		if(temp > 0 && temp < fAmount)
			fAmount = temp;

		temp = (sCircle.myCenter.x + sCircle.myRadius) - x;
		if(temp > 0 && temp < fAmount)
			fAmount = temp;

		temp = (sCircle.myCenter.y + sCircle.myRadius) - y;
		if(temp > 0 && temp < fAmount)
			fAmount = temp;

		fAmount /= sCircle.myRadius;

		return true;
	}

	inline bool doesContain(const SCircle& sCircle, FLOAT_TYPE& fAmount) const
	{
		if(doesContain(sCircle.myCenter) && 
			sCircle.myCenter.x - sCircle.myRadius >= x &&
			sCircle.myCenter.y - sCircle.myRadius >= y &&
			sCircle.myCenter.x + sCircle.myRadius <= x + w &&
			sCircle.myCenter.y + sCircle.myRadius <= y + h)
		{
			fAmount = 0.0;
			return true;
		}
		else
		{	
			fAmount = 999.0;
			FLOAT_TYPE fTemp;
			if(sCircle.myCenter.x - x < sCircle.myRadius)
			{
				fTemp = (sCircle.myCenter.x - x)/sCircle.myRadius;
				if(fTemp < fAmount) fAmount = fTemp;
			}
			if(sCircle.myCenter.y - y < sCircle.myRadius)
			{
				fTemp = (sCircle.myCenter.y - y)/sCircle.myRadius;
				if(fTemp < fAmount) fAmount = fTemp;
			}
			if(x + w - sCircle.myCenter.x < sCircle.myRadius)
			{
				fTemp = (x + w - sCircle.myCenter.x)/sCircle.myRadius;
				if(fTemp < fAmount) fAmount = fTemp;
			}
			if(y + h - sCircle.myCenter.y < sCircle.myRadius)
			{
				fTemp = (y + h - sCircle.myCenter.y)/sCircle.myRadius;
				if(fTemp < fAmount) fAmount = fTemp;
			}
			
			fAmount = 1.0 - fAmount;

			return false;
		}
	}

	inline bool doesContain(FLOAT_TYPE fX, FLOAT_TYPE fY) const
	{
		if(fX >= x && fX <= x + w &&
			fY >= y && fY <= y + h)
			return true;
		else
			return false;
	}

	inline bool doesContainExclusive(FLOAT_TYPE fX, FLOAT_TYPE fY) const
	{
		if(fX > x && fX < x + w &&
			fY > y && fY < y + h)
			return true;
		else
			return false;
	}

	inline bool doesContain(const SVector2D& svPoint) const
	{
		if(svPoint.x >= x && svPoint.x <= x + w &&
			svPoint.y >= y && svPoint.y <= y + h)
			return true;
		else
			return false;
	}

	inline bool doesContainExclusive(const SVector2D& svPoint) const
	{
		if(svPoint.x >= x && svPoint.x < x + w &&
			svPoint.y >= y && svPoint.y < y + h)
			return true;
		else
			return false;
	}

	inline bool doesIntersect(const TRect2D<T>& other) const
	{
		// See if they're disjoint
		T own_x2 = x + w;
		if(x < other.x && own_x2 < other.x)
			return false;
		
		T own_y2 = y + h;
		if(y < other.y && own_y2 < other.y)
			return false;
		
		T other_x2 = other.x + other.w;
		if(x > other_x2 && own_x2 > other_x2)
			return false;

		T other_y2 = other.y + other.h;
		if(y > other_y2 && own_y2 > other_y2)
			return false;
		
		return true;
	}

	inline void toPoints(SVector2D svPointsOut[4]) const
	{
		svPointsOut[0].set(x, y);
		svPointsOut[1].set(x + w, y);
		svPointsOut[2].set(x + w, y + h);
		svPointsOut[3].set(x, y + h);
	}

	inline void toPointVector(TPointVector& vecOut, bool bCloseLastPoint) const
	{
		int iNumVerts = 4;
		if(bCloseLastPoint)
			iNumVerts++;
		vecOut.resize(iNumVerts);

		vecOut[0].set(x, y);
		vecOut[1].set(x + w, y);
		vecOut[2].set(x + w, y + h);
		vecOut[3].set(x, y + h);

		if(bCloseLastPoint)
			vecOut[4].set(x, y);
	}

	inline void getOverlap( const TRect2D<T>& B, TRect2D<T>& srRes ) const 
	{
		T this_right = x + w;
		T this_bottom = y + h;
		T b_right = B.x + B.w;
		T b_bottom = B.y + B.h;
			
		srRes.x = ( this->x > B.x ? this->x : B.x );
		srRes.y = ( this->y > B.y ? this->y : B.y );
		srRes.w = ( this_right < b_right ? this_right : b_right ) - srRes.x;
		srRes.h = ( this_bottom < b_bottom ? this_bottom : b_bottom ) - srRes.y;
		
		// Now, if they don't intersect, we may get a negative w/h. Set them to 0
		// in that case. One side effect of not doing that is that if we try to 
		// create an image with negative dimensions, we'll crash. This will prevent
		// it.
		if(srRes.w < 0)
			srRes.w = 0;
		if(srRes.h < 0 )
			srRes.h = 0;
	}	

	bool doesIntersectSegment(const SVector2D& svStart, const SVector2D& svEnd) const 
	{
		SRay2D svRay(svStart);
		svRay.myDir = svEnd - svStart;
		FLOAT_TYPE fLen = svRay.normalize();

		FLOAT_TYPE fDist1, fDist2;
		if(doesIntersect(svRay, fDist1, fDist2))
		{
			if( (fDist1 >= FLOAT_EPSILON && fDist1 <= fLen) || (fDist2 >= FLOAT_EPSILON && fDist2 <= fLen))
				return true;
			else
				return false;
		}

		return false;
	}
	
	bool doesIntersect(const SRay2D& pRay, FLOAT_TYPE &dT1Out, FLOAT_TYPE &dT2Out, int *iSideIdx1Out = NULL, int *iSideIdx2Out = NULL) const 
	{
		// Test all four sides.
		SVector2D nvStart;
		SVector2D svLine2Dir;
		FLOAT_TYPE dRes;
		bool bDidFindOneIntersection = false;
		
		dT1Out = FLOAT_TYPE_MAX;
		dT2Out = FLOAT_TYPE_MAX;

		if(iSideIdx1Out)
			*iSideIdx1Out = -1;
		if(iSideIdx2Out)
			*iSideIdx2Out = -1;
		
		nvStart.set(x, y);
		svLine2Dir.set(1,0);
		dRes = pRay.doesIntersectLineSegment(nvStart, svLine2Dir, w, NULL, NULL);
		if(dRes != FLOAT_TYPE_MAX)
		{
			if(!bDidFindOneIntersection)
			{
				dT1Out = dRes;
				bDidFindOneIntersection = true;
				if(iSideIdx1Out)
					*iSideIdx1Out = 0;
			}
			else
			{
				dT2Out = dRes;
				if(iSideIdx2Out)
					*iSideIdx2Out = 0;
				return true;
			}
		}
		
		nvStart.set(x + w, y);
		svLine2Dir.set(0, 1);
		dRes = pRay.doesIntersectLineSegment(nvStart, svLine2Dir, h, NULL, NULL);
		// To prevent the ray that falls right onto the corner from intersecting two sides at
		// the joint
		if(bDidFindOneIntersection && fabs(dRes - dT1Out) <= FLOAT_EPSILON)
			dRes = FLOAT_TYPE_MAX;
		if(dRes != FLOAT_TYPE_MAX)
		{
			
			if(!bDidFindOneIntersection)
			{
				dT1Out = dRes;
				bDidFindOneIntersection = true;
				if(iSideIdx1Out)
					*iSideIdx1Out = 1;
			}
			else
			{
				dT2Out = dRes;
				if(iSideIdx2Out)
					*iSideIdx2Out = 1;
				return true;
			}
		}	
		
		nvStart.set(x + w, y + h);
		svLine2Dir.set(-1, 0);
		dRes = pRay.doesIntersectLineSegment(nvStart, svLine2Dir, w, NULL, NULL);
		// To prevent the ray that falls right onto the corner from intersecting two sides at
		// the joint
		if(bDidFindOneIntersection && fabs(dRes - dT1Out) <= FLOAT_EPSILON)
			dRes = FLOAT_TYPE_MAX;
		if(dRes != FLOAT_TYPE_MAX)
		{
			if(!bDidFindOneIntersection)
			{
				dT1Out = dRes;
				bDidFindOneIntersection = true;
				if(iSideIdx1Out)
					*iSideIdx1Out = 2;

			}
			else
			{
				if(iSideIdx2Out)
					*iSideIdx2Out = 2;
				dT2Out = dRes;
				return true;
			}
		}	
		
		nvStart.set(x, y + h);
		svLine2Dir.set(0, -1);
		dRes = pRay.doesIntersectLineSegment(nvStart, svLine2Dir, h, NULL, NULL);
		// To prevent the ray that falls right onto the corner from intersecting two sides at
		// the joint
		if(bDidFindOneIntersection && fabs(dRes - dT1Out) <= FLOAT_EPSILON)
			dRes = FLOAT_TYPE_MAX;
		if(dRes != FLOAT_TYPE_MAX)
		{
			if(!bDidFindOneIntersection)
			{
				dT1Out = dRes;
				bDidFindOneIntersection = true;
				if(iSideIdx1Out)
					*iSideIdx1Out = 3;
			}
			else
			{
				if(iSideIdx2Out)
					*iSideIdx2Out = 3;
				dT2Out = dRes;
				return true;
			}
		}	
		
		if(dT1Out ==  FLOAT_TYPE_MAX && dT2Out == FLOAT_TYPE_MAX)
			return false;
		else
			return true;

	}

	template < class POINT_TYPE > 
	bool doesIntersectOrContain(const vector<POINT_TYPE>& vecPoints) const 
	{
		int iCurr, iNum = vecPoints.size();
		for(iCurr = 0 ;iCurr < iNum; iCurr++)
		{
			if(this->doesContain(vecPoints[iCurr].getPoint()))
				return true;
		}

		// Now check all segmetns for intersection
		for(iCurr = 1 ;iCurr < iNum; iCurr++)
		{
			if(doesIntersectSegment(vecPoints[iCurr - 1].getPoint(), vecPoints[iCurr].getPoint()))
				return true;
		}
		
		return false;
	}

	inline void includeRect(const TRect2D<T>& other)
	{
		if(w == 0.0 && h == 0.0)
		{
			*this = other;
			return;
		}

		// Include the four points
		includePoint(other.x, other.y);
		includePoint(other.x + other.w, other.y);
		includePoint(other.x, other.y + other.h);
		includePoint(other.x + other.w, other.y + other.h);
	}

	inline void fromTwoPoints(const SVector2D& svPoint1, const SVector2D& svPoint2)
	{
		x = min(svPoint1.x, svPoint2.x);
		y = min(svPoint1.y, svPoint2.y);
		w = fabs(svPoint2.x - svPoint1.x);
		h = fabs(svPoint2.y - svPoint1.y);
	}

	inline void includePoint(const SVector2D& svPoint) 
	{ 
		includePoint(svPoint.x, svPoint.y); 
	}

	inline void includePoint(FLOAT_TYPE fX, FLOAT_TYPE fY)
	{
		if(fX < x) 
		{
			T fIncr = x - fX;
			x = fX;
			w += fIncr;
		}
		if(fY < y) 
		{
			T fIncr = y - fY;
			y = fY;
			h += fIncr;
		}
		if(fX > x + w) w = fX - x;
		if(fY > y + h) h = fY - y;
	}

	inline void includePoints(const SVector2D* pPoints, int iNumPoints)
	{
		int iCurrPoint;
		for(iCurrPoint = 0; iCurrPoint < iNumPoints; iCurrPoint++)
		{
			if(w == 0 && h == 0 && iCurrPoint == 0)
			{
				x = pPoints[iCurrPoint].x;
				y = pPoints[iCurrPoint].y;
				continue;
			}

			includePoint(pPoints[iCurrPoint]);
		}
	}

	inline void getCenter(SVector2D& svOut) const
	{
		svOut.x = x + w/2.0;
		svOut.y = y + h/2.0;
	}

	inline void fromObb(const SOBB& rOther)
	{
		SVector2D svDir, svPerp;
		
		svDir.set(1,0);
		svDir.rotateCCW(rOther.myAngle);
		svPerp.set(svDir.y, -svDir.x);
		
		SVector2D pts[4];
		
		pts[0].x = rOther.myCenter.x + svDir.x*rOther.mySize.x/2.0 + svPerp.x*rOther.mySize.y/2.0;
		pts[0].y = rOther.myCenter.y + svDir.y*rOther.mySize.x/2.0 + svPerp.y*rOther.mySize.y/2.0;
		
		pts[1].x = rOther.myCenter.x + svDir.x*rOther.mySize.x/2.0 - svPerp.x*rOther.mySize.y/2.0;
		pts[1].y = rOther.myCenter.y + svDir.y*rOther.mySize.x/2.0 - svPerp.y*rOther.mySize.y/2.0;
		
		pts[2].x = rOther.myCenter.x - svDir.x*rOther.mySize.x/2.0 - svPerp.x*rOther.mySize.y/2.0;
		pts[2].y = rOther.myCenter.y - svDir.y*rOther.mySize.x/2.0 - svPerp.y*rOther.mySize.y/2.0;
		
		pts[3].x = rOther.myCenter.x - svDir.x*rOther.mySize.x/2.0 + svPerp.x*rOther.mySize.y/2.0;
		pts[3].y = rOther.myCenter.y - svDir.y*rOther.mySize.x/2.0 + svPerp.y*rOther.mySize.y/2.0;
		
		x = pts[0].x;
		y = pts[0].y;
		w = h = 1;
		
		this->includePoint(pts[1].x, pts[1].y);
		this->includePoint(pts[2].x, pts[2].y);
		this->includePoint(pts[3].x, pts[3].y);
	}

	inline bool operator==(const TRect2D<T>& srOther) const
	{
		return (x == srOther.x) && (y == srOther.y) && (w == srOther.w) && (h == srOther.h);
	}

	inline bool operator!=(const TRect2D<T>& srOther) const
	{
		return !((*this) == srOther);
	}

	inline bool getIsIntEqual(const TRect2D<T>& srOther) const
	{
		return ((int)srOther.x) == ((int)x) && ((int)srOther.y) == ((int)y) && ((int)srOther.w) == ((int)w) && ((int)srOther.h) == ((int)h);
	}

	inline bool getIsIntEqualOrLarger(const TRect2D<T>& srOther) const
	{
		return ((int)srOther.x) >= ((int)x) && ((int)srOther.y) >= ((int)y) && 
			((int)(srOther.w + srOther.x)) <= ((int)(x + w)) && ((int)(srOther.y + srOther.h)) <= ((int)(y + h));
	}

	inline void clampToRect(const TRect2D<T>& srOther)	
	{ 
		clampToRect(srOther.x, srOther.y, srOther.w, srOther.h); 
	}

	inline void clampToRect(T fX, T fY, T fW, T fH)
	{
		if(x < fX)
		{
			w -= (fX - x);
			x = fX;
		}
		if(y < fY)
		{
			h -= (fY - y);
			y = fY;
		}
		if(x + w > fX + fW)
			w = fX + fW - x;
		if(y + h > fY + fH)
			h = fY + fH - y;
	}

	inline void upToScreen()
	{
		this->x = HyperCore::upToScreen(x);
		this->y = HyperCore::upToScreen(y);
		this->w = HyperCore::upToScreen(w);
		this->h = HyperCore::upToScreen(h);
	}

	inline TRect2D<T>& operator*=(FLOAT_TYPE fMultiplicant)
	{
		x *= fMultiplicant;
		y *= fMultiplicant;
		w *= fMultiplicant;
		h *= fMultiplicant;
		return *this;
	}

	inline TRect2D<T> operator*(FLOAT_TYPE fMultiplicant) const
	{
		TRect2D<T> srRes;
		srRes.x = x*fMultiplicant;
		srRes.y = y*fMultiplicant;
		srRes.w = w*fMultiplicant;
		srRes.h = h*fMultiplicant;
		return srRes;
	}

	inline TRect2D<T> operator/(FLOAT_TYPE fMultiplicant) const
	{
		TRect2D<T> srRes;
		srRes.x = x/fMultiplicant;
		srRes.y = y/fMultiplicant;
		srRes.w = w/fMultiplicant;
		srRes.h = h/fMultiplicant;
		return srRes;
	}

	inline TRect2D<T>& operator/=(FLOAT_TYPE fMultiplicant)
	{
		x /= fMultiplicant;
		y /= fMultiplicant;
		w /= fMultiplicant;
		h /= fMultiplicant;
		return *this;
	}

	T x,y,w,h;

	static const TRect2D Zero;
};
/*****************************************************************************/
typedef TRect2D < FLOAT_TYPE > SRect2D;
/*****************************************************************************/
typedef vector < SRect2D > TRectVector;
/*****************************************************************************/
