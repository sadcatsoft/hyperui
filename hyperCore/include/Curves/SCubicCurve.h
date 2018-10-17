#pragma once

/*****************************************************************************/
struct SIdPoint
{
	SVector2D myPoint;
	int myId;
};
typedef vector < SIdPoint > TIdPointsVector;
/*****************************************************************************/
class SCubicCurve
{
public:
	HYPERCORE_API SCubicCurve();
	~SCubicCurve() { }
	
	HYPERCORE_API int insertPoint(const SVector2D& svPoint, bool bPreventDuplicates = false, int iOptId = -1);
	HYPERCORE_API void movePointById(int iId, const SVector2D& svPoint, bool bKeepBetweenFirstLast);
	HYPERCORE_API void removePointById(int iId);
	HYPERCORE_API void evaluate(FLOAT_TYPE fXPos, int iStartPointIndex, SVector2D& svPointOut) const;

	HYPERCORE_API void convertToPolyline(const SRect2D* pOptClampingRect, int iPixelStep, TPointVector& vecOut);

	HYPERCORE_API bool loadFromString(const char* pcsString);
	HYPERCORE_API void saveToString(string& strOut) const;

	HYPERCORE_API void clear();

	void copyFrom(const SCubicCurve& rOther) { *this = rOther; }
	HYPERCORE_API void refitToRectangle(const SRect2D& srRect, bool bInvertY, SVector2D* pOptScaleOut = NULL, SBBox2D* pOptBBoxOut = NULL);

	inline int getNumPoints() const { return myPoints.size(); }
	HYPERCORE_API void getPoint(int iIndex, SVector2D& svOut) const;
	HYPERCORE_API void getPointById(int iId, SVector2D& svOut) const;
	HYPERCORE_API int getPointId(int iIndex) const;

	HYPERCORE_API int getIndexById(int iId) const;

	void setMinMaxY(FLOAT_TYPE fMinY, FLOAT_TYPE fMaxY) { myMinY = fMinY; myMaxY = fMaxY; }

	HYPERCORE_API bool getIsIdentity() const;
	HYPERCORE_API void convertToLUT(FLOAT_TYPE fScale, FLOAT_TYPE* pTableOut) const;
	HYPERCORE_API UNIQUEID_TYPE generateId();

private:

	void ensureDerivativesValid() const;
	void computeSecondDerivatives(TFloatVector& vecOut) const;

private:

	UNIQUEID_TYPE myLastId;
	TIdPointsVector myPoints;

	mutable TFloatVector myDerivatives;
	mutable bool myAreDerivativesValid;

	FLOAT_TYPE myMinY, myMaxY;
};
/*****************************************************************************/
