#pragma once

template < class TYPE > class HYPERUI_API TPolyline2D;
typedef TPolyline2D < SBezierPoint > BezierPolyline2D;

#define EDGE_ID_TOP			1
#define EDGE_ID_RIGHT		2
#define EDGE_ID_BOTTOM		3
#define EDGE_ID_LEFT		4
#define EDGE_ID_TR			5
#define EDGE_ID_BR			6
#define EDGE_ID_BL			7
#define EDGE_ID_TL			8

template < class TYPE > class HYPERUI_API TBezierCurveCollection;
/********************************************************************************************/
// A curve made up of multiple deformable beziers
template < class TYPE >
class HYPERUI_API TBezierCurve
{
public:
	TBezierCurve();
	virtual ~TBezierCurve();

	TYPE* addLinear(const SVector2D& svStart, const SVector2D& svEnd, UNIQUEID_TYPE lOptEdgeId = 0);
	TYPE* addBezier(const SVector2D& svStart, const SVector2D& svControl1, const SVector2D& svControl2, const SVector2D& svEnd, bool bIsLinear = false, UNIQUEID_TYPE lOptEdgeId = 0);
	TYPE* insertLinear(int iPos, SVector2D& svStart, SVector2D& svEnd, UNIQUEID_TYPE lOptEdgeId = 0);
	TYPE* insertBezier(int iPos, SVector2D& svStart, SVector2D& svControl1, SVector2D& svControl2, SVector2D& svEnd, bool bIsLinear = false, UNIQUEID_TYPE lOptEdgeId = 0);
	int getNumSegments(void) const;
	inline const TYPE* getSegment(int iIndex) const { if(iIndex >= 0 && iIndex < mySegments.size()) return mySegments[iIndex]; else return NULL; }
	inline TYPE* getSegment(int iIndex) { return mySegments[iIndex]; }
	void clear();

	void operator=(const TBezierCurve& other);
	void reverse();
	void append(const TBezierCurve& rSource);

	FLOAT_TYPE getLength() const;

	void resampleCurves(FLOAT_TYPE dError, FLOAT_TYPE dMaxPointSpacing, TPointVector &rPointsOut, TPointVector* optTangentsOut) const;
	bool loadFromItem(const char*pcsItem);

	void onTimerTick(GTIME lCurrentTime);

	void setIsClosed(bool bValue) { myIsClosed = bValue; }
	inline bool getIsClosed() const { return myIsClosed; }

	TBezierCurve* cloneSelf(FLOAT_TYPE fPerfScalingFactor = 1.0) const;
	void copyFrom(const TBezierCurve& rOther, FLOAT_TYPE fPerfScalingFactor = 1.0);

	void getBBox(SRect2D& svBBoxOut) const;
	TYPE* findSegmentByEdgeId(const EdgeId& idEdge) const;

	void onSegmentPointChanged(TYPE* pSegment, BezierPointType ePointType, FLOAT_TYPE fX, FLOAT_TYPE fY, bool bIgnoreTiedControls);

	const TYPE* findPointClosestTo(const SVector2D& svPoint, SVector2D& svPointOut, FLOAT_TYPE* fOptParmDist = NULL, int* iOptSegmentIndexOut = NULL) const;
	const TYPE* findVertexClosestTo(const SVector2D& svPoint, FLOAT_TYPE fOptMaxRadius,  SVector2D& svResPointOut, BezierPointType& ePointTypeOut) const;
	void splitEdge(TYPE* pSegment, FLOAT_TYPE fSplitRatio);
	void deletePoint(TYPE* pSegment, BezierPointType ePointType);
	void convertPoint(TYPE* pSegment, BezierPointType ePointType, BezierPointCurveType eCornerType);

	void onChildEdgeIdChanged(const EdgeId& idOld, const EdgeId& idNew);

	void setParent(TBezierCurveCollection<TYPE> *pParent) { myParent = pParent; }
	UNIQUEID_TYPE getNextId();

	UNIQUEID_TYPE getMaxId() const;

	inline void setMaterialId(UNIQUEID_TYPE id) { myMaterialId = id; }
	inline UNIQUEID_TYPE getMaterialId() const { return myMaterialId; }

	void transformBy(const SMatrix2D& smTransform);

	inline void setIsBackground(bool bValue) { myIsBackground = bValue; }
	inline bool getIsBackground() const { return myIsBackground; }

	void saveToItem(StringResourceItem& rItemOut) const;
	void loadFromItem(const StringResourceItem& rItem);

	// If the curve is closed, ensures the first and last points coincide.
	void ensurePointsClosed(bool bMoveLast);

	// Note that these assume we only have linear segments
 	void toPolyline(BezierPolyline2D& rPolylineOut) const;
 	void fromPolyline(const BezierPolyline2D& rPolylineIn, bool bIsClosed);
	
