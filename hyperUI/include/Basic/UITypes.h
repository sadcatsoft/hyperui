#pragma once

#define MAIN_FONT_TINY						"ftMainFontTiny"
#define MAIN_FONT_TINY_DEF_SIZE				upToScreen(5)
#define MAIN_FONT_SMALL						"ftMainFontSmall"
#define MAIN_FONT_SMALL_DEF_SIZE			upToScreen(6.5)
#define MAIN_FONT_MED						"ftMainFontMed"
#define MAIN_FONT_MED_DEF_SIZE				upToScreen(8)
#define MAIN_FONT_LARGE						"ftMainFontLarge"
#define MAIN_FONT_LARGE_DEF_SIZE			upToScreen(10)

// UI Actions
// First we have standard actions, which rely on action value
// being present to determine what to do.
#define	UIA_SHOW_LAYER						"showLayer"
#define	UIA_HIDE_LAYER						"hideLayer"
#define	UIA_HIDE_ELEMENT					"hideElement"
#define	UIA_HIDE_TOP_PARENT					"hideTopParent"
#define	UIA_HIDE_ALL						"hideAllLayers"
#define	UIA_HIDE_LAYER_SHOW_LAYER			"hideLayerShowLayer"
#define	UIA_HIDE_LAST_SHOW_LAYER			"hideLastShowLayer"
#define	UIA_SHOW_LAYER_EXCLUSIVE			"showLayerExclusive"
#define	UIA_SLIDE_NEXT						"slideNext"
#define UIA_SLIDE_PREVIOUS					"slidePrevious"
#define UIA_TOGGLE_LAYER					"toggleLayer"
#define UIA_SHOW_MESSAGE					"showMessage"
#define UIA_OPEN_FILE						"openFile"
#define UIA_OPEN_FOLDER						"openFolder"
#define UIA_UNDO							"undo"
#define UIA_REDO							"redo"
#define UIA_QUIT_APP						"quitApp"
#define UIA_ACCEPT_COLOR_FROM_PICKER		"acceptNewColorFromPicker"
#define UIA_CANCEL_COLOR_PICKER				"cancelColorPicker"
#define UIA_SHOW_POPUP						"showPopup"