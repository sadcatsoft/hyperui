#pragma once

/********************************************************************************************/
class HYPERUI_API BezierAnimController : public IAnimController
{
public:
	BezierAnimController();
	virtual ~BezierAnimController();	

	virtual SVector2D evaluate(GTIME currTime);
	virtual IAnimController* clone(void);
	virtual GTIME computeEndTime(GTIME startTime, FLOAT_TYPE dSpeed);

	void addBezier(SVector2D& svStart, SVector2D& svControl1, SVector2D& svControl2, SVector2D& svEnd, bool bIsLinear = false);
	void addLinear(SVector2D& svStart, SVector2D& svEnd);
	//virtual void finalize(void);

	// NOTE: The length is cached, and is only computed on the first call
	FLOAT_TYPE getTotalLength(void);
	GTIME getTimeLength();
	GTIME getLastCurveEndTime();

	GTIME setFromItem(const char* pcsItemType, GTIME lStartTime);
	// Scales individual curve times (proportionally to their original values)
	// so that the overall animation length matches the given time in seconds.
	void scaleTimeToMatchDuration(FLOAT_TYPE fOverallTime);
	void offsetTimeBy(GTIME lTime);

	void clear();

	void saveToItem(ResourceItem& rItemOut, const CHAR_TYPE* pcsTagId);
	void loadFromItem(ResourceItem* pItem);

	virtual bool getIsAnimating(GTIME lTime);

	void copyCurvesFrom(const SBezierCurve* pOther);
	void extractCurvesTo(SBezierCurve& rCurve) const;

protected:
	virtual SVector2D getTangent(GTIME currTime);

private:
	void getSegmentIndexAndPos(GTIME timeIn, int& iSegmentIndexOut, FLOAT_TYPE& dInterpOut);

	void recomputeSegmentTimes();

private:
	FLOAT_TYPE myTotalLength;
	SAnimBezierCurve myAnimBezierCurve;

	STRING_TYPE myCurveName;

	TFloatVector myFloatVector;
};
/********************************************************************************************/
