#include "stdafx.h"
#include "CustomUIElement.h"
#include "SimpleWindow.h"

/*****************************************************************************/
void initialize()
{
	HyperUI::initialize("HyperUI_SimpleWindow");

	// We need to tell our application where the resources from the library
	// are. In this case, we store them in the subfolder "hyperUI" relative
	// to our main application folder.
	ResourceManager::setStandardDataPathPrefix("hyperUI");

	// Register our own window class. This is necessary to handle any actual
	// custom button presses, other events, messages, etc.
	REGISTER_MAIN_WINDOW_TYPE(SimpleWindow);

	// Optionally, add our own custom UI element. This is not strictly speaking
	// necessary, but for building larger projects, it is often convenient
	// to define custom parent windows for each screen/dialog to handle the event
	// logic for that dialog.
	REGISTER_CUSTOM_UI_ELEMENT(CustomUIElement);

	// Add our own graphics collection, in addition to the default, built-in one.
	// This allows us to use our own bitmaps. Note the last argument, which is
	// actually a relative path to the folder where the image files themselves are.
	ResourceManager::getInstance()->addCollection("customUI" FOLDER_SEP "resGraphics.txt", true, CollectionRoleGraphics, "customGraphics");

	// Set some window parameters
	HyperUI::SWindowParms rParms;
	rParms.myWidth = 1024;
	rParms.myHeight = 768;
	rParms.myMainMenuElementId = "mainMenu";
    rParms.myTitle = "Simple Window";

	// This tells us which UI collection to use in this window.
	rParms.myInitCollectionType = ResourceManager::getInstance()->addCollection("customUI" FOLDER_SEP "resUI.txt", true);

	// Run it!
	HyperUI::run(rParms);
}
/*****************************************************************************/
