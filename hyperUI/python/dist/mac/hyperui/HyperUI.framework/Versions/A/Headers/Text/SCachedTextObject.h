#pragma once

/***********************************************************************************************************/
struct SCachedTextObject
{
	STextParams myBaseInfo;

	SCachedTextObject()
	{
		onAllocated();
	}
	
	void onAllocated(IBaseObject* pData = NULL)
	{
		myBlendMode = BlendModeNormal;
		myHasShadow = false;
		myHorAlign = HorAlignCenter;
		myVertAlign = VertAlignCenter;
		myScale = 1.0;
		myDidRenderDuringStencil = false;
		myForceDrawingIfUncached = false;
		myBaseInfo.reset();
	}

	void onDeallocated(void) { }

	void setMemoryBlockId(long rMemId)
	{
		myMemId = rMemId;
	}

	long getMemoryBlockId(void)
	{
		return myMemId;
	}


	string myText;

	bool myDidRenderDuringStencil;
	bool myForceDrawingIfUncached;
	SVector2D myCenter;
	SColor myColor;
	SColor myShadowColor;
	SVector2D myShadowOffset;
	bool myHasShadow;
	BlendModeType myBlendMode;

	HorAlignType myHorAlign;
	VertAlignType myVertAlign;
	FLOAT_TYPE myScale;

	long myMemId;
};
/***********************************************************************************************************/
typedef MemoryCacher < SCachedTextObject > TCachedTextObjectCacher;
typedef vector < SCachedTextObject* > TCachedTextObjects;
/***********************************************************************************************************/