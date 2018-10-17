#pragma once

#define SAVE_MAT_ID					"svMatId"

/*****************************************************************************/
class HYPERUI_API Material
{
public:
	Material();
	Material(UNIQUEID_TYPE id);
	~Material();

	void clear();
	inline UNIQUEID_TYPE getId() const { return myId; }
	Material* cloneSelf() const;
	void copyFrom(const Material& rOther, bool bCopyId, bool bCopyName);

	inline void getFillColor(SColor& scolRes) const { scolRes = myFillColor; } 
	inline void setFillColor(const SColor& scolFill) { myFillColor = scolFill;  } 
	inline void setFillColorAndEnableFill(const SColor& scolFill) { myFillColor = scolFill; myIsFillEnabled = true; } 
	inline bool getIsFillEnabled() const { return myIsFillEnabled; }
	inline void setIsFillEnabled(bool bValue) { myIsFillEnabled = bValue; }

	inline void setFillOpacity(FLOAT_TYPE fVal) { myFillColor.alpha = fVal; }
	inline void setStrokeOpacity(FLOAT_TYPE fVal) { myStrokeColor.alpha = fVal; }

	inline bool getIsStrokeEnabled() const { return myIsStrokeEnabled; }
	inline void setIsStrokeEnabled(bool bValue) { myIsStrokeEnabled = bValue; }
	inline bool getIsStrokeDashEnabled() const { return myIsStrokeDashEnabled; }
	inline void setIsStrokeDashEnabled(bool bValue) { myIsStrokeDashEnabled = bValue; }
	inline void getStrokeColor(SColor& scolRes) const { scolRes = myStrokeColor; }
	inline FLOAT_TYPE getStrokeThickness() const { return myStrokeThickness; }
	inline const TFloatVector* getStrokeDashPattern() const { return &myDashPattern; }
	inline void setStrokeColor(const SColor& scolStroke) { myStrokeColor = scolStroke; }
	inline void setStrokeColorAndEnableStroke(const SColor& scolStroke) { myStrokeColor = scolStroke; myIsStrokeEnabled = true; }
	inline void setStrokeThickness(FLOAT_TYPE fValue) { myStrokeThickness = fValue; }

	inline void setStrokeDashPattern(const TFloatVector& vecDashPattern) { myDashPattern = vecDashPattern; }

	bool isEqualContentTo(const Material& rOther) const;

	void saveToItem(StringResourceItem& rItemOut) const;
	void loadFromItem(const StringResourceItem& rItem);

	void multiplyByOpacity(FLOAT_TYPE fOpacity);
	void scale(FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY);

protected:
	void initCommon(UNIQUEID_TYPE id);

private:

	UNIQUEID_TYPE myId;

	string myName;

	bool myIsFillEnabled;
	SColor myFillColor;

	// Stroke parms
	bool myIsStrokeEnabled;
	bool myIsStrokeDashEnabled;
	SColor myStrokeColor;
	FLOAT_TYPE myStrokeThickness;
	TFloatVector myDashPattern;

	mutable string myCommonString, myCommonString2;
};
/*****************************************************************************/
