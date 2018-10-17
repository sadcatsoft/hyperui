#pragma once

#include "PyObjectMap.h"

namespace PythonAPI
{
/*****************************************************************************/
class BridgeElementBase
{
public:
	virtual ~BridgeElementBase() { }

	void setMappingId(UNIQUEID_TYPE id, boost::python::object pyObj);
	inline UNIQUEID_TYPE getMappingId() const { return myMappingId; }

	inline boost::python::object getPyObject() { return myPyObject; }
	bool getHasPythonBaseClass(const char* pcsBase);

protected:
	void deallocate();
	inline void resetId() { myMappingId = -1;  }

private:
	UNIQUEID_TYPE myMappingId;
	boost::python::object myPyObject;
};
/*****************************************************************************/
typedef PyObjectMap<PythonAPI::BridgeElementBase> TPyBridgeUIElementMap;
/*****************************************************************************/
};