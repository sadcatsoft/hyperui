#pragma once

class CachedSequence;
/***********************************************************************************************************/
struct HYPERUI_API SPathVertexInfo
{
	float x,y;
#ifdef USE_OPENGL2
	float z, w;
#endif

	// Texture coords
#ifdef USE_OPENGL2
	float r, g, b, a;
	// No padding needed
#else
	unsigned char r,g,b,a;
	float padding[4];
#endif

#ifdef USE_OPENGL2
	inline static int getNumVertexComponents() { return 4; }
#else
	inline static int getNumVertexComponents() { return 2; }
#endif
};
/********************************************************************************************/
struct HYPERUI_API SGenData
{
    SGenData()
    {
		/*
		myVertexArray = NULL;
		myColorArray = NULL;
		myTexCoords = NULL;

		myVertexArraySize = myColorArraySize = 0;
		 */

		myVerts = NULL;
		myVertexArraySize = 0;

		myNumLines = 0;
    }

    ~SGenData()
    {
		/*
		if(myVertexArray)
			delete[] myVertexArray;
		if(myColorArray)
			delete[] myColorArray;
		if(myTexCoords)
			delete[] myTexCoords;

		myVertexArraySize = myColorArraySize = 0;
		 */
		myNumLines = 0;
		if(myVerts)
			delete[] myVerts;
		myVerts = NULL;
		myVertexArraySize = 0;
    }

	void ensureCapacity(int iVertexArraySize, int iColorArraySize)
	{
		if(myVertexArraySize < iVertexArraySize)
		{
			/*
			if(myVertexArray)
				delete[] myVertexArray;
			if(myTexCoords)
				delete[] myTexCoords;

			myVertexArray = new GLfloat[iVertexArraySize];
			myTexCoords = new GLfloat[iVertexArraySize];
			myVertexArraySize = iVertexArraySize;
			*/

			if(myVerts)
				delete[] myVerts;
			myVerts = new SPathVertexInfo[iVertexArraySize];
			myVertexArraySize = iVertexArraySize;
		}
/*
		if(myColorArraySize < iColorArraySize)
		{
			if(myColorArray)
				delete[] myColorArray;
			myColorArray = new GLfloat[iColorArraySize];
			myColorArraySize = iColorArraySize;
		}
 */
	}
	/*
    int myNumTris;
    GLfloat *myVertexArray;
    GLfloat *myColorArray;
	GLfloat *myTexCoords;
	*/

	int myNumLines;
	SPathVertexInfo* myVerts;

private:
	int myVertexArraySize;
	// int myVertexArraySize, myColorArraySize;
};
/********************************************************************************************/
struct HYPERUI_API STrailPoint
{
	STrailPoint() { myMarker = 0; }
    SVector2D myPoint;
    GTIME myTime;
	int myMarker;
};
// typedef vector < STrailPoint > TTrailPoints;
// typedef list < STrailPoint > TTrailPoints;
typedef RingBuffer < STrailPoint > TTrailPoints;
/********************************************************************************************/
class HYPERUI_API PathTrail
{
public:
    PathTrail();
    ~PathTrail();

    void clear(void);
    void addPoint(FLOAT_TYPE iX, FLOAT_TYPE iY, GTIME lTime, bool bForceAddition = false, int iMarkerValue = 0);

	void reverse();
	void trimFromEnd(FLOAT_TYPE fDistance);
	void trimFromEndUseMarkers(FLOAT_TYPE fDistance);

    int getNumPoints(void);
    void getPoint(int iIndex, SVector2D& svOut);
	GTIME getPointTime(int iIndex);
	void setPointMaker(int iIndex, int iMarker);

	void getAsPointVector(TPointVector& rPointsOut, bool bReverse = false);
	void getAsLines(TLineVector& rVecOut, bool bReverse = false);
	void reassembleAsSinglePath(TPointVector& rPointsOut, TPointVector& rBreakPointOffsetsOut, TPointVector& rBreakPointsOut, TPointVector& rBreakInDirsOut, bool bReverse = false);

	// Extra rotation added to the sprites
	FLOAT_TYPE getAdditionalRotation(void);
	void setAdditionalRotation(FLOAT_TYPE fDegRot);

//    void generateTriangles(SGenData& rDataOut);
	void generateLines(SVector2D svScroll, SGenData& rDataOut, FLOAT_TYPE dOpacityMult, FLOAT_TYPE fScale);
    bool getIsDeleted(void);

	static void simplifyPath(TPointVector& rPath, FLOAT_TYPE fAngleTolerance, FLOAT_TYPE fDistanceTolerance);

	void setFadeSpeed(FLOAT_TYPE fSpeed);

	// NOTE that setting a const opacity will prevent this from being deleted!
	void setConstOpacity(FLOAT_TYPE fValue);
	void setExtendMultiplier(FLOAT_TYPE dMult);
	void setThickness(FLOAT_TYPE dValue);
	void setSampleFreq(int iFreq);
	void setSampleAnim(DrawingCache* pDrawingCache, const char* pcsAnim);
	void setTrailFreqMult(int iMult);
	void setOffset(int iX, int iY);
	void getOffset(SVector2D& svOffsetOut);

	void setMinDistance(FLOAT_TYPE fValue) { myMinPointDistance = fValue; }
	void setMaxDistance(FLOAT_TYPE fValue) { myMaxPointDistance = fValue; }
	void setAngleTolerance(FLOAT_TYPE fValue) { myAngleTolerance = fValue; }

	int getSampleFreq(void);

	void setColor(FLOAT_TYPE fR, FLOAT_TYPE fG, FLOAT_TYPE fB, FLOAT_TYPE fA);
	void getColor(SColor& colOut);

	void setMaxPoints(int iNum);

	FLOAT_TYPE getThickness(void);

	void setIsEnabled(bool bValue) { myIsEnabled = bValue; }
	bool getIsEnabled(void) { return myIsEnabled; }

	CachedSequence* getSampleAnim(void) { return mySampleAnim; }
	int getSampleAnimAtlasFrame(void) { return mySampleAnimAtlasFrame; }

	void setClockType(ClockType eType) { myClockType = eType; }

	void saveToItem(ResourceItem& rItemOut, const char* pcsTagId);
	void loadFromItem(ResourceItem* pItem, DrawingCache* pDrawingCache);

private:
	void clearTransparentPoints(void);

private:

	ClockType myClockType;

	int mySampleFreq;

    TTrailPoints myPoints;

    // In opacity units (0-1) per second
    FLOAT_TYPE myFadeSpeed;

	FLOAT_TYPE myConstOpacity;
    bool myIsDeleted;

	FLOAT_TYPE myThickness;
	FLOAT_TYPE myExtendMultiplier;

	int mySampleAnimAtlasFrame;
	CachedSequence* mySampleAnim;

	SColor myColor;
	SVector2D myOffset;

	int myTrailFreqMult;
	int myMaxPoints;

	FLOAT_TYPE myMinPointDistance, myMaxPointDistance, myAngleTolerance;

	bool myIsEnabled;

	FLOAT_TYPE myAdditionalRotation;

	//static TPointVector thePointVector;
};
/********************************************************************************************/
