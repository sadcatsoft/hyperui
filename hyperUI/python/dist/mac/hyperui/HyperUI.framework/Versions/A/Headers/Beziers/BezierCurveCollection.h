#pragma once

enum CornerType
{
	CornerNone = 0,
	CornerRound,
	CornerStraight,
};

const char* const g_pcsCornerTypeStrings[] =
{
	"none",
	"round",
	"straight",

	0
};

template < class TYPE > class HYPERUI_API TBezierPolygonCollection;
#define BEZIER_CURVE_TYPE		TBezierCurve< TYPE >
/*****************************************************************************/
template < class TYPE >
class HYPERUI_API TBezierCurveCollection : public IBaseObject
{
public:
	TBezierCurveCollection();
	virtual ~TBezierCurveCollection();

	void clear();
	void addCurve(BEZIER_CURVE_TYPE* pCurve, bool bInsertInFront = false);
	BEZIER_CURVE_TYPE* addNewCurve(bool bIsClosed);
	void deleteCurve(BEZIER_CURVE_TYPE* pCurve);

	int getNumCurves() const { return myCurves.size(); }
	const BEZIER_CURVE_TYPE* getCurve(int iIndex) const { return myCurves[iIndex]; }
	BEZIER_CURVE_TYPE* getCurve(int iIndex) { return myCurves[iIndex]; }

	void copyFrom(const TBezierCurveCollection& rOther, bool bIgnoreBackgroundCurves = false, FLOAT_TYPE fPerfScalingFactor = 1.0);
	void mergeFrom(const TBezierCurveCollection& rOther, bool bIgnoreBackgroundCurves = false, bool bInsertInFront = false, FLOAT_TYPE fPerfScalingFactor = 1.0);

	bool getBBox(SRect2D& svBBoxOut, bool bIgnoreBackgroundCurves = false) const;

	BEZIER_CURVE_TYPE* createAndAddEllipse(FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fRadiusX, FLOAT_TYPE fRadiusY);
	BEZIER_CURVE_TYPE* createAndAddRectangle(FLOAT_TYPE fStartX, FLOAT_TYPE fStartY, FLOAT_TYPE fW, FLOAT_TYPE fH, FLOAT_TYPE fCornerRadius, CornerType eCornerType);
	BEZIER_CURVE_TYPE* createAndAddPolygon(FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fRadiusX, FLOAT_TYPE fRadiusY, int iNumPoints);
	BEZIER_CURVE_TYPE* createAndAddStar(FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fRadiusX, FLOAT_TYPE fRadiusY, int iNumPoints, FLOAT_TYPE fIndentPerc);
	BEZIER_CURVE_TYPE* createAndAddPieSlice(FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fRadiusX, FLOAT_TYPE fRadiusY, FLOAT_TYPE fStartAngle = 0, FLOAT_TYPE fSpan = 360, bool bIsPie = true, HyperCore::OrientationType eOrientation = OrientationCW);
	BEZIER_CURVE_TYPE* createAndAddRing(FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fOuterRadiusX, FLOAT_TYPE fOuterRadiusY, FLOAT_TYPE fInnerRadiusX, FLOAT_TYPE fInnerRadiusY, FLOAT_TYPE fStartAngle = 0, FLOAT_TYPE fSpan = 360, HyperCore::OrientationType eOrientation = OrientationCW);
	BEZIER_CURVE_TYPE* createAndAddLine(FLOAT_TYPE fStartX, FLOAT_TYPE fStartY, FLOAT_TYPE fEndX, FLOAT_TYPE fEndY);

	TYPE* findSegmentByEdgeId(const EdgeId& idEdge) const;
	const TYPE* findPointClosestTo(const SVector2D& svPoint, FLOAT_TYPE fMaxDistance, SVector2D& svPointOut, FLOAT_TYPE* fOptParmDistOut = NULL) const;
	const TYPE* findVertexClosestTo(const SVector2D& svPoint, FLOAT_TYPE fOptMaxRadius, SVector2D& svResPointOut, BezierPointType& ePointTypeOut) const;

	void onChildSegmentEdgeIdChanged(const EdgeId& idOld, const EdgeId& idNew);
	UNIQUEID_TYPE getNextId();

	void transformBy(const SMatrix2D& smTransform);
	void translateBy(const SVector2D& svDiff);

	void deleteCurvesWithBBoxSmallerThan(FLOAT_TYPE fBBoxSize);

	template < class MY_TYPE, class LIN_POINT_TYPE >
	void resampleLinearInto(TPolylineCollection < LIN_POINT_TYPE >& rCollOut, FLOAT_TYPE fMaxError)
	{
		rCollOut.clear();
		TPolyline2D<LIN_POINT_TYPE> *pLine;

		int iCurr, iNum = myCurves.size();
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
			pLine = rCollOut.addPolyline();
			myCurves[iCurr]->resampleCurves(fMaxError, -1, *pLine, NULL);
		}
	}

	void saveToItem(StringResourceItem& rItemOut) const;
	void loadFromItem(const StringResourceItem& rItem);
	static bool canLoadFrom(const StringResourceItem& rItem);

	TBezierCurveCollection<TYPE>* cloneSelf(bool bIgnoreBackgroundCurves = false) const;

	void setParent(TBezierPolygonCollection<TYPE> *pParent) { myParent = pParent; }

	inline UNIQUEID_TYPE getMaxId() const { return myMaxId; }

	void resetEdgeIdsWithString(const char* pcsStringPart);
	void applyMaterialId(UNIQUEID_TYPE idMaterial);

	// Attempts to automatically compute which countours are holes.
	// The assumption is that the outermost curve is not a hole, and all
	// fully contained curves are alternating.
	void classifyHoles();

private:

	int setToArc(BEZIER_CURVE_TYPE* pNew, FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fRadiusX, 
		FLOAT_TYPE fRadiusY, FLOAT_TYPE fStartAngle, FLOAT_TYPE fSpan, HyperCore::OrientationType eOrientation, int iStartSegmentId);

	void computeArcBezierApproximation(FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fRadiusX, FLOAT_TYPE fRadiusY, FLOAT_TYPE fStartAngle, FLOAT_TYPE fSpan, 
		HyperCore::OrientationType eOrientation, SVector2D& svStartOut, SVector2D& svControl1Out, SVector2D& svControl2Out, SVector2D& svEndOut);

private:

	vector < BEZIER_CURVE_TYPE* > myCurves;
	TBezierPolygonCollection<TYPE> *myParent;

	UNIQUEID_TYPE myMaxId;
};
typedef TBezierCurveCollection<SBezierSegment> BezierCurveCollection;
/*****************************************************************************/