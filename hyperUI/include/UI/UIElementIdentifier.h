#pragma once

/*****************************************************************************/
class HYPERUI_API IUIElementIdentifierCallback
{
public:
	~IUIElementIdentifierCallback() { }
	virtual bool getIsChildIdentifierEnabled(UIElement* pIdentifierOwnerElem) { return true; }
};
/*****************************************************************************/
class HYPERUI_API UIElementIdentifier: public IGenericIdentifier
{
public:
	UIElementIdentifier(UIElement* pTargetElem);
	UIElementIdentifier();
	virtual ~UIElementIdentifier() { }

	virtual IUndoItem* createUndoItemForSelfChange(UIElement* pSourceElem);
	virtual const char* getUndoStringForSelfChange() { return "edit gradient"; }

	virtual void refreshStoredValueFromUI(UIElement* pSourceElem, bool bIsChangingContinuously);
	virtual void refreshUIFromStoredValue(UIElement* pTargetElem);

	virtual bool isValid() const { return myElemId.length() > 0; }
	virtual bool getIsEnabled(UIElement* pIdentifierOwnerElem);
	virtual bool getIsVisible() { return true; }

private:

	string myElemId;
	UNIQUEID_TYPE myParentWindowId;
};
/*****************************************************************************/
