#include "stdafx.h"

// Track collection loading times. Works in release, too.
///#define TRACK_LOADING_STATS

// If enabled, we allow defaults inheritance. Might slow down the loading time, though.
#define ENABLE_INHERITED_DEFAULTS
#define ENABLE_INCLUDES

// Log *each* parsed line...
//#define LOG_PARSED_LINES

#define PREPROC_DEFINED_TOKEN		"defined"

namespace HyperCore
{
enum PreprocessorTokenType
{
	PreprocessorTokenInclude = 0,
	PreprocessorTokenIfdef,
	PreprocessorTokenIfndef,
	PreprocessorTokenIf,
	PreprocessorTokenElse,
	PreprocessorTokenElif,
	PreprocessorTokenEndIf,

	PreprocessorTokenLastPlaceholder
};

const string g_pcsPreprocessorTokens[] =
{
	"#include",
	"#ifdef",
	"#ifndef",
	"#if",
	"#else",
	"#elif",
	"#endif",

	""
};

enum CompoundValueType
{
	CompoundValueNone = 0,
	CompoundValueArray2D
};

enum BoolOpType
{
	BoolOpAnd = 0,
	BoolOpOr,

	BoolOpLastPlaceholder
};

const char* const g_pcsBoolOpTypeStrings[BoolOpLastPlaceholder + 1] =
{
	"&&",
	"||",

	0
};


//template < class KEY_TYPE, class STORAGE_TYPE > MemoryCacher< TArrayHolder< AlphaTreeNode< TResourceItem<KEY_TYPE, STORAGE_TYPE>* >, NUM_SYMBOLS > > TResourceCollection<KEY_TYPE, STORAGE_TYPE>::theMemCacher(16);
template < class KEY_TYPE, class STORAGE_TYPE > int TResourceCollection<KEY_TYPE, STORAGE_TYPE>::theUniqueIdCounter = 1;
/********************************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
TResourceCollection<KEY_TYPE, STORAGE_TYPE>::TResourceCollection()
	: myCacher()
{
	initCommon();
}
/********************************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
TResourceCollection<KEY_TYPE, STORAGE_TYPE>::TResourceCollection(string& strContents)
	: myCacher()
{
	initCommon();
	loadFromString(strContents);
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceCollection<KEY_TYPE, STORAGE_TYPE>::initCommon()
{
	//myType = -1;
	myLoadedElemDefaults = NULL;

	myCached2DArray = NULL;
	myCachedArraySize = 0;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
TResourceCollection<KEY_TYPE, STORAGE_TYPE>::~TResourceCollection()
{
	clear();
}
/********************************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceCollection<KEY_TYPE, STORAGE_TYPE>::clear(bool bPreserveOwnDefaults)
{
	typename RES_COLL_DEFAULTS_MAP::iterator mi;
	for(mi = myItems.begin(); mi != myItems.end(); mi++)
		delete mi->second;
	myItems.clear();
	myCacher.clearAll();

	if(!bPreserveOwnDefaults)
	{
		if(myLoadedElemDefaults)
		{
			clearAndDeleteContainer(*myLoadedElemDefaults);
			delete myLoadedElemDefaults;
		}
		myLoadedElemDefaults = NULL;
	}
}
/********************************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceCollection<KEY_TYPE, STORAGE_TYPE>::deleteItem(const char* pcsId)
{
	typename RES_COLL_DEFAULTS_MAP::iterator mi = myItems.find(pcsId);
	if(mi == myItems.end())
		ASSERT_RETURN;

	myCacher.clear(pcsId);

	delete mi->second;
	myItems.erase(mi);
}
/********************************************************************************************
template < class KEY_TYPE, class STORAGE_TYPE >
ResourceType TResourceCollection<KEY_TYPE, STORAGE_TYPE>::getType()
{
	return myType;
}
/********************************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
bool TResourceCollection<KEY_TYPE, STORAGE_TYPE>::loadFromFile(const char* pcsFilename, FileSourceType eFileSource, IProgressCallbacks* pStatsCallbacks, bool bPreserveOwnDefaults)
{
	// New, fast code
	this->clear(bPreserveOwnDefaults);
	string strFile;

#ifdef TRACK_LOADING_STATS
	{
		START_TIMING_NO_OUTPUT()
#endif
		FileUtils::loadFromFile(pcsFilename, strFile, eFileSource);
#ifdef TRACK_LOADING_STATS
		END_TIMING_NO_OUTPUT();
		if(strFile.length() > 0)
		{ OUTPUT_TO_FILE_RELEASE("db_coll_load_times.txt", "%s File load: %gs", pcsFilename, dDur); }
	}

	if(strFile.length() > 0)
	{
		START_TIMING_NO_OUTPUT()
		strlen(strFile.c_str());
		END_TIMING_NO_OUTPUT();
		OUTPUT_TO_FILE_RELEASE("db_coll_load_times.txt", " Strlen: %gs", dDur);
	}
#endif
	bool bRes = false;
#ifdef TRACK_LOADING_STATS
	{
		START_TIMING_NO_OUTPUT()
#endif
		bRes = this->loadFromStringInternal(strFile, pcsFilename, eFileSource, pStatsCallbacks, true, bPreserveOwnDefaults);
#ifdef TRACK_LOADING_STATS
		END_TIMING_NO_OUTPUT();
		if(strFile.length() > 0)
		{ OUTPUT_TO_FILE_RELEASE("db_coll_load_times.txt", " Parsing: %gs\n", dDur); }
	}
#endif
	return bRes;
}
/********************************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
bool TResourceCollection<KEY_TYPE, STORAGE_TYPE>::loadFromString(string& strContents)
{
	this->clear();
	return this->loadFromStringInternal(strContents, NULL, FileSourceFullPath, NULL, false, false);
}
/********************************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
const TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceCollection<KEY_TYPE, STORAGE_TYPE>::getItem(int iIndex, const char* pcsOptIgnoreId) const
{
	// Note: right now we do a terrible thing and iterate
	// over a collection for every request. It'd be much smarter
	// to have a vector that we create on the fly during the first request,
	// and which is emptied if we modify the collection in any way.
	KEY_TYPE tIdProp = ResourceProperty::getIdPropertyValue((KEY_TYPE)0);
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pItem;
	Iterator ci;
	int iCurr = 0;
	for(ci = itemsBegin(); !ci.isEnd(); ci++)
	{
		pItem = ci.getItem();

		if(pcsOptIgnoreId && IS_STRING_EQUAL(pcsOptIgnoreId, pItem->getStringProp(tIdProp)))
			continue;

		if(iCurr == iIndex)
			return pItem;
		iCurr++;
	}
	return NULL;
}
/********************************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
int TResourceCollection<KEY_TYPE, STORAGE_TYPE>::getNumItems() const
{
	return myItems.size();
}
/********************************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceCollection<KEY_TYPE, STORAGE_TYPE>::saveToString(string& strOut) const
{
	// NOTE: This doesn't save my type.
	typename RES_COLL_DEFAULTS_MAP::const_iterator mi;
	for(mi = myItems.begin(); mi != myItems.end(); mi++)
	{
		mi->second->saveToString(strOut);
	}
}
/********************************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
bool TResourceCollection<KEY_TYPE, STORAGE_TYPE>::loadFromStringInternal(string& strFileContents, const char* pcsOptBaseFile, FileSourceType eFileSource, IProgressCallbacks* pStatsCallbacks, bool bEnablePreprocessor, bool bPreserveOwnDefaults)
{
	const int iUpdateEveryNLines = 300;

	if(strFileContents.length() == 0)
		return false;

	_ASSERT(myCached2DArray == NULL);

	// For universal pixels.
	bool bIsAnUPNumber, bIsAnUPEnum;


	if(myLoadedElemDefaults && !bPreserveOwnDefaults)
		clearAndDeleteContainer(*myLoadedElemDefaults);

	if(!myLoadedElemDefaults)
		myLoadedElemDefaults = new RES_COLL_DEFAULTS_MAP;

	typename RES_COLL_DEFAULTS_MAP::iterator mi;
	const char* pFinalStringPtr;
	string strToken, strLower;
	bool bIsDefault;

	TResourceItem<KEY_TYPE, STORAGE_TYPE> *pCurrNode = NULL;
	TResourceItem<KEY_TYPE, STORAGE_TYPE> *pChildNode;

	// Tokenize by lines
	TStringVector rLines;
	TokenizeUtils::tokenizeString(strFileContents, NEWLINE_SEPARATORS, rLines);

#ifdef ENABLE_INCLUDES
	if(bEnablePreprocessor)
		runThroughPreprocessor(rLines, pcsOptBaseFile, eFileSource);
#endif

#if 0
#ifdef _DEBUG
	if(strFile.find("ENABLE_NATIVE_OSX_MENU") != string::npos)
	{
		string strDbTest;
		gStringVectorToString(rLines, strDbTest, true);
		FileUtils::saveToFile("db_preprocessor_output.txt", strDbTest.c_str(), false, true);
		int bp = 0;
	}
#endif
#endif

	int iLine, iNumLines = rLines.size();
	string strRes;

	if(pStatsCallbacks)
		pStatsCallbacks->setTotalNumItems(iNumLines + 1);


	vector < TResourceItem<KEY_TYPE, STORAGE_TYPE>* > rNodesToParentTo;

	TBoolVector rDefaultNodeFlag;

	int iMultilineBeginLength = strlen(MUTLINE_STRING_BEGIN_MARKER);

	CompoundValueType eCompType;
	int iQuote2;
#ifdef ENABLE_INHERITED_DEFAULTS
	string strDefaultSourceType;
	string strTempId;
#endif

	const CHAR_TYPE* pcsFoundRes;
	bool bIsDoublePrecision;
	KEY_TYPE tIdProp = ResourceProperty::getIdPropertyValue((KEY_TYPE)0);
	int iFinalPos, iEnd1, iEnd2, iStart;
	int iParentNodesNum;
	string strTag;
	string strType;
	bool bIsNumeric;
	int iEnumValue;
	int iDefaultsSize;
	TResourceItem<KEY_TYPE, STORAGE_TYPE> *pPrevAddedNode = NULL;
	bool bAddToEnum;
	string strTemp, strTemp2, strAccum;
	FLOAT_TYPE fTemp;
	SColor scolTempConvCol;

	int iStrLowerLen = 0;

	PropertyMapper* pPropertyMapper = PropertyMapper::getInstance();
	TResourceProperty<KEY_TYPE, STORAGE_TYPE> rTempValue;

	string strDelimeters("= \t");

	TStringMemoryCacher rLineMemCacher(32);
	TCharPtrVector rSingleLine;

	bool bLoadResult = true;
	string strTempLineCopy;

	for(iLine = 0; iLine < iNumLines; iLine++)
	{
		// Break apart each line with an equals
		if(iLine % 16 == 0)
			rLineMemCacher.markAllAsUnused();

		//TokenizeStringUsingMemCacher(rLines[iLine], "= \t", rSingleLine, &rLineMemCacher, true);
		// Notice that this hackey assumption is because we can't directly tokenize all lines -
		// lines with tags, type="default" and within multi-line text segments cannot have zeros
		// in them, and use rLines[]. On the other hand, in our app, we have huge images stored
		// as Base64-encoded strings all on one line, and we:
		// a) Know for sure that we can easily tokenize that line
		// b) Know that we don't want to make an extra copy of it here for both performance and
		//    memory reasons.
		// So we guesstimate that none of our "normal" lines will be over 20,000 bytes long.
		if(rLines[iLine].length() < 20000)
		{
			// We do this because the tokenization below inserts 0s into the line. It would have been ok,
			// since the rLines are stored; however, we use rLines to search for things such as tags,
			// which breaks since it encounters zeros early on.
			strTempLineCopy = rLines[iLine].c_str();
			TokenizeUtils::tokenizeStringToCharPtrsInPlace(strTempLineCopy.c_str(), strDelimeters, rSingleLine);
		}
		else
			TokenizeUtils::tokenizeStringToCharPtrsInPlace(rLines[iLine].c_str(), strDelimeters, rSingleLine);

#ifdef _DEBUG
#ifdef LOG_PARSED_LINES
		gLog(rLines[iLine].c_str());
		gLog("\n");
#endif
#endif


		if(pStatsCallbacks && iLine % iUpdateEveryNLines == 0)
			pStatsCallbacks->setNewProgress(iLine);

		if(rSingleLine.size() <= 0)
			continue;

		// See if it's a comment
		if(rSingleLine[0][0] == '#')
			continue;

		if(rSingleLine[0][0] == '<')
		{
			// See if we have a new child node
			strToken = rSingleLine[0];

			if(strToken.length() > 2)
			{
				if(strToken[0] == '<' && strToken[1] == '/')
				{
					// In this clause, we found the closing tag
					// starting with "</"

					// This child was a default for the set of nodes,
					// add it to the default map
					_ASSERT(rDefaultNodeFlag.size() > 0);
					iDefaultsSize = rDefaultNodeFlag.size();
					if(iDefaultsSize == 0)
					{
						bLoadResult = false;
						goto loadError;
					}

					if(rDefaultNodeFlag[iDefaultsSize - 1])
					{
						// This was a defaults node.
						// Add it to the map.
						typename RES_COLL_DEFAULTS_MAP::iterator di = myLoadedElemDefaults->find(pCurrNode->getTag());
						if(di == myLoadedElemDefaults->end())
							(*myLoadedElemDefaults)[pCurrNode->getTag()] = pCurrNode;
						else
							di->second->mergeResourcesFrom(*pCurrNode, true);

						// Defaults can only be at the root, not on the children.
						pCurrNode = NULL;
					}
					else
					{
						// This was a regular node.
						if(!pCurrNode)
						{
							bLoadResult = false;
							goto loadError;
						}

						// If the below fails, we have an orphan node, most likely
						// becasue of missing typeName property.
						// _ASSERT(pCurrNode->getParent() == pNodeToParentTo);
						_ASSERT(rNodesToParentTo.size() > 0);
						// Note that this now may fail also when parsing invalid files.
						// On Mac, we comment it out since that's fatal
#ifdef _DEBUG
						_ASSERT(rNodesToParentTo[rNodesToParentTo.size() - 1] == pCurrNode->getParent());
#endif

						// Closing the last child
						pCurrNode = pCurrNode->getParent();
						rNodesToParentTo.pop_back();
					}

					rDefaultNodeFlag.pop_back();
				}
				else if(strToken[0] == '<')
				{
					// If this tag starts with a < and is not
					// a closing tag, it must be an opening tag.

					bIsDefault = false;
#ifdef ENABLE_INHERITED_DEFAULTS
					strDefaultSourceType = "";
#endif
					// Get the tag value
					eCompType = CompoundValueNone;
					iStart = rLines[iLine].find("<");
					iEnd1 = rLines[iLine].find(">");
					iEnd2 = rLines[iLine].find(" ", iStart);
					if(iEnd2 != string::npos && iEnd2 < iEnd1)
						iFinalPos = iEnd2;
					else
						iFinalPos = iEnd1;
					strTag = rLines[iLine].substr(iStart + 1, iFinalPos - iStart - 1);

					int iTagToken = 0;
					if(strToken.length() <= 1)
						iTagToken = 1;

					// See if we have a type specified
					if(iEnd2 != string::npos && iEnd2 < iEnd1 && iTagToken + 1 < rSingleLine.size())
					{
						pcsFoundRes = strstr(rSingleLine[iTagToken + 1], ">");
						if(pcsFoundRes)
						{
							iQuote2 = pcsFoundRes - rSingleLine[iTagToken + 1];
							if(iQuote2 > 0)
								const_cast<char*>(rSingleLine[iTagToken + 1])[iQuote2] = 0;
						}
						
						// Get the type
						strType = rSingleLine[iTagToken + 1];
						transform(strType.begin(), strType.end(), strType.begin(), ::tolower);

						// Now see if this is a type we recognize, and if so, set it.
						if(strType == "array2d")
							eCompType = CompoundValueArray2D;
						else if(strType == "definition")
						{
							// This is a set of defaults for all entries
							// with the corresponding name tag.
							bIsDefault = true;

							// This could be the named inheritance clause
#ifdef ENABLE_INHERITED_DEFAULTS
							int iExtendsToken;
							for(iExtendsToken = iTagToken + 2; iExtendsToken < rSingleLine.size(); iExtendsToken++)
							{
								if(IS_STRING_EQUAL(rSingleLine[iExtendsToken], "extends"))
									break;
							}

							if(iExtendsToken + 1 < rSingleLine.size())
							{
								// The next one should be the name of the class to extend from:
								pcsFoundRes = strstr(rSingleLine[iExtendsToken + 1], ">");
								if(pcsFoundRes)
								{
									iQuote2 = pcsFoundRes - rSingleLine[iExtendsToken + 1];
									if(iQuote2 > 0)
										const_cast<char*>(rSingleLine[iExtendsToken + 1])[iQuote2] = 0;
								}
								strDefaultSourceType = rSingleLine[iExtendsToken + 1];
							}
#endif
						}
						ELSE_ASSERT;
					}

					rDefaultNodeFlag.push_back(bIsDefault);

					// Opening a new child
					// Note that for arrays, we don't want to create a new
					// node, we just add an entry to the existing node
					// instead.
					if(eCompType != CompoundValueArray2D)
					{
						// The default is special, and never gets added to the
						// normal tree.
						if(!rDefaultNodeFlag[rDefaultNodeFlag.size() - 1])
						{
							// Regular node

							// Added later when the type prop is parsed.
							pChildNode = new TResourceItem<KEY_TYPE, STORAGE_TYPE>();
							// pNodeToParentTo = pCurrNode;
							rNodesToParentTo.push_back(pCurrNode);

							// See if we have a default set
							mi = myLoadedElemDefaults->find(strTag);
							// Attempt to find the default node by the node tag.
							if(mi != myLoadedElemDefaults->end())
							{
								// We have defaults.

								// Copy the resources.
								pChildNode->copyResourcesFrom(*mi->second, true);

								if(mi->second->getNumChildren() > 0)
								{
								    // If we have children, we have to rename their
								    // type names with something specific to this child node,
								    // since otherwise we'll have name conflicts.
								    // However, since we don't know anything about this node yet,
								    // and the order is not guaranteed, we simply use a counter.
								    pChildNode->makeIdsUniqueRecursive(theUniqueIdCounter);
								}
							}
						}
						else
						{
							pChildNode = new TResourceItem<KEY_TYPE, STORAGE_TYPE>;

#ifdef ENABLE_INHERITED_DEFAULTS
							// This is a default. However, now it can have its
							// own source defaults, so check for that:
							if(strDefaultSourceType.length() > 0)
							{
							    mi = myLoadedElemDefaults->find(strDefaultSourceType.c_str());
							    if(mi != myLoadedElemDefaults->end())
									pChildNode->copyResourcesFrom(*mi->second, true);
							}
#endif

						}

						pChildNode->setTag(strTag.c_str());
						pCurrNode = pChildNode;
					}

					if(eCompType == CompoundValueNone) { }
					else if(eCompType == CompoundValueArray2D)
						iLine = readArray2D(rLines, iLine + 1, pCurrNode, strTag.c_str());
					ELSE_ASSERT;
				}
			}
		}
		else
		{
			if(rSingleLine.size() < 2)
			{
				bLoadResult = false;
				goto loadError;
			}

			// Normal name-value pair
			rTempValue.reset(true, PropertyDataNumber);
			rTempValue.setPropertyName(ResourceProperty::mapStringToPropertyType(pPropertyMapper, rSingleLine[0], (KEY_TYPE)0));

			// See if it's a number
			bIsAnUPNumber = false;
			if(rSingleLine[1] && (StringUtils::isANumber(rSingleLine[1], false, &bIsDoublePrecision) || (bIsAnUPNumber = StringUtils::isAnUPNumber(rSingleLine[1])) ) &&
				// The second case is to handle comments on the same line as a number
				( rSingleLine.size() == 2 || (rSingleLine.size() > 2 && rSingleLine[2][0] == '#' )) )
			{
				if(bIsAnUPNumber)
				{
					if(bIsDoublePrecision)
						rTempValue.setDouble(convertUPNumberToScreenPixels(rSingleLine[1]));
					else
						rTempValue.setNum(convertUPNumberToScreenPixels(rSingleLine[1]));

				}
				else
				{
					if(bIsDoublePrecision)
						rTempValue.setDouble(ATOF(rSingleLine[1]));
					else
						rTempValue.setNum(atof(rSingleLine[1]));
				}
			}
			else if(IS_STRING_EQUAL(rSingleLine[1], "{"))
			{
                bIsNumeric = false;
				bIsAnUPEnum = false;
				strAccum = "";
				strTemp = "";

#if 0
#ifdef _DEBUG
				/*if( (*(rSingleLine[0])) == "changes" )
				{
					int bp = 0;
				}
*/
#endif
#endif

