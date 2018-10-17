#pragma once

#define POLYLINE_COLL_TYPE TPolylineCollection< POINT_TYPE >
/********************************************************************************************/
template < class POINT_TYPE >
class HYPERUI_API TPolygon2DCollection
{
public:
	TPolygon2DCollection();
	~TPolygon2DCollection();

	void clear();
	POLYLINE_COLL_TYPE* addPolygon();

	void performBooleanOp(const TPolygon2DCollection& rOther, BooleanOpType eOpType, TPolygon2DCollection& rResultOut) const;

private:

#ifdef HAVE_POLYBOOL
	::POLYBOOLEAN::PAREA* convertToPBArea() const;
	void setFromPBArea(const ::POLYBOOLEAN::PAREA* pSrcArea);
#endif

private:

	vector < POLYLINE_COLL_TYPE* > myPolygons;
};
/*****************************************************************************/
typedef TPolygon2DCollection < SVector2D > Polygon2DCollection;