#pragma once

namespace PythonAPI
{
/*****************************************************************************/
template < class TYPE >
class PyObjectMap
{
public:

	static PyObjectMap* getInstance()
	{
		if(!theInstance)
			theInstance = new PyObjectMap();
		return theInstance;
	}

	// Used to find the Python window given our id.
	TYPE* findById(UNIQUEID_TYPE id)
	{
		typename std::map<UNIQUEID_TYPE, TYPE* >::iterator mi = myMap.find(id);
		if(mi != myMap.end())
			return mi->second;
		else
			return NULL;
	}

	void remove(UNIQUEID_TYPE id)
	{
		if(id < 0)
			return;

		typename std::map<UNIQUEID_TYPE, TYPE* >::iterator mi = myMap.find(id);
		if(mi != myMap.end())
			myMap.erase(mi);
		ELSE_ASSERT;
	}

	void add(UNIQUEID_TYPE id, TYPE* pItem)
	{
		if(id < 0)
			ASSERT_RETURN;
#ifdef _DEBUG
		TYPE* pExistingItem = findById(id);
#endif
		_ASSERT(pExistingItem == NULL);
		myMap[id] = pItem;
	}

private:
	PyObjectMap() { }

private:
	std::map<UNIQUEID_TYPE, TYPE* > myMap;

	static PyObjectMap* theInstance;
};
/*****************************************************************************/
template < class TYPE >
PyObjectMap<TYPE>* PyObjectMap<TYPE>::theInstance = NULL;
/*****************************************************************************/
};
