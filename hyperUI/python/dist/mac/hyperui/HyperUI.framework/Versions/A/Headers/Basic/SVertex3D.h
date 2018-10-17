#pragma once

/*****************************************************************************/
struct HYPERUI_API SVertex3DSaveCore
{
	float x,y,z;
#ifdef USE_OPENGL2
	float w;
#endif
	float nx,ny,nz;
};
/*****************************************************************************/
struct HYPERUI_API SVertex3D
{
	float x,y,z;
#ifdef USE_OPENGL2
	float w;
#endif
	float nx,ny,nz;
	float u,v;
	// No padding if only above
#ifdef USE_OPENGL2
#ifdef ENABLE_NORMAL_MAPPING
	float tangent_x,tangent_y,tangent_z;
	float bitangent_x,bitangent_y,bitangent_z;
	char padding[4];   // Padding
#else
	char padding[8];   // Padding
#endif
#else
	// No padding needed...
	// char padding[12];   // Padding
#endif

#ifdef USE_OPENGL2
	SVertex3D() { w = 1.0; }
	inline static int getNumVertexComponents() { return 4; }
#else
	inline static int getNumVertexComponents() { return 3; }
#endif	

};
/*****************************************************************************/
