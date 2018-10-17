#pragma once

#define SAVE_EDGE_ID_PROP					"svEdgeId"
#define SAVE_EDGE_SOURCE_ELEM_ID_PROP		"svSourceElemId"

/********************************************************************************************/
class HYPERCORE_API EdgeId
{
public:

	EdgeId() { myNumericBase = 0; myAlphaId = NULL; mySourceElemId = -1; }
	//explicit EdgeId(UNIQUEID_TYPE idNumeric) { myNumericBase = idNumeric; myAlphaId = NULL; }
	~EdgeId() { clear(); }

	inline bool operator==(const EdgeId& rOther) const
	{
		return (myNumericBase == rOther.myNumericBase) && 
			( (!myAlphaId && !rOther.myAlphaId) || (myAlphaId && rOther.myAlphaId && *myAlphaId == *rOther.myAlphaId) );
	}

	static void generateMergeId(const EdgeId& idLeft, const EdgeId& idRight, EdgeId& idOut)
	{
		// Make sure the string is allocated
		string strTemp;
		idOut.setFromString("");
		idLeft.getAsString(*idOut.myAlphaId);
		idRight.getAsString(strTemp);

		*idOut.myAlphaId += "M";
		*idOut.myAlphaId += strTemp;

		// For the stored id, we just pick one. Presumably, these are the same,
		// but we really can't merge this.
		idOut.mySourceElemId = idLeft.mySourceElemId;
	}

	void generateSplitIds(EdgeId& idLeftOut, EdgeId& idRightOut) const
	{
		// Based on our current id, generate the other
		string strOwnId, strTemp;
		getAsString(strOwnId);

		// Now, add letters to it
		strTemp = strOwnId + "L";
		idLeftOut.setFromString(strTemp.c_str(), mySourceElemId);

		strTemp = strOwnId + "R";
		idRightOut.setFromString(strTemp.c_str(), mySourceElemId);
	}

	void clear()
	{
		if(myAlphaId) 
			delete myAlphaId; 
		myAlphaId = NULL;
		myNumericBase = 0;
		mySourceElemId = -1;
	}

	//void operator=(UNIQUEID_TYPE lNumId)
	void setFromNumeric(UNIQUEID_TYPE lNumId)
	{
		_ASSERT(!myAlphaId);
		myNumericBase = lNumId;
	}

	//void operator=(const char* pcsId)
	void setFromString(const char* pcsId, UNIQUEID_TYPE idOptSecondary = -1)
	{
		if(!myAlphaId)
			myAlphaId = new string;
		*myAlphaId = pcsId;
		myNumericBase = 0;

		if(idOptSecondary >= 0)
			mySourceElemId = idOptSecondary;
	}

	void operator=(const EdgeId& rOther)
	{
		clear();
		myNumericBase = rOther.myNumericBase;
		if(rOther.myAlphaId)
		{
			if(!myAlphaId)
				myAlphaId = new string;
			*myAlphaId = *rOther.myAlphaId;
		}
		mySourceElemId = rOther.mySourceElemId;
	}

	inline UNIQUEID_TYPE getNumeric() const { return myNumericBase; }
	void getAsString(string& strOut) const
	{
		if(myAlphaId)
		{
			strOut = *myAlphaId;
			return;
		}

		StringUtils::numberToString(myNumericBase, strOut);
	}

	void saveToItem(StringResourceItem& rItemOut) const
	{
		// Save as number or string
		if(myAlphaId)
			rItemOut.setStringProp(SAVE_EDGE_ID_PROP, myAlphaId->c_str());
		else
			rItemOut.setNumProp(SAVE_EDGE_ID_PROP, myNumericBase);
		
		rItemOut.setNumProp(SAVE_EDGE_SOURCE_ELEM_ID_PROP, mySourceElemId);
	}

	bool loadFromItem(const StringResourceItem& rItemOut)
	{
		// Load as string
		clear();

		if(!rItemOut.doesPropertyExist(SAVE_EDGE_ID_PROP))
			return false;

		if(rItemOut.getPropertyDataType(SAVE_EDGE_ID_PROP) == PropertyDataNumber)
			myNumericBase = rItemOut.getNumProp(SAVE_EDGE_ID_PROP);
		else
			setFromString(rItemOut.getStringProp(SAVE_EDGE_ID_PROP));

		if(rItemOut.doesPropertyExist(SAVE_EDGE_SOURCE_ELEM_ID_PROP))
			mySourceElemId = rItemOut.getNumProp(SAVE_EDGE_SOURCE_ELEM_ID_PROP);

		return true;
	}

	inline void setSourceId(UNIQUEID_TYPE idValue) { mySourceElemId = idValue; }
	inline UNIQUEID_TYPE getSourceId() const { return mySourceElemId; } 

private:

	// Our edge id may be one of these
	UNIQUEID_TYPE myNumericBase;
	string* myAlphaId;

	// This is not strictly part of our id, but rather a second id carried with the
	// edge id. For example, it may be used to track which stroke the curve came from.
	UNIQUEID_TYPE mySourceElemId;
};
/********************************************************************************************/