#pragma once

/*****************************************************************************/
template < class ID_CLASS >
class HYPERUI_API GlobalIdGeneratorInternal  : public IBaseObject
{
public:

    GlobalIdGeneratorInternal() {   myId = generateId();  }
    virtual ~GlobalIdGeneratorInternal() { }

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