	template < class POINT_TYPE > void fitBezierFromPoints(TPolyline2D<POINT_TYPE>& rPolylineInMofiable, FLOAT_TYPE fMaxError, bool bIsClosed);
	
	void setEdgeSourceIds(UNIQUEID_TYPE idSourceId);
	void resetEdgeIdsWithString(const char* pcsStringPart);

	void setIsHole(bool bValue) { myIsAHole = bValue; }
	bool getIsHole() const { return myIsAHole; }

	void makeIntoRectangle(FLOAT_TYPE fStartX, FLOAT_TYPE fStartY, FLOAT_TYPE fW, FLOAT_TYPE fH);

	inline void setIsStartArrowEnabled(bool bIsEnabled) { myIsStartArrowEnabled = bIsEnabled; }
	inline void setStartArrowParms(FLOAT_TYPE fLengthMult, FLOAT_TYPE fWidthMult) { myStartArrowLengthMult = fLengthMult; myStartArrowWidthMult = fWidthMult; }

	inline void setIsEndArrowEnabled(bool bIsEnabled) { myIsEndArrowEnabled = bIsEnabled; }
	inline void setEndArrowParms(FLOAT_TYPE fLengthMult, FLOAT_TYPE fWidthMult) { myEndArrowLengthMult = fLengthMult; myEndArrowWidthMult = fWidthMult; }

	inline bool getIsStartArrowEnabled() const { return myIsStartArrowEnabled; }
	inline bool getIsEndArrowEnabled() const { return myIsEndArrowEnabled; }
	inline FLOAT_TYPE getStartArrowLengthMult() const { return myStartArrowLengthMult; }
	inline FLOAT_TYPE getStartArrowWidthMult() const { return myStartArrowWidthMult; }
	inline FLOAT_TYPE getEndArrowLengthMult() const { return myEndArrowLengthMult; }
	inline FLOAT_TYPE getEndArrowWidthMult() const { return myEndArrowWidthMult; }

private:

	void clearSegments();

	TYPE* addSegment();
	void addExistingSegment(TYPE* pSeg);

	template < class POINT_TYPE > void fitBezierRecursive(const TPolyline2D<POINT_TYPE>& rPolylineIn, int iFirst, int iLast, const SVector2D& rLeftTangent, const SVector2D& rRightTangent, FLOAT_TYPE dErrorSq);
	template < class POINT_TYPE > void chordLengthParameterize(const TPolyline2D<POINT_TYPE>& rPoints, int iFirst, int iLast, TFloatVector &rUParams);
	template < class POINT_TYPE > void generateBezier(const TPolyline2D<POINT_TYPE>& rPoints, int iFirst, int iLast, const TFloatVector &rUParams, const SVector2D& rLeftTangent, const SVector2D& rRightTangent, TYPE& rNewBezierOut);
	template < class POINT_TYPE > void reparameterize(const TPolyline2D<POINT_TYPE>& rPoints, int iFirst, int iLast, TFloatVector &rUParams, TYPE& rNewBezier, TFloatVector &rNewUParamsOut);
	FLOAT_TYPE doNewtonRaphsonRootFind(const TYPE& rNewBezier, const SVector2D& rPoint, FLOAT_TYPE dUParam);

private:

	// When adding new methods, don't forget to add the to clone self and copy from
	bool myIsClosed;

	// TODO: Load bezier edge ids and save them to file, and all other new attrs
	vector<TYPE*> mySegments;

	UNIQUEID_TYPE myMaterialId;

	TBezierCurveCollection<TYPE> *myParent;

	// Special tag, currently used for canvas.
	bool myIsBackground;
	bool myIsAHole;

	bool myIsStartArrowEnabled;
	FLOAT_TYPE myStartArrowLengthMult, myStartArrowWidthMult;
	bool myIsEndArrowEnabled;
	FLOAT_TYPE myEndArrowLengthMult, myEndArrowWidthMult;

	// This is temp storage
	TPointVector myFittingVecA1, myFittingVecA2;
	TFloatVector myFittingUNewParams;
	TFloatVector myFittingUParams;

};
/*****************************************************************************/
//#include "BezierCurve.hpp"
/*****************************************************************************/
// DO NOT SIMPLY UNCOMMENT THIS! DeformableBezier is in a different file because
// it can't be compiled with Intel's complier because it has a string field
// that corrupts the heap in AnimatedValue if it is compiled by both.
//typedef TBezierCurve<DeformableBezier> DeformableBezierCurve;
typedef HYPERUI_API TBezierCurve<SAnimBezierSegment> SAnimBezierCurve;
typedef HYPERUI_API TBezierCurve<SBezierSegment> SBezierCurve;
/*****************************************************************************/
