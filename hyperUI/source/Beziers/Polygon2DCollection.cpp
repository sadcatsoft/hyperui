#include "stdafx.h"
#ifdef HAVE_POLYBOOL
#include "polybool.h" 

using namespace POLYBOOLEAN; 
#endif

namespace HyperUI
{
/********************************************************************************************/
template < class POINT_TYPE >
TPolygon2DCollection<POINT_TYPE>::TPolygon2DCollection()
{

}
/********************************************************************************************/
template < class POINT_TYPE >
TPolygon2DCollection<POINT_TYPE>::~TPolygon2DCollection()
{
	clear();
}
/*****************************************************************************/
template < class POINT_TYPE >
POLYLINE_COLL_TYPE* TPolygon2DCollection<POINT_TYPE>::addPolygon()
{
	POLYLINE_COLL_TYPE* pRes = new POLYLINE_COLL_TYPE;
	myPolygons.push_back(pRes);
	return pRes;
}
/*****************************************************************************/
template < class POINT_TYPE >
void TPolygon2DCollection<POINT_TYPE>::clear()
{
	clearAndDeleteContainer(myPolygons);
}
/*****************************************************************************/
#ifdef HAVE_POLYBOOL
template < class POINT_TYPE >
void TPolygon2DCollection<POINT_TYPE>::setFromPBArea(const ::POLYBOOLEAN::PAREA* pSrcArea)
{
	clear();
	if(!pSrcArea)
		return;

	// Go over the linked list of these areas, include as polygon.
	POLYLINE_COLL_TYPE* pPoly;
	const PAREA* pCurrPtr;
	for(pCurrPtr = pSrcArea; pCurrPtr; pCurrPtr = pCurrPtr->f)
	{
		pPoly = addPolygon();
		pPoly->setFromPBArea(pCurrPtr);
	}
}
/*****************************************************************************/
template < class POINT_TYPE >
::POLYBOOLEAN::PAREA* TPolygon2DCollection<POINT_TYPE>::convertToPBArea() const
{
	// Go over all of our polygons and append each one as a separate area
	PAREA* pFinalRes = NULL;
	PAREA* pArea = NULL;
	int iCurr, iNum = myPolygons.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pArea = myPolygons[iCurr]->convertToPBArea();
		if(pArea)
			PAREA::InclParea(&pFinalRes, pArea);
	}

	return pFinalRes;
}
/*****************************************************************************/
template < class POINT_TYPE >
void TPolygon2DCollection<POINT_TYPE>::performBooleanOp(const TPolygon2DCollection& rOther, BooleanOpType eOpType, TPolygon2DCollection& rResultOut) const
{
	// Convert both to the necessary form
	PAREA *pThisArea = this->convertToPBArea(); 
	PAREA *pOtherArea = rOther.convertToPBArea();
	PAREA *pResult = NULL;

	// Perform the op
	PAREA::PBOPCODE eNativeOpType = PAREA::UN;
	if(eOpType == BooleanOpUnion)
		eNativeOpType = PAREA::UN;
	else if(eOpType == BooleanOpSubtraction)
		eNativeOpType = PAREA::SB;
	else if(eOpType == BooleanOpIntersection)
		eNativeOpType = PAREA::IS;
	ELSE_ASSERT;

	int iError = PAREA::Boolean(pThisArea, pOtherArea, &pResult, eNativeOpType);

	// Stuff into our result out
	rResultOut.setFromPBArea(pResult);

	// Delete intermediate result memory
	PAREA::Del(&pThisArea); 
	PAREA::Del(&pOtherArea); 
	PAREA::Del(&pResult);
}
#endif
/*****************************************************************************/
template class TPolygon2DCollection< SVector2D >;
};