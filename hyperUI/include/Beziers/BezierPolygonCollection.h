#pragma once

#define BEZIER_POLY_TYPE		TBezierCurveCollection< TYPE >
/*****************************************************************************/
template < class TYPE >
class HYPERUI_API TBezierPolygonCollection : public IBaseObject
{
public:
	TBezierPolygonCollection();
	virtual ~TBezierPolygonCollection();

	void clear();
	BEZIER_POLY_TYPE* addNewPolygon();
	void addPolygon(BEZIER_POLY_TYPE* pPoly, bool bInsertInFront = false);

	bool getBBox(SRect2D& svBBoxOut, bool bIgnoreBackgroundCurves = false) const;
	void copyFrom(const TBezierPolygonCollection& rOther, bool bIgnoreBackgroundCurves = false);
	void mergeFrom(const TBezierPolygonCollection& rOther, bool bIgnoreBackgroundCurves = false, bool bInsertInFront = false);

	void transformBy(const SMatrix2D& smTransform);
	void translateBy(const SVector2D& svDiff);

	int getNumPolygons() const { return myPolys.size(); }
	const BEZIER_POLY_TYPE* getPolygon(int iIndex) const { return myPolys[iIndex]; }
	BEZIER_POLY_TYPE* getPolygon(int iIndex) { return myPolys[iIndex]; }

	template < class MY_TYPE, class LIN_POINT_TYPE >
	void resampleLinearInto(TBezierPolygonCollection < LIN_POINT_TYPE >& rCollOut, FLOAT_TYPE fMaxError)
	{
		rCollOut.clear();
		TPolygon2DCollection<LIN_POINT_TYPE> *pLineColl;

		int iCurr, iNum = myPolys.size();
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
			pLineColl = rCollOut.addPolygon();
			myPolys[iCurr]->resampleLinearInto(*pLineColl, fMaxError);
		}
	}

	void saveToItem(StringResourceItem& rItemOut) const;
	void loadFromItem(const StringResourceItem& rItem);

	TYPE* findSegmentByEdgeId(const EdgeId& idEdge) const;
	const TYPE* findPointClosestTo(const SVector2D& svPoint, SVector2D& svPointOut, FLOAT_TYPE* fOptParmDistOut = NULL) const;
	const TYPE* findVertexClosestTo(const SVector2D& svPoint, FLOAT_TYPE fOptMaxRadius, SVector2D& svResPointOut, BezierPointType& ePointTypeOut) const;

	void onChildSegmentEdgeIdChanged(const EdgeId& idOld, const EdgeId& idNew);
	UNIQUEID_TYPE getNextId();

private:

	vector < BEZIER_POLY_TYPE* > myPolys;

	UNIQUEID_TYPE myMaxId;
};
/*****************************************************************************/
typedef TBezierPolygonCollection<SBezierSegment> BezierPolygonCollection;
