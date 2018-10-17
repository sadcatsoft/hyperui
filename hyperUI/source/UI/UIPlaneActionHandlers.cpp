#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
void UIPlane::processAction(UIElement* pElem, const string& strAction, const char* pcsTargetDataSource, PropertyType eActionValue1Prop,
												PropertyType eActionValue2Prop, PropertyType eActionValue3Prop, MouseButtonType eButton)
{
	if(getParentWindow()->processAction(pElem, strAction, pcsTargetDataSource, eActionValue1Prop, eActionValue2Prop, eActionValue3Prop, eButton))
		return;

	const char* pcsActionValue = NULL;

	if(strAction == UIA_SHOW_LAYER)
	{
		// The action value should contain which layer to show.
		pcsActionValue = pElem->getStringProp(eActionValue1Prop);
		this->showUI(pcsActionValue, false, pcsTargetDataSource);
	}
	else if(strAction == UIA_HIDE_LAYER)
	{
		// The action value should contain which layer to hide.
		pcsActionValue = pElem->getStringProp(eActionValue1Prop);
		this->hideUI(pcsActionValue);

		KeyManager::getInstance()->resetAll();
	}
	else if(strAction == UIA_HIDE_ELEMENT)
	{
		// Hide a specific element.
		UIElement* pNewElem = this->getElementById<UIElement>(pElem->getStringProp(eActionValue1Prop));
		if(pNewElem)
			pNewElem->setIsVisible(false);
		ELSE_ASSERT;
	}
	else if(strAction == UIA_HIDE_ALL)
	{
		this->hideAll();
	}
	else if(strAction == UIA_TOGGLE_LAYER)
	{
		pcsActionValue = pElem->getStringProp(eActionValue1Prop);
		if(this->getIsLayerShown(pcsActionValue))
			this->hideUI(pcsActionValue);
		else
			this->showUI(pcsActionValue);
	}
	else if(strAction == UIA_SHOW_LAYER_EXCLUSIVE)
	{
		// Same as above, but hide all other layers first.
		this->hideAll();
		pcsActionValue = pElem->getStringProp(eActionValue1Prop);
		this->showUI(pcsActionValue);
	}
	else if(strAction == UIA_HIDE_TOP_PARENT)
	{
		this->hideUI(pElem->getTopmostParent<UIElement>()->getStringProp(PropertyLayer));
	}
	else if(strAction == UIA_HIDE_LAYER_SHOW_LAYER)
	{
		// Hide previously shown layer, show a new one
		this->hideUI(pElem->getStringProp(eActionValue1Prop));
		this->showUI(pElem->getStringProp(eActionValue2Prop));
	}
	else if(strAction == UIA_HIDE_LAST_SHOW_LAYER)
	{
		// Hide previously shown layer, show a new one
		this->hideUI(this->getLastShownLayerName());
		this->showUI(pElem->getStringProp(eActionValue1Prop));
	}
	else if(strAction == "uiaSetLinkedElemValueToFile"
		|| strAction == "uiaSetLinkedElemValueToSaveFile")
	{
		UIElement *pFinalTarget = pElem->getLinkedToElement();
		if(pFinalTarget)
		{
			// Open up a file chooser
			string strFilePath, strError;
			bool bRes = false;
			if(strAction == "uiaSetLinkedElemValueToFile")
			{
				TStringVector vecFiles;
				bRes = Application::showOpenFileDialog(NULL, false, vecFiles, getFormatsManager());
				if(bRes)
					strFilePath = vecFiles[0];
			}
			else
			{
				ResultCodeType eRes = Application::showSaveFileDialog(strFilePath, strError, getFormatsManager(), false, false);
				bRes = (eRes == ResultCodeOk);
			}

			if(bRes)
			{
				AutoUndoBlock rIncUndo(pFinalTarget->getUndoStringForSelfChange(), pFinalTarget->createUndoItemForSelfChange(), pFinalTarget->getParentWindow(), NULL);
				pFinalTarget->setText(strFilePath.c_str());
				pFinalTarget->handleTargetElementUpdate(false);

				theSharedString2 = FILE_SELECTED_ACTION;
				pFinalTarget->handleActionUpParentChain(theSharedString2, false);

			}

		}
		ELSE_ASSERT;
	}
	else if(strAction == "uiaChangeLinkedElemValueBy")
	{
		FLOAT_TYPE fAmount = pElem->getNumProp(eActionValue1Prop);
		UIElement *pFinalTarget = pElem->getLinkedToElement();
		if(pFinalTarget)
		{
			AutoUndoBlock rIncUndo(pFinalTarget->getUndoStringForSelfChange(), pFinalTarget->createUndoItemForSelfChange(), pFinalTarget->getParentWindow(), NULL);
			pFinalTarget->changeValueTo(fAmount + pFinalTarget->getNumericValue(UnitLastPlaceholder), NULL, true, false);
		}
		ELSE_ASSERT;
	}
	else if(strAction == UIA_SLIDE_NEXT || strAction == "uiaSlideNextHideLayer")
	{
		// Find the topmost parent of our element and slide its specified
		// children to the next one from the current child.
		slideParentElement(pElem,1);

		if(strAction == "uiaSlideNextHideLayer")
		{
			pcsActionValue = pElem->getStringProp(eActionValue1Prop);
			this->hideUI(pcsActionValue);
		}
	}
	else if(strAction == UIA_SLIDE_PREVIOUS)
	{
		// Find the topmost parent of our element and slide its specified
		// children to the previous one from the current child.
		slideParentElement(pElem, -1);
	}
	else if(strAction == "uiaSaveStartupOption")
	{
		const char* pcsPropName = pElem->getStringProp(eActionValue1Prop);
		SettingsCollection::synchSingleStartupSettingFromUI(pcsPropName, pElem);
		SettingsCollection::saveStartupSettings();
	}
	else if(strAction == UIA_QUIT_APP)
		Application::quitApp();
	else if(strAction == "uiaToggleContextMenu"
		|| strAction == "uiaToggleContextMenuAtMouseLocation")
	{
		pcsActionValue = pElem->getStringProp(eActionValue1Prop);
		UIElement* pTargetElem = this->getElementById<UIElement>(pcsActionValue);

		myCommonUiVElems.resize(1);
		myCommonUiVElems[0] = pTargetElem;
		this->hideAllWithTag("submenu", &myCommonUiVElems, true, this, AnimOverActionGenericCallback);
		setHideAllMenus(false);

		const char* pcsLayerName = pTargetElem->getStringProp(PropertyLayer);
		SideType eSide = mapStringToType(pElem->getStringProp(eActionValue2Prop), g_pcsSideStrings, SideTop);
		FLOAT_TYPE fOffset = 0;
		if(pElem->doesPropertyExist(PropertyPopupOffset))
			fOffset = pElem->getNumProp(PropertyPopupOffset);
		if(pTargetElem->getIsBeingShown() || pTargetElem->getIsFullyShown())
		{
			this->hideUI(pcsLayerName);
		}
		else
		{
			this->showUI(pcsLayerName, false, pcsTargetDataSource);
			if(strAction == "uiaToggleContextMenuAtMouseLocation")
			{
				SVector2D svLastMousePos;
				getParentWindow()->getLastMousePos(svLastMousePos);
				pTargetElem->positionAsPopupRelativeTo(svLastMousePos, eSide, fOffset, true);
			}
			else
				pTargetElem->positionAsPopupRelativeTo(pElem, eSide, fOffset);
		}

	}
	else if(strAction == UIA_SHOW_POPUP)
	{
		this->hideLayersWithTag("popup");

		pcsActionValue = pElem->getStringProp(eActionValue1Prop);

		UIElement* pTargetElem = this->getElementById<UIElement>(pcsActionValue);
		//pTargetElem->setStringProp(PropertyUiObjPopupPosition, pElem->getStringProp(PropertyUiObjActionValue2));
		const char* pcsLayerName = pTargetElem->getStringProp(PropertyLayer);
		SideType eSide = mapStringToType(pElem->getStringProp(PropertyActionValue2), g_pcsSideStrings, SideTop);

		FLOAT_TYPE fOffset = 0;
		if(pElem->doesPropertyExist(PropertyPopupOffset))
			fOffset = pElem->getNumProp(PropertyPopupOffset);

		pTargetElem->setStringProp(PropertyPopupSourceUiElement, pElem->getStringProp(PropertyId));

		// Now show it
		this->showUI(pcsLayerName, false, pcsTargetDataSource);

		// We need to do this *after* we show the popup, since the showing
		// code sets the target data source onto each shown element, and that
		// allows us to refresh the text correctly (and thus, the size).
		pTargetElem->positionAsPopupRelativeTo(pElem, eSide, fOffset);

	}
	else if(strAction == UIA_UNDO)
	{
		UndoManager* pTargetUndoManager = UndoManager::getCurrentUndoManager();
		_ASSERT(pTargetUndoManager);
		if(pTargetUndoManager)
			pTargetUndoManager->undo();
	}
	else if(strAction == UIA_REDO)
	{
		UndoManager* pTargetUndoManager = UndoManager::getCurrentUndoManager();
		_ASSERT(pTargetUndoManager);
		if(pTargetUndoManager)
			pTargetUndoManager->redo();
	}
	else if(strAction == "uiaCrash")
	{
		UIElement *pCrashElem = NULL;
		pCrashElem->getId();
	}
	else if(strAction == "uiaDebugToggleFrameRate")
	{
		Application::getInstance()->setShowFramerate(!Application::getInstance()->getShowFramerate());
	}
	else if(strAction == "uiaDebugOpenNewWindow")
	{
		// Open a new child window
		Application::openNewWindow(getParentWindow(), ResourceColorPicker, 1200, 900, true, false);
	}
	else if(strAction == UIA_ACCEPT_COLOR_FROM_PICKER || strAction == UIA_CANCEL_COLOR_PICKER)
	{
		UIColorPicker *pPicker = pElem->getParentOfType<UIColorPicker>();
		if(strAction == UIA_ACCEPT_COLOR_FROM_PICKER)
		{
			SColor scolNewColor;
			pPicker->getCurrentColor(scolNewColor);
			pElem->getParentWindow()->onAcceptNewColorFromPicker(scolNewColor);
		}
		else
		{
			// If we're cancelling, apply old color
			pPicker->cancelColorSelection();
		}

		pElem->getParentWindow()->close();
	}
	else if(strAction == "uiaCallHandleActionAndClose")
	{
		pcsActionValue = pElem->getStringProp(eActionValue1Prop);
		// Find the elem
		UIElement* pTopmostParent = pElem->getTopmostParent<UIElement>();

		UIElement* pCallback = NULL;
		if(!pTopmostParent->getHandleActionCallback())
			pCallback = getElementById(pTopmostParent->getGenericDataSource(), true, true);

		// Hackish... only works for rename for now.
		// This is if need any data back from the dialog.
		UIElement* pSrcDataElem = pTopmostParent->getChildAndSubchild("renameTextField", "defTextBox");
		if(pSrcDataElem && pCallback)
		{
			pSrcDataElem->getTextAsString(mySharedString);
			pCallback->setGenericDataSource(mySharedString.c_str());
		}

		bool bHideCallingElem = true;
		if(pCallback)
		{
			theSharedString2 = pcsActionValue;
			pCallback->handleActionUpParentChain(theSharedString2, false);
		}
		else if(pTopmostParent->getHandleActionCallback())
		{
			bHideCallingElem = pTopmostParent->getHandleActionCallback()->handleAction(pcsActionValue, pTopmostParent);
		}
		else
		{
			// Since not all dialogs have an action handler,
			// pretend this is an action type (not value) and
			// try handling it this way:
			performUiAction(pcsActionValue);
		}
		if(bHideCallingElem)
			this->hideUI(pTopmostParent->getStringProp(PropertyLayer));
	}
	else if(strAction == "uiaTargetDataCloseTab")
	{
		UIElement* pSourceElem = getElementById<UIElement>(pElem->getGenericDataSource());
		if(pSourceElem)
		{
			UITabWindowElement* pTabParent = pSourceElem->getParentOfType<UITabWindowElement>();
			int iTabIdx = -1;
			if(pTabParent)
				iTabIdx = pTabParent->getChildIndex(pSourceElem);
			if(iTabIdx >= 0)
				pTabParent->closeTab(iTabIdx);
		}
	}
	else if(strAction == "uiaResetCurveEditor")
	{
		// Get parent, get child editor:
		UICurveEditor *pEditor = pElem->getParent<UIElement>()->getChildByClass<UICurveEditor>(true);
		if(pEditor)
			pEditor->resetCurve();
	}
	else if(strAction == UIA_SHOW_MESSAGE)
	{
		pElem->getAsString(PropertyActionValue, mySharedString);
		const char* pcsTitle = NULL;
		if(pElem->doesPropertyExist(PropertyActionValue2))
			pcsTitle = pElem->getStringProp(PropertyActionValue2);
		showMessageBox(mySharedString.c_str(), pcsTitle);
	}
	else if(strAction == "uiaDebugReloadTextures")
	{
		Logger::log("Manually reloading all textues");
		getParentWindow()->getDrawingCache()->reloadAllTextures();
	}
}
/*****************************************************************************/
};