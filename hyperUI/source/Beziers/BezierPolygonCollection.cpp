#include "stdafx.h"

#define SAVE_CURVE_COLL						"curvePoly"
#define SAVE_POLY_COLLECTION_BLOCK			"polyColl"
#define SAVE_CURVEPOLY_COLLECTION_MAX_ID	"plyMaxId"

namespace HyperUI
{
/*****************************************************************************/
template < class TYPE >
TBezierPolygonCollection<TYPE>::TBezierPolygonCollection()
{

}
/*****************************************************************************/
template < class TYPE >
TBezierPolygonCollection<TYPE>::~TBezierPolygonCollection()
{
	clear();
}
/*****************************************************************************/
template < class TYPE >
void TBezierPolygonCollection<TYPE>::clear()
{
	clearAndDeleteContainer(myPolys);
	myMaxId = 0;
}
/*****************************************************************************/
template < class TYPE >
BEZIER_POLY_TYPE* TBezierPolygonCollection<TYPE>::addNewPolygon()
{
	BEZIER_POLY_TYPE* pNew = new BEZIER_POLY_TYPE;
	addPolygon(pNew);
	return pNew;
}
/*****************************************************************************/
template < class TYPE >
void TBezierPolygonCollection<TYPE>::addPolygon(BEZIER_POLY_TYPE* pPoly, bool bInsertInFront)
{
	pPoly->setParent(this);
	if(bInsertInFront)
		myPolys.insert(myPolys.begin(), pPoly);
	else
		myPolys.push_back(pPoly);
}
/*****************************************************************************/
template < class TYPE >
void TBezierPolygonCollection<TYPE>::translateBy(const SVector2D& svDiff)
{
	int iCurr, iNum = myPolys.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
		myPolys[iCurr]->translateBy(svDiff);
}
/*****************************************************************************/
template < class TYPE >
void TBezierPolygonCollection<TYPE>::transformBy(const SMatrix2D& smTransform)
{
	int iCurr, iNum = myPolys.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
		myPolys[iCurr]->transformBy(smTransform);
}
/*****************************************************************************/
template < class TYPE >
void TBezierPolygonCollection<TYPE>::mergeFrom(const TBezierPolygonCollection& rOther, bool bIgnoreBackgroundCurves, bool bInsertInFront)
{
	const BEZIER_POLY_TYPE* pPoly;
	UNIQUEID_TYPE iMaxId;
	int iCurr, iNum = rOther.getNumPolygons();
	int iCurrIdx;
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(bInsertInFront)
			iCurrIdx = iNum - iCurr - 1;
		else
			iCurrIdx = iCurr;

		pPoly = rOther.getPolygon(iCurrIdx);
		addPolygon(pPoly->cloneSelf(bIgnoreBackgroundCurves), bInsertInFront);

		iMaxId = pPoly->getMaxId();
		if(iMaxId > myMaxId)
			myMaxId = iMaxId;
	}
}
/*****************************************************************************/
template < class TYPE >
void TBezierPolygonCollection<TYPE>::copyFrom(const TBezierPolygonCollection& rOther, bool bIgnoreBackgroundCurves)
{
	clear();
	mergeFrom(rOther, bIgnoreBackgroundCurves);
}
/*****************************************************************************/
template < class TYPE >
bool TBezierPolygonCollection<TYPE>::getBBox(SRect2D& svBBoxOut, bool bIgnoreBackgroundCurves) const
{
	svBBoxOut.set(0, 0, 0, 0);

	int iCurr, iNum = myPolys.size();
	if(iNum <= 0)
		return false;

	myPolys[0]->getBBox(svBBoxOut);
	SRect2D svTempRect;
	for(iCurr = 1; iCurr < iNum; iCurr++)
	{
		myPolys[iCurr]->getBBox(svTempRect);
		svBBoxOut.includeRect(svTempRect);
	}

	return true;
}
/*****************************************************************************/
template < class TYPE >
void TBezierPolygonCollection<TYPE>::loadFromItem(const StringResourceItem& rItem)
{
	clear();

	// Load from item
	const StringResourceItem* pBlock = rItem.getChildById(SAVE_POLY_COLLECTION_BLOCK, false);
	if(!pBlock)
		ASSERT_RETURN;

	BEZIER_POLY_TYPE* pCurrPoly;
	const StringResourceItem* pChild;
	int iChild, iNumChildren = pBlock->getNumChildren();
	for(iChild = 0; iChild < iNumChildren; iChild++)
	{
		pChild = pBlock->getChild(iChild);

		pCurrPoly = addNewPolygon();
		pCurrPoly->loadFromItem(*pChild);
	}

	myMaxId = pBlock->getAsLong(SAVE_CURVEPOLY_COLLECTION_MAX_ID);
}
/*****************************************************************************/
template < class TYPE >
void TBezierPolygonCollection<TYPE>::saveToItem(StringResourceItem& rItemOut) const
{
	StringResourceItem* pChild;
	StringResourceItem* pBlock = rItemOut.addChildAndSetId(SAVE_POLY_COLLECTION_BLOCK);
	int iCurve, iNumPolys = myPolys.size();
	for(iCurve = 0; iCurve < iNumPolys; iCurve++)
	{
		pChild = pBlock->addChildAndSetId(SAVE_CURVE_COLL);
		myPolys[iCurve]->saveToItem(*pChild);
	}

	pBlock->setAsLong(SAVE_CURVEPOLY_COLLECTION_MAX_ID, myMaxId);
}
/*****************************************************************************/
template < class TYPE >
const TYPE* TBezierPolygonCollection<TYPE>::findVertexClosestTo(const SVector2D& svPoint, FLOAT_TYPE fOptMaxRadius, SVector2D& svResPointOut, BezierPointType& ePointTypeOut) const
{
	SVector2D svTempPoint;
	FLOAT_TYPE fDistSq = FLOAT_TYPE_MAX;
	FLOAT_TYPE fCurrDistSq;

	const TYPE* pRes = NULL;
	const TYPE* pCurrRes;
	BezierPointType eCurrPointType;
	const BEZIER_POLY_TYPE *pPoly;

	int iCurr, iNum = getNumPolygons();
	if(iNum <= 0)
		return pRes;

	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pPoly = getPolygon(iCurr);
		pCurrRes = pPoly->findVertexClosestTo(svPoint, fOptMaxRadius, svTempPoint, eCurrPointType);

		fCurrDistSq = (svTempPoint - svPoint).lengthSquared();
		if(pCurrRes && fCurrDistSq < fDistSq)
		{
			fDistSq = fCurrDistSq;
			svResPointOut = svTempPoint;
			pRes = pCurrRes;
			ePointTypeOut = eCurrPointType;
		}
	}

	return pRes;
}
/*****************************************************************************/
template < class TYPE >
const TYPE* TBezierPolygonCollection<TYPE>::findPointClosestTo(const SVector2D& svPoint, SVector2D& svPointOut, FLOAT_TYPE* fOptParmDistOut) const
{
	SVector2D svTempPoint;
	FLOAT_TYPE fDistSq = FLOAT_TYPE_MAX;
	FLOAT_TYPE fCurrDistSq;

	const TYPE* pRes = NULL;
	const TYPE* pCurrRes;

	const BEZIER_POLY_TYPE *pPoly;

	int iCurr, iNum = getNumPolygons();
	if(iNum <= 0)
		return pRes;

	FLOAT_TYPE fParmDist;

	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pPoly = getPolygon(iCurr);
		pCurrRes = pPoly->findPointClosestTo(svPoint, -1, svTempPoint, &fParmDist);
		if(!pCurrRes)
			continue;

		fCurrDistSq = (svTempPoint - svPoint).lengthSquared();
		if(fCurrDistSq < fDistSq)
		{
			fDistSq = fCurrDistSq;
			svPointOut = svTempPoint;
			pRes = pCurrRes;
			if(fOptParmDistOut)
				*fOptParmDistOut = fParmDist;
		}
	}

	return pRes;
}
/*****************************************************************************/
template < class TYPE >
TYPE* TBezierPolygonCollection<TYPE>::findSegmentByEdgeId(const EdgeId& idEdge) const
{
	TYPE* pRes = NULL;
	int iCurr, iNum = getNumPolygons();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pRes = getPolygon(iCurr)->findSegmentByEdgeId(idEdge);
		if(pRes)
			break;
	}
	return pRes;
}
/*****************************************************************************/
template < class TYPE > 
void TBezierPolygonCollection<TYPE>::onChildSegmentEdgeIdChanged(const EdgeId& idOld, const EdgeId& idNew)
{
	if(idNew.getNumeric() > myMaxId)
		myMaxId = idNew.getNumeric();
}
/*****************************************************************************/
template < class TYPE >
UNIQUEID_TYPE TBezierPolygonCollection<TYPE>::getNextId() 
{ 
	myMaxId++;
	return myMaxId;
}
/*****************************************************************************/
template class TBezierPolygonCollection<SBezierSegment>;
template class TBezierPolygonCollection<SAnimBezierSegment>;
template class TBezierPolygonCollection<DeformableBezier>;
};