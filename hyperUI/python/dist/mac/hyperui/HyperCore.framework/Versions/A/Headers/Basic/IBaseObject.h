#pragma once

/*****************************************************************************/
// An empty class that serves as the most common possible base for everything.
// This allows us to send any object as an event argument, for example.
class HYPERCORE_API IBaseObject
{
public:
	virtual ~IBaseObject() { }

	// Do NOT uncomment this seemingly convenient function. First, it conflicts
	// with the global templated as(), and second, it encourages unsafe coding
	// since it allows changing dynamic casts without checking for NULLs.
	//template < class TYPE > TYPE* as() { return dynamic_cast<TYPE*>(this); }
};
/*****************************************************************************/
