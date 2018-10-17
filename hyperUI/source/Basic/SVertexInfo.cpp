#include "stdafx.h"

namespace HyperUI
{
/***********************************************************************************************************/
void SVertexInfo::copyFrom(const SBrushPoint& svIn) 
{ 
	x = svIn.getPoint().x; 
	y = svIn.getPoint().y; 
}
/***********************************************************************************************************/
void SVertexInfo::copyFrom(const SColorPoint& svIn) 
{ 
	x = svIn.getPoint().x; 
	y = svIn.getPoint().y; 
	const SColor& scol = svIn.getColor();

#ifdef USE_OPENGL2
	r = scol.alpha*scol.r;
	g = scol.alpha*scol.g;
	b = scol.alpha*scol.b;
	a = scol.alpha;
#else
	r = scol.r*scol.alpha*255.0;
	g = scol.g*scol.alpha*255.0;
	b = scol.b*scol.alpha*255.0;
	a = scol.alpha*255.0;
#endif
}
/***********************************************************************************************************/
};