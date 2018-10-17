#include "stdafx.h"

#define PropertySvGradStopIsOpacity			"svGradStopIsOpacity"
#define PropertySvGradStopColor				"svGradStopColor"
#define PropertySvGradStopSecColor			"svGradStopSecColor"
#define PropertySvGradStopId				"svGradStopId"
#define PropertySvGradStopPos				"svGradStopPos"
#define PropertySvGradStopLCenter			"svGradStopLCenter"
#define PropertySvGradStopRCenter			"svGradStopRCenter"
#define PropertySvColStopIsSplt				"svColStopIsSplt"

namespace HyperCore
{
/*****************************************************************************/
void SGradientStop::saveToItem(StringResourceItem& rItemOut) const
{
	rItemOut.setBoolProp(PropertySvGradStopIsOpacity, this->myIsOpacityStop);
	rItemOut.setAsColor(PropertySvGradStopColor, myColor);
	rItemOut.setAsColor(PropertySvGradStopSecColor, mySecondaryColor);
	rItemOut.setAsLong(PropertySvGradStopId, myId);
	rItemOut.setNumProp(PropertySvGradStopPos, myPos);
	rItemOut.setNumProp(PropertySvGradStopLCenter, myLeftInterpCenter);
	rItemOut.setNumProp(PropertySvGradStopRCenter, myRightInterpCenter);
	rItemOut.setBoolProp(PropertySvColStopIsSplt, myIsSplit);
}
/*****************************************************************************/
void SGradientStop::loadFromItem(StringResourceItem& rItemIn)
{
	// 	SColor scolTemp;
	// 	bool bResult;

	myIsOpacityStop = rItemIn.getBoolProp(PropertySvGradStopIsOpacity);
	// 	bResult = pScene ? pScene->evaluateColorVariable(rItemIn.getStringProp(PropertySaveGradStopColor), myColor) : false;
	// 	if(!bResult)
	//	rItemIn.getAsColor(PropertySaveGradStopColor, myColor);
	rItemIn.getAsColor(PropertySvGradStopColor, myColor);
	// 	bResult = pScene ? pScene->evaluateColorVariable(rItemIn.getStringProp(PropertySaveGradStopSecColor), mySecondaryColor) : false;
	// 	if(!bResult)
	rItemIn.getAsColor(PropertySvGradStopSecColor, mySecondaryColor);
	myId = rItemIn.getAsLong(PropertySvGradStopId);
	setPos(rItemIn.getNumProp(PropertySvGradStopPos));
	myLeftInterpCenter = rItemIn.getNumProp(PropertySvGradStopLCenter);
	myRightInterpCenter = rItemIn.getNumProp(PropertySvGradStopRCenter);
	myIsSplit = rItemIn.getBoolProp(PropertySvColStopIsSplt);
}
/*****************************************************************************/
};