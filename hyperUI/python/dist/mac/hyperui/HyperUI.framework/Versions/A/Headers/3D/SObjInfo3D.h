#pragma once

/*****************************************************************************/
struct HYPERUI_API SObjInfo3D
{
	SObjInfo3D() 
	{
		myMemId = -1;
		myVertexData = NULL;
		reset();
	}

	void setMemoryBlockId(long rMemId) { myMemId = rMemId;  }
	long getMemoryBlockId(void) {  return myMemId; }

	void onAllocated(IBaseObject* pData)
	{
		reset();
	}

	~SObjInfo3D()
	{
		reset();
	}

	void reset();
	void onTimerTick(void);
	void ensureNumVertsDestructive(int iNumVerts);
	void computeCentroid(SVector3D& svOut);
	FLOAT_TYPE computeTotalArea();
	void translateBy(SVector3D& svAmount);

	void setVertexDataPointer(SVertex3D* pData) { _ASSERT(!myVertexData); myVertexData = pData; myDoOwnVertexData = false; }
	void copyAndRetainFrom(SVertex3D* pData, int iNumVerts);
	inline SVertex3D* getVertexData() { return myVertexData; }

	SObjInfo3D& operator=(const SObjInfo3D& other)
	{
		reset();

		// Never copy the block id
		myCenter = other.myCenter;
		myDir = other.myDir;
		mySpeed = other.mySpeed;

		myNumVerts = other.myNumVerts;
		myTexture = other.myTexture;
		myDiffuseMults = other.myDiffuseMults;
		myAmbMults = other.myAmbMults;

		// Note that these are only for DrawingCacher use.
		myAlpha = other.myAlpha;
		myScale = other.myScale;
		myRotations = other.myRotations;
		myRotationDirs = other.myRotations;

		// Do not copy!
		//long myMemId;

		// Just copy the pointer
		myVertexData = other.myVertexData;
		myDoOwnVertexData = false;

		return *this;
	}

	SVector3D myCenter;
	SVector3D myDir;
	FLOAT_TYPE mySpeed;

	int myNumVerts;
	CachedSequence* myTexture;
	SColor myDiffuseMults, myAmbMults;

	// Note that these are only for DrawingCacher use.
	FLOAT_TYPE myAlpha, myScale;
	SVector3D myRotations;

	// Rotation directions and speeds
	SVector3D myRotationDirs;


	long myMemId;

private:
	SVertex3D *myVertexData;
	bool myDoOwnVertexData;
};
/*****************************************************************************/