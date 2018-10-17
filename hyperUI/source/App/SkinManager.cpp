#include "stdafx.h"

#define SKINS_FOLDER			"skins"
#define SKIN_FILENAME			"skin.def"
#define SKIN_COLOR_MAP_NAME		"textColorMap"

// TODO: Make textures use these
//const char* const g_pcsDensityMultPrefixesNew[g_iMaxDensityPrefix] = { "1x", "2x" };
const char* const g_pcsDensityMultPrefixesNew[] = { "1x", "2x" };

namespace HyperUI
{
SkinManager* SkinManager::theInstance = NULL;
/*****************************************************************************/
SkinManager::SkinManager()
{
	myCurrentSkin = NULL;
	init();

	// See what the initial theme is on load:
	const char* pcsTheme = PROPERTY_NONE;
	ResourceItem* pSettingItem = SettingsCollection::getInstance()->getSettingsItem();
	if(pSettingItem->doesPropertyExist(PropertySetTheme))
		setCurrentSkin(pSettingItem->getStringProp(PropertySetTheme));

#ifdef _DEBUG
//	setCurrentSkin("Light");
#endif
}
/*****************************************************************************/
void SkinManager::init()
{
	// Get the folder, see all subfolders
	string strSkinsPath;
	strSkinsPath = ResourceManager::getFullAppPath();
	strSkinsPath += SKINS_FOLDER;

	clearAndDeleteContainer(mySkins);

	string strSkinFilename;
	string strTemp, strCollName;

	int iChild, iNumChildren;

	string strSkinFolderPath;

	ResourceItem* pChild;
	ResourceItem* pDefItem;
	ResourceCollection *pResColl;
	TStringVector vecSubfolders;
	FileUtils::listSubfoldersOf(strSkinsPath.c_str(), vecSubfolders);
	int iCurr, iNum = vecSubfolders.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		// Generate the skin file name
		strSkinFilename = vecSubfolders[iCurr];
		strSkinFilename += FOLDER_SEP;
		strSkinFolderPath = strSkinFilename;
		strSkinFilename += SKIN_FILENAME;

		pResColl = new ResourceCollection;
		if(!pResColl->loadFromFile(strSkinFilename.c_str(), FileSourceFullPath) || pResColl->getNumItems() == 0)
		{
			Logger::log(LogLevelError, "Could not load skin at %s\n", strSkinFilename.c_str());
			delete pResColl;
			continue;
		}

		pDefItem = pResColl->getItemById(SKIN_DEF_ITEM_NAME);
		if(!pDefItem || !pDefItem->doesPropertyExist(PropertyName))
		{
			Logger::log(LogLevelError, "Skin at %s needs a " SKIN_DEF_ITEM_NAME " item with description\n", strSkinFilename.c_str());
			delete pResColl;
			continue;
		}

		strCollName = pDefItem->getStringProp(PropertyName);
		StringUtils::ensureValidForAlphaNumMap(strCollName);
		mySkins[strCollName] = pResColl;
		pDefItem->setStringProp(PropertyOldId, strCollName.c_str());

		pDefItem->setStringProp(PropertyPath, strSkinFolderPath.c_str());

		// Go through all the items and set mangled anim names
		iNumChildren = pResColl->getNumItems();
		for(iChild = 0; iChild < iNumChildren; iChild++)
		{
			pChild = pResColl->getItem(iChild);
			if(!pChild->doesPropertyExist(PropertyFile))
				continue;

			strTemp = pChild->getStringProp(PropertyFile);
			strTemp = strCollName + "_" + strTemp;
			StringUtils::ensureValidForAlphaNumMap(strTemp);
			pChild->setStringProp(PropertySvPTAnimName, strTemp.c_str());
		}
	}
}
/*****************************************************************************/
SkinManager* SkinManager::getInstance()
{
	if(!theInstance)
		theInstance = new SkinManager;
	return theInstance;
}
/*****************************************************************************/
SkinManager::~SkinManager()
{
	clearAndDeleteContainer(mySkins);
}
/*****************************************************************************/
void SkinManager::onTextureInit(TextureManager* pTexManager, const char* pcsAnimName, const ResourceItem* pAnimItem, bool bIsPVR, BlendModeType eBlendMode)
{
	int iDensityMult = getScreenDensityScalingFactor();

	// Now, go over all our collections and see if any specify alternative
	string strFullPath;
	ResourceItem* pResOverride;
	ResourceCollection* pColl;
	TStringResourceCollectionsMap::iterator mi;
	for(mi = mySkins.begin(); mi != mySkins.end(); mi++)
	{
		// Find the child with this name:
		pColl = mi->second;
		pResOverride = pColl->getItemById(pcsAnimName);

		if(!pResOverride)
			continue;

		strFullPath = pColl->getItemById(SKIN_DEF_ITEM_NAME)->getStringProp(PropertyPath);
		STRING_TYPE strSuffix;
		ResourceManager::getDensityFileSuffix(strSuffix);
		strFullPath += pResOverride->getStringProp(PropertyFile);
		strFullPath += strSuffix;
		strFullPath += ".";
		strFullPath += pAnimItem->getStringProp(PropertyFormat);

		pTexManager->initAnimation(pResOverride->getStringProp(PropertySvPTAnimName), strFullPath.c_str(), bIsPVR, eBlendMode, true, pAnimItem, NULL);
	}
}
/*****************************************************************************/
void SkinManager::setCurrentSkin(const char* pcsName)
{
	// Attempt to find this skin:
	TStringResourceCollectionsMap::iterator mi = mySkins.find(pcsName);
	if(mi == mySkins.end())
		myCurrentSkin = NULL;
	else
		myCurrentSkin = mi->second;
}
/*****************************************************************************/
const char* SkinManager::mapOrigNameToSkinned(const char* pcsOrigAnimName)
{
	if(!myCurrentSkin)
		return pcsOrigAnimName;

	ResourceItem* pOverride = myCurrentSkin->getItemById(pcsOrigAnimName);
	if(!pOverride)
		return pcsOrigAnimName;

	return pOverride->getStringProp(PropertySvPTAnimName);
}
/*****************************************************************************/
void SkinManager::mapOverlayColor(const char* pcsMangledAnimName, SColor& scolInOut)
{
#ifdef _DEBUG
	if(IS_STRING_EQUAL(pcsMangledAnimName, "button") || IS_STRING_EQUAL(pcsMangledAnimName, "Light_button"))
	{
		int bp = 0;
	}
#endif

	if(!myCurrentSkin)
		return;

	// Our name here is mangled...
	ResourceItem* pOverride = myCurrentSkin->findItemWithPropValue(PropertySvPTAnimName, pcsMangledAnimName, false);

	//ResourceItem* pOverride = myCurrentSkin->findItemByType(pcsOrigAnimName);
	if(!pOverride)
		return;

	if(pOverride->doesPropertyExist(PropertyOverlayColor) && !pOverride->getIsNone(PropertyOverlayColor))
	{
		// Set it, otherwise, set it to infinity:
		pOverride->getAsColor(PropertyOverlayColor, scolInOut);
	}
	else
		scolInOut.r = FLOAT_TYPE_MAX;
}
/*****************************************************************************/
void SkinManager::mapTextColor(SColor& scolInOut)
{
	if(!myCurrentSkin)
		return;

	ResourceItem* pColorMap = myCurrentSkin->getItemById(SKIN_DEF_ITEM_NAME)->getChildById(SKIN_COLOR_MAP_NAME, false);
	if(!pColorMap)
		return;

	const ResourceItem* pFound = pColorMap->getChildByPropertyValue(PropertySource, scolInOut, false);
	if(!pFound)
		return;

	pFound->getAsColor(PropertyTarget, scolInOut);
}
/*****************************************************************************/
bool SkinManager::getDoAllowTextShadow()
{
	if(!myCurrentSkin)
		return true;

	return myCurrentSkin->getItemById(SKIN_DEF_ITEM_NAME)->getBoolProp(PropertyAllowShadow);
}
/*****************************************************************************/
int SkinManager::getNumSkins()
{
	return mySkins.size();
}
/*****************************************************************************/
const ResourceCollection* SkinManager::getSkin(int iIndex)
{
	TStringResourceCollectionsMap::iterator mi;
	int iCurr;
	for(iCurr = 0, mi = mySkins.begin(); mi != mySkins.end(); mi++, iCurr++)
	{
		if(iCurr == iIndex)
			return mi->second;
	}
	return NULL;
}
/*****************************************************************************/
bool SkinManager::getIsSelected(const char* pcsId)
{
	if(!IS_VALID_STRING_AND_NOT_NONE(pcsId) || IS_STRING_EQUAL(pcsId, SKINS_DEFAULT_ID))
		return myCurrentSkin == NULL;

	TStringResourceCollectionsMap::iterator mi = mySkins.find(pcsId);
	if(mi != mySkins.end() && mi->second == myCurrentSkin)
		return true;
	else
		return false;
}
/*****************************************************************************/
};