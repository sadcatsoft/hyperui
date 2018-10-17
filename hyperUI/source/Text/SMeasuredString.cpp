#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
SMeasuredString::SMeasuredString(Window* pWindow, IStringMeasurer* pMeasurer)
{
	myParentWindow = pWindow;
	myMeasurer = pMeasurer;
	initCommon();
}
/*****************************************************************************/
SMeasuredString::SMeasuredString(Window* pWindow, IStringMeasurer* pMeasurer, const char* pcsString, const char* pcsFont, FLOAT_TYPE fMaxWidth)
{
	myParentWindow = pWindow;
	myMeasurer = pMeasurer;
	initCommon();

	myMaxWidth = fMaxWidth;
	myFont = pcsFont;
	setText(pcsString);
}
/*****************************************************************************/
SMeasuredString::~SMeasuredString()
{

}
/*****************************************************************************/
void SMeasuredString::initCommon()
{
	myRawCursorIndex = -1;
	//myAdjustedCursorIndex = INVALID_POS;
	myIsDirty = true;
	myLastStrokeWidth = 0;
	myLineSpacing = 0;
	myKerning = 0;
	myMaxWidth = 0;
	myFontSizeInPixels = 0;
	myMeasuredLineHeight = 0;
	myTrimLeadingSpaces = false;
}
/*****************************************************************************/
int SMeasuredString::adjustCursorIndexFromRaw(int iRawCursor)
{
	ensureFreshMeasurements(NULL);

	// Recompute. See how many chars we've inserted before the cursor pos.
	int iMaxChars = myWrappedChars.size();
	if(iMaxChars == 0)
		return iRawCursor;

	int iInsertedChars = 0;
	int iCurr;
	for(iCurr = 0; iCurr < iMaxChars && myWrappedChars[iCurr].myOrigStringIndex < iRawCursor; iCurr++)
	{
		if(myWrappedChars[iCurr].myIsNative == false)
			iInsertedChars++;
	}

	return iRawCursor + iInsertedChars;
}
/*****************************************************************************/
int SMeasuredString::adjustCursorIndexToRaw(int iAdjustedCursor)
{
	ensureFreshMeasurements(NULL);

	// Recompute. See how many chars we've inserted before the cursor pos.
	int iMaxChars = myWrappedChars.size();
	if(iMaxChars == 0)
		return iAdjustedCursor;

	int iInsertedChars = 0;
	int iCurr;
	for(iCurr = min(iAdjustedCursor, iMaxChars - 1); iCurr >= 0; iCurr--)
	{
		if(myWrappedChars[iCurr].myIsNative == false)
			iInsertedChars++;
	}

	return iAdjustedCursor - iInsertedChars;
}
/*****************************************************************************/
void SMeasuredString::ensureFreshMeasurements(Material* pMaterial)
{
	if(!myIsDirty)
		return;

	// Notice that we keep it dirty if we have no info set
	if(myFont.length() == 0)
		return;

	FLOAT_TYPE fLineHeight = myMeasurer->getLineHeight(myParentWindow, myFont.c_str(), myFontSizeInPixels, pMaterial, myKerning, myLineSpacing);

	SMeasuredChar rCharInfo;
	myWrappedChars.clear();
	myLineInfos.clear();
	string strOrigCopy(myOriginalString);
	FLOAT_TYPE fPrevCumulLen = 0;
	char cTemp;
	char *pBadBuffer = const_cast<char*>(strOrigCopy.c_str());
	char *pLineStartPtr = pBadBuffer;
	int iCurrChar, iNumChars = strOrigCopy.size();

	char pcsSmallBuff[2] = { ' ', 0 };
	SVector2D svSpaceSize;
	myMeasurer->measureString(myParentWindow, pcsSmallBuff, myFont.c_str(), myFontSizeInPixels, pMaterial, myKerning, myLineSpacing, svSpaceSize);
	_ASSERT(svSpaceSize.x > 0);

	// First pass - measure all the characters in one line
	//FLOAT_TYPE fCurrLineVertOffset = 0;
	SVector2D svCurrSize;
	bool bHaveSeenNewlines = false;
	for(iCurrChar = 0; iCurrChar < iNumChars; iCurrChar++)
	{
		rCharInfo.myOrigStringIndex = iCurrChar;
		rCharInfo.myChar = pBadBuffer[iCurrChar];

		if(pBadBuffer[iCurrChar] == ' ')
			svCurrSize.set(fPrevCumulLen + svSpaceSize.x, svSpaceSize.y);
		else
		{
			cTemp = pBadBuffer[iCurrChar + 1];
			pBadBuffer[iCurrChar + 1] = 0;
			myMeasurer->measureString(myParentWindow, pLineStartPtr, myFont.c_str(), myFontSizeInPixels, pMaterial, myKerning, myLineSpacing, svCurrSize);
			pBadBuffer[iCurrChar + 1] = cTemp;
		}
		
		rCharInfo.myWidth = svCurrSize.x - fPrevCumulLen;
		fPrevCumulLen = svCurrSize.x;

		rCharInfo.myPosition.set(svCurrSize.x - rCharInfo.myWidth, 0);
		_ASSERT(rCharInfo.myPosition.x >= 0);
		rCharInfo.myOrigLinePosition = rCharInfo.myPosition.x;

		if(pBadBuffer[iCurrChar] == '\n')
		{
			bHaveSeenNewlines = true;
			if(iCurrChar + 1 < iNumChars)
				pLineStartPtr = pBadBuffer + iCurrChar + 1;
			fPrevCumulLen = 0;
		}

		myWrappedChars.push_back(rCharInfo);
	}

	// Second pass - break them apart into lines
	if(myMaxWidth > 0 || bHaveSeenNewlines)
	{
		SMeasuredChar* pCharInfo;
		int iCurrVertLine = 0;
		FLOAT_TYPE fXOffsetToSubtract = 0;
		int iLastSpacePos = -1;
		// Since we erase and insert stuff, we must always recompute size.
		for(iCurrChar = 0; iCurrChar < myWrappedChars.size(); iCurrChar++)
		{
			pCharInfo = &myWrappedChars[iCurrChar];
			pCharInfo->myPosition.x = pCharInfo->myOrigLinePosition - fXOffsetToSubtract;
			pCharInfo->myPosition.y = iCurrVertLine*fLineHeight;

			if(pCharInfo->myPosition.x <= 0.0 && pCharInfo->myChar == ' ' && iCurrVertLine > 0 && myTrimLeadingSpaces)
			{
				// Trim leading spaces
				fXOffsetToSubtract = myWrappedChars[iCurrChar].myOrigLinePosition + myWrappedChars[iCurrChar].myWidth;
				myWrappedChars.erase(myWrappedChars.begin() + iCurrChar);
				iCurrChar--;
				continue;
			}

			if(pCharInfo->myChar == ' ')
				iLastSpacePos = iCurrChar;

			// Now, see if we're over the max width
			if((myMaxWidth > 0 && pCharInfo->myPosition.x + pCharInfo->myWidth > myMaxWidth) || pCharInfo->myChar == '\n')
			{
				// Insert a line break at last valid space or here if we
				// can't find one.
				if(pCharInfo->myChar != '\n' && myMaxWidth > 0)
				{
					int iInsertPos = iCurrChar;
					if(iLastSpacePos >= 0 && iLastSpacePos + 1 <= myWrappedChars.size())
						iInsertPos = iLastSpacePos + 1;

					SMeasuredChar* pFinalCharInfo = &myWrappedChars[iInsertPos - 1];

					SMeasuredChar rNewline;
					rNewline.myChar = '\n';
					rNewline.myOrigStringIndex = -1;
					rNewline.myIsNative = false;

					rNewline.myPosition.x = pFinalCharInfo->myPosition.x + pFinalCharInfo->myWidth;
					rNewline.myPosition.y = pFinalCharInfo->myPosition.y;
					_ASSERT(rNewline.myPosition.x  >= 0);
					rNewline.myWidth = 0;
					rNewline.myOrigLinePosition = pFinalCharInfo->myOrigLinePosition;

					// Reset us to that position:
					///fXOffsetToSubtract = pFinalCharInfo->myOrigLinePosition; // +pFinalCharInfo->myWidth;

					// Note that we have to insert this *after* the last time we use pFinalCharInfo,
					// since we may reallocate the vector, and the pointer becomes invalid.
					myWrappedChars.insert(myWrappedChars.begin() + iInsertPos, rNewline);
					// Skip it when measuring. Otherwise we end up in an infinite loop...
					iCurrChar = iInsertPos;
					if(iInsertPos + 1 < myWrappedChars.size())
						pFinalCharInfo = &myWrappedChars[iInsertPos + 1];
					else
						pFinalCharInfo = &myWrappedChars[iInsertPos];
					fXOffsetToSubtract = pFinalCharInfo->myOrigLinePosition;
				}
				else
					fXOffsetToSubtract = 0;

				// Increment line pos:
				iCurrVertLine++;
				iLastSpacePos = -1;
			}
		}
	}
	
	// Reassemble the wrapped string
	iNumChars = myWrappedChars.size();
	myWrappedString = "";
	int iLineStartChar = 0;
	int iTempIndex;
	FLOAT_TYPE fPrevLineYPos;
	for(iCurrChar = 0; iCurrChar < iNumChars; iCurrChar++)
	{
		pcsSmallBuff[0] = myWrappedChars[iCurrChar].myChar;
		myWrappedString += pcsSmallBuff;

		if(iCurrChar == 0)
			fPrevLineYPos = myWrappedChars[iCurrChar].myPosition.y;
		else
		{
			// See if we've strated a new line:
			if(myWrappedChars[iCurrChar].myPosition.y != fPrevLineYPos)
			{
				SMeasuredLine rLine;
				rLine.myStartChar = iLineStartChar;
				// Find the first valid orig char:			
				// 				for(iTempIndex = iCurrChar - 1; iTempIndex >= rLine.myStartChar && myWrappedChars[iTempIndex].myOrigStringIndex < 0; iTempIndex--)
				// 					; // deliberate
				// 				rLine.myEndChar = myWrappedChars[iTempIndex].myOrigStringIndex;
				rLine.myEndChar = iCurrChar - 1;
				rLine.myLineNum = myLineInfos.size();
				myLineInfos.push_back(rLine);

				//iLineStartChar = myWrappedChars[iCurrChar].myOrigStringIndex;
				iLineStartChar = iCurrChar;
				fPrevLineYPos = myWrappedChars[iCurrChar].myPosition.y;
			}
		}
	}

	if(iNumChars > 0 && iLineStartChar < iNumChars)
	{
		// Push the last one on:
		SMeasuredLine rLine;
		rLine.myStartChar = iLineStartChar;
		// 		for(iTempIndex = iNumChars - 1; iTempIndex >= rLine.myStartChar && myWrappedChars[iTempIndex].myOrigStringIndex < 0; iTempIndex--)
		// 			; // deliberate
		// 		rLine.myEndChar = myWrappedChars[iTempIndex].myOrigStringIndex;
		rLine.myEndChar = iNumChars- 1;
		rLine.myLineNum = myLineInfos.size();
		myLineInfos.push_back(rLine);
	}

	myMeasuredLineHeight = fLineHeight;

	myIsDirty = false;
}
/*****************************************************************************/
const char* SMeasuredString::getWrappedString(Material* pMaterial)
{
	// Check the stroke
	int iNewStrokeWidth = 0;
	if(pMaterial && pMaterial->getIsStrokeEnabled())
		iNewStrokeWidth = pMaterial->getStrokeThickness();
	if(iNewStrokeWidth != myLastStrokeWidth)
	{
		myLastStrokeWidth = iNewStrokeWidth;
		markDirty();
	}

	ensureFreshMeasurements(pMaterial);
	
	return myWrappedString.c_str();
}
/*****************************************************************************/
void SMeasuredString::getWrappedString(Material* pMaterial, string& strOut)
{
	strOut = getWrappedString(pMaterial);
}
/*****************************************************************************/
const TMeasuredCharVector* SMeasuredString::getCharInfos(Material* pMaterial)
{
	getWrappedString(pMaterial);
	return &myWrappedChars;
}
/*****************************************************************************/
FLOAT_TYPE SMeasuredString::getMeasuredLineHeight(Material* pMaterial)
{
	getWrappedString(pMaterial);
	return myMeasuredLineHeight;
}
/*****************************************************************************/
void SMeasuredString::getCursorPos(int iCursorPos, SVector2D& svOut, bool bCarryNewline)
{
	svOut.set(0, 0);
	if(iCursorPos < 0)
		return;

	//iCursorPos = adjustCursorIndexFromRaw(iCursorPos);

	const TMeasuredCharVector* pCharInfos = getCharInfos(NULL);

	// See where the cursor is:
	int iClampedCurPos = iCursorPos;
	int iNumCharInfos = pCharInfos->size();
	if(iNumCharInfos == 0)
		return;

	if(iClampedCurPos >= iNumCharInfos)
		iClampedCurPos = iNumCharInfos - 1;

	const SMeasuredChar *pCharInfo = &(*pCharInfos)[iClampedCurPos];
	svOut = pCharInfo->myPosition;
	if(iCursorPos >= iNumCharInfos)
	{
		svOut.x += pCharInfo->myWidth;
		if(pCharInfo->myChar == '\n' && bCarryNewline)
		{
			svOut.x = 0;
			svOut.y += getMeasuredLineHeight(NULL);
		}
	}
}
/*****************************************************************************/
int SMeasuredString::getCursorPosForRelativePoint(bool bAlwaysReturnValidPos, bool bClampVertically, bool bIncResult, const SVector2D& svRelPoint)
{
	int iAdjustedCursorPos = getCursorPosForRelativePointInternal(bAlwaysReturnValidPos, bClampVertically, bIncResult, svRelPoint);
	return iAdjustedCursorPos;
	//return adjustCursorIndexToRaw(iAdjustedCursorPos);
}
/*****************************************************************************/
int SMeasuredString::getCursorPosForRelativePointInternal(bool bAlwaysReturnValidPos, bool bClampVertically, bool bIncResult, const SVector2D& svRelPointIn)
{
	const TMeasuredCharVector* pCharInfos = getCharInfos(NULL);	
	FLOAT_TYPE fLineHeight = getMeasuredLineHeight(NULL);

	SVector2D svRelPoint = svRelPointIn;

	// Get the char:
	const SMeasuredChar *pCharInfo;
	int iCurr, iNum = pCharInfos->size();

	if(iNum > 0 && svRelPointIn.y < (*pCharInfos)[0].myPosition.y)
	{
		if(bAlwaysReturnValidPos)
			return 0;
		else
			return -1;
	}

/*
	if(bClampVertically && iNum > 0)
	{
		pCharInfo = &(*pCharInfos)[0];
		if(svRelPoint.y < pCharInfo->myPosition.y)
			svRelPoint.y = pCharInfo->myPosition.y + 0.01;
		pCharInfo = &(*pCharInfos)[iNum - 1];
		if(svRelPoint.y >= pCharInfo->myPosition.y + fLineHeight)
			svRelPoint.y = pCharInfo->myPosition.y + fLineHeight - 0.01;
	}*/

	int iLastMatchingLineIndex = -1;
	int iLastMatchingLineChar = -1;
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pCharInfo = &(*pCharInfos)[iCurr];
		// If the point is not within the line, ignore it
		if(!(svRelPoint.y >= pCharInfo->myPosition.y && svRelPoint.y < pCharInfo->myPosition.y + fLineHeight))
			continue;

		iLastMatchingLineChar = iCurr;
		// If the point is within our character, return it
		if(svRelPoint.x >= pCharInfo->myPosition.x && svRelPoint.x < pCharInfo->myPosition.x + pCharInfo->myWidth)
		{
			if(iCurr < iNum && bIncResult)
				return iCurr + 1;
			else
			{
				if(svRelPoint.x >= pCharInfo->myPosition.x + pCharInfo->myWidth*0.5)
					return iCurr + 1; 
				else
					return iCurr;
			}
		}
	}

	// If we don't have an exact match, use the last character that matches our line.
	if(iLastMatchingLineChar >= 0)
	{
		SMeasuredLine* pLine = getLineForPos(iLastMatchingLineChar);
		int iStartLineChar = pLine->myStartChar;
		if(svRelPoint.x < (*pCharInfos)[iStartLineChar].myPosition.x)
			iLastMatchingLineChar = iStartLineChar;

		if(iLastMatchingLineChar < iNum && (iLastMatchingLineChar != iStartLineChar || iLastMatchingLineChar == 0))
		{
			// Only do that if they are on the same line (or the last char):
			if(iLastMatchingLineChar + 1 < iNum && (*pCharInfos)[iLastMatchingLineChar].myPosition.y == (*pCharInfos)[iLastMatchingLineChar + 1].myPosition.y)
				iLastMatchingLineChar += 1;
			else if(iLastMatchingLineChar + 1 >= iNum && svRelPoint.x >= (*pCharInfos)[iNum - 1].myPosition.x + (*pCharInfos)[iNum - 1].myWidth && (*pCharInfos)[iLastMatchingLineChar].myWidth > 0.0)
				iLastMatchingLineChar = iNum;
		}
		return iLastMatchingLineChar;

/*
		if(iLastMatchingLineChar < iNum && (iLastMatchingLineChar != iStartLineChar || iLastMatchingLineChar == 0))
			return iLastMatchingLineChar + 1;
		else
			return iLastMatchingLineChar;*/
	}

	if(bAlwaysReturnValidPos)
	{
		if(iNum > 0 && svRelPoint.x < (*pCharInfos)[0].myPosition.x)
			return 0;
		else
			return iNum;
	}
	else
		return -1;
}
/*****************************************************************************/
int SMeasuredString::getLineIndexForPos(int iCharPos)
{
	//iCharPos = adjustCursorIndexFromRaw(iCharPos);

	SMeasuredLine* pLine;
	int iLine, iNum = myLineInfos.size();
	for(iLine = 0; iLine < iNum; iLine++)
	{
		pLine = &myLineInfos[iLine];
		if(iCharPos >= pLine->myStartChar && iCharPos <= pLine->myEndChar)
			return iLine;
	}

	// Last char:
	if(iNum > 0 && iCharPos >= myLineInfos[iNum - 1].myEndChar)
		return iNum - 1;

	return -1;
}
/*****************************************************************************/
SMeasuredLine* SMeasuredString::getLineForPos(int iCharPos)
{
	//iCharPos = adjustCursorIndexFromRaw(iCharPos);

	SMeasuredLine* pLine;
	int iLine, iNum = myLineInfos.size();
	for(iLine = 0; iLine < iNum; iLine++)
	{
		pLine = &myLineInfos[iLine];
		if(iCharPos >= pLine->myStartChar && iCharPos <= pLine->myEndChar)
			return pLine;
	}

	// Last char:
	if(iNum > 0 && iCharPos >= myLineInfos[iNum - 1].myEndChar)
		return &myLineInfos[iNum - 1];

	return NULL;
}
/*****************************************************************************/
SMeasuredLine* SMeasuredString::getLineInfo(int iIndex)
{
	if(iIndex >= 0 && iIndex < myLineInfos.size())
		return &myLineInfos[iIndex];
	else
		return NULL;
}
/*****************************************************************************
int SMeasuredString::getAdjustedCursorIndex()
{
	if(myAdjustedCursorIndex == INVALID_POS)
		myAdjustedCursorIndex = adjustCursorIndexFromRaw(myRawCursorIndex);
	return myAdjustedCursorIndex;
}
/*****************************************************************************/
int SMeasuredString::setToStartOfLine(int iRawCursorPos)
{
	SMeasuredLine *pLineInfo = getLineForPos(iRawCursorPos);
	if(!pLineInfo)
		return iRawCursorPos;

	iRawCursorPos = pLineInfo->myStartChar;
	setRawCursorIndex(iRawCursorPos);
	return iRawCursorPos;
}
/*****************************************************************************/
int SMeasuredString::setToEndOfLine(int iRawCursorPos)
{
	SMeasuredLine *pLineInfo = getLineForPos(iRawCursorPos);
	if(!pLineInfo)
		return iRawCursorPos;	

	//	int iAdjustedEnd = adjustCursorIndexFromRaw(pLineInfo->myEndChar);
	if(pLineInfo->myEndChar + 1 < myWrappedChars.size())
		iRawCursorPos = pLineInfo->myEndChar;
	else
		iRawCursorPos = pLineInfo->myEndChar + 1;

	setRawCursorIndex(iRawCursorPos);
	return iRawCursorPos;
}
/*****************************************************************************/
};