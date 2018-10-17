#pragma once

class HYPERUI_API UIElement;
/*****************************************************************************/
class HYPERUI_API UndoItemUIValueChange : public IUndoItem
{
public:

	UndoItemUIValueChange(UIElement * pElem);
	virtual ~UndoItemUIValueChange();

	virtual void undo(IBaseObject* pContext, UIElement* pElement);
	virtual void redo(IBaseObject* pContext, UIElement* pElement);

	virtual const char* getType() { return "uiValueUndo"; }
	virtual void generateDescriptiveString(string& strOut);

protected:

	void storeValue(UIElement* pElem, ResourceProperty& rItem, int& iSecInfo);
	void retrieveValue(ResourceProperty& rFromItem, int iSecInto, UIElement* pToElem);

private:

	string myTargetElementName;

	ResourceProperty	myInitElemValue;
	int					myInitSecInfo;
	ResourceProperty	myRedoElemValue;
	int					myRedoSecInfo;
};
/*****************************************************************************/