#pragma once

class UITabWindowElement;

#define TAB_TOP_OFFSET_X		upToScreen(0.5)
#define TAB_TOP_OFFSET_Y		upToScreen(0)

#define SEL_TAB_DRAWING_OFFSET_X	upToScreen(0.0)
#define SEL_TAB_DRAWING_OFFSET_Y	upToScreen(0.0)

#define NOCLOSE_BUTTON_RIGHT_PADDING	upToScreen(2.0)

#define TAB_ICON_PADDING_X_LEFT		upToScreen(-1.0)
#define TAB_ICON_PADDING_X_RIGHT	upToScreen(1.0)

struct STabInfo;
/*****************************************************************************/
struct HYPERUI_API STabRenderCommonInfo
{
	FLOAT_TYPE myFinalOpacity;
	FLOAT_TYPE myTabHeight;
	FLOAT_TYPE mySelTabHeight;
	int mySelCornerW, mySelCornerH;
	SColor mySelTextColor;
	SColor myTextColor;
	const char* pcsFont;
	int iFontSize;
	SVector2D myPos;
	int myCornerW, myCornerH;
	SColor *myShadowColor;
	SColor myActualShadowColor;
	int myCloseW, myCloseH;
	SVector2D myShadowOffset;
	SVector2D mySize;
	FLOAT_TYPE myScale;
};
/*****************************************************************************/
class HYPERUI_API UITabRenderer : public ICustomDragRenderer
{
public:
	virtual ~UITabRenderer() { }
	virtual void render(UIElement* pElem, SVector2D svPos);

	static UITabRenderer* getInstance() 
	{
		if(!theInstance)
			theInstance = new UITabRenderer;
		return theInstance;
	}

	static void renderTab(UITabWindowElement* pParent, STabInfo* pInfo, int iTab, STabRenderCommonInfo& rCommonInfo,  int& iSelTabIdx, SRect2D& srSelTabRect, const SVector2D& svExtraOffset);

private: 
	UITabRenderer() { }

	static UITabRenderer* theInstance;
};
/*****************************************************************************/

