#pragma once

/*****************************************************************************/
class HYPERUI_API CombinedTransform
{
public:

	void setFrom(const SVector2D& svPreTranslate, const SMatrix2D& smMatrixIn, const SVector2D& svPostTranslate);
	bool invert();
	void reset();

	//inline void getPreTranslation(SVector2D& svOut) { svOut = myPreTranslation; }
	//inline void getPostTranslation(SVector2D& svOut) { svOut = myPostTranslation; }
	inline SVector2D operator*(const SVector2D& svOther) const { return myMatrix*(svOther + myPreTranslation) + myPostTranslation; }
	inline void getMatrix(SMatrix2D& smMatrix) const { smMatrix = myMatrix; }

	void getCombinedMatrix(SMatrix2D& smMatrixOut) const;

private:

	SVector2D myPreTranslation;
	SMatrix2D myMatrix;
	SVector2D myPostTranslation;
};
/*****************************************************************************/