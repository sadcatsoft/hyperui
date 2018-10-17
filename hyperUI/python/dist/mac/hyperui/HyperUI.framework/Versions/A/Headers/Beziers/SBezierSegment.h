#pragma once

template < class TYPE > class HYPERUI_API TBezierCurve;

/*****************************************************************************/
class HYPERUI_API SBezierSegment
{
public:
	SBezierSegment();
	virtual ~SBezierSegment();

	void evaluate(FLOAT_TYPE dInterpPos, SVector2D& svOut) const;
	static void evaluate(FLOAT_TYPE dInterpPos, SVector2D* svPoints, SVector2D& svOut);
	void getTangent(FLOAT_TYPE dInterpPos, SVector2D& svTangentOut) const;

	void setStart(FLOAT_TYPE dx, FLOAT_TYPE dy);
	void setEnd(FLOAT_TYPE dx, FLOAT_TYPE dy);
	void setControl1(FLOAT_TYPE dx, FLOAT_TYPE dy);
	void setControl2(FLOAT_TYPE dx, FLOAT_TYPE dy);
	void makeLinear();
	//void setIsLinear(bool bValue);

	inline BezierPointCurveType getStartPointCurveType() const { return myStartPointType; }
	inline BezierPointCurveType getEndPointCurveType() const { return myEndPointType; }

	inline void setStartPointCurveType(BezierPointCurveType eType) { myStartPointType = eType; }
	inline void setEndPointCurveType(BezierPointCurveType eType) { myEndPointType = eType; }

	FLOAT_TYPE getLength(FLOAT_TYPE dTolerance, FLOAT_TYPE dMaxPointSpacing) const;

	inline void getStart(SVector2D& svOut) const { svOut = myStart; }
	inline void getControl1(SVector2D& svOut) const { svOut = myControl1; }
	inline void getControl2(SVector2D& svOut) const { svOut = myControl2; }
	inline void getEnd(SVector2D& svOut) const { svOut = myEnd; }

	void resample(FLOAT_TYPE dError, FLOAT_TYPE dMaxPointSpacing, bool bAddFirstPoint, TPointVector& rPointsOut, TPointVector* optTangentsOut);

	void getBBox(SRect2D& srRectOut) const;

	virtual SBezierSegment* cloneSelf(FLOAT_TYPE fPerfScalingFactor = 1.0) const;

	//inline bool getIsLinear() const { return myIsLinear; }

	void setEdgeId(UNIQUEID_TYPE id);
	inline void setEdgeId(const char* pcsId) { myEdgeId.setFromString(pcsId); }
	void setEdgeId(const EdgeId& rOther) { myEdgeId = rOther; }
	inline const EdgeId& getEdgeId() const { return myEdgeId; }
	inline void setEdgeIdSourceId(UNIQUEID_TYPE idSource) { myEdgeId.setSourceId(idSource); }

	void getPointByType(BezierPointType ePointType, SVector2D& svOut) const;
	void setPointByType(BezierPointType ePointType, const SVector2D& svIn);

	inline void setParent(TBezierCurve<SBezierSegment>* pParent) { myParent = pParent; }
	inline TBezierCurve<SBezierSegment>* getParent() const { return myParent; }

	void movePointBy(BezierPointType ePointType, FLOAT_TYPE fX, FLOAT_TYPE fY, bool bAllowParentCallback, bool bIgnoreTiedControls);
	void ensureIsLinear();

	// Returns the t-interp value that can be used for evaluation
	FLOAT_TYPE findPositionClosestTo(const SVector2D& svPoint) const;
	// Returns the actual position on the curve
	void findPointClosestTo(const SVector2D& svPoint, SVector2D& svPointOut, FLOAT_TYPE* fOptParmDistOut = NULL) const;
	bool findVertexClosestTo(const SVector2D& svPoint, FLOAT_TYPE fOptMaxRadius, SVector2D& svResPointOut, BezierPointType& ePointTypeOut) const;

