#pragma once

#define POLYLINE_TYPE TPolyline2D< POINT_TYPE >

enum BooleanOpType
{
	BooleanOpUnion = 0,
	BooleanOpSubtraction,
	BooleanOpIntersection
};

/********************************************************************************************/
template < class POINT_TYPE >
class HYPERUI_API TPolylineCollection
{
public:

	TPolylineCollection();
	~TPolylineCollection();

	void clear();
	POLYLINE_TYPE* addPolyline();

	int getNumLines() const { return myLines.size(); }
	inline POLYLINE_TYPE* getLine(int iIndex) { return myLines[iIndex]; }

	void reverse();

	void removeCoincident(FLOAT_TYPE fDistance);

#ifdef HAVE_POLYBOOL
	::POLYBOOLEAN::PAREA* convertToPBArea() const;
	void setFromPBArea(const ::POLYBOOLEAN::PAREA* pSrcArea);
#endif

	void transformBy(const SMatrix2D& smMatrix);
	void translate(FLOAT_TYPE fX, FLOAT_TYPE fY);

	void copyFrom(const TPolylineCollection& rOther);

private:

	vector < POLYLINE_TYPE* > myLines;
};
/*****************************************************************************/
typedef TPolylineCollection < SVector2D > PolylineCollection;
