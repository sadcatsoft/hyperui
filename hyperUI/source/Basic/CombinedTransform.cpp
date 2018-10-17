#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
void CombinedTransform::setFrom(const SVector2D& svPreTranslate, const SMatrix2D& smMatrixIn, const SVector2D& svPostTranslate)
{
	myPreTranslation = svPreTranslate;
	myPostTranslation = svPostTranslate;
	myMatrix = smMatrixIn;
}
/*****************************************************************************/
bool CombinedTransform::invert()
{
	SVector2D svTemp;
	svTemp = myPostTranslation;
	myPostTranslation = myPreTranslation*-1.0;
	myPreTranslation = svTemp*-1.0;

	//myPostTranslation *= -1.0;
	return myMatrix.invert();
}
/*****************************************************************************/
void CombinedTransform::reset()
{
	myMatrix.resetToIdentity();
	myPostTranslation.set(0, 0);
	myPreTranslation.set(0, 0);
}
/*****************************************************************************/
void CombinedTransform::getCombinedMatrix(SMatrix2D& smMatrixOut) const
{
	smMatrixOut.leftAppendTranslationMatrix(myPreTranslation.x, myPreTranslation.y);
	smMatrixOut.multiplyLeft(myMatrix);
	smMatrixOut.leftAppendTranslationMatrix(myPostTranslation.x, myPostTranslation.y);
}
/*****************************************************************************/
};