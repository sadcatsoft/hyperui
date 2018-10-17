#pragma once

struct SBrushPoint;
struct SColorPoint;
/***********************************************************************************************************/
struct SVertexInfo
{
	float x, y;
#ifdef USE_OPENGL2
	float z, w;
#endif
	// Texture coords
	float s0, t0;

#ifdef USE_OPENGL2
	float r,g,b,a;
	float padding[6];
#else
	unsigned char r,g,b,a;
	float padding[3];
#endif

#ifdef USE_OPENGL2
	inline static int getNumVertexComponents() { return 4; }
#else
	inline static int getNumVertexComponents() { return 2; }
#endif

	void copyFrom(const SVector2D& svIn) { x = svIn.x; y = svIn.y; }
	HYPERUI_API void copyFrom(const SBrushPoint& svIn);
	HYPERUI_API void copyFrom(const SColorPoint& svIn);
};
/***********************************************************************************************************/
typedef TLossyDynamicArray< SVertexInfo > TVertexLossyDynamicArray;
