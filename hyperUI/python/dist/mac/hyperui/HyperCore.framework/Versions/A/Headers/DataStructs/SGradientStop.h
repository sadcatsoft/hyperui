#pragma once

#define SGRAD_MULT_FACTOR		10000.0

struct GradStopCompare;
/*****************************************************************************/
struct HYPERCORE_API SGradientStop
{
public:
	SGradientStop() { myLeftInterpCenter = myRightInterpCenter = 0.5; myIsSplit = false; }

	inline void getColor(SColor& scolOut) const { scolOut = myColor; }
	inline FLOAT_TYPE getOpacity() const { return myColor.alpha; }

	inline void setPos(FLOAT_TYPE fPos) { myPos = fPos; myMultPos = myPos*SGRAD_MULT_FACTOR; }
	inline FLOAT_TYPE getPos() const { return myPos; }

	void saveToItem(StringResourceItem& rItemOut) const;
	void loadFromItem(StringResourceItem& rItemIn);

	// Not used now, for compat only.
	FLOAT_TYPE myLeftInterpCenter, myRightInterpCenter;
	// Also unused, but for possible sharp-transition grads.
	// Conceptually, it is always on the right.
	SColor mySecondaryColor;
	bool myIsSplit;

	SColor myColor;
	bool myIsOpacityStop;
	UNIQUEID_TYPE myId;

	// We need these for template export
	bool operator < (const SGradientStop& c) const
	{
		return myMultPos < c.myMultPos;
	}
	bool operator == (const SGradientStop& c) const
	{
		return myMultPos == c.myMultPos;
	}

private:

	friend struct GradStopCompare;

	// Position *100 for lookups
	int myMultPos;
	FLOAT_TYPE myPos;
};
/*****************************************************************************/
struct HYPERCORE_API GradStopCompare
{
	bool operator()(const SGradientStop& s1, const SGradientStop& s2) const
	{
		return s1.myMultPos < s2.myMultPos;
	}
};
#ifdef WIN32
//HYPERCORE_EXTERN template class HYPERCORE_API std::set < HyperCore::SGradientStop, HyperCore::GradStopCompare >;
#endif
typedef std::set < SGradientStop, GradStopCompare > TGradientStops;
/*****************************************************************************/
