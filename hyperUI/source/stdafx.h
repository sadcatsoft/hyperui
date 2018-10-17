#pragma once

#pragma warning(disable: 4231 4251)

#ifdef WIN32
    #include "HyperCore.h"
#elif defined(MAC_BUILD)
    #import <HyperCore/HyperCore.h>
#endif
using namespace HyperCore;

#include "HyperUI.h"

