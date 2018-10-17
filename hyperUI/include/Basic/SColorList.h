#pragma once

/*****************************************************************************/
struct HYPERUI_API SColorListEntry
{
	SColorListEntry() { myIsEnabled = true; myIsSelected = false; }

	void saveToItem(StringResourceItem& rItemOut) const;
	void loadFromItem(StringResourceItem& rItemIn);

	bool myIsSelected;
	bool myIsEnabled;
	SColor myColor;
};
typedef vector < SColorListEntry > TColorListEntries;
/*****************************************************************************/
class HYPERUI_API SColorList
{
public:
	SColorList();

	bool loadFromString(const char* pcsString);
	void saveToString(string& strOut) const;

	void clear();

	inline int getNumEntries() const { return myColors.size(); }
	inline const SColorListEntry* getEntry(int iIndex) const { return &myColors[iIndex]; }
	inline SColorListEntry* getEntry(int iIndex) { return &myColors[iIndex]; }

	void toColorVector(TColorVector& vecColors);

	void deleteColor(int iIndex) { myColors.erase(myColors.begin() + iIndex); }
	void addColor(const SColor& scolNew) { SColorListEntry rEntry; rEntry.myColor = scolNew; myColors.push_back(rEntry); }

private:

	TColorListEntries myColors;

	string mySharedString;
};
/*****************************************************************************/