#pragma once

class UISplitterElement;
class UITabWindowElement;
/*****************************************************************************/
struct SLayoutInfo
{
	string myLabel;
	string myFileName;
	string myInternalName;
	bool myIsBuiltIn;
};
typedef map < string, SLayoutInfo > TStringLayoutMap;
/*****************************************************************************/
class HYPERUI_API UILayoutManager
{
public:
	static UILayoutManager* getInstance();
	~UILayoutManager();

	void saveLastUsedLayout();
	void saveLayout(const char* pcsLayoutName, const char* pcsFileName, bool bAddToLayouts);
	void loadLayout(const char* pcsOptLayoutName) const;

	inline const TStringLayoutMap* getLayouts() const { return &myLayouts; }

	void setDidChangeLayoutThisSession(bool bValue) { myDidChangeLayoutThisSession = bValue; }
	bool getDidChangeLayoutThisSession() { return myDidChangeLayoutThisSession; }

	bool getHaveValidLastUsedLayout() const;

private:

	UILayoutManager();

	void saveWindowLayout(Window* pWindow, ResourceCollection& rCollOut) const;
	void saveElementRecursive(UIElement* pElem, int iChildIndex, ResourceItem* pItem) const;

	UISplitterElement* getRootUiElement(UIPlane* pUiPlane) const;

	void loadElementRecursive(UIPlane* pUiPlane, ResourceItem* pElemItem, UIElement* pParentElem, TStringUIElementMap* pSrcElemsMap) const;

	void collectKeyUiElementsRecursive(UIElement* pElem, TStringUIElementMap& mapOut, TUIElementVector& vecImageWindowsOut) const;

	void getLayoutPath(const char* pcsLayoutName, string& strOut) const;

	void addFromCollection(ResourceCollection& rCollIn, bool bIsBuiltIn);

	UITabWindowElement* getFirstEmptyTabWindow(UIElement* pParent) const;

	void processAllSplitters(UIElement* pParent) const;

private:

	static UILayoutManager* theInstance;

	mutable string mySharedString;

	TStringLayoutMap myLayouts;
	
	mutable bool myDidChangeLayoutThisSession;
};
/*****************************************************************************/