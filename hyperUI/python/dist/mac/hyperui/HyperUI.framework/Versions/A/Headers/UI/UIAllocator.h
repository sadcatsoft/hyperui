#pragma once

#define REGISTER_CUSTOM_UI_ELEMENT(x) Application::getInstance()->getUIAllocator()->registerConstructor(x::getTypeString(), &x::allocateSelf)
#define REGISTER_STANDARD_UI_ELEMENT(x, y) pAllocator->registerConstructor(g_pcsUiElemTypes[y], &x::allocateSelf);

class HYPERUI_API UIElement;
class HYPERUI_API UIPlane;
/*****************************************************************************/
typedef	UIElement* (*UIElementConstructorType)(UIPlane* pParentPlane);

struct HYPERUI_API AllocatorEntry
{
	UIElementConstructorType myConstructor;
	string myType;
};
typedef map < string, AllocatorEntry > TStringAllocatorMap;
/*****************************************************************************/
class HYPERUI_API UIAllocator : public ResourceItemClassAllocator
{
public:
	virtual ~UIAllocator() { }

	virtual ResourceItem* allocateNewItem(const ResourceItem* pSourceItem, IBaseObject* pData, const char* pcsTag = NULL);
	ResourceItem* allocateItemFromType(const CHAR_TYPE* pcsTypeToAlloc, IBaseObject* pData, const char* pcsTag = NULL);

	bool getDoHaveConstructorFor(const CHAR_TYPE* pcsTypeToAlloc);

	void registerConstructor(const char* pcsType, UIElementConstructorType pSimpleConstructor);

	void getAllRegisteredTypes(TStringVector& rVecOut);

protected:
	UIPlane* extractParentPlane(IBaseObject* pData);

private:
	TStringAllocatorMap myConstructors;
};
/*****************************************************************************/