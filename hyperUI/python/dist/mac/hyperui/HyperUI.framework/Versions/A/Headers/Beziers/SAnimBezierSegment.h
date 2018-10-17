#pragma once

/********************************************************************************************/
class HYPERUI_API SAnimBezierSegment : public SBezierSegment
{
public:	
	SAnimBezierSegment() { }
	virtual ~SAnimBezierSegment() { } 

	GTIME getStartTime(void) const { return myStartTime; }
	GTIME getEndTime(void) const { return myEndTime; }

	void setStartTime(GTIME rTime) { myStartTime = rTime; }
	void setEndTime(GTIME rTime) { myEndTime = rTime; }

	virtual SBezierSegment* cloneSelf(FLOAT_TYPE fPerfScalingFactor = 1.0) const
	{
		SAnimBezierSegment* pNew = new SAnimBezierSegment;
		pNew->copyFrom(*this, fPerfScalingFactor);
		pNew->myParentAnim = NULL;
		return pNew;
	}

	inline void setParent(TBezierCurve<SAnimBezierSegment>* pParent) { myParentAnim = pParent; }
	inline TBezierCurve<SAnimBezierSegment>* getParent() const { return myParentAnim; }

	virtual void copyFrom(const SAnimBezierSegment& rOther, FLOAT_TYPE fPerfScalingFactor = 1.0)
	{
		SBezierSegment::copyFrom(rOther, fPerfScalingFactor);
		myStartTime = rOther.myStartTime;
		myEndTime = rOther.myEndTime;

		// Don't touch
		// myParentAnim
	}


	virtual void saveToItem(StringResourceItem& rItemOut) const { _ASSERT(0); }
	virtual void loadFromItem(const StringResourceItem& rItem) { _ASSERT(0); }

	SAnimBezierSegment& operator=(const SAnimBezierSegment& rOther) { this->copyFrom(rOther); return *this; }

private:
	GTIME myStartTime;
	GTIME myEndTime;

	TBezierCurve<SAnimBezierSegment>* myParentAnim;
};

typedef vector < SAnimBezierSegment > TAnimBezierVector;
/********************************************************************************************/
