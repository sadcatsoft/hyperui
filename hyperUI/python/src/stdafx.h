#pragma once

#pragma warning(disable: 4251)	// ImageMagick warning

#define ENABLE_PYTHON

#ifdef WIN32
#include "HyperUI.h"
#else
#import <HyperCore/HyperCore.h>
#import <HyperUI/HyperUI.h>
#endif

#ifdef _DEBUG
	#undef _DEBUG
	#define _FORMER_DEBUG
#endif

//#define BOOST_PYTHON_STATIC_LIB
#include <boost/python.hpp>

// Add the has attr function:
namespace boost 
{ 
namespace python 
{
inline bool hasattr(object o, const char* name) 
{
	return PyObject_HasAttrString(o.ptr(), name);
}
} 
}

#ifdef _FORMER_DEBUG
	#define _DEBUG
#endif
