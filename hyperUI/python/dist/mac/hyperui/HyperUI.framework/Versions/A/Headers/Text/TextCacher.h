#pragma once

class Window;
/*****************************************************************************/
class HYPERUI_API TextCacher : public IOffscreenCacher
{
public:
	TextCacher(Window* pWindow);
	virtual ~TextCacher();

	TX_MAN_RETURN_TYPE getTextureFor(string& strText, STextParams& rParms, SVector2D& svTextDimsOut, SVector2D& svBitmapDimsOut, bool bCreate);

	void checkForCleaning();

	void queueForCreation(const char* pcsText, STextParams& rParms, bool bForceCaching);
	virtual void createQueued();


	STextParams* addTextureFor(const char* pcsText, STextParams& rParms);
	void cleanAll();

	inline bool getHaveQueuedTexts() const { return myToBeCreated.size() > 0; }

private:
	void cleanUnused();

private:
	TTextParmsMap myEntries;

	TTextParmsVector myToBeCreated;
	TStringVector myToBeCreatedStrings;

	TTextParmsCacher myCacher;


	long myMemoryUsed;
	TTextParmsMemoryCacher myTextParmsMemoryCacher;
};
/*****************************************************************************/