#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
UISolidColorElement::UISolidColorElement(UIPlane* pParentPlane)
	: UIElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
void UISolidColorElement::onAllocated(IBaseObject* pData)
{
	UIElement::onAllocated(pData);
}
/*****************************************************************************/
void UISolidColorElement::render(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	preRender(svScroll, fOpacity, fScale);	

	// Render the background of the corresponding color
	SColor scolOut;
	this->getAsColor(PropertyBackColor, scolOut);

	SVector2D svPos;
	FLOAT_TYPE fFinalOpac, fLocScale;
	this->getLocalPosition(svPos, &fFinalOpac, &fLocScale);
	fFinalOpac *= fOpacity;

	SVector2D svSize;
	this->getBoxSize(svSize);

	svSize *= fLocScale*fScale;
	if(this->getParent())
		svPos *= fScale;

	scolOut.alpha *= fFinalOpac;

	// OMG... we flush every time we have a solid element... but they're
	// now used for cell backgrounds in brush collections, etc. We could
	// have hundreds of them, literally, in a given frame...
	// But dialogs don't work otherwise...

	SRect2D srRect;
	srRect.x = svPos.x + svScroll.x - svSize.x/2.0;
	srRect.y = svPos.y + svScroll.y - svSize.y/2.0;
	srRect.w = svSize.x;
	srRect.h = svSize.y;

	if(RenderUtils::doesIntersectScissorRect(srRect))
	{
		getDrawingCache()->flush();
		getDrawingCache()->addRectangle(srRect, scolOut);
		//getDrawingCache()->addSprite("smallButtonHiAnim", srRect.center().x, srRect.center().y, fOpacity, 0, 1.0, 1.0, true);
		getDrawingCache()->flush();
	}

	renderTextInternal(svScroll, fOpacity, fScale, false);

	renderChildren(svScroll, fOpacity, fScale);
	postRender(svScroll, fOpacity, fScale);

	
}
/*****************************************************************************/
};