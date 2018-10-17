#pragma once

/********************************************************************************************/
class HYPERUI_API DeformableBezier
{
public:

	DeformableBezier();
	virtual ~DeformableBezier();

	void evaluate(FLOAT_TYPE dInterpPos, SVector2D& svOut) const;
	void getTangent(FLOAT_TYPE dInterpPos, SVector2D& svTangentOut) const;

	void setStart(FLOAT_TYPE dx, FLOAT_TYPE dy);
	void setEnd(FLOAT_TYPE dx, FLOAT_TYPE dy);
	void setControl1(FLOAT_TYPE dx, FLOAT_TYPE dy);
	void setControl2(FLOAT_TYPE dx, FLOAT_TYPE dy);
	void setIsLinear(bool bValue);

	void onTimerTick(GTIME lCurrentTime);
	FLOAT_TYPE getLength(FLOAT_TYPE dTolerance, FLOAT_TYPE dMaxPointSpacing);
	void resample(FLOAT_TYPE dError, FLOAT_TYPE dMaxPointSpacing, bool bAddFirstPoint, TPointVector& rPointsOut, TPointVector* optTangentsOut) const;

	AnimatedPoint* getStart(void);
	AnimatedPoint* getEnd(void);

	DeformableBezier* cloneSelf(FLOAT_TYPE fPerfScalingFactor = 1.0) const;

	inline bool getIsLinear() const  { return myIsLinear; }

	void getBBox(SRect2D& srRectOut) const;
	inline void setEdgeId(UNIQUEID_TYPE id) { myEdgeId.setFromNumeric(id); }
	inline void setEdgeId(const char* pcsId) { myEdgeId.setFromString(pcsId); }
	void setEdgeId(const EdgeId& rOther) { myEdgeId = rOther; }

	void getPointByType(BezierPointType ePointType, SVector2D& svOut) const;
	void setPointByType(BezierPointType ePointType, const SVector2D& svIn);

	inline void setParent(TBezierCurve<DeformableBezier>* pParent) { myParent = pParent; }
	inline TBezierCurve<DeformableBezier>* getParent() const { return myParent; }

	void movePointBy(BezierPointType ePointType, FLOAT_TYPE fX, FLOAT_TYPE fY, bool bAllowParentCallback, bool bIgnoreTiedControls);
	void ensureIsLinear();

	void copyFrom(const DeformableBezier& rOther, FLOAT_TYPE fPerfScalingFactor = 1.0);
	void transformBy(const SMatrix2D& smTransform);

	void findPointClosestTo(const SVector2D& svPoint, SVector2D& svPointOut, FLOAT_TYPE* fOptParmDistOut = NULL) const { _ASSERT(0); }
	inline BezierPointCurveType getPointCurveType(BezierPointType ePointType) const { _ASSERT(0); return BezierPointCurveTied; }

	void reverse();

	inline void setStartPointCurveType(BezierPointCurveType eType) { _ASSERT(0); }
	inline void setEndPointCurveType(BezierPointCurveType eType) { _ASSERT(0); }

	void saveToItem(StringResourceItem& rItemOut) const { _ASSERT(0); }
	void loadFromItem(const StringResourceItem& rItem) { _ASSERT(0); }

	void makeLinear() { _ASSERT(0); }
	inline const EdgeId& getEdgeId() const { return myEdgeId; }
	bool findVertexClosestTo(const SVector2D& svPoint, FLOAT_TYPE fOptMaxRadius, SVector2D& svResPointOut, BezierPointType& ePointTypeOut) const { _ASSERT(0); return true; }

	inline void getStart(SVector2D& svOut) const { _ASSERT(0); } 
	inline void getControl1(SVector2D& svOut) const { _ASSERT(0); } 
	inline void getControl2(SVector2D& svOut) const { _ASSERT(0); } 
	inline void getEnd(SVector2D& svOut) const { _ASSERT(0); } 

	inline BezierPointCurveType getStartPointCurveType() const { _ASSERT(0); return BezierPointCurveTied; }
	inline BezierPointCurveType getEndPointCurveType() const { _ASSERT(0); return BezierPointCurveTied; }

	FLOAT_TYPE computeMaxError(const BezierPolyline2D& rPoints, int iFirst, int iLast, TFloatVector &rUParams, int &iMaxErrorPoint) { _ASSERT(0); return 0; } 
	inline void setEdgeIdSourceId(UNIQUEID_TYPE idSource) { myEdgeId.setSourceId(idSource); }

	// Disable the equality operator, since we have poitners now.
	// NOT IMPLEMENTED
	DeformableBezier& operator=(const DeformableBezier& rOther) { _ASSERT(0); return *this; }

private:
	void resampleSingleBezierRecursive(FLOAT_TYPE dTStart, FLOAT_TYPE dTEnd, FLOAT_TYPE dErrorSq, FLOAT_TYPE dMaxPointSpacingSq, TPointVector& rPointsOut, TPointVector* optTangentsOut) const;
	FLOAT_TYPE computeDistancesOnPiecewiseLinear(TPointVector& rRawPoints);

private:
	AnimatedPoint myStart, myEnd;
	AnimatedPoint myControl1, myControl2;
	// Hack to treat it as a line
	bool myIsLinear;

	ClockType myActiveClock;

	EdgeId myEdgeId;

	TBezierCurve<DeformableBezier>* myParent;
};
typedef vector < DeformableBezier* > TDeformableBezierVector;
/********************************************************************************************/
