#include "stdafx.h"
#include "CustomWindow.h"

/*****************************************************************************/
void initialize()
{
	HyperUI::initialize("Blank HyperUI App");

	// We need to tell our application where the resources from the library
	// are. In this case, we store them in the subfolder "hyperUI" relative
	// to our main application folder.
	ResourceManager::setStandardDataPathPrefix("hyperUI");

	// Register our own window class. This is necessary to handle any actual
	// custom button presses, other events, messages, etc.
	//REGISTER_MAIN_WINDOW_TYPE(CustomWindow);

	// Set some window parameters
	HyperUI::SWindowParms rParms;
    rParms.myTitle = "Blank HyperUI App";

	// This tells us which UI collection to use in this window.
	rParms.myInitCollectionType = ResourceManager::getInstance()->addCollection("customUI" FOLDER_SEP "resUI.txt", true);

	// Run it!
	HyperUI::run(rParms);
}
/*****************************************************************************/
