#include "stdafx.h"

#define SAVE_CUBIC_CURVE	"cubCurve"

#define StringSvPTMinPointDist		"svPTMinPointDist"
#define StringSvPTMaxPointDist		"svPTMaxPointDist"
#define StringSvLastId				"svLastId"
#define StringSvPTPosX				"svPTPosX"
#define StringSvPTPosY				"svPTPosY"
#define StringSvObjId				"svObjId"

namespace HyperCore
{
static bool comparePointsByX(const SIdPoint& svPoint1,const SIdPoint& svPoint2) {	return svPoint1.myPoint.x < svPoint2.myPoint.x; }
/*****************************************************************************/
SCubicCurve::SCubicCurve()
{
	myAreDerivativesValid = false;
	myMinY = myMaxY = FLOAT_TYPE_MAX;
}
/*****************************************************************************/
int SCubicCurve::insertPoint(const SVector2D& svPoint, bool bPreventDuplicates, int iOptId)
{
	if(bPreventDuplicates)
	{
		int iPoint, iNumPoints = myPoints.size();
		//for(iPoint = iNumPoints - 1; iPoint >= 0; iPoint--)
		// Keep first-last intact
		for(iPoint = iNumPoints - 2; iPoint >= 1; iPoint--)
		{
			if(fabs(myPoints[iPoint].myPoint.x - svPoint.x) < FLOAT_EPSILON) 
				myPoints.erase(myPoints.begin() + iPoint);
		}
	}

	SIdPoint srNewPoint;
	srNewPoint.myPoint = svPoint;
	if(iOptId >= 0)
		srNewPoint.myId = iOptId;
	else
		srNewPoint.myId = generateId();
	myPoints.push_back(srNewPoint);
	// Sort it by x, since we rely on that
	std::sort(myPoints.begin(), myPoints.end(), comparePointsByX);
	myAreDerivativesValid = false;

	return srNewPoint.myId;
}
/*****************************************************************************/
void SCubicCurve::computeSecondDerivatives(TFloatVector& vecOut) const
{
	int n = myPoints.size();
	FLOAT_TYPE yp1=0.0; 
	FLOAT_TYPE ypn=0.0;

	// build the tridiagonal system 
	// (assume 0 boundary conditions: y2[0]=y2[-1]=0) 
	SVector3D *matrix = new SVector3D[n];
	FLOAT_TYPE* result = new FLOAT_TYPE[n];
	memset(result, 0, sizeof(FLOAT_TYPE)*n);
	matrix[0][1]=1;
	int i;
	for(i=1;i < n-1; i++) 
	{
		matrix[i][0]=(FLOAT_TYPE)(myPoints[i].myPoint.x-myPoints[i-1].myPoint.x)/6;
		matrix[i][1]=(FLOAT_TYPE)(myPoints[i+1].myPoint.x-myPoints[i-1].myPoint.x)/3;
		matrix[i][2]=(FLOAT_TYPE)(myPoints[i+1].myPoint.x-myPoints[i].myPoint.x)/6;
		result[i]=(FLOAT_TYPE)(myPoints[i+1].myPoint.y-myPoints[i].myPoint.y)/(myPoints[i+1].myPoint.x-myPoints[i].myPoint.x) - (FLOAT_TYPE)(myPoints[i].myPoint.y-myPoints[i-1].myPoint.y)/(myPoints[i].myPoint.x-myPoints[i-1].myPoint.x);
	}
	matrix[n-1][1]=1;

	// solving pass1 (up->down)
	for(i=1;i<n;i++) 
	{
		FLOAT_TYPE k = matrix[i][0]/matrix[i-1][1];
		matrix[i][1] -= k*matrix[i-1][2];
		matrix[i][0] = 0;
		result[i] -= k*result[i-1];
	}
	// solving pass2 (down->up)
	for(i=n-2;i>=0;i--) 
	{
		FLOAT_TYPE k = matrix[i][2]/matrix[i+1][1];
		matrix[i][1] -= k*matrix[i+1][0];
		matrix[i][2] = 0;
		result[i] -= k*result[i+1];
	}

	// return second derivative value for each point P
	//FLOAT_TYPE[] y2 = new FLOAT_TYPE[n];
	vecOut.clear();
	vecOut.resize(n);
	for(i=0;i<n;i++)
		vecOut[i] = result[i]/matrix[i][1];

	delete[] matrix;
	delete[] result;
}
/*****************************************************************************/
void SCubicCurve::evaluate(FLOAT_TYPE fXPos, int iStartPointIndex, SVector2D& svPointOut) const
{
	ensureDerivativesValid();

	svPointOut.set(fXPos, 0);
	int iNumPoints = myPoints.size();
	if(iNumPoints == 0)
		return;
	else if(iNumPoints == 1)
	{
		svPointOut = myPoints[0].myPoint;
		return;
	}

	if(iStartPointIndex >= iNumPoints - 1)
	{
		svPointOut = myPoints[iNumPoints - 1].myPoint;
		return;
	}

	FLOAT_TYPE t, a, b, h;

	SVector2D cur = myPoints[iStartPointIndex].myPoint;
	SVector2D next = myPoints[iStartPointIndex + 1].myPoint;

	t = (FLOAT_TYPE)(fXPos - cur.x)/(next.x - cur.x);
	a = 1 - t;
	b = t;
	h = next.x - cur.x;
	svPointOut.y = a*cur.y + b*next.y + (h*h/6.0)*((a*a*a - a)*myDerivatives[iStartPointIndex] + (b*b*b - b)*myDerivatives[iStartPointIndex + 1]);
}
/*****************************************************************************/
void SCubicCurve::ensureDerivativesValid() const
{
	if(myAreDerivativesValid)
		return;

	// Recompute derivatives
	computeSecondDerivatives(myDerivatives);
	myAreDerivativesValid = true;
}
/*****************************************************************************/
void SCubicCurve::convertToPolyline(const SRect2D* pOptClampingRect, int iPixelStep, TPointVector& vecOut)
{
	vecOut.clear();
	int iPoint, iNumPoints = myPoints.size();
	if(iNumPoints == 0)
		return;
	if(iNumPoints == 1)
	{
		vecOut.push_back(myPoints[0].myPoint);
		if(pOptClampingRect)
			vecOut[0].y = HyperCore::clampToRange<FLOAT_TYPE>(pOptClampingRect->y, pOptClampingRect->y + pOptClampingRect->h - 1, vecOut[0].y);
		return;
	}

	// Otherwise, render every pixel
	int iStartPixel = myPoints[0].myPoint.x;
	int iEndPixel = myPoints[iNumPoints - 1].myPoint.x;
	int iPixel;
	SVector2D svPoint;
	for(iPixel = iStartPixel, iPoint = 1; iPixel <= iEndPixel; iPixel += iPixelStep)
	{
		evaluate(iPixel, iPoint - 1, svPoint);
		if(pOptClampingRect)
			svPoint.y = HyperCore::clampToRange<FLOAT_TYPE>(pOptClampingRect->y, pOptClampingRect->y + pOptClampingRect->h - 1, svPoint.y);

		vecOut.push_back(svPoint);

		if(iPoint < iNumPoints && iPixel >= (int)myPoints[iPoint].myPoint.x)
			iPoint++;
	}
}
/*****************************************************************************/
void SCubicCurve::clear()
{
	myPoints.clear();
	myAreDerivativesValid = false;
}
/*****************************************************************************/
void SCubicCurve::refitToRectangle(const SRect2D& srRect, bool bInvertY, SVector2D* pOptScaleOut, SBBox2D* pOptBBoxOut)
{
	int iNumPoints = myPoints.size();
	if(iNumPoints == 0)
		return;

	// Get the min-max for the x:
	FLOAT_TYPE fMin = myPoints[0].myPoint.x;
	FLOAT_TYPE fMax = myPoints[iNumPoints - 1].myPoint.x;
	FLOAT_TYPE fDist = fMax - fMin;
	FLOAT_TYPE fScaleX = 1.0;
	if(fDist > FLOAT_EPSILON)
		fScaleX = srRect.w/fDist;

	int iPoint;
	FLOAT_TYPE fMinY = FLOAT_TYPE_MAX;
	FLOAT_TYPE fMaxY = -FLOAT_TYPE_MAX;

	if(myMinY != FLOAT_TYPE_MAX)
	{
		fMinY = myMinY;
		fMaxY = myMaxY;
	}
	else
	{
		for(iPoint = 0; iPoint < iNumPoints; iPoint++)
		{
			if(myPoints[iPoint].myPoint.y > fMaxY)
				fMaxY = myPoints[iPoint].myPoint.y;
			if(myPoints[iPoint].myPoint.y < fMinY)
				fMinY = myPoints[iPoint].myPoint.y;
		}
	}
	fDist = fMaxY - fMinY;
	FLOAT_TYPE fScaleY = 1.0;
	if(fDist > FLOAT_EPSILON)
		fScaleY = srRect.h/fDist;

	if(pOptScaleOut)
		pOptScaleOut->set(fScaleX, fScaleY);

	if(pOptBBoxOut)
	{
		pOptBBoxOut->myMin.set(fMin, fMinY);
		pOptBBoxOut->myMax.set(fMax, fMaxY);
	}

	SVector2D svNewPoint;
	for(iPoint = 0; iPoint < iNumPoints; iPoint++)
	{
		svNewPoint = myPoints[iPoint].myPoint;
		if(bInvertY)
			svNewPoint.y = fMaxY - (svNewPoint.y - fMinY);

		svNewPoint.x *= fScaleX;
		svNewPoint.y *= fScaleY;

		svNewPoint.x += srRect.x;
		svNewPoint.y += srRect.y;

		myPoints[iPoint].myPoint = svNewPoint;
	}
}
/*****************************************************************************/
void SCubicCurve::getPoint(int iIndex, SVector2D& svOut) const
{
	if(iIndex >= 0 && iIndex < myPoints.size())
		svOut = myPoints[iIndex].myPoint;
}
/*****************************************************************************/
int SCubicCurve::getPointId(int iIndex) const
{
	if(iIndex >= 0 && iIndex < myPoints.size())
		return myPoints[iIndex].myId;
	else
		return -1;
}
/*****************************************************************************/
void SCubicCurve::movePointById(int iId, const SVector2D& svPointIn, bool bKeepBetweenFirstLast)
{
	int iIndex = getIndexById(iId);
	if(iIndex < 0)
		return;

	SVector2D svPoint = svPointIn;
	if(bKeepBetweenFirstLast && myPoints.size() > 0)
	{
		if(svPoint.x <= myPoints[0].myPoint.x && iIndex != 0)
			svPoint.x = myPoints[0].myPoint.x + 1;
		int iLastPoint = myPoints.size() - 1;
		if(svPoint.x >= myPoints[iLastPoint].myPoint.x && iIndex != iLastPoint)
			svPoint.x = myPoints[iLastPoint].myPoint.x - 1;
	}

	myPoints[iIndex].myPoint = svPoint;
	std::sort(myPoints.begin(), myPoints.end(), comparePointsByX);
	myAreDerivativesValid = false;
}
/*****************************************************************************/
int SCubicCurve::getIndexById(int iId) const
{
	int iPoint, iNumPoints = myPoints.size();
	for(iPoint = 0; iPoint < iNumPoints; iPoint++)
	{
		if(myPoints[iPoint].myId == iId)
			return iPoint;
	}

	return -1;
}
/*****************************************************************************/
void SCubicCurve::removePointById(int iId)
{
	int iIndex = getIndexById(iId);
	if(iIndex < 0)
		return;

	myPoints.erase(myPoints.begin() + iIndex);
	myAreDerivativesValid = false;
}
/*****************************************************************************/
void SCubicCurve::getPointById(int iId, SVector2D& svOut) const
{
	getPoint(getIndexById(iId), svOut);
}
/*****************************************************************************/
bool SCubicCurve::getIsIdentity() const
{
	int iNumPoints = myPoints.size();
	if(iNumPoints != 2)
		return false;

	return myPoints[0].myPoint.x == 0.0 && myPoints[0].myPoint.y == 0.0 &&
		myPoints[1].myPoint.x == 1.0 && myPoints[1].myPoint.y == 1.0;
}
/*****************************************************************************/
void SCubicCurve::convertToLUT(FLOAT_TYPE fScale, FLOAT_TYPE* pTableOut) const
{
	// Assumes the table is allocated and has enough space
	SCubicCurve rCopy;
	rCopy.copyFrom(*this);
	SRect2D srRect(0, 0, fScale, fScale);
	rCopy.refitToRectangle(srRect, false);

	SRect2D* pOptClampingRect = &srRect;
	SCubicCurve* pCurve = &rCopy;

	int iPoint, iNumPoints = pCurve->myPoints.size();
	if(iNumPoints == 0)
	{
		memset(pTableOut, 0, sizeof(FLOAT_TYPE)*(int)fScale);
		return;
	}
	if(iNumPoints == 1)
	{
		SVector2D svPoint = pCurve->myPoints[0].myPoint;
		if(pOptClampingRect)
			svPoint.y = HyperCore::clampToRange<FLOAT_TYPE>(pOptClampingRect->y, pOptClampingRect->y + pOptClampingRect->h - 1, svPoint.y);
		int iCurr;
		for(iCurr = 0; iCurr <= (int)fScale; iCurr++)
			pTableOut[0] = svPoint.y;
		return;
	}

	// Otherwise, render every pixel
	int iStartPixel = 0;
	int iEndPixel = fScale;
	int iPixel;
	SVector2D svPoint;
	// NOTE that we assume one-pixel step for conversion to LUTs, as well.
	for(iPixel = iStartPixel, iPoint = 1; iPixel <= iEndPixel; iPixel++)
	{
		pCurve->evaluate(iPixel, iPoint - 1, svPoint);
		if(pOptClampingRect)
			svPoint.y = HyperCore::clampToRange<FLOAT_TYPE>(pOptClampingRect->y, pOptClampingRect->y + pOptClampingRect->h - 1, svPoint.y);

		_ASSERT((int)svPoint.x == iPixel);
		pTableOut[iPixel] = svPoint.y;

		if(iPoint < iNumPoints && iPixel >= (int)pCurve->myPoints[iPoint].myPoint.x)
			iPoint++;
	}

}
/*****************************************************************************/
UNIQUEID_TYPE SCubicCurve::generateId()
{
	myLastId++;
	return myLastId;
}
/*****************************************************************************/
bool SCubicCurve::loadFromString(const char* pcsString)
{
	clear();

	if(!IS_VALID_STRING_AND_NOT_NONE(pcsString))
		return false;

	StringResourceCollection rColl;
	// DO NOT move this to the class definition! On Mac,
	// the headers are compiled by GCC & LLVM separately,
	// and thus will crash if we try to use it that way.
	string strTempCopy;
	strTempCopy = pcsString;
	bool bResult = rColl.loadFromString(strTempCopy);
	if(!bResult || rColl.getNumItems() < 1)
		return false;

	StringResourceItem* pItem = rColl.getItem(0);
	if(!pItem)
		ASSERT_RETURN_FALSE;

	myLastId = pItem->getAsLong(StringSvLastId);

	myMinY = pItem->getNumProp(StringSvPTMinPointDist);
	myMaxY = pItem->getNumProp(StringSvPTMaxPointDist);

	SIdPoint rPoint;
	int iPoint, iNumPoints = pItem->getNumericEnumPropCount(StringSvPTPosX);
	for(iPoint = 0; iPoint < iNumPoints; iPoint++)
	{
		rPoint.myPoint.x = pItem->getNumericEnumPropValue(StringSvPTPosX, iPoint);
		rPoint.myPoint.y = pItem->getNumericEnumPropValue(StringSvPTPosY, iPoint);
		rPoint.myId = pItem->getNumericEnumPropValue(StringSvObjId, iPoint);
		myPoints.push_back(rPoint);
	}

	return true;
}
/*****************************************************************************/
void SCubicCurve::saveToString(string& strOut) const
{
	strOut = "";
	StringResourceItem rDummy(SAVE_CUBIC_CURVE);
	rDummy.setStringProp(STRING_ID_PROP, SAVE_CUBIC_CURVE);
	rDummy.setNumProp(StringSvPTMinPointDist, myMinY);
	rDummy.setNumProp(StringSvPTMaxPointDist, myMaxY);

	rDummy.setAsLong(StringSvLastId, myLastId);

	// Save the points
	int iPoint, iNumPoints = myPoints.size();
	for(iPoint = 0; iPoint < iNumPoints; iPoint++)
	{
		rDummy.addToNumericEnumProp(StringSvPTPosX, myPoints[iPoint].myPoint.x);
		rDummy.addToNumericEnumProp(StringSvPTPosY, myPoints[iPoint].myPoint.y);
		rDummy.addToNumericEnumProp(StringSvObjId, myPoints[iPoint].myId);
	}
	rDummy.saveToString(strOut);
}
/*****************************************************************************/
};