				int iCurrEnumTokenLen;
				bool bIsInMultiline = false;
				for(iEnumValue = 2; iEnumValue < (int)rSingleLine.size(); iEnumValue++)
				{
					bAddToEnum = false;

					iCurrEnumTokenLen = strlen(rSingleLine[iEnumValue]);
					if(rSingleLine[iEnumValue][0] == '_'
						&& strstr(rSingleLine[iEnumValue], MUTLINE_STRING_BEGIN_MARKER) == rSingleLine[iEnumValue])
						//&& (*(rSingleLine[iEnumValue])).find(MUTLINE_STRING_BEGIN_MARKER) == 0)
					{
						bIsInMultiline = true;
						if(iEnumValue == 2)
							bIsNumeric = bIsAnUPEnum = false;

						//if((*(rSingleLine[iEnumValue])).length() > iMultilineBeginLength)
						if(iCurrEnumTokenLen > iMultilineBeginLength)
						{
							strTemp = rSingleLine[iEnumValue] + iMultilineBeginLength;
							//strTemp = (*(rSingleLine[iEnumValue])).substr(iMultilineBeginLength);
						}
					}
					//else if( (*(rSingleLine[iEnumValue]))[0] == ']' && (*(rSingleLine[iEnumValue])).find(MUTLINE_STRING_END_MARKER) == 0)
					else if(rSingleLine[iEnumValue][0] == ']'
						&& strstr(rSingleLine[iEnumValue], MUTLINE_STRING_END_MARKER) == rSingleLine[iEnumValue])
					{
						bIsInMultiline = false;
						strTemp = "";

						if(rSingleLine[iEnumValue][iCurrEnumTokenLen - 1] == ',')
							bAddToEnum = true;
					}
					else if(bIsInMultiline)
						strTemp = rSingleLine[iEnumValue];
					else
					{
						if(IS_STRING_EQUAL(rSingleLine[iEnumValue], "}"))
							break;

						if(rSingleLine[iEnumValue][iCurrEnumTokenLen - 1] == ',')
							bAddToEnum = true;

						// Add the enum
						TokenizeUtils::takeFirstToken(rSingleLine[iEnumValue], ",", &rLineMemCacher, strTemp);
						if(iEnumValue == 2)
						{
							bIsNumeric = StringUtils::isANumber(strTemp, false);
							if(!bIsNumeric)
							{
								bIsAnUPEnum = StringUtils::isAnUPNumber(strTemp);
								bIsNumeric |= bIsAnUPEnum;
							}
							// The below clears the enum in case there were
							// defaults.
							//rTempValue.reset(true);
						}
					}

					// The below clears the enum in case there were
					// defaults.
					if(iEnumValue == 2)
						rTempValue.reset(true);

					if(!strTemp.empty())
					{
						if(strAccum.length() > 0)
							strAccum += " ";
						strAccum += strTemp;
					}


					if(bAddToEnum)
					{
						if(bIsNumeric)
						{
							if(StringUtils::isAnUPNumber(strAccum))
								fTemp = convertUPNumberToScreenPixels(strAccum.c_str());
							else
								fTemp = atof(strAccum.c_str());
							rTempValue.addNumericEnumValue(fTemp);
						}
						else
							rTempValue.addEnumValue(strAccum.c_str());

						strAccum = "";
						strTemp = "";
					}
				} // end for over the line chunks

#if 0
#ifdef _DEBUG
/*
				if( (*(rSingleLine[0])) == "changes" )
				{
					int bp = 0;
				}*/
#endif
#endif
				if(bIsInMultiline)
				{
					// If we're here, that means we have a multiline array that spans multiple
					// lines in one entry.
					int iInnerLine = iLine;
					bool bBreakOutOfOuter = false;

					while(!bBreakOutOfOuter)
					{
						for(; iInnerLine < iNumLines; iInnerLine++)
						{
							if(iInnerLine > iLine)
							{
								strAccum += "\n" + rLines[iInnerLine];
								if(rLines[iInnerLine].find(MUTLINE_STRING_END_MARKER) != string::npos)
									break;
							}
						}

						// Now, just remove the string markers and we're done
						int iPosBegin, iPosEnd, iPos;

						iPosEnd = strAccum.find(MUTLINE_STRING_END_MARKER);
						if(iPosEnd != string::npos)
						{
							strTemp2 = strAccum.substr(iPosEnd + strlen(MUTLINE_STRING_END_MARKER));
							strAccum = strAccum.substr(0, iPosEnd);
						}
						else
							strTemp2 = "";

						iPosBegin = strAccum.find(MUTLINE_STRING_BEGIN_MARKER);
						if(iPosBegin != string::npos)
							strAccum = strAccum.substr(iPosBegin + strlen(MUTLINE_STRING_BEGIN_MARKER));


						// Note that it should really be +1, but since we're going
						// to increment it anyway, we don't do it here.
						iLine = iInnerLine;

						_ASSERT(!bIsNumeric);

						// Now we would be done - but since we're in an enum,
						// we may be starting the next enum member. We need to check for it and act accordingly.
						// What we know is that we're guaranteed to be a multiline enum at this point in this
						// clause...
						iPosBegin = strTemp2.find(MUTLINE_STRING_BEGIN_MARKER);
						iPos = strTemp2.find(",");
						if(iPos != string::npos && (iPos < iPosBegin || iPosBegin == string::npos) )
						{
							rTempValue.addEnumValue(strAccum.c_str());

							// Now, go over all the others and add them, too
							//iPos = strTemp2.find(MUTLINE_STRING_BEGIN_MARKER);
							if(iPosBegin != string::npos)
							{
								int iCurrSubPos = iPosBegin + strlen(MUTLINE_STRING_BEGIN_MARKER);
								strAccum = strTemp2.substr(iCurrSubPos);
								// Now, we might a short, on-the-same-line array. Iterate for as long as we have closing ends and commas
								iPosEnd = strTemp2.find(MUTLINE_STRING_END_MARKER, iCurrSubPos);
								while(iPosEnd != string::npos)
								{
									// So we have the end, too. Submit it, move.
									strAccum = strTemp2.substr(iCurrSubPos, iPosEnd - iCurrSubPos);
									rTempValue.addEnumValue(strAccum.c_str());

									iCurrSubPos = iPosEnd + strlen(MUTLINE_STRING_END_MARKER);

									// See if we have a comma before the next begin
									iPosBegin = strTemp2.find(MUTLINE_STRING_BEGIN_MARKER, iCurrSubPos);
									iPos = strTemp2.find(",", iCurrSubPos);
									if(iPos != string::npos && (iPos < iPosBegin || iPosBegin == string::npos) )
									{
										// We are continuing the list
										if(iPosBegin != string::npos)
										{
											// We have both the next beginning and a coma
											iCurrSubPos = iPosBegin + strlen(MUTLINE_STRING_BEGIN_MARKER);
											iPosEnd = strTemp2.find(MUTLINE_STRING_END_MARKER, iCurrSubPos);
											strAccum = strTemp2.substr(iCurrSubPos);
										}
										else
										{
											// We have a comma, but not the start of the next guy.
											strAccum = "";
											break;
										}
									}
									else
									{
										// No comma, we're done
										bBreakOutOfOuter = true;
										strAccum = "";
										break;
									}
								}
							}
							else
								strAccum = "";
							iInnerLine++;
						}
						else
						{
							break;
						}

					}
				}

