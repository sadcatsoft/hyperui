#pragma once

/*****************************************************************************/
struct STextParams
{
	STextParams()
	{
#ifdef DIRECTX_PIPELINE
		myActualTexturePtr = NULL;
#endif

		reset();
	}

	void reset(void)
	{
		myFont = EMPTY_STRING;
		myFontSize = 0;
		myWidth = 0;
		myTextureIndex = 0;
#ifdef DIRECTX_PIPELINE
		if(myActualTexturePtr)
			myActualTexturePtr->Release();
		myActualTexturePtr = NULL;
#endif
		myLastUsedTime = 0;
		myExtraKerning = myLineSpacing = 0.0;
		myResampleNearest = true;
	}
	
	void onAllocated(IBaseObject* pData)
	{
		reset();
	}

	void onDeallocated()
	{
#ifdef DIRECTX_PIPELINE
		if(myActualTexturePtr)
			myActualTexturePtr->Release();
		myActualTexturePtr = NULL;
		//myActualTexturePtr = nullptr; 
#endif
	}
	
	void setMemoryBlockId(long rMemId)
	{
		myMemId = rMemId;
	}
	
	long getMemoryBlockId(void)
	{
		return myMemId;
	}
	

	bool isEqualTo(STextParams& rOther)
	{
		// TODO: Finish properly.
	//	if(myText != rOther.myText)
	//		return false;

		if(myWidth != rOther.myWidth)
			return false;

		if(myFontSize != rOther.myFontSize)
			return false;

/*
		if(myFont && !rOther.myFont)
			return false;

		if(!myFont && rOther.myFont)
			return false;

		if(myFont && strcmp(myFont, rOther.myFont) != 0)
//		if(myFont != rOther.myFont)
			return false;*/

		if(myFont != rOther.myFont)
			return false;

		if(myExtraKerning != rOther.myExtraKerning)
			return false;

		if(myLineSpacing != rOther.myLineSpacing)
			return false;

		if(myResampleNearest != rOther.myResampleNearest)
			return false;

		return true;
	}

	int getBytesUsed(void)
	{
		return myBitmapsDims.x*myBitmapsDims.y*4;
	}

	bool myResampleNearest;
	STRING_TYPE myFont;
	int myWidth;
	int myFontSize;
	FLOAT_TYPE myExtraKerning, myLineSpacing;

	GTIME myLastUsedTime;
	TX_MAN_RETURN_TYPE myTextureIndex;

#ifdef DIRECTX_PIPELINE
	ID3D11ShaderResourceView* myActualTexturePtr;
#endif


	// This currently isn't factored into equality 
	// (on purpose).
	SColor myHighlightColor;

	// The dimensions of the text object.
	SVector2D myTextDims;
	SVector2D myBitmapsDims;
	
	long myMemId;
};
/*****************************************************************************/
typedef multimap < string, STextParams* > TTextParmsMap;
typedef vector < STextParams > TTextParmsVector;
typedef StlStringMapCacher < STextParams > TTextParmsCacher;
typedef MemoryCacher < STextParams > TTextParmsMemoryCacher;
/*****************************************************************************/