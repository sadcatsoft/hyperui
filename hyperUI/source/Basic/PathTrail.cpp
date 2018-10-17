#include "stdafx.h"

#define _USE_MATH_DEFINES
#include <cmath>

namespace HyperUI
{
//TPointVector PathTrail::thePointVector;
/********************************************************************************************/
PathTrail::PathTrail()
{
	myTrailFreqMult = 3;
	myIsEnabled = true;

	myClockType = ClockMainGame;
	myMaxPoints = 0;
    myIsDeleted = false;
    myFadeSpeed = 0.2;
	myConstOpacity = -1;
	myExtendMultiplier = 0.0;
	myThickness = 1.0;
	mySampleFreq = -1;
	myColor.set(1,1,1,1);
	myAdditionalRotation = 0.0;
	mySampleAnim = NULL;
	mySampleAnimAtlasFrame = -1;

	myMinPointDistance = -1;
	myMaxPointDistance = -1;
	myAngleTolerance = -1;
}
/********************************************************************************************/
PathTrail::~PathTrail()
{
    clear();
}
/********************************************************************************************/
FLOAT_TYPE PathTrail::getAdditionalRotation(void)
{
	return myAdditionalRotation;
}
/********************************************************************************************/
void PathTrail::setAdditionalRotation(FLOAT_TYPE fDegRot)
{
	myAdditionalRotation = fDegRot;
}
/********************************************************************************************/
void PathTrail::getOffset(SVector2D& svOffsetOut)
{
	svOffsetOut = myOffset;
}
/********************************************************************************************/
void PathTrail::setOffset(int iX, int iY)
{
	myOffset.set(iX, iY);
}
/********************************************************************************************/
void PathTrail::setMaxPoints(int iNum)
{
	myMaxPoints = iNum;
}
/********************************************************************************************/
void PathTrail::setFadeSpeed(FLOAT_TYPE fSpeed)
{
	myFadeSpeed = fSpeed;
}
/********************************************************************************************/
void PathTrail::setTrailFreqMult(int iMult)
{
	myTrailFreqMult = iMult;
}
/********************************************************************************************/
void PathTrail::setSampleAnim(DrawingCache* pDrawingCache, const char* pcsAnim)
{
	mySampleAnim = pDrawingCache->getCachedSequence(pcsAnim, &mySampleAnimAtlasFrame);
}
/********************************************************************************************/
void PathTrail::setSampleFreq(int iFreq)
{
	mySampleFreq = iFreq;
}
/********************************************************************************************/
void PathTrail::setThickness(FLOAT_TYPE dValue)
{
	myThickness = dValue;
}
/********************************************************************************************/
FLOAT_TYPE PathTrail::getThickness(void)
{
	return myThickness;
}
/********************************************************************************************/
void PathTrail::setExtendMultiplier(FLOAT_TYPE dMult)
{
	myExtendMultiplier = dMult;
}
/********************************************************************************************/
void PathTrail::clear(void)
{
    myPoints.clear();
}
/********************************************************************************************/
int PathTrail::getNumPoints(void)
{
    return myPoints.size();
}
/********************************************************************************************/
GTIME PathTrail::getPointTime(int iIndex)
{
	return myPoints[iIndex].myTime;
}
/********************************************************************************************/
void PathTrail::setPointMaker(int iIndex, int iMarker)
{
	myPoints[iIndex].myMarker = iMarker;
}
/********************************************************************************************/
void PathTrail::getPoint(int iIndex, SVector2D& svOut)
{
	svOut = myPoints[iIndex].myPoint;
}
/********************************************************************************************
void PathTrail::getPoint(int iIndex, SVector2D& svOut)
{
    svOut = myPoints[iIndex].myPoint;
}
/********************************************************************************************/
void PathTrail::getAsLines(TLineVector& rVecOut, bool bReverse)
{
	SVector2D svTemp;
	rVecOut.clear();
	int iCurr, iNumPoints = myPoints.size();
	int iCurrLines;
	rVecOut.push_back(Polyline2D());
	iCurrLines = rVecOut.size();

	if(bReverse)
	{
		for(iCurr = iNumPoints - 1; iCurr >= 0; iCurr--)
		{
			svTemp.x = myPoints[iCurr].myPoint.x;
			svTemp.y = myPoints[iCurr].myPoint.y;

			rVecOut[iCurrLines - 1].push_back(svTemp);
			if(myPoints[iCurr].myMarker != 0)
			{
				rVecOut.push_back(Polyline2D());
				iCurrLines++;
				rVecOut[iCurrLines - 1].push_back(svTemp);
			}
		}

	}
	else
	{
		for(iCurr = 0; iCurr < iNumPoints; iCurr++)
		{
			svTemp.x = myPoints[iCurr].myPoint.x;
			svTemp.y = myPoints[iCurr].myPoint.y;

			rVecOut[iCurrLines - 1].push_back(svTemp);
			if(myPoints[iCurr].myMarker != 0)
			{
				rVecOut.push_back(Polyline2D());
				iCurrLines++;
				rVecOut[iCurrLines - 1].push_back(svTemp);
			}
		}

	}
}
/********************************************************************************************/
void PathTrail::getAsPointVector(TPointVector& rPointsOut, bool bReverse)
{
	SVector2D svTemp;
	rPointsOut.clear();
	int iCurr, iNumPoints = myPoints.size();
	if(bReverse)
	{
		for(iCurr = iNumPoints - 1; iCurr >= 0; iCurr--)
		{
			svTemp.x = myPoints[iCurr].myPoint.x;
			svTemp.y = myPoints[iCurr].myPoint.y;
			rPointsOut.push_back(svTemp);
		}
	}
	else
	{
		for(iCurr = 0; iCurr < iNumPoints; iCurr++)
		{
			svTemp.x = myPoints[iCurr].myPoint.x;
			svTemp.y = myPoints[iCurr].myPoint.y;
			rPointsOut.push_back(svTemp);
		}
	}
}
/********************************************************************************************/
void PathTrail::addPoint(FLOAT_TYPE iX, FLOAT_TYPE iY, GTIME lTime, bool bForceAddition, int iMarkerValue)
{
	if(!myIsEnabled)
		return;

	if(mySampleFreq <= 0 && myTrailFreqMult > 0 && lTime % myTrailFreqMult != 0 && !bForceAddition)
	{
		return;
	}

    // See if this point lies in an almost-a-straight
    // line relative to the previous two points
    int iNumPoints = myPoints.size();
	FLOAT_TYPE dPrevsDistance = 0;

    if(iNumPoints >= 2 && !bForceAddition)
    {		
		const FLOAT_TYPE g_dMinDistance = (mySampleFreq > 0 ? mySampleFreq : upToScreen(5)); // min distance before a new point is added, in pixels
		const FLOAT_TYPE g_dMaxDistance = (mySampleFreq > 0 ? mySampleFreq : upToScreen(60)); // max distance after which a new point is forced onto
		static const FLOAT_TYPE g_dTolerance = F_COS_DEG(5.0);
		
		FLOAT_TYPE dMinDistance, dMaxDistance, dTolerance;
		if(myMinPointDistance >= 0)
			dMinDistance = myMinPointDistance;
		else
			dMinDistance = g_dMinDistance;

		if(myMaxPointDistance >= 0)
			dMaxDistance = myMaxPointDistance;
		else
			dMaxDistance = g_dMaxDistance;

		if(myAngleTolerance >= 0)
			dTolerance = F_COS_DEG(myAngleTolerance);
		else
			dTolerance = g_dTolerance;

		bool bDoModifyLastPoint = false;
		SVector2D svNewPoint;
		svNewPoint.x = iX;
		svNewPoint.y = iY;
		SVector2D svOldDir;
		SVector2D svNewDir;
		svOldDir = myPoints[iNumPoints - 1].myPoint - myPoints[iNumPoints - 2].myPoint;

		dPrevsDistance = svOldDir.lengthSquared();
		if(dPrevsDistance <= dMaxDistance*dMaxDistance && mySampleFreq <= 0)
		{	
			if(dPrevsDistance < dMinDistance*dMinDistance)
				bDoModifyLastPoint = true;
			else
			{
				svNewDir = svNewPoint - myPoints[iNumPoints - 1].myPoint;
				FLOAT_TYPE fLenTo = svNewDir.normalize();
				
				dPrevsDistance = svOldDir.normalize();
				
				// The last clause is wrong, since we'll keep moving the last point
				// continuously - i.e. too much, if we add too oftren.
				if(fabs(svOldDir.dot(svNewDir) ) >= dTolerance) //  || fLenTo <= dMinDistance)
					bDoModifyLastPoint = true;
			}
			
			if(bDoModifyLastPoint)
			{
				// Just change the old number to be the new one
				_ASSERT(iNumPoints > 0);
				myPoints[iNumPoints - 1].myPoint.x = iX;
				myPoints[iNumPoints - 1].myPoint.y = iY;
				myPoints[iNumPoints - 1].myTime = lTime;
				myPoints[iNumPoints - 1].myMarker = iMarkerValue;
	
				return;
			}
		}
    }

	bool bAddNewPoint = true;


	if(mySampleFreq > 0 && (dPrevsDistance > 1 || iNumPoints == 1) && myPoints.size() > 0)
	{
		// We may need to add more than one point
//		if((int)dPrevsDistance % (int)mySampleFreq != 0)
//			iNumToAdd++;
		int iCurr;
		STrailPoint svPoint;
		SVector2D svLastPoint = myPoints[myPoints.size() - 1].myPoint;
		SVector2D svDir = SVector2D(iX, iY) - svLastPoint;
		dPrevsDistance = svDir.normalize();
		int iNumToAdd = dPrevsDistance/mySampleFreq;
		if(iNumToAdd <= 0)
			bAddNewPoint = false;
		for(iCurr = 0; iCurr < iNumToAdd - 1; iCurr++)
		{
			svPoint.myPoint = svLastPoint + svDir*((FLOAT_TYPE)(iCurr + 1)/(FLOAT_TYPE)iNumToAdd)*dPrevsDistance;
			svPoint.myTime = lTime;
			svPoint.myMarker = iMarkerValue;
			myPoints.push_back(svPoint);	
		}
	}

	if(mySampleFreq > 0 && iNumPoints >= 2 && !bAddNewPoint)
	{
		// See if we're far enough from the *second* last point.
//		int iCurr;
		STrailPoint svPoint;
		SVector2D svLastPoint = myPoints[myPoints.size() - 2].myPoint;
		SVector2D svDir = SVector2D(iX, iY) - svLastPoint;
		dPrevsDistance = svDir.normalize();
		int iNumToAdd = dPrevsDistance/mySampleFreq;
		if(dPrevsDistance >= mySampleFreq)
		{
			_ASSERT(iNumPoints > 0);
			myPoints[iNumPoints - 1].myPoint.x = iX;
			myPoints[iNumPoints - 1].myPoint.y = iY;
			myPoints[iNumPoints - 1].myTime = lTime;
			myPoints[iNumPoints - 1].myMarker = iMarkerValue;
			bAddNewPoint = true;
		}
	}

	// Insert a new line here
	if(bAddNewPoint || iNumPoints < 2)
	{
		STrailPoint svPoint;
		svPoint.myPoint.x = iX;
		svPoint.myPoint.y = iY;
		svPoint.myTime = lTime;
		svPoint.myMarker = iMarkerValue;
		myPoints.push_back(svPoint);
	}
	else
	{

		_ASSERT(iNumPoints > 0);
		myPoints[iNumPoints - 1].myPoint.x = iX;
		myPoints[iNumPoints - 1].myPoint.y = iY;
		myPoints[iNumPoints - 1].myTime = lTime;
		myPoints[iNumPoints - 1].myMarker = iMarkerValue;
	}

	// See if need to pop some (oh yeah...)
	if(myMaxPoints > 0)
	{
		int iToPop = myPoints.size() - myMaxPoints;
		// "Front" really means the start of all points
		if(iToPop > 0)
			myPoints.eraseFront(iToPop);
	}
}
/********************************************************************************************/
#if 0
void PathTrail::generateTriangles(SGenData& rDataOut)
{
	/*
	myPoints.clear();
	addPoint(10, 400, 2);
	addPoint(110, 330, 2);
	addPoint(210, 390, 2);
	*/
	
	clearTransparentPoints();
	
    int iCurrPoint, iNumPoints = myPoints.size();
    if(iNumPoints < 2)
    {
		rDataOut.myNumTris = 0;
		return;
    }

	FLOAT_TYPE dThickness = myThickness;
	
	bool bDoExtend = false;
	if(myExtendMultiplier > 0)
		bDoExtend = true;

    FLOAT_TYPE dHalfThickness = dThickness/2.0;
    int iArrayOffset, iCArrayOffset;
    // Two triangles per segment, with two coords each
    int iVArrayLen = (iNumPoints - 1)*3*2*2;
    rDataOut.myNumTris = (iNumPoints - 1)*2;

    // 4 color elems
    int iCArraylen = (iNumPoints - 1)*3*2*4;
    GTIME lCurrStartTickAge, lCurrEndTickAge; // , lFirstPointAge;
    GTIME lGlobalTime = Application::getInstance()->getGlobalTime(myClockType);

    // lFirstPointAge = myPoints[0].myTime;

    FLOAT_TYPE dFadePerTick = (FLOAT_TYPE)myFadeSpeed/(FLOAT_TYPE)Application::secondsToTicks(1.0);
    FLOAT_TYPE dPoint1Opacity, dPoint2Opacity;
	FLOAT_TYPE dOrigLen;
	/*
    rDataOut.myVertexArray = new GLfloat[iVArrayLen];
	rDataOut.myTexCoords = new GLfloat[iVArrayLen];
    rDataOut.myColorArray = new GLfloat[iCArraylen];
	 */
	rDataOut.ensureCapacity(iVArrayLen, iCArraylen);
    SVector2D svDir, svNorm;
    SVector2D svVert, svExt;
    for(iCurrPoint = 1; iCurrPoint < iNumPoints; iCurrPoint++)
    {
		svDir = myPoints[iCurrPoint].myPoint - myPoints[iCurrPoint - 1].myPoint;
		dOrigLen = svDir.normalize();

		svNorm.x = svDir.y;
		svNorm.y = -svDir.x;
		
		svExt = svDir*(myExtendMultiplier*dOrigLen);
		
		iArrayOffset = 12*(iCurrPoint - 1);
		iCArrayOffset = 24*(iCurrPoint - 1);

		lCurrStartTickAge = lGlobalTime - myPoints[iCurrPoint - 1].myTime; 
		lCurrEndTickAge = lGlobalTime - myPoints[iCurrPoint].myTime;

		dPoint1Opacity = 1.0 - (FLOAT_TYPE)lCurrStartTickAge*dFadePerTick;
		dPoint2Opacity = 1.0 - (FLOAT_TYPE)lCurrEndTickAge*dFadePerTick;
		if(dPoint1Opacity > 1)
			dPoint1Opacity = 1.0;
		if(dPoint1Opacity < 0)
			dPoint1Opacity = 0.0;
		if(dPoint2Opacity > 1)
			dPoint2Opacity = 1.0;
		if(dPoint2Opacity < 0)
			dPoint2Opacity = 0.0;

		if(myConstOpacity >= 0.0)
		{
			dPoint1Opacity = myConstOpacity;
			dPoint2Opacity = myConstOpacity;
		}
			
		if((iCurrPoint + 1) >= iNumPoints && dPoint2Opacity < 0.01)
			myIsDeleted = true;

		// Vertex positions
		// First tri
		svVert = myPoints[iCurrPoint - 1].myPoint - svNorm*dHalfThickness;
	//	if(iCurrPoint != 1 && bDoExtend)
		if(bDoExtend)
			svVert = svVert - svExt;
		rDataOut.myVertexArray[iArrayOffset] = svVert.x;
		rDataOut.myVertexArray[iArrayOffset + 1] = svVert.y;
		rDataOut.myTexCoords[iArrayOffset] = 0;
		rDataOut.myTexCoords[iArrayOffset + 1]	= 0;
			
		svVert = myPoints[iCurrPoint - 1].myPoint + svNorm*dHalfThickness;
	//	if(iCurrPoint != 1 && bDoExtend)
		if(bDoExtend)
			svVert = svVert - svExt;
		rDataOut.myVertexArray[iArrayOffset + 2] = svVert.x;
		rDataOut.myVertexArray[iArrayOffset + 3] = svVert.y;
		rDataOut.myTexCoords[iArrayOffset + 2] = 1;
		rDataOut.myTexCoords[iArrayOffset + 3]	= 0;
			
		svVert = myPoints[iCurrPoint].myPoint + svNorm*dHalfThickness;
	//	if(iCurrPoint + 1 != iNumPoints && bDoExtend)
		if(bDoExtend)
			svVert = svVert + svExt;
		rDataOut.myVertexArray[iArrayOffset + 4] = svVert.x;
		rDataOut.myVertexArray[iArrayOffset + 5] = svVert.y;
		rDataOut.myTexCoords[iArrayOffset + 4] = 1;
		rDataOut.myTexCoords[iArrayOffset + 5]	= 1;
			
		// Second tri
		svVert = myPoints[iCurrPoint].myPoint + svNorm*dHalfThickness;
	//	if(iCurrPoint + 1 != iNumPoints && bDoExtend)
		if(bDoExtend)
			svVert = svVert + svExt;
		rDataOut.myVertexArray[iArrayOffset + 6] = svVert.x;
		rDataOut.myVertexArray[iArrayOffset + 7] = svVert.y;
		rDataOut.myTexCoords[iArrayOffset + 6] = 1;
		rDataOut.myTexCoords[iArrayOffset + 7]	= 1;
			
		svVert = myPoints[iCurrPoint].myPoint - svNorm*dHalfThickness;
	//	if(iCurrPoint + 1 != iNumPoints && bDoExtend)
		if(bDoExtend)
			svVert = svVert + svExt;
		rDataOut.myVertexArray[iArrayOffset + 8] = svVert.x;
		rDataOut.myVertexArray[iArrayOffset + 9] = svVert.y;
		rDataOut.myTexCoords[iArrayOffset + 8] = 0;
		rDataOut.myTexCoords[iArrayOffset + 9]	= 1;

		svVert = myPoints[iCurrPoint - 1].myPoint - svNorm*dHalfThickness;
	//	if(iCurrPoint != 1 && bDoExtend)
		if(bDoExtend)
			svVert = svVert - svExt;
		rDataOut.myVertexArray[iArrayOffset + 10] = svVert.x;
		rDataOut.myVertexArray[iArrayOffset + 11] = svVert.y;
		rDataOut.myTexCoords[iArrayOffset + 10] = 0;
		rDataOut.myTexCoords[iArrayOffset + 11]	= 0;
			
	//	dPoint1Opacity = dPoint2Opacity = 1.0;	
			
		// Color values
		// First tri
		rDataOut.myColorArray[iCArrayOffset] = 1;
		rDataOut.myColorArray[iCArrayOffset + 1] = 1;
		rDataOut.myColorArray[iCArrayOffset + 2] = 1;
		rDataOut.myColorArray[iCArrayOffset + 3] = dPoint1Opacity;

		rDataOut.myColorArray[iCArrayOffset + 4] = 1;
		rDataOut.myColorArray[iCArrayOffset + 5] = 1;
		rDataOut.myColorArray[iCArrayOffset + 6] = 1;
		rDataOut.myColorArray[iCArrayOffset + 7] = dPoint1Opacity;

		rDataOut.myColorArray[iCArrayOffset + 8] = 1;
		rDataOut.myColorArray[iCArrayOffset + 9] = 1;
		rDataOut.myColorArray[iCArrayOffset + 10] = 1;
		rDataOut.myColorArray[iCArrayOffset + 11] = dPoint2Opacity;

		// Second tri
		rDataOut.myColorArray[iCArrayOffset + 12] = 1;
		rDataOut.myColorArray[iCArrayOffset + 13] = 1;
		rDataOut.myColorArray[iCArrayOffset + 14] = 1;
		rDataOut.myColorArray[iCArrayOffset + 15] = dPoint2Opacity;

		rDataOut.myColorArray[iCArrayOffset + 16] = 1;
		rDataOut.myColorArray[iCArrayOffset + 17] = 1;
		rDataOut.myColorArray[iCArrayOffset + 18] = 1;
		rDataOut.myColorArray[iCArrayOffset + 19] = dPoint2Opacity;

		rDataOut.myColorArray[iCArrayOffset + 20] = 1;
		rDataOut.myColorArray[iCArrayOffset + 21] = 1;
		rDataOut.myColorArray[iCArrayOffset + 22] = 1;
		rDataOut.myColorArray[iCArrayOffset + 23] = dPoint1Opacity;
    }
}
#endif
/********************************************************************************************/
bool PathTrail::getIsDeleted(void)
{
    return myIsDeleted;
}
/********************************************************************************************/
void PathTrail::setConstOpacity(FLOAT_TYPE fValue)
{
	myConstOpacity = fValue;
}
/********************************************************************************************/
void PathTrail::clearTransparentPoints(void)
{
	GTIME lCurrEndTickAge; // , lFirstPointAge;
    GTIME lGlobalTime = Application::getInstance()->getGlobalTime(myClockType);

	if(myTrailFreqMult > 0 && lGlobalTime % myTrailFreqMult != 0)
		return;
	
	FLOAT_TYPE dFadePerTick = (FLOAT_TYPE)myFadeSpeed/(FLOAT_TYPE)Application::secondsToTicks(1.0);
	FLOAT_TYPE dPoint2Opacity;
	//TTrailPoints::iterator li1;
	int iCurrPoint, iNumPoints = myPoints.size();
//    for(iCurrPoint = 0; iCurrPoint < iNumPoints; iCurrPoint++)
	
//	for(li1 = myPoints.begin(), iCurrPoint = 0; li1 != myPoints.end(); li1++, iCurrPoint++)
	for(iCurrPoint = 0; iCurrPoint < iNumPoints; iCurrPoint++)
    {
//		svDir = myPoints[iCurrPoint].myPoint - myPoints[iCurrPoint - 1].myPoint;
//		dOrigLen = svDir.normalize();
		
//		svNorm.x = svDir.y;
//		svNorm.y = -svDir.x;
		
		
		//	svExt = svDir*(dHalfThickness/2.0);
//		svExt = svDir*(myExtendMultiplier*dOrigLen);
		
//		iArrayOffset = 12*(iCurrPoint - 1);
//		iCArrayOffset = 24*(iCurrPoint - 1);
		
//		lCurrStartTickAge = lGlobalTime - myPoints[iCurrPoint - 1].myTime; 
		lCurrEndTickAge = lGlobalTime - myPoints[iCurrPoint].myTime;
		//lCurrEndTickAge = lGlobalTime - (*li1).myTime;
		
//		dPoint1Opacity = 1.0 - (FLOAT_TYPE)lCurrStartTickAge*dFadePerTick;
		dPoint2Opacity = 1.0 - (FLOAT_TYPE)lCurrEndTickAge*dFadePerTick;
		if(dPoint2Opacity >= 0.02)
			break;
	}		
	
	if(iCurrPoint < iNumPoints && iCurrPoint > 1 && iCurrPoint - 2 > 0)
	//if(li1 != myPoints.begin())
	{
		// myPoints.erase(myPoints.begin(), myPoints.begin() + iCurrPoint - 1);
		//myPoints.erase(myPoints.begin(), li1 );
		// "Front" really means the start of all points
		myPoints.eraseFront(iCurrPoint - 1);
	}
}
/********************************************************************************************/
void PathTrail::generateLines(SVector2D svScroll, SGenData& rDataOut, FLOAT_TYPE dOpacityMult, FLOAT_TYPE fScale)
{
	clearTransparentPoints();
	
    int iCurrPoint;
	int iNumPoints = myPoints.size();
    if(iNumPoints < 2)
    {
		rDataOut.myNumLines = 0;
		return;
    }

//	TTrailPoints::iterator li1;
	
    int iArrayOffset, iCArrayOffset;
    // Two triangles per segment, with two coords each
  //  int iVArrayLen = iNumPoints*2;
	// Interleaved
	int iVArrayLen = iNumPoints;
    rDataOut.myNumLines = (iNumPoints - 1);
	
    // 4 color elems
//    int iCArraylen = iNumPoints*4;
    GTIME lCurrStartTickAge; 
    GTIME lGlobalTime = Application::getInstance()->getGlobalTime(myClockType);
	
    FLOAT_TYPE dFadePerTick = (FLOAT_TYPE)myFadeSpeed/(FLOAT_TYPE)Application::secondsToTicks(1.0);
    FLOAT_TYPE dPoint1Opacity;

	rDataOut.ensureCapacity(iVArrayLen, -1);
//	rDataOut.ensureCapacity(iVArrayLen, iCArraylen);

	iArrayOffset = 0;
	iCArrayOffset = 0;
	// for(iCurrPoint = iNumPoints - 1; iCurrPoint >= 0; iCurrPoint--)
//	iCurrPoint = iNumPoints - 1;
//	for(li1 = myPoints.end(); li1 != myPoints.begin(); )
	for(iCurrPoint = iNumPoints - 1; iCurrPoint >= 0; iCurrPoint--)
    {	
	//	li1--;
		lCurrStartTickAge = lGlobalTime - myPoints[iCurrPoint].myTime; 
	//	lCurrStartTickAge = lGlobalTime - (*li1).myTime; 
		
		dPoint1Opacity = 1.0 - (FLOAT_TYPE)lCurrStartTickAge*dFadePerTick;
		if(dPoint1Opacity > 1)
			dPoint1Opacity = 1.0;
		if(dPoint1Opacity < 0)
			dPoint1Opacity = 0.0;
		dPoint1Opacity *= dOpacityMult;

		if(myConstOpacity >= 0.0)
		{
			dPoint1Opacity = myConstOpacity;
		}
		
		if((iCurrPoint + 1) >= iNumPoints && dPoint1Opacity < 0.01)
//		if(li1 == (myPoints.end()--) && dPoint1Opacity < 0.01)
			myIsDeleted = true;

		rDataOut.myVerts[iArrayOffset].x = myPoints[iCurrPoint].myPoint.x - svScroll.x;
		rDataOut.myVerts[iArrayOffset].y = myPoints[iCurrPoint].myPoint.y - svScroll.y;
		
	//	rDataOut.myVerts[iArrayOffset].x = (*li1).myPoint.x - svScroll.x;
	//	rDataOut.myVerts[iArrayOffset].y = (*li1).myPoint.y - svScroll.y;
#ifdef USE_OPENGL2
		rDataOut.myVerts[iArrayOffset].z = 0.0;
		rDataOut.myVerts[iArrayOffset].w = 1.0;

		rDataOut.myVerts[iArrayOffset].r = myColor.r;
		rDataOut.myVerts[iArrayOffset].g = myColor.g;
		rDataOut.myVerts[iArrayOffset].b = myColor.b;
		rDataOut.myVerts[iArrayOffset].a = dPoint1Opacity*myColor.alpha;

#else

		rDataOut.myVerts[iArrayOffset].r = (unsigned char)(myColor.r*255.0);
		rDataOut.myVerts[iArrayOffset].g = (unsigned char)(myColor.g*255.0);
		rDataOut.myVerts[iArrayOffset].b = (unsigned char)(myColor.b*255.0);
		rDataOut.myVerts[iArrayOffset].a = (unsigned char)(dPoint1Opacity*myColor.alpha*255.0);
#endif		

/*		
		rDataOut.myVertexArray[iArrayOffset] = myPoints[iCurrPoint].myPoint.x - svScroll.x;
		rDataOut.myVertexArray[iArrayOffset + 1] = myPoints[iCurrPoint].myPoint.y - svScroll.y;
		
		rDataOut.myColorArray[iCArrayOffset] = myColor.r;
		rDataOut.myColorArray[iCArrayOffset + 1] = myColor.g;
		rDataOut.myColorArray[iCArrayOffset + 2] = myColor.b;
		rDataOut.myColorArray[iCArrayOffset + 3] = dPoint1Opacity*myColor.alpha;
	//	rDataOut.myColorArray[iCArrayOffset + 3] = 0.1;
		*/
		if(dPoint1Opacity < 0.01 && iCurrPoint < iNumPoints - 1)
		{
			rDataOut.myNumLines = iNumPoints - 1 - iCurrPoint;
			
			_ASSERT(rDataOut.myNumLines > 0);
			
			break;
		} 
		
		iArrayOffset++;

	//	iCurrPoint--;
		
//		iArrayOffset += 2;
//		iCArrayOffset += 4;

	}
}
/********************************************************************************************/
int PathTrail::getSampleFreq(void)
{
	return mySampleFreq;
}
/********************************************************************************************/
void PathTrail::setColor(FLOAT_TYPE fR, FLOAT_TYPE fG, FLOAT_TYPE fB, FLOAT_TYPE fA)
{
	myColor.set(fR, fG, fB, fA);
}
/********************************************************************************************/
void PathTrail::getColor(SColor& colOut)
{
	colOut = myColor;
}
/********************************************************************************************/
void PathTrail::simplifyPath(TPointVector& rPath, FLOAT_TYPE fAngleTolerance, FLOAT_TYPE fDistanceTolerance)
{
	int iNext, iNext2, iCurr, iNum = rPath.size();
	if(iNum <= 2)
		return;

	bool bIsClosed = false;

	// Remove points too close together
	if(fDistanceTolerance > 0)
	{
		for(iCurr = iNum - 2; iCurr >= 0; iCurr--)
		{
			iNext = iCurr + 1;
			if((rPath[iCurr] - rPath[iNext]).lengthSquared() <= fDistanceTolerance*fDistanceTolerance)
			{
				// Remove this point
				rPath.erase(rPath.begin() + iCurr);

//				iCurr++;
			}
		}

		iNum = rPath.size();
		if(iNum > 2)
		{
			if((rPath[iNum - 1] - rPath[0]).lengthSquared() <= fDistanceTolerance*fDistanceTolerance)
				bIsClosed = true;
		}
	}

	
	
	TPointVector rRes;

	// Remove points with too little angle
	SVector2D svTemp1, svTemp2;
	if(fAngleTolerance > 0)
	{
		int iStop = 1;
//		if(bIsClosed)
//			iStop = 1;
		iNum = rPath.size();

		rRes.push_back(rPath[iNum - 1]);

		FLOAT_TYPE dTolerance = F_COS_DEG(fAngleTolerance);
	//	for(iCurr = 0; iCurr < iNum; iCurr++)
	//	{
			for(iCurr = iNum - 1; iCurr >= iStop; iCurr--)
			{
				iNext = iCurr - 1;
				if(iNext < 0)
					iNext += rPath.size();
				iNext2 = iCurr - 2;
				if(iNext2 < 0)
					iNext2 = rPath.size() - 1;
//				if(iNext2 < 0)
//					iNext2 += rPath.size();
				svTemp1 = rPath[iCurr] - rPath[iNext];
				svTemp1.normalize();
				svTemp2 = rPath[iCurr] - rPath[iNext2];
				svTemp2.normalize();

				if(fabs(svTemp1.dot(svTemp2)) >= dTolerance)
				{
					// Remove the mid point
///					rPath.erase(rPath.begin() + iNext);
					//iCurr++;
				}
				else
					rRes.push_back(rPath[iNext]);
			}
	//	}

		rRes.push_back(rPath[0]);

		rPath = rRes;
	}


	if(rPath.size() <= 1)
		rPath.clear();
}
/********************************************************************************************/
void PathTrail::trimFromEndUseMarkers(FLOAT_TYPE fDistance)
{
	// Reassemble using markers, then break apart,
	FLOAT_TYPE fTotalLen = 0, fLen;
	SVector2D svDiff;
	int iCurr, iNum = myPoints.size();
	if(iNum <= 1)
		return;

	for(iCurr = iNum - 1; iCurr > 0; iCurr--)
	{
		if(myPoints[iCurr - 1].myMarker != 0)
			continue;

		svDiff = myPoints[iCurr - 1].myPoint - myPoints[iCurr].myPoint;
		fLen = svDiff.normalize();
		if(fTotalLen + fLen > fDistance)
			break;

		fTotalLen += fLen;
	}

	// Shift the markers
	int iMarker;
	for(iMarker = iNum - 1; iMarker > 0; iMarker--)
		myPoints[iMarker].myMarker = myPoints[iMarker - 1].myMarker;

	// Found the last point to fully add. Interp to the next one.
	SVector2D svLastPoint;
	_ASSERT(fDistance >= fTotalLen);
	if(iCurr >= 0)
		svLastPoint = myPoints[iCurr].myPoint + svDiff*(fDistance - fTotalLen);

	// "Front" really means the start of all points
	if(iCurr > 0)
	{
		int iOldMarker = myPoints[iCurr].myMarker;
		myPoints.eraseFront(iCurr - 1);
		myPoints[0].myPoint = svLastPoint;
		myPoints[0].myMarker = iOldMarker;
	}
}
/********************************************************************************************/
void PathTrail::trimFromEnd(FLOAT_TYPE fDistance)
{
	FLOAT_TYPE fTotalLen = 0, fLen;
	SVector2D svDiff;
	int iCurr, iNum = myPoints.size();
	if(iNum <= 1)
		return;
	for(iCurr = iNum - 1; iCurr > 0; iCurr--)
	{
		svDiff = myPoints[iCurr - 1].myPoint - myPoints[iCurr].myPoint;
		fLen = svDiff.normalize();
		if(fTotalLen + fLen > fDistance)
			break;

		fTotalLen += fLen;
	}

	// Found the last point to fully add. Interp to the next one.
	SVector2D svLastPoint;
	_ASSERT(fDistance >= fTotalLen);
	if(iCurr >= 0)
		svLastPoint = myPoints[iCurr].myPoint + svDiff*(fDistance - fTotalLen);

	// "Front" really means the start of all points
	if(iCurr > 0)
	{
		
#ifdef _DEBUG
		TPointVector dbBefore;
		this->getAsPointVector(dbBefore);
#endif
		
		myPoints.eraseFront(iCurr - 1);
		
#ifdef _DEBUG
		TPointVector dbAfter;
		this->getAsPointVector(dbAfter);
#endif
		myPoints[0].myPoint = svLastPoint;

#ifdef _DEBUG
		TPointVector dbFinal;
		this->getAsPointVector(dbFinal);
		int bp = 0;
#endif
		
	}
}
/********************************************************************************************/
void PathTrail::reverse()
{
	STrailPoint svTemp;
	int iCurr, iNum = myPoints.size();
	for(iCurr = 0; iCurr < iNum/2; iCurr++)
	{
		svTemp = myPoints[iCurr];
		myPoints[iCurr] = myPoints[iNum - iCurr - 1];
		myPoints[iNum - iCurr - 1] = svTemp;
	}
}
/********************************************************************************************/
void PathTrail::saveToItem(ResourceItem& rItemOut, const char* pcsTagId)
{
	ResourceItem* pOwnItem = rItemOut.addChild(pcsTagId);
	pOwnItem->setStringProp(PropertyId, pcsTagId);

	pOwnItem->setNumProp(PropertySvPTClockType, (int)myClockType);
	pOwnItem->setNumProp(PropertySvPTSampleFreq, mySampleFreq);
	pOwnItem->setNumProp(PropertySvPTFadeSpeed, myFadeSpeed);
	pOwnItem->setNumProp(PropertySvPTConstOpacity, myConstOpacity);
	pOwnItem->setBoolProp(PropertySvPTIsDeleted, myIsDeleted);
	pOwnItem->setNumProp(PropertySvPTThickness, myThickness);
	pOwnItem->setNumProp(PropertySvPTExtendMult, myExtendMultiplier);
	pOwnItem->setNumProp(PropertySvPTSampleAtlasFrame, mySampleAnimAtlasFrame);
	if(mySampleAnim)
		pOwnItem->setStringProp(PropertySvPTAnimName, mySampleAnim->getName());
	pOwnItem->setAsColor(PropertySvPTColor, myColor);
	pOwnItem->setNumProp(PropertySvPTOffsetX, myOffset.x);
	pOwnItem->setNumProp(PropertySvPTOffsetY, myOffset.y);
	pOwnItem->setNumProp(PropertySvPTFreqMult, myTrailFreqMult);
	pOwnItem->setNumProp(PropertySvPTMaxPoints, myMaxPoints);
	pOwnItem->setNumProp(PropertySvPTMinPointDist, myMinPointDistance);
	pOwnItem->setNumProp(PropertySvPTMaxPointDist, myMaxPointDistance);
	pOwnItem->setNumProp(PropertySvPTAngleTol, myAngleTolerance);
	pOwnItem->setBoolProp(PropertySvPTIsEnabled, myIsEnabled);
	pOwnItem->setNumProp(PropertySvPTAddRot, myAdditionalRotation);


	char pcsBuff[128];
	int iCurr, iNum = myPoints.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pOwnItem->addToNumericEnumProp(PropertySvPTPosX, myPoints[iCurr].myPoint.x);
		pOwnItem->addToNumericEnumProp(PropertySvPTPosY, myPoints[iCurr].myPoint.y);

		pOwnItem->addToNumericEnumProp(PropertySvPTMarker, myPoints[iCurr].myMarker);

		sprintf(pcsBuff, "%c%lld", SAVE_LONG_AS_STRING_SYMBOL, myPoints[iCurr].myTime);
		pOwnItem->addToEnumProp(PropertySvPTTime, pcsBuff);
	}
}
/********************************************************************************************/
void PathTrail::loadFromItem(ResourceItem* pItem, DrawingCache* pDrawingCache)
{
	PropertyType eCurrPropName;

	clear();
	mySampleAnim = NULL;

	STrailPoint rTrailPoint;

	// Traverse all properties
	ResourceItem::PropertyIterator pi;
	for(pi = pItem->propertiesFirst(); !pi.isEnd(); pi++)
	{
		eCurrPropName = pi.getProperty();
		if(eCurrPropName == PropertyId)
		{
			// Do nothing
		}
		else if(eCurrPropName == PropertySvPTClockType)
			myClockType = (ClockType)(int)pItem->getNumProp(eCurrPropName);
		else if(eCurrPropName == PropertySvPTSampleFreq)
			mySampleFreq = pItem->getNumProp(eCurrPropName);
		else if(eCurrPropName == PropertySvPTFadeSpeed)
			myFadeSpeed = pItem->getNumProp(eCurrPropName);
		else if(eCurrPropName == PropertySvPTConstOpacity)
			myConstOpacity = pItem->getNumProp(eCurrPropName);
		else if(eCurrPropName == PropertySvPTIsDeleted)
			myIsDeleted = pItem->getBoolProp(eCurrPropName);
		else if(eCurrPropName == PropertySvPTThickness)
			myThickness = pItem->getNumProp(eCurrPropName);
		else if(eCurrPropName == PropertySvPTExtendMult)
			myExtendMultiplier = pItem->getNumProp(eCurrPropName);
		else if(eCurrPropName == PropertySvPTSampleAtlasFrame)
			mySampleAnimAtlasFrame = pItem->getNumProp(eCurrPropName);
		else if(eCurrPropName == PropertySvPTAnimName)
		{
			int iDummy;
			mySampleAnim = pDrawingCache->getCachedSequence(pItem->getStringProp(eCurrPropName), &iDummy);
		}
		else if(eCurrPropName == PropertySvPTColor)
			 pItem->getAsColor(eCurrPropName, myColor);
		else if(eCurrPropName == PropertySvPTOffsetX)
			myOffset.x = pItem->getNumProp(eCurrPropName);
		else if(eCurrPropName == PropertySvPTOffsetY)
			myOffset.y = pItem->getNumProp(eCurrPropName);
		else if(eCurrPropName == PropertySvPTFreqMult)
			myTrailFreqMult = pItem->getNumProp(eCurrPropName);
		else if(eCurrPropName == PropertySvPTMaxPoints)
			myMaxPoints = pItem->getNumProp(eCurrPropName);
		else if(eCurrPropName == PropertySvPTMinPointDist)
			myMinPointDistance = pItem->getNumProp(eCurrPropName);
		else if(eCurrPropName == PropertySvPTMaxPointDist)
			myMaxPointDistance = pItem->getNumProp(eCurrPropName);
		else if(eCurrPropName == PropertySvPTAngleTol)
			myAngleTolerance = pItem->getNumProp(eCurrPropName);
		else if(eCurrPropName == PropertySvPTIsEnabled)
			myIsEnabled = pItem->getBoolProp(eCurrPropName);
		else if(eCurrPropName == PropertySvPTAddRot)
			myAdditionalRotation = pItem->getNumProp(eCurrPropName);
		else if(eCurrPropName == PropertySvPTPosX)
		{
			int iCurr, iNum = pItem->getNumericEnumPropCount(eCurrPropName);
			for(iCurr = 0; iCurr < iNum; iCurr++)
			{
				rTrailPoint.myPoint.x = pItem->getNumericEnumPropValue(eCurrPropName, iCurr);
				rTrailPoint.myPoint.y = pItem->getNumericEnumPropValue(PropertySvPTPosY, iCurr);

				rTrailPoint.myMarker = pItem->getNumericEnumPropValue(PropertySvPTMarker, iCurr);

				rTrailPoint.myTime = atol(pItem->getEnumPropValue(PropertySvPTTime, iCurr));

				myPoints.push_back(rTrailPoint);
			}
		}
		else if(eCurrPropName == PropertySvPTPosY)
		{
			// Do nothing
		}
		else if(eCurrPropName == PropertySvPTTime)
		{
			// Do nothing
		}
		else if(eCurrPropName == PropertySvPTMarker)
		{
			// Do nothing
		}
		else
		{
			_ASSERT(0);
		}
	}

	// Traverse all *children* not properties
	int iChild, iNumChildren = pItem->getNumChildren();
	const char *pcsTag;
	ResourceItem* pChild;
	for(iChild = 0; iChild < iNumChildren; iChild++)
	{
		pChild = pItem->getChild(iChild);
		pcsTag = pChild->getTag();
		
		{
			// Unaccounted game engine tag
			_ASSERT(0);
		}
	}
}
/********************************************************************************************/
void PathTrail::reassembleAsSinglePath(TPointVector& rPointsOut, TPointVector& rBreakPointOffsetsOut, TPointVector& rBreakPointsOut, TPointVector& rBreakInDirsOut, bool bReverse)
{
	SVector2D svTemp;
	rPointsOut.clear();
	int iCurr, iNumPoints = myPoints.size();

	rBreakInDirsOut.clear();
	rBreakPointsOut.clear();
	rPointsOut.clear();
	rBreakPointOffsetsOut.clear();

	if(iNumPoints < 2)
		return;

	SVector2D svDir;
	SVector2D svBreakOffset;
	rBreakPointOffsetsOut.push_back(svBreakOffset);
	svDir = myPoints[1].myPoint - myPoints[0].myPoint;
	svDir.normalize();
	rBreakInDirsOut.push_back(svDir);
	//rBreakPointsOut.push_back(myPoints[0].myPoint);
	for(iCurr = 0; iCurr < iNumPoints; iCurr++)
	{
		svTemp.x = myPoints[iCurr].myPoint.x - svBreakOffset.x;
		svTemp.y = myPoints[iCurr].myPoint.y - svBreakOffset.y;
		rPointsOut.push_back(svTemp);

		if(myPoints[iCurr].myMarker != 0 && iCurr + 1 < iNumPoints)
		{
			svBreakOffset = myPoints[iCurr + 1].myPoint - myPoints[iCurr].myPoint;
			rBreakPointOffsetsOut.push_back(svBreakOffset);
			rBreakPointsOut.push_back(myPoints[iCurr].myPoint);
			rBreakPointsOut.push_back(myPoints[iCurr + 1].myPoint);

			if(iCurr > 0)
			{
				svDir = myPoints[iCurr].myPoint - myPoints[iCurr - 1].myPoint;
				svDir.normalize();
			}
			else
				svDir.set(1,0);
			rBreakInDirsOut.push_back(svDir);

			svDir.set(0, 0);
			rBreakInDirsOut.push_back(svDir);
		}
	}

	if(bReverse)
	{
		std::reverse(rPointsOut.begin(), rPointsOut.end());
		std::reverse(rBreakPointOffsetsOut.begin(), rBreakPointOffsetsOut.end());
		std::reverse(rBreakInDirsOut.begin(), rBreakInDirsOut.end());
		std::reverse(rBreakPointsOut.begin(), rBreakPointsOut.end());
	}
}
/********************************************************************************************/
};