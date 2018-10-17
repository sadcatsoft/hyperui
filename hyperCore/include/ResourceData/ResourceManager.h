#pragma once

#define CORE_COLLECTIONS_START_VALUE		256
#define ResourceTypeNone                    -1

/********************************************************************************************/
enum CollectionRoleType
{
	CollectionRoleGraphics = 0,
	CollectionRoleUI,

	CollectionRoleLastPlaceholder
};

const CHAR_TYPE* const g_pcsCollectionRoleTypeStrings[CollectionRoleLastPlaceholder + 1] =
{
	"Graphics",
	"UI",

	NULL
};
/********************************************************************************************/
struct HYPERCORE_API SCollectionAdder
{
	SCollectionAdder(const CHAR_TYPE* pcsToken, int iValue, CollectionRoleType eRole = CollectionRoleLastPlaceholder);
};

#define DECLARE_COLLECTION(collectionName) \
	const int collectionName = (__LINE__); \
	static const HyperCore::SCollectionAdder collectionName##_adder = HyperCore::SCollectionAdder(#collectionName, (__LINE__));

#define DECLARE_COLLECTION_EXPLICIT(collectionName, iValue) \
	const int collectionName = (iValue); \
	static const HyperCore::SCollectionAdder collectionName##_adder = HyperCore::SCollectionAdder(#collectionName, (iValue));

#define DECLARE_COLLECTION_EXPLICIT_WITH_ROLE(collectionName, iValue, eRole) \
	const int collectionName = (iValue); \
	static const HyperCore::SCollectionAdder collectionName##_adder = HyperCore::SCollectionAdder(#collectionName, (iValue), (eRole));
/********************************************************************************************/
//DECLARE_COLLECTION_EXPLICIT(ResourceObjects, CORE_COLLECTIONS_START_VALUE + 0);
/********************************************************************************************/
struct SCollectionDef
{
	SCollectionDef()
	{
		myCollection = NULL;
		reset();
	}

	SCollectionDef(const CHAR_TYPE* pcsName, ResourceType eType, bool bLeaveFilenameUnchanged = false, CollectionRoleType eRole = CollectionRoleLastPlaceholder, const CHAR_TYPE* pcsTargetDataFolder = NULL)
	{
		myType = eType;
		if(!bLeaveFilenameUnchanged)
			myFileName = "res";
		myFileName += pcsName;
		myCollection = NULL;
		myIsFullNameSpecified = bLeaveFilenameUnchanged;
		myRole = eRole;

		if(pcsTargetDataFolder)
			myTargetDataFolderPath = pcsTargetDataFolder;
		else
			myTargetDataFolderPath = EMPTY_STRING;
	}

	~SCollectionDef()
	{
		reset();
	}

	inline void reset()
	{
		delete myCollection;
		myCollection = NULL;

		myType = ResourceTypeNone;
		myIsFullNameSpecified = false;
		myRole = CollectionRoleLastPlaceholder;
		myTargetDataFolderPath = EMPTY_STRING;
	}

	ResourceType myType;
	ResourceCollection *myCollection;

	// Without the extension
	STRING_TYPE myFileName;
	STRING_TYPE myTargetDataFolderPath;
	bool myIsFullNameSpecified;

	CollectionRoleType myRole;
};
typedef vector < SCollectionDef > TCollectionDefVector;
typedef map < ResourceType, SCollectionDef > TCollectionDefMap;
/********************************************************************************************/
struct SCachedPreprocessorInclude
{
	TStringVector myContents;
};
typedef map < string, SCachedPreprocessorInclude* > TStringCachedPreprocessorIncludeMap;
/********************************************************************************************/
class IVariableDefinedCheckCallback
{
public:
	~IVariableDefinedCheckCallback() { }
	virtual bool getIsVariableDefined(const char* pcsVar) = 0;
};
/********************************************************************************************/
class HYPERCORE_API ResourceManager 
{
public:
	static ResourceManager* getInstance();
	~ResourceManager();

	void setCallback(IVariableDefinedCheckCallback* pCallback) { myVarDefCallback = pCallback; }

	inline const ResourceCollection* getCollection(ResourceType eType) const 
	{ 
		const_cast<ResourceManager*>(this)->ensureCollectionLoaded(eType); 
		if(eType < 0 || eType >= myCollections.size())
			return NULL;
		else
			return myCollections[eType].myCollection; 
	}

	inline ResourceCollection* getCollection(ResourceType eType) 
	{ 
		ensureCollectionLoaded(eType); 
		if(eType < 0 || eType >= myCollections.size())
			return NULL;
		else
			return myCollections[eType].myCollection; 
	}

	// Do not move this to protected, it's used by the UI editor.
	void getResourcePath(ResourceType eResType, string& strOut);

	static void getRelativeFile(const char* pcsBaseFileName, const char* pcsFileName, string& strOut);

	static const char* getFullAppPath();
	static const char* getStandardDataPathPrefix();
	static void setStandardDataPathPrefix(const char* pcsPrefix);
	static void getFullPathFor(const CHAR_TYPE* pcsRelativeFilePath, STRING_TYPE& strOut);

	TStringVector* getCachedIncludeFile(const char* pcsFileName);
	void setCachedIncludeFile(const char* pcsFileName, TStringVector& rParsedContents);

	ResourceType addCollection(const CHAR_TYPE* pcsStringValue, bool bRelativeToExecutable = false, CollectionRoleType eRole = CollectionRoleLastPlaceholder, const CHAR_TYPE* pcsTargetDataFolder = NULL);
	void addCollection(ResourceType eCollType, const CHAR_TYPE* pcsStringValue, bool bLeaveFilenameUnchanged = false, CollectionRoleType eRole = CollectionRoleLastPlaceholder, const CHAR_TYPE* pcsTargetDataFolder = NULL);
	bool getIsVariableDefined(const char* pcsVar, TStringSet* pOptExtraDefinesSet = NULL);

	static void getDensityFileSuffix(STRING_TYPE& strOut);

	TCollectionDefMap* getCollectionsForRole(CollectionRoleType eRole);

//protected:

	bool getResourcePathResSpecific(ResourceType eResType, string& strOut);
	void getResourcePathGeneric(ResourceType eResType, string& strOut);
	bool getAdditionalPathOverrides(ResourceType eResType, string& strOut);
	bool getProPathOverrides(ResourceType eResType, string& strOut);
	void loadCollectionsData(bool bIsLoadingPrelimStage);

	void clearPreprocessorCaches();

	static void setOverrideAppPathName(const CHAR_TYPE* pcsPath);

	void resetAllContent();

private:
	ResourceManager();

	void loadSingleCollection(ResourceType eType, ResourceCollection* pOptTargetIn = NULL);
	void ensureCollectionLoaded(ResourceType eType);

private:

	static string theAppPath;
	static string theStandardDataPathPrefix;
	static ResourceManager* theInstance;

	TStringCachedPreprocessorIncludeMap myIncludesMap;
	TCollectionDefVector myCollections;
	IVariableDefinedCheckCallback* myVarDefCallback;

	TCollectionDefMap myRoleCollections[CollectionRoleLastPlaceholder];
};
/********************************************************************************************/

