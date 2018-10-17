#include "stdafx.h"

#define SAVE_MAT_NAME				"svMatName"
#define SAVE_MAT_FILL_COLOR			"svMatFillCol"
#define SAVE_MAT_FILL_ENABLED		"svMatFillEnbl"

#define SAVE_MAT_STROKE_ISENABLED			"svMatStrkEnbl"
#define SAVE_MAT_STROKE_DASH_ISENABLED		"svMatStrkDashEnbl"
#define SAVE_MAT_STROKE_COLOR				"svMatStrkCol"
#define SAVE_MAT_STROKE_THICKNESS			"svMatStrkThck"
#define SAVE_MAT_STROKE_DASH_PATTERN		"svMatStrkPtrn"

namespace HyperUI
{
/*****************************************************************************/
Material::Material(UNIQUEID_TYPE id)
{
	initCommon(id);
}
/*****************************************************************************/
Material::Material()
{
	initCommon(0);
}
/*****************************************************************************/
void Material::initCommon(UNIQUEID_TYPE id)
{
	myId = id;
	StringUtils::numberToString(id, myName);
	myName = "Material " + myName;

	myIsStrokeEnabled = false;
	myIsStrokeDashEnabled = false;
	myIsFillEnabled = true;
	myStrokeThickness = 1.0;
}
/*****************************************************************************/
Material::~Material()
{
}
/*****************************************************************************/
Material* Material::cloneSelf() const
{
	Material* pNew = new Material(myId);
	pNew->copyFrom(*this, true, true);
	return pNew;
}
/*****************************************************************************/
void Material::copyFrom(const Material& rOther, bool bCopyId, bool bCopyName)
{
	if(bCopyId)
		myId = rOther.myId;
	if(bCopyName)
		myName = rOther.myName;
	myFillColor = rOther.myFillColor;

	myIsStrokeEnabled = rOther.myIsStrokeEnabled;
	myIsStrokeDashEnabled = rOther.myIsStrokeDashEnabled;
	myStrokeColor = rOther.myStrokeColor;
	myStrokeThickness = rOther.myStrokeThickness;
	myDashPattern = rOther.myDashPattern;
	myIsFillEnabled = rOther.myIsFillEnabled;
}
/*****************************************************************************/
bool Material::isEqualContentTo(const Material& rOther) const
{
	if(myFillColor != rOther.myFillColor)
		return false;

	if(myIsStrokeEnabled != rOther.myIsStrokeEnabled)
		return false;

	if(myIsFillEnabled != rOther.myIsFillEnabled)
		return false;

	if(myIsStrokeDashEnabled != rOther.myIsStrokeDashEnabled)
		return false;

	if(myStrokeColor != rOther.myStrokeColor)
		return false;

	if(myStrokeThickness != rOther.myStrokeThickness)
		return false;

	if(myDashPattern != rOther.myDashPattern)
		return false;

	return true;
}
/*****************************************************************************/
void Material::saveToItem(StringResourceItem& rItemOut) const
{
	rItemOut.setAsLong(SAVE_MAT_ID, myId);
	rItemOut.setStringProp(SAVE_MAT_NAME, myName.c_str());
	rItemOut.setAsColor(SAVE_MAT_FILL_COLOR, myFillColor);
	rItemOut.setBoolProp(SAVE_MAT_FILL_ENABLED, myIsFillEnabled);

	rItemOut.setBoolProp(SAVE_MAT_STROKE_ISENABLED, myIsStrokeEnabled);
	rItemOut.setBoolProp(SAVE_MAT_STROKE_DASH_ISENABLED, myIsStrokeDashEnabled);
	rItemOut.setAsColor(SAVE_MAT_STROKE_COLOR, myStrokeColor);
	rItemOut.setNumProp(SAVE_MAT_STROKE_THICKNESS, myStrokeThickness);
	rItemOut.setFromVector(SAVE_MAT_STROKE_DASH_PATTERN, myDashPattern);
}
/*****************************************************************************/
void Material::loadFromItem(const StringResourceItem& rItem)
{
	myName = rItem.getStringProp(SAVE_MAT_NAME);
	rItem.getAsColor(SAVE_MAT_FILL_COLOR, myFillColor);
	_ASSERT(rItem.getAsLong(SAVE_MAT_ID) == myId);
	myIsFillEnabled = rItem.getBoolProp(SAVE_MAT_FILL_ENABLED);

	myIsStrokeEnabled = rItem.getBoolProp(SAVE_MAT_STROKE_ISENABLED);
	myIsStrokeDashEnabled = rItem.getBoolProp(SAVE_MAT_STROKE_DASH_ISENABLED);
	rItem.getAsColor(SAVE_MAT_STROKE_COLOR, myStrokeColor);
	myStrokeThickness = rItem.getNumProp(SAVE_MAT_STROKE_THICKNESS);
	rItem.getAsNumVector(SAVE_MAT_STROKE_DASH_PATTERN, myDashPattern);
}
/*****************************************************************************/
void Material::multiplyByOpacity(FLOAT_TYPE fOpacity)
{
	myFillColor.alpha *= fOpacity;
	myStrokeColor.alpha *= fOpacity;
}
/*****************************************************************************/
void Material::clear()
{
	initCommon(0);
	myIsFillEnabled = false;
}
/*****************************************************************************/
void Material::scale(FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY)
{
	myStrokeThickness *= fScaleX;
	int iDash, iNum = myDashPattern.size();
	for(iDash = 0; iDash < iNum; iDash++)
		myDashPattern[iDash] *= fScaleX;
}
/*****************************************************************************/
};