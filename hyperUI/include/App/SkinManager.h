#pragma once

#define SKINS_DEFAULT_ID		"__defaultSkin__"
#define SKIN_DEF_ITEM_NAME		"__def__"

class HYPERUI_API TextureManager;
/*****************************************************************************/
class HYPERUI_API SkinManager
{
public:
	static SkinManager* getInstance();
	~SkinManager();

	void onTextureInit(TextureManager* pTexManager, const char* pcsAnimName, const ResourceItem* pAnimItem, bool bIsPVR, BlendModeType eBlendMode);

	void setCurrentSkin(const char* pcsName);

	const char* mapOrigNameToSkinned(const char* pcsOrigAnimName);
	void mapOverlayColor(const char* pcsMangledAnimName, SColor& scolInOut);
	void mapTextColor(SColor& scolInOut);

	bool getDoAllowTextShadow();

	int getNumSkins();
	const ResourceCollection* getSkin(int iIndex);

	bool getIsSelected(const char* pcsId);

private:
	SkinManager();

	void init();

private:

	static SkinManager* theInstance;

	TStringResourceCollectionsMap mySkins;

	ResourceCollection* myCurrentSkin;
};
/*****************************************************************************/