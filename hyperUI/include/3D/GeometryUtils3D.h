#pragma once

class CachedFrame3D;
class CachedPart3D;
/*****************************************************************************/
class HYPERUI_API GeometryUtils3D
{
public:

	static void extrude(TPointVector& rPoints, SVector3D& svDir, FLOAT_TYPE fAmount, bool bFlipNormal, IGeoData* pGeoDataOut);

	/// Computes the plane normal from 3 points. not more, not less.
	static void computeNormal(SVector3D* svPoints, SVector3D& svNormalOut);

	/// Computes the signed area of the triangle
	static FLOAT_TYPE computeTriangleArea(SVector3D& svPoint1, SVector3D& svPoint2, SVector3D& svPoint3);

	/// Computes tangent and bitangent of a triangle.
	static void computeTangentAndBiTangent(SVector3D rPoints[3], SVector3D& svNormal, FLOAT_TYPE fUCoords[3], FLOAT_TYPE fVCoords[3], SVector3D& svTangentOut, SVector3D& svBitangentOut);

	/// Blends between two frames. Interp at 0 is pFrom, at 1 is pTo. Frames are assumed to have the same
	/// number of parts and vertices. Assumes all the parts in the rOut frame are already allocated.
	static void blendFrame(CachedFrame3D* pFrom, CachedFrame3D* pTo, FLOAT_TYPE fInterp, CachedFrame3D& rOut);

	/// Same as blendFrame(), but for parts.
	static void blendPart(CachedPart3D* pFrom, CachedPart3D* pTo, FLOAT_TYPE fInterp, CachedPart3D& rOut);

	static FLOAT_TYPE rayCircleIntersection(const SVector3D& svRayOrigin, const SVector3D& svRayDir, const SVector3D& svDiscCenter, const SVector3D& svNormal, FLOAT_TYPE fDiscRadius, SVector3D* pInterPointOut = NULL, FLOAT_TYPE* pDiscDistOut = NULL);

private:

};
/*****************************************************************************/