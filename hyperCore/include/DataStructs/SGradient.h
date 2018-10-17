#pragma once

#define ALLOW_GRAD_CACHING

/*****************************************************************************/
class HYPERCORE_API SGradient
{
public:

	SGradient();
	SGradient(const SGradient& rOther);
	~SGradient();

	void setColorAndOpacityStop(FLOAT_TYPE fPos, const SColor& scol);
	void setColorAndOpacityStop(FLOAT_TYPE fPos, FLOAT_TYPE fR, FLOAT_TYPE fG, FLOAT_TYPE fB, FLOAT_TYPE fAlpha);

	void changeStopColorTo(UNIQUEID_TYPE lId, const SColor& scolTemp);
	void changeSecondaryStopColorTo(UNIQUEID_TYPE lId, const SColor& scolTemp);
	void changeStopOpacityTo(UNIQUEID_TYPE lId, FLOAT_TYPE fOpacity);
	UNIQUEID_TYPE setColorStop(FLOAT_TYPE fPos, FLOAT_TYPE fR, FLOAT_TYPE fG, FLOAT_TYPE fB);
	UNIQUEID_TYPE setOpacityStop(FLOAT_TYPE fPos, FLOAT_TYPE fAlpha);

	void setIsStopSplit(UNIQUEID_TYPE lId, bool bIsSplit);
	void swapKeyColors(UNIQUEID_TYPE lId);

	void getColorAndOpacityFor(FLOAT_TYPE fPos, SColor& scolOut) const;
	void getSingleStopsArray(TGradientStops& rStopsOut) const;

	void moveStopTo(UNIQUEID_TYPE lId, FLOAT_TYPE fNewPos);

	const SGradientStop* findStopById(UNIQUEID_TYPE lId) const;

	void removeStop(UNIQUEID_TYPE lId, SGradientStop* pOptStopCopyOut = NULL);
	// If the last is true, mirror opacity, otherwise, mirror color
	void mirror(SideType eSide, bool bDoOpacity);
	void clear(void);

	void copyFrom(const SGradient& rOther);
	void operator=(const SGradient& rOther) { copyFrom(rOther); }

	bool loadFromString(const char* pcsString);
	void saveToString(string& strOut) const;

	class StopsIterator
	{
	public:
		StopsIterator(const SGradient& rParentGrad, bool bDoColor)
		{
			myParent = &rParentGrad;
			myDoColor = bDoColor;
			if(bDoColor)
				myIterator = rParentGrad.myColorStops.begin();
			else
				myIterator = rParentGrad.myOpacityStops.begin();
		}

		StopsIterator() { myParent = NULL; }

		void operator++(int) { myIterator++; }
		void operator--(int) { myIterator--; }
		void toEnd() { if(myDoColor) myIterator = myParent->myColorStops.end(); else myIterator = myParent->myOpacityStops.end(); }
		bool isEnd() const { if(myDoColor) return !myParent || myIterator == myParent->myColorStops.end(); else return !myParent || myIterator == myParent->myOpacityStops.end(); }
		bool operator==(const StopsIterator& other) const { return myIterator == other.myIterator && myParent == other.myParent && myDoColor == other.myDoColor; }
		bool operator!=(const StopsIterator& other) const { return ! (*this == other); }

		inline const SGradientStop* getStop() const { return &(*myIterator); }

	private:
		bool myDoColor;
		const SGradient* myParent;
		TGradientStops::iterator myIterator;
	};

	StopsIterator colorStopsBegin() const;
	StopsIterator opacityStopsBegin() const;

private:

	inline UNIQUEID_TYPE getNextId() { myLastId++; return myLastId; }

	void interpFor(FLOAT_TYPE fPos, bool bIsDoingColor, SColor& scolOut) const;

	SGradientStop* internalFindStopById(UNIQUEID_TYPE lId);

	void invalidateColorStopsCache();
	void invalidateOpacityStopsCache();

	void ensureColorStopsCacheValid() const;
	void ensureOpacityStopsCacheValid() const;

	void convertSetToArray(const TGradientStops& setStops, SGradientStop* pArrayOut) const; 

private:
	
	UNIQUEID_TYPE myLastId;

	TGradientStops myColorStops;
	TGradientStops myOpacityStops;

#ifdef ALLOW_GRAD_CACHING
	mutable SGradientStop *myCachedColorStopsArray;
	mutable int myCachedColorArrayLen;
	mutable SGradientStop *myCachedOpacityStopsArray;
	mutable int myCachedOpacityArrayLen;
	mutable Mutex myCacheLock;
#endif


};
/*****************************************************************************/
