#pragma once

class IUndoItem;
/*****************************************************************************/
class HYPERUI_API IGenericIdentifier : public IBaseObject
{
public:
	virtual ~IGenericIdentifier() { }

	virtual IUndoItem* createUndoItemForSelfChange(UIElement* pSourceElem) = 0;
	virtual const char* getUndoStringForSelfChange() { return "change UI value"; }

	// Handles updates to the underlying value when the UI changes. 
	// Sets the new value to the underlying pointed to element.
	virtual void refreshStoredValueFromUI(UIElement* pSourceElem, bool bIsChangingContinuously) = 0;

	// Refreshes the UI from the values in the pointed to element.
	virtual void refreshUIFromStoredValue(UIElement* pTargetElem) = 0;

	virtual bool isValid() const = 0;

	// Determines whether the pointing UI is enabled or not.
	// Basically, whether whatever the id is for is allowed
	// to be edited at this moment.
	virtual bool getIsEnabled(UIElement* pIdentifierOwnerElem) = 0;

	// Determines whether the controls linked to the parm receptor
	// control are disabled. Note that this is only called if the control
	// overall is enabled - i.e. it can force-disable the control, but
	// it can't force-enable it.
	virtual bool getDisableLinkedControls(UIElement* pIdentifierOwnerElem) { return false; }

	// Determines whether the UI is visible at all or not
	virtual bool getIsVisible() = 0;

	virtual IBaseObject* getIdentifierTarget() { return NULL; }

protected:

	string mySharedString;
};
/*****************************************************************************/
template < class PROPERTY_TYPE >
class HYPERUI_API IParmBasedIdentifier : public IGenericIdentifier
{
public:
	virtual ~IParmBasedIdentifier() { }

protected:

	void refreshParmFromUI(UIElement* pSourceElem, PROPERTY_TYPE* pParm, bool bIsChangingContinuously);
	void refreshUIFromParm(UIElement* pTargetElem, PROPERTY_TYPE* pParm);
};
/*****************************************************************************/