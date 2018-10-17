#pragma once

#include <objc/runtime.h>
#define objc_dynamic_cast(obj, cls) \
([obj isKindOfClass:(Class)objc_getClass(#cls)] ? (cls *)obj : NULL)

namespace HyperUI
{
void releaseAsMutableArray(void* pObj);
void refreshSizeOnAllWindows();
}