				if(strAccum.length() > 0)
				{
					if(bIsNumeric)
					{
						if(StringUtils::isAnUPNumber(strAccum))
							fTemp = convertUPNumberToScreenPixels(strAccum.c_str());
						else
							fTemp = atof(strAccum.c_str());
						rTempValue.addNumericEnumValue(fTemp);
					}
					else
						rTempValue.addEnumValue(strAccum.c_str());
				}

			}
			else
			{
				if(rSingleLine.size() == 2)
				{
					// See if it's a true of false string
					int iTempCntr;
					for(iTempCntr = 0; rSingleLine[1][iTempCntr] && iTempCntr <= 8; iTempCntr++)
					{
						// Nothing.
					}

					// Note that we only need to do this for testing
					// bool cases. However, in our app, we can an entire
					// 400Mb image stored as property in base64 - in which
					// we will go and stupidly transform the whole thing,
					// which will take forever (22%). So instead, we just
					// do it when at least by length, we could possibly
					// match the true/false statements...
					if(iTempCntr == 4 || iTempCntr == 5)
					{
						strLower = rSingleLine[1];
						transform(strLower.begin(), strLower.end(), strLower.begin(), ::tolower);
						pFinalStringPtr = strLower.c_str();
					}
					else
						pFinalStringPtr = rSingleLine[1];

					if(iTempCntr == 4 && IS_STRING_EQUAL(pFinalStringPtr, "true"))
						rTempValue.setBool(true);
					else if(iTempCntr == 5 && IS_STRING_EQUAL(pFinalStringPtr, "false"))
						rTempValue.setBool(false);
					else if(iTempCntr == 7 && pFinalStringPtr[0] == '#')
					{
						// Assume this is a hex color
						// Convert it to such
						scolTempConvCol.fromHex(pFinalStringPtr + 1);
						rTempValue.setAsColor(scolTempConvCol);
					}
					else
					{
						bool bHaveSetAlready = false;

						// Now, we have to see if this is a multiline string.
						if(rSingleLine[1] && rSingleLine[1][0] == '_')
						{
							if(strstr(rSingleLine[1], MUTLINE_STRING_BEGIN_MARKER) == rSingleLine[1])
							{
								strRes = rSingleLine[1];

								// Yay! Go over the lines until we hit the end marker
								int iInnerLine;
								for(iInnerLine = iLine; iInnerLine < iNumLines; iInnerLine++)
								{
									if(iInnerLine > iLine)
										strRes += "\n" + rLines[iInnerLine];
									if(rLines[iInnerLine].find(MUTLINE_STRING_END_MARKER) != string::npos)
										break;
								}

								// Now, just remove the string markers and we're done
								int iPos;
								iPos = strRes.find(MUTLINE_STRING_BEGIN_MARKER);
								strRes = strRes.substr(iPos + strlen(MUTLINE_STRING_BEGIN_MARKER));
								iPos = strRes.find(MUTLINE_STRING_END_MARKER);
								strRes = strRes.substr(0, iPos);
								// Note that it should really be +1, but since we're going
								// to increment it anyway, we don't do it here.
								iLine = iInnerLine;

								const CHAR_TYPE* pcsTrimmedValue = StringUtils::modifyStringPointerToTrim(' ', strRes);
								rTempValue.setStringOrExpression(pcsTrimmedValue);
								
								bHaveSetAlready = true;
							}
						}

						if(!bHaveSetAlready)
							rTempValue.setStringOrExpression(rSingleLine[1]);
					}
				}
				else
				{
					// This line contains a long string with spaces, etc.
					// Re-assemble it.

					int iFirstEqPos = rLines[iLine].find("=");
					_ASSERT(iFirstEqPos != string::npos);

					iFirstEqPos++;

					// Skip any spaces we migh thave
					while(iFirstEqPos < (int)rLines[iLine].length() && rLines[iLine][iFirstEqPos] == ' ')
						iFirstEqPos++;

					if(iFirstEqPos >= (int)rLines[iLine].length())
						strRes = "";
					else
						strRes = rLines[iLine].substr(iFirstEqPos, rLines[iLine].length() - iFirstEqPos);

					// Now, we have to see if this is a multiline string.
					const CHAR_TYPE* pcsTrimmedValue = strRes.c_str();
					if(strRes.length() > 0 && strRes[0] == '_')
					{
						if(strRes.find(MUTLINE_STRING_BEGIN_MARKER) == 0)
						{
							// Yay! Go over the lines until we hit the end marker
							int iInnerLine;
							for(iInnerLine = iLine; iInnerLine < iNumLines; iInnerLine++)
							{
								if(iInnerLine > iLine)
									strRes += "\n" + rLines[iInnerLine];
								if(rLines[iInnerLine].find(MUTLINE_STRING_END_MARKER) != string::npos)
									break;
							}

							// Now, just remove the string markers and we're done
							int iPos;
							iPos = strRes.find(MUTLINE_STRING_BEGIN_MARKER);
							strRes = strRes.substr(iPos + strlen(MUTLINE_STRING_BEGIN_MARKER));
							iPos = strRes.find(MUTLINE_STRING_END_MARKER);
							strRes = strRes.substr(0, iPos);
							// Note that it should really be +1, but since we're going
							// to increment it anyway, we don't do it here.
							iLine = iInnerLine;

							pcsTrimmedValue = StringUtils::modifyStringPointerToTrim(' ', strRes);
						}
					}

					rTempValue.setStringOrExpression(pcsTrimmedValue);

				}
			}

			if(!pCurrNode)
			{
				bLoadResult = false;
				goto loadError;
			}
			pCurrNode->copyPropertyFrom(rTempValue);

			if(ResourceProperty::isIdProperty(rTempValue.getPropertyName()) &&
				!rDefaultNodeFlag[rDefaultNodeFlag.size() - 1])
			{
				pCurrNode->convertPropToString(tIdProp);
				// Parent the node here
				iParentNodesNum = rNodesToParentTo.size();
				if(iParentNodesNum > 0 && rNodesToParentTo[iParentNodesNum - 1])
				{
#ifdef ENABLE_INHERITED_DEFAULTS
				    // See if we can find an existing node possibly copied
				    // from the parent default in our current parent.
				    // In that case, we don't need to create a new node,
				    // but instead point to an existing node

#ifdef _DEBUG
					if(strcmp(pCurrNode->getStringProp(tIdProp), "defUnitedNumUpdown") == 0)
					{
						int bp = 0;
					}
#endif

				    TResourceItem<KEY_TYPE, STORAGE_TYPE>* pExistingMatchingChild = this->parserFindExistingNodeForOverride(rNodesToParentTo[iParentNodesNum - 1], pCurrNode->getStringProp(tIdProp));
				    if(pExistingMatchingChild)
				    {
						// So we already have a node to override. Merge all existing properties from this guy
						// (except typename), destroy it, and point the curr node to existing node.
						// Need a string since we'll get overwritten.
						strTempId = pExistingMatchingChild->getStringProp(tIdProp);
						pExistingMatchingChild->mergeResourcesFrom(*pCurrNode, true, NULL, true, true);
						pExistingMatchingChild->setStringProp(tIdProp, strTempId.c_str());
						delete pCurrNode;
						pCurrNode = pExistingMatchingChild;
				    }
				    else
#endif
					rNodesToParentTo[iParentNodesNum - 1]->parentExistingNode(pCurrNode);
				}
				else
				{
					// Note: do not add an item if this is the same item as before. This may happen
					// with malformed input.
					if(pPrevAddedNode == NULL || pPrevAddedNode != pCurrNode)
					{
						this->addExistingItem(pCurrNode);
						pPrevAddedNode = pCurrNode;
					}
					ELSE_ASSERT;
				}
			}
		}
	}
