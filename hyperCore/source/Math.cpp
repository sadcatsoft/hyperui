#include "stdafx.h"
#include "Math.h"

namespace HyperCore
{
FLOAT_TYPE *BasicMath::theSinLookupTable = NULL;
FLOAT_TYPE *BasicMath::theCosLookupTable = NULL;
FLOAT_TYPE *BasicMath::theSinDegLookupTable = NULL;
FLOAT_TYPE *BasicMath::theCosDegLookupTable = NULL;
int BasicMath::theSinCosDegArrayLength = 0;
int BasicMath::theSinCosRadArrayLength = 0;
/*****************************************************************************/
void BasicMath::initialize()
{
	if(theCosLookupTable)
		return;

	FLOAT_TYPE fAngle, fStep;
	int iArrayPos;
	FLOAT_TYPE fAllocSize;

	/// fastCos()
	fStep = SIN_COS_STEP;
	fAllocSize = ((FLOAT_TYPE)(2.0f*M_PI)/(FLOAT_TYPE)fStep) + 1.0f;
	theSinCosRadArrayLength = (int)fAllocSize;
	theCosLookupTable = new FLOAT_TYPE[(int)fAllocSize];
	for(iArrayPos = 0, fAngle = 0.0; fAngle <= 2.0*M_PI; fAngle += fStep, iArrayPos++)
		theCosLookupTable[iArrayPos] = cos(fAngle);


	/// fastSin()
	fStep = SIN_COS_STEP;
	fAllocSize = ((FLOAT_TYPE)(2.0f*M_PI)/(FLOAT_TYPE)fStep) + 1.0f;
	theSinLookupTable = new FLOAT_TYPE[(int)fAllocSize];
	for(iArrayPos = 0, fAngle = 0.0; fAngle <= 2.0*M_PI; fAngle += fStep, iArrayPos++)
		theSinLookupTable[iArrayPos] = sin(fAngle);

	/// fastCosDeg()
	fStep = SIN_COS_STEP_DEG;
	fAllocSize = ((FLOAT_TYPE)(360.0)/(FLOAT_TYPE)fStep) + 1.0f;
	theSinCosDegArrayLength = (int)fAllocSize;
	theCosDegLookupTable = new FLOAT_TYPE[(int)fAllocSize];
	for(iArrayPos = 0, fAngle = 0.0; fAngle <= 360.0; fAngle += fStep, iArrayPos++)
		theCosDegLookupTable[iArrayPos] = cos(fAngle/180.0f*(FLOAT_TYPE)M_PI);

	/// fastSinDeg()
	fStep = SIN_COS_STEP_DEG;
	fAllocSize = ((FLOAT_TYPE)(360.0)/(FLOAT_TYPE)fStep) + 1.0f;
	theSinDegLookupTable = new FLOAT_TYPE[(int)fAllocSize];
	for(iArrayPos = 0, fAngle = 0.0; fAngle <= 360.0; fAngle += fStep, iArrayPos++)
		theSinDegLookupTable[iArrayPos] = sin(fAngle/180.0f*(FLOAT_TYPE)M_PI);
}
/*****************************************************************************/
void normalizeProbList(TFloatVector& rProbsIn, bool bInvert, FLOAT_TYPE fMinProb)
{
	normalizeProbList(&rProbsIn[0], rProbsIn.size(), bInvert, fMinProb);
}
/*****************************************************************************/
void normalizeProbList(FLOAT_TYPE* fProbs, int iNum, bool bInvert, FLOAT_TYPE fMinProb)
{
	if(iNum <= 0)
		return;

	FLOAT_TYPE iWeight = 0;
	int iDummy;
	for(iDummy = 0; iDummy < iNum; iDummy++)
		iWeight += fProbs[iDummy];

	if(bInvert)
	{
		for(iDummy = 0; iDummy < iNum; iDummy++)
			fProbs[iDummy] = iWeight - fProbs[iDummy];

		iWeight = 0;
		for(iDummy = 0; iDummy < iNum; iDummy++)
			iWeight += fProbs[iDummy];
	}

	bool bRenomalizeAgain = false;
	for(iDummy = 0; iDummy < iNum; iDummy++)
	{
		fProbs[iDummy] = (FLOAT_TYPE)fProbs[iDummy]/(FLOAT_TYPE)iWeight;
		if(fMinProb > 0 && fProbs[iDummy] < fMinProb)
		{
			bRenomalizeAgain = true;
			fProbs[iDummy] = fMinProb;
		}
	}

	iWeight = 0;
	for(iDummy = 0; iDummy < iNum; iDummy++)
		iWeight += fProbs[iDummy];
	for(iDummy = 0; iDummy < iNum; iDummy++)
		fProbs[iDummy] = (FLOAT_TYPE)fProbs[iDummy]/(FLOAT_TYPE)iWeight;
}
/********************************************************************************************/
int forcePowerOfTwo(FLOAT_TYPE fValue)
{
	// We have to do this since if we have something like 32.01,
	// we actually *must* go to 64, and not truncate it.
	// Otherwise text, for example, gets cut off.
	int iValue = fValue;
	if(fValue > (FLOAT_TYPE)iValue)
		iValue++;

	int iRes = iValue;
	FLOAT_TYPE fPow = log((FLOAT_TYPE)iValue)/log(2.0);
	if(fabs(fPow - (FLOAT_TYPE)((int)fPow)) > FLOAT_EPSILON)
	{
		iRes = (int) pow(2.0, ((int)fPow) + 1);
	}

	return iRes;
}
/********************************************************************************************/
bool isValidNumber(FLOAT_TYPE fValue)
{
	if(fValue != fValue)
		return false;

	if(!(fValue <= FLOAT_TYPE_MAX && fValue  >= -FLOAT_TYPE_MAX))
		return false;

	return true;
}
/********************************************************************************************/
};