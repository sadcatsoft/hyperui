#pragma once

#define INVALID_POS			-999

class Material;
/*****************************************************************************/
class HYPERUI_API IStringMeasurer
{
public:
	virtual ~IStringMeasurer() { } 
	virtual void measureString(Window* pWindow, const char* pcsString, const char* pcsFont, FLOAT_TYPE fPixelSize, Material* pMaterial, FLOAT_TYPE fKerning, FLOAT_TYPE fLineSpacing, SVector2D& svSizeOut) = 0;
	virtual FLOAT_TYPE getLineHeight(Window* pWindow, const char* pcsFont, FLOAT_TYPE fPixelSize, Material* pMaterial, FLOAT_TYPE fKerning, FLOAT_TYPE fLineSpacing) = 0;

protected:
	IStringMeasurer() { }
};
/*****************************************************************************/
struct HYPERUI_API SMeasuredChar
{
	SMeasuredChar() { myIsNative = true;  }
	int myOrigStringIndex;

	char myChar;
	FLOAT_TYPE myWidth;

	// Position relative to (0, 0) which denotes the
	// start of the string. Top-left corner of the letter.
	SVector2D myPosition;

	// Orig x pos when in one line
	FLOAT_TYPE myOrigLinePosition;

	// If native, the char was present in the original string.
	// If not, we've inserted it.
	bool myIsNative;
};
typedef vector < SMeasuredChar > TMeasuredCharVector;
/*****************************************************************************/
struct HYPERUI_API SMeasuredLine
{
	int myStartChar, myEndChar;
	int myLineNum;
};
typedef vector < SMeasuredLine > TMeasuredLineVector;
/*****************************************************************************/
class HYPERUI_API SMeasuredString
{
public:

	SMeasuredString(Window* pWindow, IStringMeasurer* pMeasurer);
	SMeasuredString(Window* pWindow, IStringMeasurer* pMeasurer, const char* pcsString, const char* pcsFont, FLOAT_TYPE fMaxWidth);
	~SMeasuredString();

	inline void setMaxWidth(FLOAT_TYPE fMaxWidth) { myMaxWidth = fMaxWidth; markDirty(); }
	inline void setText(const char* pcsString) { if(pcsString) myOriginalString = pcsString; else myOriginalString = ""; markDirty(); }
	inline void setFontSizeInPixels(FLOAT_TYPE fPixelSize) { myFontSizeInPixels = fPixelSize; markDirty(); }
	inline void setFont(const char* pcsFont) { myFont = pcsFont; markDirty(); }
	inline void setLineSpacing(FLOAT_TYPE fValue) { myLineSpacing = fValue; markDirty(); }
	inline void setKerning(FLOAT_TYPE fValue) { myKerning = fValue; markDirty(); }
	inline void setTrimLeadingSpaces(bool bValue) { myTrimLeadingSpaces = bValue; }

	inline void markDirty() { myIsDirty = true; }
	inline bool isDirty() const { return myIsDirty; }

	// Careful using this one! It returns a pointer to
	// the internal state, so make sure we don't use it 
	// in multi-threaded environments!
	const char* getWrappedString(Material* pMaterial);
	void getWrappedString(Material* pMaterial, string& strOut);

	FLOAT_TYPE getMeasuredLineHeight(Material* pMaterial);

	inline void setStringMeasurer(IStringMeasurer* pMeasurer) { myMeasurer = pMeasurer; }

	void getCursorPos(int iCursorPos, SVector2D& svOut, bool bCarryNewline = true);
	int getCursorPosForRelativePoint(bool bAlwaysReturnValidPos, bool bClampVertically, bool bIncResult, const SVector2D& svRelPoint);

	SMeasuredLine* getLineForPos(int iCharPos);
	int getLineIndexForPos(int iCharPos);
	SMeasuredLine* getLineInfo(int iIndex);

	inline int getNumLines() { ensureFreshMeasurements(NULL); return myLineInfos.size(); }
	inline int getNumWrappedChars() { ensureFreshMeasurements(NULL); return myWrappedChars.size(); }

	inline void setRawCursorIndex(int iPos) { myRawCursorIndex = iPos; /*myAdjustedCursorIndex = INVALID_POS;*/ }
	//int getAdjustedCursorIndex();

	int setToStartOfLine(int iRawCursorPos);
	int setToEndOfLine(int iRawCursorPos);

	int adjustCursorIndexFromRaw(int iRawCursor);
	int adjustCursorIndexToRaw(int iRawCursor);

private:

	void ensureFreshMeasurements(Material* pMaterial);
	const TMeasuredCharVector* getCharInfos(Material* pMaterial);

	void initCommon();

	int getCursorPosForRelativePointInternal(bool bAlwaysReturnValidPos, bool bClampVertically, bool bIncResult, const SVector2D& svRelPoint);

private:

	IStringMeasurer *myMeasurer;
	string myOriginalString;
	string myFont;
	FLOAT_TYPE myMaxWidth, myFontSizeInPixels;
	bool myTrimLeadingSpaces;
	FLOAT_TYPE myKerning, myLineSpacing;

	int myLastStrokeWidth;

	string myWrappedString;
	TMeasuredCharVector myWrappedChars;
	TMeasuredLineVector myLineInfos;

	int myRawCursorIndex;
	// Cursor index adjusted to wrapping, if any
	//int myAdjustedCursorIndex;

	bool myIsDirty;

	FLOAT_TYPE myMeasuredLineHeight;

	Window* myParentWindow;
};
/*****************************************************************************/