loadError:
	// If this fails, we didn't close one of the tags.
	_ASSERT(!bLoadResult || rNodesToParentTo.size() == 0);

	// We now store our loaded defaults
	/*
	// Delete any default nodes we may have accumulated
	for(mi = mDefaults.begin(); mi != mDefaults.end(); mi++)
		delete mi->second;
	mDefaults.clear();
	*/

	if(myCached2DArray)
	{
		delete[] myCached2DArray;
		myCached2DArray = NULL;

		myCachedArraySize = 0;
	}

	if(pStatsCallbacks)
		pStatsCallbacks->setNewProgress(iNumLines + 1);

	this->setDataSourceTypeForAllItems(SourceFile);

	myCacher.clearAll();
#if defined(_DEBUG) || defined(_DEBUG)
	rLineMemCacher.markAllAsUnused();
#endif
	return bLoadResult;

}
/********************************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceCollection<KEY_TYPE, STORAGE_TYPE>::readArraySizes(TStringVector& rLines, int iStartLine, int& iWOut, int& iHOut)
{
	iWOut = iHOut = 0;
	myReadArraySizeVec.clear();
	int iLine, iNumLines = rLines.size();
	int iChar, iNumChars;
	char cChar;
	for(iLine = iStartLine; iLine < iNumLines; iLine++)
	{
		if(iWOut <= 0)
		{
			TokenizeUtils::tokenizeString(rLines[iLine], " =\t", myReadArraySizeVec);

			if(myReadArraySizeVec.size() <= 0)
				continue;

			if(myReadArraySizeVec[0][0] == '<')
				return;

			iWOut = myReadArraySizeVec.size();
		}
		else
		{
			iNumChars = rLines[iLine].length();
			for(iChar = 0; iChar < iNumChars; iChar++)
			{
				cChar = rLines[iLine][iChar];
				if(cChar == ' ' || cChar == '\t' || cChar == '=')
					continue;

				if(cChar == '<')
					return;

				break;
			}

			if(iChar >= iNumChars)
				continue;
		}


		iHOut++;

		/*
		// Break apart each line with an equals
		TokenizeString(rLines[iLine], " =\t", theReadArraySizeVec);

		if(theReadArraySizeVec.size() <= 0)
			continue;

		if(theReadArraySizeVec[0][0] == '<')
			return;

		if(iWOut <= 0)
			iWOut = theReadArraySizeVec.size();

		_ASSERT(iWOut == theReadArraySizeVec.size());
		iHOut++;
		 */
	}
}
/********************************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
int TResourceCollection<KEY_TYPE, STORAGE_TYPE>::readArray2D(TStringVector& rLines, int iStartLine, TResourceItem<KEY_TYPE, STORAGE_TYPE>* pNode, const char* pcsTag)
{
	int iArrayW = 0;
	int iArrayH = 0;
	int iOffset = 0;
//	int iElem;

	readArraySizes(rLines, iStartLine, iArrayW, iArrayH);
	if(iArrayW <= 0 || iArrayH <= 0)
	{
		_ASSERT(0);
		return iStartLine;
	}

	///	fArray = new FLOAT_TYPE[iArrayW*iArrayH];
	int iNeededMemSize = iArrayW*iArrayH;
	if(iNeededMemSize > myCachedArraySize)
	{
		// Reallocate the array
		if(myCached2DArray)
			delete[] myCached2DArray;
		myCachedArraySize = iNeededMemSize*2;
		myCached2DArray = new FLOAT_TYPE[myCachedArraySize];
	}
	memset(myCached2DArray, 0, sizeof(FLOAT_TYPE)*iNeededMemSize);

	PropertyMapper* pPropertyMapper = PropertyMapper::getInstance();

	//	TStringVector rSingleLine;
	bool bFinish;
	string strTag(pcsTag);
	myReadArrayVec.clear();
	int iLine, iNumLines = rLines.size();
	for(iLine = iStartLine; iLine < iNumLines; iLine++)
	{

		bFinish = TokenizeUtils::tokenizeStringToFloats(rLines[iLine], " =\t", myCached2DArray, iOffset);
		if(bFinish)
		{
			// We're done. Finish the array and leave.
			TResourceProperty<KEY_TYPE, STORAGE_TYPE> rValue;
			rValue.setPropertyName(ResourceProperty::mapStringToPropertyType(pPropertyMapper, strTag, (KEY_TYPE)0));
			//rValue.setPropertyName(pPropertyMapper->mapProperty(strTag));

			_ASSERT(iOffset == iArrayW*iArrayH);

			rValue.setFloatArray(myCached2DArray, iArrayW, iArrayH);
			pNode->copyPropertyFrom(rValue);
			return iLine;
		}


		/*
		// Break apart each line with an equals
		TokenizeString(rLines[iLine], " =\t", theReadArrayVec);

		if(theReadArrayVec.size() <= 0)
			continue;

		if(theReadArrayVec[0][0] == '<')
		{
			// We're done. Finish the array and leave.
			ResourceProperty rValue;
			rValue.setPropertyName(pPropertyMapper->mapProperty(strTag));

			rValue.setFloatArray(theCached2DArray, iArrayW, iArrayH);
			pNode->copyPropertyFrom(rValue);
			return iLine;
		}

		// Otherwise, we assume it's a valid array line.
		_ASSERT(iArrayW == theReadArrayVec.size());

		// Set its elements
		for(iElem = 0; iElem < iArrayW; iElem++)
		{
			theCached2DArray[iOffset + iElem] = atof(theReadArrayVec[iElem].c_str());
		}

		iOffset += iArrayW;
		 */

		_ASSERT(iOffset <= iArrayW*iArrayH);
	}

	// Getting here means we never closed the tag.
	_ASSERT(0);
	return iNumLines;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceCollection<KEY_TYPE, STORAGE_TYPE>::setDataSourceTypeForAllItems(SourceType eValue)
{
	typename RES_COLL_DEFAULTS_MAP::iterator mi;
	for(mi = myItems.begin(); mi != myItems.end(); mi++)
	{
		mi->second->setDataSourceType(eValue);
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceCollection<KEY_TYPE, STORAGE_TYPE>::findItemWithPropValue(KEY_TYPE eProp, const char* pcsValue, bool bRecursive)
{
	if(!pcsValue)
		ASSERT_RETURN_NULL;

	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pTempRes;
	const char* pcsCurrValue;
	typename RES_COLL_DEFAULTS_MAP::iterator mi;
	for(mi = myItems.begin(); mi != myItems.end(); mi++)
	{
		if(mi->second->doesPropertyExist(eProp))
			pcsCurrValue = mi->second->getStringProp(eProp);
		else
			pcsCurrValue = NULL;
		if(pcsCurrValue && strcmp(pcsCurrValue, pcsValue) == 0)
			return mi->second;

		// Otherwise, see if the children of items have it:
		if(bRecursive)
		{
			pTempRes = mi->second->getChildByPropertyValue(eProp, pcsValue, true);
			if(pTempRes)
				return pTempRes;
		}
	}

	return NULL;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceCollection<KEY_TYPE, STORAGE_TYPE>::findItemWithPropValue(KEY_TYPE eProp, FLOAT_TYPE fValue)
{
	FLOAT_TYPE fCurrValue;
	typename RES_COLL_DEFAULTS_MAP::iterator mi;
	for(mi = myItems.begin(); mi != myItems.end(); mi++)
	{
		fCurrValue = mi->second->getNumProp(eProp);
		if(fabs(fCurrValue - fValue) < FLOAT_EPSILON)
			return mi->second;
	}

	return NULL;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceCollection<KEY_TYPE, STORAGE_TYPE>::findAllItemsWithPropValue(KEY_TYPE eProp, const char* pcsValue, vector <TResourceItem <KEY_TYPE, STORAGE_TYPE>* >& vecOut, bool bClear)
{
	if(!pcsValue)
		ASSERT_RETURN;

	if(bClear)
		vecOut.clear();

	PropertyDataType ePropType;
	bool bAddChild;
	//TResourceItem<KEY_TYPE, STORAGE_TYPE>* pTempRes;
	const char* pcsCurrValue;
	typename map < string, TResourceItem<KEY_TYPE, STORAGE_TYPE>* >::iterator mi;
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pResItem;
	for(mi = myItems.begin(); mi != myItems.end(); mi++)
	{
		pResItem = mi->second;
		bAddChild = false;
		if(pResItem->doesPropertyExist(eProp))
		{
			ePropType = pResItem->getPropertyDataType(eProp);
			if(ePropType == PropertyDataStringList)
				bAddChild = pResItem->findStringInEnumProp(eProp, pcsValue) >= 0;
			else if(ePropType == PropertyDataString)
			{
				pcsCurrValue = pResItem->getStringProp(eProp);
				if(pcsCurrValue && strcmp(pcsCurrValue, pcsValue) == 0)
					bAddChild = true;
			}
		}

		if(bAddChild)
			vecOut.push_back(pResItem);
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceCollection<KEY_TYPE, STORAGE_TYPE>::findAllItemsWithPropValue(KEY_TYPE eProp, bool bValue, vector <TResourceItem <KEY_TYPE, STORAGE_TYPE>* >& vecOut, bool bClear)
{
	if(bClear)
		vecOut.clear();

	PropertyDataType ePropType;
	bool bAddChild;
	typename map < string, TResourceItem<KEY_TYPE, STORAGE_TYPE>* >::iterator mi;
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pResItem;
	for(mi = myItems.begin(); mi != myItems.end(); mi++)
	{
		pResItem = mi->second;
		bAddChild = false;
		if(pResItem->doesPropertyExist(eProp))
		{
			ePropType = pResItem->getPropertyDataType(eProp);
			if(ePropType == PropertyDataBool)
				bAddChild = pResItem->getBoolProp(eProp) == bValue;
		}
		// Absence of a bool prop treated as false
		else if(bValue == false)
			bAddChild = true;

		if(bAddChild)
			vecOut.push_back(pResItem);
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceCollection<KEY_TYPE, STORAGE_TYPE>::itemsToVector(vector <TResourceItem <KEY_TYPE, STORAGE_TYPE>* >& vecOut)
{
	vecOut.clear();
	typename map < string, TResourceItem<KEY_TYPE, STORAGE_TYPE>* >::iterator mi;
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pResItem;
	for(mi = myItems.begin(); mi != myItems.end(); mi++)
	{
		pResItem = mi->second;
		vecOut.push_back(pResItem);
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceCollection<KEY_TYPE, STORAGE_TYPE>::mergeFrom(TResourceCollection<KEY_TYPE, STORAGE_TYPE>& rOther)
{
	// Traverse all the resource items of the other collection, find
	// corresponding resource items.
	typename RES_COLL_DEFAULTS_MAP::iterator mi_other;
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pOther, *pThis;

#if defined(_DEBUG) && defined(PARANOID_CACHE_CHECKS)
	int iDbNumMerged = 0;
	int iDbNumNew = 0;
#endif
	KEY_TYPE eIdProp = ResourceProperty::getIdPropertyValue((KEY_TYPE)0);
	for(mi_other = rOther.myItems.begin();  mi_other != rOther.myItems.end(); mi_other++)
	{
		pOther = mi_other->second;

		pThis = this->getItemById(pOther->getStringProp(eIdProp));
		myCacher.clearAll();
		if(pThis)
		{
			// This item already exists, add whatever properties except the type
			// name here.
			pThis->mergeResourcesFrom(*pOther, true);
#if defined(_DEBUG) && defined(PARANOID_CACHE_CHECKS)
			iDbNumMerged++;
#endif
		}
		else
		{
			// New item. Just add it.
			pThis = new TResourceItem<KEY_TYPE, STORAGE_TYPE>;
			pThis->copyResourcesFrom(*pOther, true);
			this->addExistingItem(pThis);
#if defined(_DEBUG) && defined(PARANOID_CACHE_CHECKS)
			iDbNumNew++;
#endif
		}
		myCacher.clearAll();
	}
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceCollection<KEY_TYPE, STORAGE_TYPE>::copyFrom(TResourceCollection<KEY_TYPE, STORAGE_TYPE>* pOther)
{
	// Resolve:
	// Way 1 doesn't copy tags.
	// Way 2 may not reset the cacher correctly and/or check children.
	_ASSERT(0);


	this->clear();

/*	// Way 1
	myType = pOther->myType;
	this->mergeFrom(*pOther);

*/	
	// Way 2
	typename RES_COLL_DEFAULTS_MAP::iterator mi;
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pNewItem;
	TResourceItem<KEY_TYPE, STORAGE_TYPE> *pItem = NULL;
	for(mi = pOther->myItems.begin(); mi != pOther->myItems.end(); mi++)
	{
		pItem = mi->second;
		pNewItem = new TResourceItem<KEY_TYPE, STORAGE_TYPE>();
		pNewItem->mergeResourcesFrom(*pItem, true);
		//pNewItem->copyResourcesFrom(*pItem, true);
		pNewItem->setTag(pItem->getTag());
		myItems[mi->first] = pNewItem;

		myCacher.clearAll();
	}

	// Same for our defaults
	copyDefaultsFrom(pOther);
}
/********************************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceCollection<KEY_TYPE, STORAGE_TYPE>::copyDefaultsFrom(TResourceCollection<KEY_TYPE, STORAGE_TYPE>* pOther)
{
	if(!pOther)
		return;

	if(myLoadedElemDefaults)
	{
		clearAndDeleteContainer(*myLoadedElemDefaults);
		delete myLoadedElemDefaults;
	}
	myLoadedElemDefaults = NULL;

	if(pOther->myLoadedElemDefaults)
	{
		typename RES_COLL_DEFAULTS_MAP::iterator mi;
		TResourceItem<KEY_TYPE, STORAGE_TYPE>* pNewItem;
		TResourceItem<KEY_TYPE, STORAGE_TYPE> *pItem = NULL;
		myLoadedElemDefaults = new RES_COLL_DEFAULTS_MAP;
		for(mi = pOther->myLoadedElemDefaults->begin(); mi != pOther->myLoadedElemDefaults->end(); mi++)
		{
			pItem = mi->second;
			pNewItem = new TResourceItem<KEY_TYPE, STORAGE_TYPE>();
			pNewItem->mergeResourcesFrom(*pItem, true);
			//pNewItem->copyResourcesFrom(*pItem, true);
			pNewItem->setTag(pItem->getTag());
			(*myLoadedElemDefaults)[mi->first] = pNewItem;
		}
	}
}
/********************************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceCollection<KEY_TYPE, STORAGE_TYPE>::parserFindExistingNodeForOverride(TResourceItem<KEY_TYPE, STORAGE_TYPE>* pParent, const char* pcsIdToOverride)
{
	// Find the type name in the immediate parent's children in the old saved names property.
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pChild;
	int iChild, iNumChildren = pParent->getNumChildren();
	KEY_TYPE tDefOldIdProp = ResourceProperty::getDefOldIdPropertyValue((KEY_TYPE)0);
	for(iChild = 0; iChild < iNumChildren; iChild++)
	{
		pChild = pParent->getChild(iChild);
		if(!pChild->doesPropertyExist(tDefOldIdProp))
			continue;

		if(strcmp(pChild->getStringProp(tDefOldIdProp), pcsIdToOverride) == 0)
		{
			// Got it!
			return pChild;
		}
	}

	return NULL;
}
/********************************************************************************************/
struct SIfDefBlock
{
    int myStartLine;
    bool myIsEnabled;
};
typedef list<SIfDefBlock> TIfDefBlocks;

template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceCollection<KEY_TYPE, STORAGE_TYPE>::runThroughPreprocessor(TStringVector& rLines, const CHAR_TYPE* pcsBaseFile, FileSourceType eFileSource, TStringSet* pOptExtraDefinesSet)
{
	TStringVector *pFinalPointer;
	TStringVector rInclLines;

	TIfDefBlocks rBlocks;
	PreprocessorTokenType eCurrTokenType;

	string strTemp;
	string strFileContents;
	string strInclFileName, strInclFileNameShort;
	string::size_type iInclStart, iQuoteStart, iQuoteEnd;
	string* strLine;
	int iLine, iNumLines = rLines.size();

	for(iLine = 0; iLine < iNumLines; iLine++)
	{
		strLine = &rLines[iLine];

		eCurrTokenType = mapStringToTypeByPrefix<PreprocessorTokenType>(strLine->c_str(), g_pcsPreprocessorTokens, PreprocessorTokenLastPlaceholder, true);
		if(eCurrTokenType == PreprocessorTokenLastPlaceholder)
			continue;

		if(eCurrTokenType == PreprocessorTokenInclude)
		{
			iInclStart = strLine->find(g_pcsPreprocessorTokens[eCurrTokenType]);
			_ASSERT(iInclStart != string::npos);

			iQuoteStart = strLine->find("\"", iInclStart);
			iQuoteEnd = strLine->find("\"", iQuoteStart + 1);
			if(iQuoteStart != string::npos && iQuoteEnd != string::npos)
			{
				// We've got the include!
				strInclFileNameShort = strLine->substr(iQuoteStart + 1, iQuoteEnd - iQuoteStart - 1);

				pFinalPointer = ResourceManager::getInstance()->getCachedIncludeFile(strInclFileNameShort.c_str());

				if(!pFinalPointer)
				{
					ResourceManager::getRelativeFile(pcsBaseFile, strInclFileNameShort.c_str(), strInclFileName);

					// Now, load the contents of that file
					FileUtils::loadFromFile(strInclFileName.c_str(), strFileContents, eFileSource);

					// Now, insert it into the lines and remove this one
					TokenizeUtils::tokenizeString(strFileContents, NEWLINE_SEPARATORS, rInclLines);

					if(rInclLines.size() == 0)
					{ 
						gLog("ERROR: Unable to find include file: %s\n", strInclFileName.c_str());
						_ASSERT(0); 
					}

					// He-he.
					runThroughPreprocessor(rInclLines, strInclFileName.c_str(), eFileSource);

					ResourceManager::getInstance()->setCachedIncludeFile(strInclFileNameShort.c_str(), rInclLines);
					pFinalPointer = &rInclLines;
				}

				// Now insert it into our actual vector
				rLines.erase(rLines.begin() + iLine);
				rLines.insert(rLines.begin() + iLine, pFinalPointer->begin(), pFinalPointer->end());
				// Skip the newly inserted text. Note that the nesting of includes has been taken
				// care of in the recursive call above.
				iLine += pFinalPointer->size() - 1;
				iNumLines = rLines.size();
			}
			ELSE_ASSERT;
		} // end if include
		else if(eCurrTokenType == PreprocessorTokenIf)
		{
			iInclStart = strLine->find(g_pcsPreprocessorTokens[eCurrTokenType]);
			SIfDefBlock rNewBlock;
			strTemp = strLine->c_str() + iInclStart + g_pcsPreprocessorTokens[eCurrTokenType].length() + 1;
			rNewBlock.myIsEnabled = evaluatePreprocessorCondition(strTemp, pOptExtraDefinesSet);
			rNewBlock.myStartLine = iLine;
			rBlocks.push_back(rNewBlock);
		}
		else if(eCurrTokenType == PreprocessorTokenIfdef
			|| eCurrTokenType == PreprocessorTokenIfndef)
		{
			iInclStart = strLine->find(g_pcsPreprocessorTokens[eCurrTokenType]);
			if(eCurrTokenType == PreprocessorTokenIfdef)
				strTemp = PREPROC_DEFINED_TOKEN;
			else
			{
				strTemp = "!";
				strTemp += PREPROC_DEFINED_TOKEN;
			}
			strTemp += "(";
			strTemp += strLine->c_str() + iInclStart + g_pcsPreprocessorTokens[eCurrTokenType].length() + 1;
			strTemp += ")";
			SIfDefBlock rNewBlock;
			rNewBlock.myIsEnabled = evaluatePreprocessorCondition(strTemp, pOptExtraDefinesSet);
			rNewBlock.myStartLine = iLine;
			rBlocks.push_back(rNewBlock);
		}
		else if(eCurrTokenType == PreprocessorTokenElse
			|| eCurrTokenType == PreprocessorTokenElif)
		{
			if(rBlocks.size() > 0)
			{
				// Take out the prev block contents if necessary, reverse
				// or reeavaluate condition
				typename TIfDefBlocks::iterator li = rBlocks.end();
				li--;
				int iStartBlock = (*li).myStartLine;
				bool bWasEnabledBlock = (*li).myIsEnabled;
				if(eCurrTokenType == PreprocessorTokenElif)
				{
					iInclStart = strLine->find(g_pcsPreprocessorTokens[eCurrTokenType]);
					strTemp = strLine->c_str() + iInclStart + g_pcsPreprocessorTokens[eCurrTokenType].length() + 1;
					(*li).myIsEnabled = evaluatePreprocessorCondition(strTemp, pOptExtraDefinesSet);
				}
				else
					(*li).myIsEnabled = !(*li).myIsEnabled;

				// Do NOT erase the last line...
				// Actually, erase it if the block is true...
				iLine = processIfdefBlock(rLines, iStartBlock, iLine, bWasEnabledBlock, false);
				iNumLines = rLines.size();
				(*li).myStartLine = iLine;
			}
			ELSE_ASSERT;
		}
		else if(eCurrTokenType == PreprocessorTokenEndIf)
		{
			if(rBlocks.size() > 0)
			{
				typename TIfDefBlocks::iterator li = rBlocks.end();
				li--;
				iLine = processIfdefBlock(rLines, (*li).myStartLine, iLine, (*li).myIsEnabled, true);
				iNumLines = rLines.size();
				rBlocks.pop_back();
			}
			ELSE_ASSERT;
		}
	}
}
/********************************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceCollection<KEY_TYPE, STORAGE_TYPE>::getDefaultFor(const char* pcsTag)
{
	if(!myLoadedElemDefaults)
		return NULL;
	typename RES_COLL_DEFAULTS_MAP::iterator mi = myLoadedElemDefaults->find(pcsTag);
	if(mi == myLoadedElemDefaults->end())
		return NULL;
	else
		return mi->second;
}
/********************************************************************************************/
struct SLogicalBlock
{
	SLogicalBlock() { myCumulResult = true; myNextBoolOp = BoolOpLastPlaceholder; myInvertResult = false; }
	void acceptNewResult(bool bRes)
	{
		if(myInvertResult)
			bRes = !bRes;

		if(myNextBoolOp == BoolOpAnd)
			myCumulResult = myCumulResult && bRes;
		else if(myNextBoolOp == BoolOpOr)
			myCumulResult = myCumulResult || bRes;
		else if(myNextBoolOp == BoolOpLastPlaceholder)
			myCumulResult = bRes;
		ELSE_ASSERT;

		myNextBoolOp = BoolOpLastPlaceholder;
	}
	bool myCumulResult;
	bool myInvertResult;
	BoolOpType myNextBoolOp;
};
typedef vector < SLogicalBlock > TLogicalBlocks;

template < class KEY_TYPE, class STORAGE_TYPE >
bool TResourceCollection<KEY_TYPE, STORAGE_TYPE>::evaluatePreprocessorCondition(string& strCondition, TStringSet* pOptExtraDefinesSet)
{
	TStringVector rTokens;
	TokenizeUtils::tokenizeStringToCharPtrsInPlace(strCondition.c_str(), " \t", "()!", rTokens);

	SLogicalBlock *pLastBlock;

	TLogicalBlocks rLogicBlocks;
	SLogicalBlock rTempBlock;
	rLogicBlocks.push_back(rTempBlock);

	bool bPrevClauseResult;

	const char* pcsToken;
	int iToken, iNumTokens = rTokens.size();
	for(iToken = 0; iToken < iNumTokens; iToken++)
	{
		// See what it is
		pcsToken = rTokens[iToken].c_str();
		if(IS_STRING_EQUAL(pcsToken, PREPROC_DEFINED_TOKEN))
		{
			// Do nothing - but in the future, specifies the operation for non-token values.
			// For now always assumed to be the "defined" operation
		}
		else if(IS_STRING_EQUAL(pcsToken, g_pcsBoolOpTypeStrings[BoolOpAnd]))
		{
			if(rLogicBlocks.size() > 0)
			{
				pLastBlock = &rLogicBlocks[rLogicBlocks.size() - 1];
				pLastBlock->myNextBoolOp = BoolOpAnd;
			}
		}
		else if(IS_STRING_EQUAL(pcsToken, g_pcsBoolOpTypeStrings[BoolOpOr]))
		{
			if(rLogicBlocks.size() > 0)
			{
				pLastBlock = &rLogicBlocks[rLogicBlocks.size() - 1];
				pLastBlock->myNextBoolOp = BoolOpOr;
			}
		}
		else if(IS_STRING_EQUAL(pcsToken, "!"))
		{
			if(rLogicBlocks.size() > 0)
			{
				pLastBlock = &rLogicBlocks[rLogicBlocks.size() - 1];
				pLastBlock->myInvertResult = !pLastBlock->myInvertResult;
			}
		}
		else if(IS_STRING_EQUAL(pcsToken, "("))
		{
			// New block!
			SLogicalBlock rNewBlock;
			rLogicBlocks.push_back(rNewBlock);
		}
		else if(IS_STRING_EQUAL(pcsToken, ")"))
		{
			// Pop block
			bool bBlockRes = false;
			if(rLogicBlocks.size() > 0)
			{
				pLastBlock = &rLogicBlocks[rLogicBlocks.size() - 1];
				bBlockRes = pLastBlock->myCumulResult;
				rLogicBlocks.erase(rLogicBlocks.begin() + rLogicBlocks.size() - 1);
			}

			if(rLogicBlocks.size() > 0)
			{
				pLastBlock = &rLogicBlocks[rLogicBlocks.size() - 1];
				pLastBlock->acceptNewResult(bBlockRes);
			}
		}
		else
		{
			// Treat it as a token to evaluate directly
			bPrevClauseResult = ResourceManager::getInstance()->getIsVariableDefined(pcsToken, pOptExtraDefinesSet);

			if(rLogicBlocks.size() > 0)
			{
				pLastBlock = &rLogicBlocks[rLogicBlocks.size() - 1];
				pLastBlock->acceptNewResult(bPrevClauseResult);
			}
		}
	}
	if(rLogicBlocks.size() > 0)
	{
		pLastBlock = &rLogicBlocks[rLogicBlocks.size() - 1];
		return pLastBlock->myCumulResult;
	}
	else
		return false;
}
/********************************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
int TResourceCollection<KEY_TYPE, STORAGE_TYPE>::processIfdefBlock(TStringVector& rLines, int iStartLine, int iEndLine, bool bIsEnabled, bool bEraseLastLine)
{
	// Now, if the condition is true, we still need to remove the start and end lines where the block is defined
	int iResLine = iEndLine;
	if(bIsEnabled)
	{
		// Remove just one line
		rLines.erase(rLines.begin() + iStartLine);
		iResLine--;
		if(bEraseLastLine)
		{
			// -1 because we just shrank the text file by one
			// above when erasing starting line
			rLines.erase(rLines.begin() + iEndLine - 1);
			iResLine--;
		}
	}
	else
	{
		rLines.erase(rLines.begin() + iStartLine, rLines.begin() + iEndLine + (bEraseLastLine ? 1 : 0));
		if(bEraseLastLine)
			iResLine = iStartLine - 1;
		else
			iResLine = iStartLine;
	}

	return iResLine;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceCollection<KEY_TYPE, STORAGE_TYPE>::sortItems(KEY_TYPE eProperty, bool bAscending, vector <TResourceItem <KEY_TYPE, STORAGE_TYPE>* >& vecOut)
{
	itemsToVector(vecOut);
	std::sort(vecOut.begin(), vecOut.end(), ResItemChildSorter<KEY_TYPE, STORAGE_TYPE>(eProperty, bAscending));
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceCollection<KEY_TYPE, STORAGE_TYPE>::extractItem(int iIndex)
{
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pRes = getItem(iIndex);
	if(!pRes)
		return NULL;

	KEY_TYPE tIdProp = ResourceProperty::getIdPropertyValue((KEY_TYPE)0);
	const CHAR_TYPE* pcsId = pRes->getStringProp(tIdProp);
	typename RES_COLL_DEFAULTS_MAP::iterator mi = myItems.find(pcsId);
	if(mi == myItems.end())
		ASSERT_RETURN_NULL;

	myCacher.clear(pcsId);
	myItems.erase(mi);

	return pRes;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
TResourceItem<KEY_TYPE, STORAGE_TYPE>* TResourceCollection<KEY_TYPE, STORAGE_TYPE>::addItem(const char* pcsId, const char* pcsTag)
{
	TResourceItem<KEY_TYPE, STORAGE_TYPE>* pNewItem = new TResourceItem<KEY_TYPE, STORAGE_TYPE>();
	pNewItem->setStringProp(ResourceProperty::getIdPropertyValue((KEY_TYPE)0), pcsId);
	pNewItem->setTag(pcsTag);
	myItems[pcsId] = pNewItem;
	//mySeqItems.push_back(pNewItem);
	return pNewItem;
}
/*****************************************************************************/
template < class KEY_TYPE, class STORAGE_TYPE >
void TResourceCollection<KEY_TYPE, STORAGE_TYPE>::addExistingItem(TResourceItem<KEY_TYPE, STORAGE_TYPE>* pItem)
{
	const char* pcsType = pItem->getStringProp(ResourceProperty::getIdPropertyValue((KEY_TYPE)0));
	_ASSERT(pcsType && strlen(pcsType) > 0);
	myItems[pcsType] = pItem;
}
/*****************************************************************************/
template class TResourceCollection<PropertyType, PropertyType>;
template class TResourceCollection<const char*, RESOURCEITEM_STRING_TYPE >;
/*****************************************************************************/
};