#pragma once

/*****************************************************************************/
class HYPERUI_API UIProgressElement : public UIElement
{
public:

	DECLARE_STANDARD_UIELEMENT(UIProgressElement, UiElemProgress);

	virtual void render(const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale);

	void setMinProgress(FLOAT_TYPE fValue);
	void setMaxProgress(FLOAT_TYPE fValue);
	void setProgress(FLOAT_TYPE fValue);
	inline FLOAT_TYPE getProgress() const { return myCurrProgress; }

	//void setProgressDataSource(DataSource& rSource);
	void setIsInverted(bool bValue);

	virtual void postInit(void);

protected:

	FLOAT_TYPE myMinProgress, myMaxProgress, myCurrProgress;
	//DataSource myProgressDataSource;
	bool myIsInverted;

#ifndef MAC_BUILD
	static 
#endif
	string theSharedString, theSharedString2;

	ProgressBarStyleType myDirection;
};
/*****************************************************************************/