#pragma once

/*****************************************************************************/
enum SourceType
{
	SourceNone = 0,
	SourceConst,
	SourceFile
};
enum FormatType
{
	FormatNative = 0,
	FormatJSON
};
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >  class TResourceItem;

template < class KEY_TYPE, class STORAGE_TYPE >
class TResourceItemClassAllocator
{
public:
	virtual ~TResourceItemClassAllocator() { }
	virtual TResourceItem<KEY_TYPE, STORAGE_TYPE>* allocateNewItem(const TResourceItem<KEY_TYPE, STORAGE_TYPE>* pSourceItem, IBaseObject* pData, const char* pcsTag = NULL) = 0;
};
/*****************************************************************************/
