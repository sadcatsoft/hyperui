#pragma once

/*****************************************************************************/
template < class ID_CLASS >
class GlobalIdGenerator  : public IBaseObject
{
public:

    GlobalIdGenerator() {   myId = generateId();  }
    virtual ~GlobalIdGenerator() { }

    inline UNIQUEID_TYPE getId() const { return myId; }

protected:

	void loadId(UNIQUEID_TYPE id) { myId = id; if(id >= myGeneratorLastId) myGeneratorLastId = id + 1; }

    static UNIQUEID_TYPE generateId()
    {
		myGeneratorLastId++;
		return myGeneratorLastId;
    }

private:
    static UNIQUEID_TYPE myGeneratorLastId;

    UNIQUEID_TYPE myId;
};
/*****************************************************************************/
template < class ID_CLASS >
UNIQUEID_TYPE GlobalIdGenerator<ID_CLASS>::myGeneratorLastId = 0;
/*****************************************************************************/
