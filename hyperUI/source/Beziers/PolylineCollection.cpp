#include "stdafx.h"

#ifdef HAVE_POLYBOOL
#include "polybool.h" 

using namespace POLYBOOLEAN; 
#endif

namespace HyperUI
{
/********************************************************************************************/
template < class POINT_TYPE > 
TPolylineCollection<POINT_TYPE>::TPolylineCollection()
{

}
/********************************************************************************************/
template < class POINT_TYPE > 
TPolylineCollection<POINT_TYPE>::~TPolylineCollection()
{
	clear();
}
/********************************************************************************************/
template < class POINT_TYPE > 
void TPolylineCollection<POINT_TYPE>::clear()
{
	clearAndDeleteContainer(myLines);
}
/********************************************************************************************/
template < class POINT_TYPE > 
POLYLINE_TYPE* TPolylineCollection<POINT_TYPE>::addPolyline()
{
	POLYLINE_TYPE* pRes = new POLYLINE_TYPE;
	myLines.push_back(pRes);
	return pRes;
}
/********************************************************************************************/
template < class POINT_TYPE > 
void TPolylineCollection<POINT_TYPE>::reverse()
{
	POLYLINE_TYPE* pLine;
	int iLine, iNumLines = myLines.size();
	for(iLine = 0; iLine < iNumLines; iLine++)
	{
		pLine = myLines[iLine];
		std::reverse(pLine->begin(), pLine->end());
	}
}
/********************************************************************************************/
template < class POINT_TYPE >
void TPolylineCollection<POINT_TYPE>::removeCoincident(FLOAT_TYPE fDistance)
{
	POLYLINE_TYPE* pLine;
	int iLine, iNumLines = myLines.size();
	for(iLine = 0; iLine < iNumLines; iLine++)
	{
		pLine = myLines[iLine];
		pLine->removeCoincident(fDistance);
	}
}
/********************************************************************************************/
#ifdef HAVE_POLYBOOL
template < class POINT_TYPE >
PAREA* TPolylineCollection<POINT_TYPE>::convertToPBArea() const
{
	PAREA *pRes = NULL;

	// Go over all lines
	PLINE2 *pLine; 
	int iCurr, iNum = myLines.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pLine = myLines[iCurr]->convertToPBLine();
		if(pLine)
			PAREA::InclPline(&pRes, pLine); 
	}

	return pRes;
}
/*****************************************************************************/
template < class POINT_TYPE >
void TPolylineCollection<POINT_TYPE>::setFromPBArea(const PAREA* pSrcArea)
{
	this->clear();
	if(!pSrcArea)
		return;

	// Note that each area is essentially a polygon, and contains
	// a number of lines
	POLYLINE_TYPE* pNewLine;
	const PLINE2* pSrcLine;
	for(pSrcLine = pSrcArea->cntr; pSrcLine; pSrcLine = pSrcLine->next)
	{
		pNewLine = addPolyline();
		pNewLine->setFromPBLine(pSrcLine);
	}
}
#endif
/*****************************************************************************/
template < class POINT_TYPE >
void TPolylineCollection<POINT_TYPE>::transformBy(const SMatrix2D& smMatrix)
{
	POLYLINE_TYPE* pLine;
	int iLine, iNumLines = myLines.size();
	for(iLine = 0; iLine < iNumLines; iLine++)
	{
		pLine = myLines[iLine];
		pLine->transform(smMatrix);
	}
}
/*****************************************************************************/
template < class POINT_TYPE >
void TPolylineCollection<POINT_TYPE>::copyFrom(const TPolylineCollection& rOther)
{
	clear();
	POLYLINE_TYPE* pNewPolyline;
	POLYLINE_TYPE* pOtherLine;
	int iLine, iNumLines = rOther.myLines.size();
	for(iLine = 0; iLine < iNumLines; iLine++)
	{
		pOtherLine = rOther.myLines[iLine];
		pNewPolyline = addPolyline();
		pNewPolyline->copyFrom(*pOtherLine);
	}
}
/*****************************************************************************/
template < class POINT_TYPE >
void TPolylineCollection<POINT_TYPE>::translate(FLOAT_TYPE fX, FLOAT_TYPE fY)
{
	POLYLINE_TYPE* pLine;
	int iLine, iNumLines = myLines.size();
	for(iLine = 0; iLine < iNumLines; iLine++)
	{
		pLine = myLines[iLine];
		pLine->translate(fX, fY);
	}
}
/*****************************************************************************/
template class TPolylineCollection< SVector2D >;
};