	virtual void copyFrom(const SBezierSegment& rOther, FLOAT_TYPE fPerfScalingFactor = 1.0);

	void transformBy(const SMatrix2D& smTransform);

	void reverse();

	inline BezierPointCurveType getPointCurveType(BezierPointType ePointType) const { return (ePointType == BezierPointStart || ePointType == BezierPointControl1) ? myStartPointType : myEndPointType; }

	virtual void saveToItem(StringResourceItem& rItemOut) const;
	virtual void loadFromItem(const StringResourceItem& rItem);

	void onTimerTick(GTIME lCurrentTime) { }

	template < class POINT_TYPE > 
	FLOAT_TYPE computeMaxError(const vector<POINT_TYPE>& rPoints, int iFirst, int iLast, TFloatVector &rUParams, int &iMaxErrorPoint)
	{
		int			iCurrPoint;
		FLOAT_TYPE	dMaxDist;		
		FLOAT_TYPE	dCurrDist;
		SVector2D svCurvePoint;

		iMaxErrorPoint = (iLast - iFirst + 1)/2 + iFirst;
		dMaxDist = 0.0;

		// NOTE: In some cases, the curve goes almost exactly through
		// the start, end, and (one) middle point, but fluctuates
		// wildly for the distance between these. 
		// Try simple check - test the middle between first and next point
		if(iLast > iFirst)
		{
			this->evaluate((rUParams[0] + rUParams[1])*0.5, svCurvePoint);
			dCurrDist = (((rPoints[iFirst].getPoint() + rPoints[iFirst+1].getPoint())*0.5) - svCurvePoint).lengthSquared();
			if (dCurrDist >= dMaxDist) 
			{
				dMaxDist = dCurrDist;
				iMaxErrorPoint = iFirst+1;
			}
		}

		for (iCurrPoint = iFirst + 1; iCurrPoint < iLast; iCurrPoint++) 
		{
			this->evaluate(rUParams[iCurrPoint - iFirst], svCurvePoint);
			dCurrDist = (svCurvePoint - rPoints[iCurrPoint].getPoint()).lengthSquared();
			if (dCurrDist >= dMaxDist) 
			{
				dMaxDist = dCurrDist;
				iMaxErrorPoint = iCurrPoint;
			}
		}
		return dMaxDist;
	}

	SBezierSegment& operator=(const SBezierSegment& rOther) { this->copyFrom(rOther); return *this; }

private:
	void resampleSingleBezierRecursive(FLOAT_TYPE dTStart, FLOAT_TYPE dTEnd, FLOAT_TYPE dErrorSq, FLOAT_TYPE dMaxPointSpacingSq, TPointVector& rPointsOut, TPointVector* optTangentsOut) const;
	FLOAT_TYPE computeDistancesOnPiecewiseLinear(TPointVector& rRawPoints) const;

	void convertToBezierForm(const SVector2D& svPoint, SVector2D* rEquationCtlPointsOut) const;
	int findRootsInternal(SVector2D* rEquationCtlPoints, int degree, FLOAT_TYPE* t, int depth) const;
	void evaluateBezierInternal(SVector2D* svPoints, int degree, FLOAT_TYPE t, SVector2D* Left, SVector2D* Right, SVector2D& svOut) const;
	FLOAT_TYPE computeXIntercept(SVector2D* rPoints, int degree) const;
	int isControlPolygonFlatEnough(SVector2D* rPoints, int degree) const;
	int computeCrossingCount(SVector2D* rPoints, int degree) const;

private:
	SVector2D myStart, myEnd;
	SVector2D myControl1, myControl2;
	// Hack to treat it as a line
	//bool myIsLinear;
	BezierPointCurveType myStartPointType, myEndPointType;

	EdgeId myEdgeId;

	TBezierCurve<SBezierSegment>* myParent;
};
/*****************************************************************************/
