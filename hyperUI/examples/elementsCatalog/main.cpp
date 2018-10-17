#include "stdafx.h"
#include "TopSplitterElement.h"

/*****************************************************************************/
void initialize()
{
	HyperUI::initialize("HyperUI_SimpleWindow");

	ResourceManager::setStandardDataPathPrefix("hyperUI");

	REGISTER_CUSTOM_UI_ELEMENT(TopSplitterElement);

	// Add our own graphics collection, in addition to the default, built-in one.
	// This allows us to use our own bitmaps. Note the last argument, which is
	// actually a relative path to the folder where the image files themselves are.
	ResourceManager::getInstance()->addCollection("customUI" FOLDER_SEP "resGraphics.txt", true, CollectionRoleGraphics, "customGraphics");

	// Set some window parameters
	HyperUI::SWindowParms rParms;
	rParms.myWidth = 1024;
	rParms.myHeight = 768;
	rParms.myInitLayerToShow = "firstLayer";
    rParms.myTitle = "UI Elements Catalog";

	// This tells us which UI collection to use in this window.
	rParms.myInitCollectionType = ResourceManager::getInstance()->addCollection("customUI" FOLDER_SEP "resUI.txt", true);

	// Run it!
	HyperUI::run(rParms);
}
/*****************************************************************************/
