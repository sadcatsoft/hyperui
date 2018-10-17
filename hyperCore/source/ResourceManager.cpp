#include "stdafx.h"

namespace HyperCore
{
ResourceManager* ResourceManager::theInstance = NULL;
string ResourceManager::theAppPath;
string ResourceManager::theStandardDataPathPrefix;
/*****************************************************************************/
ResourceManager::ResourceManager()
{
	myVarDefCallback = NULL;
	myCollections.resize(2);
}
/*****************************************************************************/
ResourceManager::~ResourceManager()
{
}
/*****************************************************************************/
ResourceManager* ResourceManager::getInstance()
{
	if(!theInstance)
		theInstance = new ResourceManager;
	return theInstance;
}
/*****************************************************************************/
bool ResourceManager::getAdditionalPathOverrides(ResourceType eResType, string& strOut)
{
	if(myCollections[eResType].myIsFullNameSpecified)
		return false;

#ifdef MAC_BUILD
	// See if there is an overrides file specifically for the mac build
	strOut = getFullAppPath();
	strOut += ResourceManager::getStandardDataPathPrefix();
	strOut += RESOURCE_PREFIX_PATH;
	strOut += FOLDER_SEP;
	strOut += myCollections[eResType].myFileName;
	strOut += RESOURCE_MAC_SUFFIX;
	strOut += RESOURCE_FILE_EXT;

	if(FileUtils::doesFileExist(strOut.c_str(), FileSourceFullPath))
		return true;

	return false;

#else
	strOut = "";
	return false;
#endif
}
/*****************************************************************************/
bool ResourceManager::getProPathOverrides(ResourceType eResType, string& strOut)
{
	if(myCollections[eResType].myIsFullNameSpecified)
		return false;

#ifdef PRO_VERSION
	// See if there is an overrides file specifically for the pro build
	strOut = getFullAppPath();
	strOut += ResourceManager::getStandardDataPathPrefix();
	strOut += RESOURCE_PRO_PREFIX_PATH;
	strOut += FOLDER_SEP;
	strOut += myCollections[eResType].myFileName;
	if(!myCollections[eResType].myIsFullNameSpecified)
	{
		strOut += RESOURCE_PRO_SUFFIX;
		strOut += RESOURCE_FILE_EXT;
	}

	if(FileUtils::doesFileExist(strOut.c_str(), FileSourceFullPath))
		return true;

	return false;
#else
	strOut = "";
	return false;
#endif
}

/*****************************************************************************/
void ResourceManager::getDensityFileSuffix(STRING_TYPE& strOut)
{
	strOut = "";
	int iDensityMult = getScreenDensityScalingFactor();
	StringUtils::numberToString(iDensityMult, strOut);
	strOut = "_x" + strOut;
}
/*****************************************************************************/
bool ResourceManager::getResourcePathResSpecific(ResourceType eResType, string& strOut)
{
	if(myCollections[eResType].myIsFullNameSpecified)
		return false;

	strOut = getFullAppPath();
	strOut += ResourceManager::getStandardDataPathPrefix();
	strOut += RESOURCE_PREFIX_PATH;
	strOut += FOLDER_SEP;
	strOut += myCollections[eResType].myFileName;
	STRING_TYPE strResSuffix;
	ResourceManager::getDensityFileSuffix(strResSuffix);
	strOut += strResSuffix;
	strOut += RESOURCE_FILE_EXT;

	// First, try a resolution-specific file.
	if(FileUtils::doesFileExist(strOut.c_str(), FileSourceFullPath))
	{
		//gLog("ResourceManager::getResourcePathResSpecific: %s exists!\n", strOut.c_str());
		return true;
	}

	//gLog("ResourceManager::getResourcePathResSpecific: %s doesn't exist!\n", strOut.c_str());
	return false;
}
/*****************************************************************************/
const char* ResourceManager::getFullAppPath()
{
	if(theAppPath.length() == 0)
	{
		getAppStartupPath(theAppPath);
		theAppPath += FOLDER_SEP;
	}
	return theAppPath.c_str();
}
/*****************************************************************************/
void ResourceManager::getRelativeFile(const char* pcsBaseFileName, const char* pcsFileName, string& strOut)
{
	// Input: a full or relative path to the base file, including the file name. Can be just the filename.
	// The filename of the target file that should be treated as relative to the parent folder of the 
	// base file.
	if(IS_VALID_STRING_AND_NOT_NONE(pcsBaseFileName))
	{
		PathUtils::extractPathFromFullFilename(pcsBaseFileName, strOut);
		PathUtils::ensureEndsWithFolderSep(strOut);
		strOut += pcsFileName;
	}
	else
	{
		// If not specified, we default to our standard location
		strOut = getFullAppPath();
		strOut += ResourceManager::getStandardDataPathPrefix();
		strOut += RESOURCE_PREFIX_PATH;
		strOut += FOLDER_SEP;
		strOut += pcsFileName;
	}	
}
/*****************************************************************************/
void ResourceManager::getResourcePathGeneric(ResourceType eResType, string& strOut)
{
	strOut = EMPTY_STRING;
	if(!FileUtils::getIsPathAbsolute(myCollections[eResType].myFileName.c_str()))
	{
		// Now try the generic file
		strOut = getFullAppPath();
		strOut += ResourceManager::getStandardDataPathPrefix();
		strOut += RESOURCE_PREFIX_PATH;
		strOut += FOLDER_SEP;
	}
	strOut += myCollections[eResType].myFileName;
	if(!myCollections[eResType].myIsFullNameSpecified)
		strOut += RESOURCE_FILE_EXT;
}
/*****************************************************************************/
void ResourceManager::getResourcePath(ResourceType eResType, string& strOut)
{
	if(getResourcePathResSpecific(eResType, strOut))
		return;

	getResourcePathGeneric(eResType, strOut);
}
/*****************************************************************************/
void ResourceManager::loadSingleCollection(ResourceType eType, ResourceCollection* pOptTargetIn)
{
	string strFullPath;

	// See if we need to postpone loading
	// 		if(g_bResourceCollectionPostponeLoading[eType] == bIsLoadingPrelimStage)
	// 			continue;

#if defined(_DEBUG) && defined(PARANOID_CACHE_CHECKS)
	///pCurrColl->checkCacheConsistency();
#endif

	// We always load a generic file first, and then see
	// if there are any overrides
#ifdef _DEBUG
	if(eType == 201)
	{
		int bp = 0;
	}
#endif
	getResourcePathGeneric((ResourceType)eType, strFullPath);
	if(!FileUtils::doesFileExist(strFullPath.c_str(), FileSourceFullPath))
	{
		gLog("%s FAILED (file not found)\n", strFullPath.c_str());
		return;
	}

	ResourceCollection rOverridesColl;
	ResourceCollection *pCurrColl = pOptTargetIn;

	if(!pCurrColl)
	{
		myCollections[eType].myCollection = new ResourceCollection();
		pCurrColl = myCollections[eType].myCollection;
	}

	gLog("Loading %s index = %d... ", myCollections[eType].myFileName.c_str(), eType);

	bool bLoadResult = pCurrColl->loadFromFile(strFullPath.c_str(), FileSourceFullPath, NULL);
	gLog(bLoadResult ? "success\n" : "FAILED\n");
	//pCurrColl->loadFromFile(strFullPath.c_str(), FileSourcePackageDir, NULL);
#if defined(_DEBUG) && defined(PARANOID_CACHE_CHECKS)
	pCurrColl->checkCacheConsistency();
#endif
	if(getResourcePathResSpecific((ResourceType)eType, strFullPath))
	{
		// We've got overrides. Load them.
		rOverridesColl.clear();
		// We need to copy the defaults from the collection being merged into...
		rOverridesColl.copyDefaultsFrom(pCurrColl);
		rOverridesColl.loadFromFile(strFullPath.c_str(), FileSourceFullPath, NULL, true);

		// Merge them into the collection.
		pCurrColl->mergeFrom(rOverridesColl);
	}
#if defined(_DEBUG) && defined(PARANOID_CACHE_CHECKS)
	pCurrColl->checkCacheConsistency();
#endif
	if(getAdditionalPathOverrides((ResourceType)eType, strFullPath))
	{
		// We've got overrides. Load them.
		rOverridesColl.clear();
		// We need to copy the defaults from the collection being merged into...
		rOverridesColl.copyDefaultsFrom(pCurrColl);
		rOverridesColl.loadFromFile(strFullPath.c_str(), FileSourceFullPath, NULL, true);

		// Merge them into the collection.
		pCurrColl->mergeFrom(rOverridesColl);
	}
#if defined(_DEBUG) && defined(PARANOID_CACHE_CHECKS)
	pCurrColl->checkCacheConsistency();
#endif

	if(getProPathOverrides((ResourceType)eType, strFullPath))
	{
		// We've got overrides. Load them.
		rOverridesColl.clear();
		// We need to copy the defaults from the collection being merged into...
		rOverridesColl.copyDefaultsFrom(pCurrColl);
		rOverridesColl.loadFromFile(strFullPath.c_str(), FileSourceFullPath, NULL, true);

		// Merge them into the collection.
		pCurrColl->mergeFrom(rOverridesColl);
	}
#if defined(_DEBUG) && defined(PARANOID_CACHE_CHECKS)
	pCurrColl->checkCacheConsistency();
#endif

	if(!pOptTargetIn)
		pCurrColl->setDataSourceTypeForAllItems(SourceConst);
}
/*****************************************************************************/
void ResourceManager::ensureCollectionLoaded(ResourceType eType)
{
	if(eType < 0 || eType >= myCollections.size())
		return; 

	if(myCollections[eType].myCollection)
		return;

	// Otherwise, load it:
	this->loadSingleCollection(eType);
}
/*****************************************************************************/
void ResourceManager::clearPreprocessorCaches()
{
	clearAndDeleteContainer(myIncludesMap);
}
/*****************************************************************************/
TStringVector* ResourceManager::getCachedIncludeFile(const char* pcsFileName)
{
	TStringCachedPreprocessorIncludeMap::iterator mi = myIncludesMap.find(pcsFileName);
	if(mi == myIncludesMap.end())
		return NULL;
	else
		return &mi->second->myContents;
}
/*****************************************************************************/
void ResourceManager::setCachedIncludeFile(const char* pcsFileName, TStringVector& rParsedContents)
{
	SCachedPreprocessorInclude* pNewContent = new SCachedPreprocessorInclude;
	pNewContent->myContents = rParsedContents;
	myIncludesMap[pcsFileName] = pNewContent;
}
/*****************************************************************************/
void ResourceManager::setOverrideAppPathName(const CHAR_TYPE* pcsPath)
{
	if(pcsPath)
		theAppPath = pcsPath;
	else
		theAppPath = EMPTY_STRING;

	if(theAppPath.length() > 0 && theAppPath[theAppPath.length() - 1] != FOLDER_SEP_CHAR)
		theAppPath += FOLDER_SEP;
}
/*****************************************************************************/
ResourceType ResourceManager::addCollection(const CHAR_TYPE* pcsStringValue, bool bRelativeToExecutable, CollectionRoleType eRole, const CHAR_TYPE* pcsTargetDataFolder)
{
	ResourceType eResCollType = myCollections.size();
	if(bRelativeToExecutable)
	{
		STRING_TYPE strFullPath;
		ResourceManager::getFullPathFor(pcsStringValue, strFullPath);
		STRING_TYPE strTargetFolderFullPath;
		if(pcsTargetDataFolder)
			ResourceManager::getFullPathFor(pcsTargetDataFolder, strTargetFolderFullPath);
		this->addCollection(eResCollType, strFullPath.c_str(), true, eRole, pcsTargetDataFolder ? strTargetFolderFullPath.c_str() : NULL);
	}
	else
		this->addCollection(eResCollType, pcsStringValue, true, eRole, pcsTargetDataFolder);

	return eResCollType;
}
/*****************************************************************************/
void ResourceManager::addCollection(ResourceType eCollType, const CHAR_TYPE* pcsStringValue, bool bLeaveFilenameUnchanged, CollectionRoleType eRole, const CHAR_TYPE* pcsTargetDataFolder)
{
	gLog("Adding collection %d with %s\n", (int)eCollType, pcsStringValue);
	if(eCollType < myCollections.size() && myCollections[eCollType].myCollection)
	{
		// Due to DLL loading, we may try to register collections again after they've
		// been loaded. Prevent us from doing so.
		gLog("Skipping since it's non-null %x\n", myCollections[eCollType].myCollection);
		return;
	}

	SCollectionDef rNewDef(pcsStringValue, eCollType, bLeaveFilenameUnchanged, eRole, pcsTargetDataFolder);
	if(myCollections.size() <= (int)eCollType)
	{
		gLog("Resizing collections registry to %d\n", (int)eCollType*2);
		myCollections.resize((int)eCollType*2);
	}

	myCollections[rNewDef.myType] = rNewDef;
	if(eRole != CollectionRoleLastPlaceholder)
		myRoleCollections[eRole][rNewDef.myType] = rNewDef;
}
/*****************************************************************************/
bool ResourceManager::getIsVariableDefined(const char* pcsVar, TStringSet* pOptExtraDefinesSet)
{
	if(pOptExtraDefinesSet && pOptExtraDefinesSet->find(pcsVar) != pOptExtraDefinesSet->end())
		return true;

	if(myVarDefCallback && myVarDefCallback->getIsVariableDefined(pcsVar))
		return true;

	return false;
}
/*****************************************************************************/
void ResourceManager::resetAllContent()
{
	this->clearPreprocessorCaches();
	ResourceCollection::resetUniqueIdCounterDebug();

	int iColl;
	for(iColl = 0; iColl < myCollections.size(); iColl++)
	{
		if(!myCollections[iColl].myCollection)
			continue;

		// Otherwise, load into a temp collection and then merge in
		ResourceCollection rTempColl;
		this->loadSingleCollection(myCollections[iColl].myType, &rTempColl);
		myCollections[iColl].myCollection->mergeFrom(rTempColl);
	}
}
/********************************************************************************************/
SCollectionAdder::SCollectionAdder(const CHAR_TYPE* pcsToken, int iValue, CollectionRoleType eRole)
{
	static const int iSkipFirstChars = STRLEN(STR_LIT("Resource"));
	ResourceManager::getInstance()->addCollection((ResourceType)iValue, pcsToken + iSkipFirstChars, false, eRole);
}
/*****************************************************************************/
void ResourceManager::getFullPathFor(const CHAR_TYPE* pcsRelativeFilePath, STRING_TYPE& strOut)
{
	strOut = ResourceManager::getFullAppPath();
	strOut += pcsRelativeFilePath;
}
/*****************************************************************************/
const char* ResourceManager::getStandardDataPathPrefix()
{
	return theStandardDataPathPrefix.c_str();
}
/*****************************************************************************/
void ResourceManager::setStandardDataPathPrefix(const char* pcsPrefix)
{
	if(IS_VALID_STRING(pcsPrefix))
	{
		theStandardDataPathPrefix = pcsPrefix;
		PathUtils::ensureEndsWithFolderSep(theStandardDataPathPrefix);
	}
	else
		theStandardDataPathPrefix = "";
}
/*****************************************************************************/
TCollectionDefMap* ResourceManager::getCollectionsForRole(CollectionRoleType eRole)
{
	return &myRoleCollections[eRole];
}
/*****************************************************************************/
};
