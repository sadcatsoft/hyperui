#include "stdafx.h"

#define TOP_PADDING			upToScreen(20.0)
#define BOTTOM_PADDING		upToScreen(5.0)
#define LEFT_PADDING		upToScreen(5.0)
#define RIGHT_PADDING		upToScreen(0.0)

#define LINE_TICK_SIZE		upToScreen(2.0)
#define AXES_THICKNESS		upToScreen(0.51)

#define POINT_CLICK_RADIUS		upToScreen(4.0)

#define CHART_SCALE		255.0

namespace HyperUI
{

const SColor g_colAxesColor(1.0, 1, 1, 0.40);

/*****************************************************************************/
UICurveEditor::UICurveEditor(UIPlane* pParentPlane)
	: UIElement(pParentPlane)
{
	onAllocated(pParentPlane);
}
/*****************************************************************************/
UICurveEditor::~UICurveEditor()
{
	onDeallocated();
}
/*****************************************************************************/
void UICurveEditor::onAllocated(IBaseObject* pData)
{
	UIElement::onAllocated(pData);

	myCurrUndoBlockId = -1;
	resetCurve(false);
	EventManager::getInstance()->registerObject(this);
}
/*****************************************************************************/
void UICurveEditor::onDeallocated(void)
{

	EventManager::getInstance()->unregisterObject(this);
}
/*****************************************************************************/
void UICurveEditor::postRender(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale)
{
	SCOPED_MUTEX_LOCK(&myDataLock);
	//SCOPED_LOCK(myDataLock);

	DrawingCache* pDrawingCache = getDrawingCache();
	SRect2D srOwnRect;
	getGlobalRectangle(srOwnRect);

	myLastWindowStart.set(srOwnRect.x, srOwnRect.y);

	ensurePointsValid(srOwnRect);

	//RenderUtils::beginScissorRectangle(myLastGraphArea, pDrawingCache);
	myResampledPointsTemp = myResampledPoints;
	Polyline2D::translate(myResampledPointsTemp, myLastWindowStart.x, myLastWindowStart.y);
	pDrawingCache->addTexturedLine(&myResampledPointsTemp, "selLineSmoothAnim", fOpacity, upToScreen(1.0));
	//RenderUtils::endScissorRectangle(pDrawingCache);
	pDrawingCache->flush();

	// Draw the points
	SVector2D svPoint;
	int iPoint, iNumPoints = myCurrSizeCurve.getNumPoints();
	for(iPoint = 0; iPoint < iNumPoints; iPoint++)
	{
		myCurrSizeCurve.getPoint(iPoint, svPoint);
		svPoint += myLastWindowStart;
		if(myCurrSizeCurve.getPointId(iPoint) == myLastSelectedPointId)
			theCommonString = "tinyElems:8";
		else
			theCommonString = "tinyElems:5";
		pDrawingCache->addSprite(theCommonString.c_str(), svPoint.x, svPoint.y, fOpacity, 0, 1.0, 1.0, true);
	}

	// Draw the axes
	pDrawingCache->addLine(srOwnRect.x + LINE_TICK_SIZE, srOwnRect.y + TOP_PADDING, srOwnRect.x + LINE_TICK_SIZE, srOwnRect.y + srOwnRect.h - BOTTOM_PADDING, g_colAxesColor, AXES_THICKNESS);
	pDrawingCache->addLine(srOwnRect.x, srOwnRect.y + TOP_PADDING, srOwnRect.x + LINE_TICK_SIZE, srOwnRect.y + TOP_PADDING, g_colAxesColor, AXES_THICKNESS);
	pDrawingCache->addLine(srOwnRect.x, srOwnRect.y + srOwnRect.h - BOTTOM_PADDING, srOwnRect.x + LINE_TICK_SIZE, srOwnRect.y + srOwnRect.h - BOTTOM_PADDING, g_colAxesColor, AXES_THICKNESS);

	pDrawingCache->addLine(srOwnRect.x + LEFT_PADDING, srOwnRect.y + srOwnRect.h - LINE_TICK_SIZE, srOwnRect.x + srOwnRect.w - RIGHT_PADDING, srOwnRect.y + srOwnRect.h - LINE_TICK_SIZE, g_colAxesColor, AXES_THICKNESS);
	pDrawingCache->addLine(srOwnRect.x + LEFT_PADDING, srOwnRect.y + srOwnRect.h, srOwnRect.x + LEFT_PADDING, srOwnRect.y + srOwnRect.h - LINE_TICK_SIZE, g_colAxesColor, AXES_THICKNESS);
	pDrawingCache->addLine(srOwnRect.x + srOwnRect.w - RIGHT_PADDING, srOwnRect.y + srOwnRect.h, srOwnRect.x + srOwnRect.w - RIGHT_PADDING, srOwnRect.y + srOwnRect.h - LINE_TICK_SIZE, g_colAxesColor, AXES_THICKNESS);

	pDrawingCache->flush();


	UIElement::postRender(svScroll, fOpacity, fScale);
}
/*****************************************************************************/
void UICurveEditor::receiveEvent(EventType eEventType, IBaseObject* pSender)
{
}
/*****************************************************************************/
void UICurveEditor::ensurePointsValid(const SRect2D& srWindowRect)
{
	if(!myArePointsDirty)
		return;

	SRect2D srActualGraphRect = srWindowRect;
	srActualGraphRect.x = srActualGraphRect.y = 0;
	srActualGraphRect.y += TOP_PADDING;
	srActualGraphRect.h -= TOP_PADDING + BOTTOM_PADDING;
	srActualGraphRect.x += LEFT_PADDING;
	srActualGraphRect.w -= LEFT_PADDING + RIGHT_PADDING;

	myLastGraphArea = srActualGraphRect;

	myCurrSizeCurve.copyFrom(myCurve);
	myCurrSizeCurve.refitToRectangle(srActualGraphRect, true, &myLastGraphScaling, &myLastCurveBBox);
	myCurrSizeCurve.convertToPolyline(&srActualGraphRect, upToScreen(1.0), myResampledPoints);

	myArePointsDirty = false;
}
/*****************************************************************************/
void UICurveEditor::onPressed(TTouchVector& vecTouches)
{
	UIElement::onPressed(vecTouches);

	if(!getIsEnabled() || vecTouches.size() == 0)
		return;

	// Save undo...
	_ASSERT(myCurrUndoBlockId < 0);
	UIElement* pParmElem = getLinkedToElementWithValidTarget();
	if(pParmElem && UndoManager::canAcceptNewUndoNow())
		myCurrUndoBlockId = UndoManager::addUndoItemToCurrentManager(pParmElem->getUndoStringForSelfChange(), pParmElem->createUndoItemForSelfChange(), true, getParentWindow(), NULL);	

	myAllowDeletingDraggedPoint = false;
	myDraggedPointId = getPointAt(vecTouches[0].myPoint, false);
	SRect2D srTestRect;
	srTestRect = myLastGraphArea;
	srTestRect.x += myLastWindowStart.x;
	srTestRect.y += myLastWindowStart.y;
	if(myDraggedPointId < 0 && srTestRect.doesContain(vecTouches[0].myPoint))
	{
		// Start a new one
		SVector2D svGraphPoint;
		convertPointFromScreenToCurveCoords(vecTouches[0].myPoint, svGraphPoint);

		myDraggedPointId = myCurve.insertPoint(svGraphPoint, true);
		myArePointsDirty = true;
	}
	
	if(myDraggedPointId >= 0)
	{
		int iPointIndex = myCurve.getIndexById(myDraggedPointId);
		myAllowDeletingDraggedPoint = (iPointIndex > 0 && iPointIndex < (myCurve.getNumPoints() - 1));
		myIsDraggedPointFirst = (iPointIndex == 0);
	}

	myLastSelectedPointId = myDraggedPointId;
	updateUiForPointId(myLastSelectedPointId);
	if(myDraggedPointId >= 0)
		handleTargetElementUpdate(false);
}
/*****************************************************************************/
void UICurveEditor::onReleased(TTouchVector& vecTouches, bool bIgnoreActions)
{
	if(myDraggedPointId >= 0)
		handleTargetElementUpdate(false);
	myDraggedPointId = -1;
	UIElement::onReleased(vecTouches, bIgnoreActions);

	finishUndo();
}
/*****************************************************************************/
void UICurveEditor::onMouseMove(TTouchVector& vecTouches)
{
	UIElement::onMouseMove(vecTouches);

	if(myDraggedPointId >= 0)
	{
		//SCOPED_LOCK(myDataLock);
		SCOPED_MUTEX_LOCK(&myDataLock);

		// Move the point back into the unscaled graph coords
		SVector2D svGraphPoint;
		convertPointFromScreenToCurveCoords(vecTouches[0].myPoint, svGraphPoint);

		if(!myAllowDeletingDraggedPoint)
		{
			if(myIsDraggedPointFirst)
				svGraphPoint.x = 0;
			else
				svGraphPoint.x = 1;
		}

		myCurve.movePointById(myDraggedPointId, svGraphPoint, true);
		myArePointsDirty = true;

		_ASSERT(myLastSelectedPointId == myDraggedPointId);
		updateUiForPointId(myLastSelectedPointId);
		handleTargetElementUpdate(true);
	}
}
/*****************************************************************************/
int UICurveEditor::getPointAt(const SVector2D& svScreenPoint, bool bIgnoreFirstLast)
{
	int iPoint, iNumPoints = myCurrSizeCurve.getNumPoints();
	int iStartPoint = bIgnoreFirstLast ? 1 : 0;
	int iEndPoint = bIgnoreFirstLast ? (iNumPoints - 1) : iNumPoints;
	SVector2D svPoint;
	SRect2D srTempRect;
	SVector2D svSize(POINT_CLICK_RADIUS*2.0, POINT_CLICK_RADIUS*2.0);
	for(iPoint = iStartPoint; iPoint < iEndPoint; iPoint++)
	{
		myCurrSizeCurve.getPoint(iPoint, svPoint);
		svPoint += myLastWindowStart;
		srTempRect.setFromCenterAndSize(svPoint, svSize);
		if(srTempRect.doesContain(svScreenPoint))
			return myCurrSizeCurve.getPointId(iPoint);
	}

	return -1;
}
/*****************************************************************************/
void UICurveEditor::convertPointFromScreenToCurveCoords(const SVector2D& svPointIn, SVector2D& svPointOut)
{
	SVector2D svGraphPoint = svPointIn - (myLastGraphArea.origin() + myLastWindowStart);
	svGraphPoint /= myLastGraphScaling;

	// And invert:
	svGraphPoint.y = myLastCurveBBox.myMax.y - (svGraphPoint.y - myLastCurveBBox.myMin.y);

	// Clamp:
	svGraphPoint.y = HyperCore::clampToRange<FLOAT_TYPE>(0, 1, svGraphPoint.y);

	svPointOut = svGraphPoint;
}
/*****************************************************************************/
void UICurveEditor::onMouseEnter(TTouchVector& vecTouches)
{
	UIElement::onMouseEnter(vecTouches);

	// If we were dragging a point, insert it back:
	if(myDraggedPointId >= 0 && myAllowDeletingDraggedPoint)
	{
		SVector2D svGraphPoint;
		convertPointFromScreenToCurveCoords(vecTouches[0].myPoint, svGraphPoint);
		myCurve.insertPoint(svGraphPoint, true, myDraggedPointId);
		myArePointsDirty = true;

		myLastSelectedPointId = myDraggedPointId;
		updateUiForPointId(myLastSelectedPointId);
		handleTargetElementUpdate(true);
	}
}
/*****************************************************************************/
void UICurveEditor::onMouseLeave(TTouchVector& vecTouches)
{
	UIElement::onMouseLeave(vecTouches);

	// If we're dragging a control point, remove it:
	if(myDraggedPointId >= 0 && myAllowDeletingDraggedPoint)
	{
		myCurve.removePointById(myDraggedPointId);
		myArePointsDirty = true;
		myLastSelectedPointId = -1;
		updateUiForPointId(-1);
		handleTargetElementUpdate(true);
	}
}
/*****************************************************************************/
void UICurveEditor::updateUiForPointId(int iId)
{
	// Update our related UI elems, if any:
	UIElement *pParent = getParent<UIElement>();
	if(!pParent)
		return;

	// Get our input/output boxes
	UIElement *pElem;
	
	// Set the enable/disable status
	pElem = pParent->getChildById("defInputValue");
	if(pElem)
		pElem->setIsEnabled(iId >= 0 && myAllowDeletingDraggedPoint);
	pElem = pParent->getChildById("defOutputValue");
	if(pElem)
		pElem->setIsEnabled(iId >= 0);

	if(iId < 0)
		return;

	// Get the actual point:
	//SCOPED_LOCK(myDataLock);
	SCOPED_MUTEX_LOCK(&myDataLock);
	SVector2D svPoint;
	myCurve.getPointById(iId, svPoint); 

	// Now, set the values, if any:
	pParent->getChildAndSubchild("defInputValue", "defNumBox")->setNumericValue((int)(svPoint.x*CHART_SCALE), DEFAULT_STORAGE_UNITS);
	pParent->getChildAndSubchild("defOutputValue", "defNumBox")->setNumericValue((int)(svPoint.y*CHART_SCALE), DEFAULT_STORAGE_UNITS);
}
/*****************************************************************************/
void UICurveEditor::updateOwnData(SUpdateInfo& rRefreshInfo)
{
	// See if we have a target element, and if so, set our value from it.
	// The reason we have a second clause is because while the parm is dragged,
	// we get this call; however, we never move, since we always refresh
	// at the initial position. This is a hack, admittedly, but a needed one
	// in order not to reset the scroll bar when doing undo on these controls.
	if(getHaveValidTargetElement() && (rRefreshInfo.mySourceEventType == EventUndoPerformed || rRefreshInfo.mySourceEventType == EventRedoPerformed))
		getTargetIdentifier()->refreshUIFromStoredValue(this);

	updateUiForPointId(myLastSelectedPointId);
}
/*****************************************************************************/
void UICurveEditor::postInit()
{
	UIElement::postInit();
	setChildTargetIdentifiers();
}
/*****************************************************************************/
void UICurveEditor::setChildTargetIdentifiers()
{
	UIElement *pParent = getParent<UIElement>();
	if(!pParent)
		return;

 	pParent->getChildAndSubchild("defInputValue", "defNumBox")->setTargetUiElem(this);
 	pParent->getChildAndSubchild("defOutputValue", "defNumBox")->setTargetUiElem(this);
}
/*****************************************************************************/
void UICurveEditor::onRefreshStoredValueFromUICallback(UIElement* pSourceElem, bool bIsChangingContinuously)
{
	if(myIsRefreshingUI || !pSourceElem || myLastSelectedPointId < 0)
		return;

	myIsRefreshingUI = true;

	SVector2D svStartPoint;
	myCurve.getPointById(myLastSelectedPointId, svStartPoint);
	UITextFieldElement* pCastElem = as<UITextFieldElement>(pSourceElem);
	if(pSourceElem->getParentById("defInputValue", true) && pCastElem && myAllowDeletingDraggedPoint)
	{
		svStartPoint.x = pCastElem->getNumericValue(DEFAULT_STORAGE_UNITS)/CHART_SCALE;
		myCurve.movePointById(myLastSelectedPointId, svStartPoint, true);
		myArePointsDirty = true;
		handleTargetElementUpdate(bIsChangingContinuously);
	}
	else if(pSourceElem->getParentById("defOutputValue", true) && pCastElem)
	{
		// This is the y coord
		svStartPoint.y = pCastElem->getNumericValue(DEFAULT_STORAGE_UNITS)/CHART_SCALE;
		myCurve.movePointById(myLastSelectedPointId, svStartPoint, true);
		myArePointsDirty = true;
		handleTargetElementUpdate(bIsChangingContinuously);
	}

	myIsRefreshingUI = false;
}
/*****************************************************************************/
void UICurveEditor::resetCurve(bool bTriggerParmChange)
{
	myArePointsDirty = true;
	myDraggedPointId = -1;
	myLastSelectedPointId = -1;
	myIsRefreshingUI = false;
	myAllowDeletingDraggedPoint = true;
	myIsDraggedPointFirst = true;

	myCurve.clear();
	myCurve.insertPoint(SVector2D(0, 0));
	myCurve.insertPoint(SVector2D(1.0, 1.0));
	myCurve.setMinMaxY(0, 1);

	if(bTriggerParmChange)
	{
		updateUiForPointId(-1);
		handleTargetElementUpdate(false);
	}
}
/*****************************************************************************/
void UICurveEditor::finishUndo()
{
	if(myCurrUndoBlockId >= 0)
	{
		UndoManager::endUndoBlockInCurrentManager(myCurrUndoBlockId, getParentWindow(), NULL);
		myCurrUndoBlockId = -1;
	}
}
/*****************************************************************************/
};