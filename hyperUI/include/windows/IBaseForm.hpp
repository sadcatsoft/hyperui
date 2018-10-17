#pragma once
#include "stdafx.h"
#include "ApplicationRefreshLoop.h"
#include "OpenGLWindow.h"

//#include "TabletCode/TabletHeaders.h"

//#include "EditorPanel.h"
//#include "GameEngine.h"
//#include "UserImageWindow.h"

#define DEFAULT_WIDTH		1024
#define DEFAULT_HEIGHT		768

#ifdef WIN_COMPILE_UNMANAGED
#pragma managed(push, on)
#endif

//#include "WinMainGameProgram.h"

//extern gcroot<GameProgram^> g_pMainWinProgram;

namespace HyperUI 
{
inline IBaseForm^ findFormByWindow(HyperUI::Window* pWindow)
{
	IBaseForm^ pCurrForm = nullptr;
	FormWindowManager::Iterator fi;
	for(fi = FormWindowManager::getInstance()->windowsBegin(); !fi.isEnd(); fi++)
	{
		pCurrForm = fi.getWindow()->myForm;
		if(pCurrForm->getWindow() == pWindow)
			return pCurrForm;
	}

	return nullptr;
}

extern HYPERUI_API gcroot<IBaseForm^> g_ptheMainForm;
using namespace System::Windows::Forms;
/*****************************************************************************/
inline int IBaseForm::mapVirtualKeyCode(System::Windows::Forms::KeyEventArgs^  e, bool &bIsAffectingControlOut, bool &bIsAffectingAltOut, bool &bIsAffectingShiftOut)
{
	// The trouble with Ctrl, Alt, and Shift is that when they themselves are pressed,
	// they don't actually set the e->Control, etc. flags, but instead show up as the
	// actual key pressed. So we check for this here.
	System::Windows::Forms::Keys eVirtualKey = e->KeyCode;

	bIsAffectingControlOut = e->Control;
	bIsAffectingAltOut = e->Alt;
	bIsAffectingShiftOut = e->Shift;

	bool bIsExtended = (HIWORD(e->KeyData) & KF_EXTENDED) != 0;

	// MAPVK_VK_TO_CHAR will map things to the unshifted character - i.e. 
	// if you're holding Shift and press 3, it will map it to 3. 
	// MAPVK_VK_TO_VSC will, however, map it to #. We need to be careful
	// because for shortcuts, we might have something like Ctrl+Alt+3,
	// where we need the three.
	// So we do a hack - if only shift is held, we map to the shifted char.
	// If not, then we map to the unshifted char. This prevents us from having
	// things like Shift+3 as a shortcut, but that's fine.
	int iMappingFlag;
	if(e->Alt || e->Control)
		iMappingFlag = MAPVK_VK_TO_CHAR;
	else
		iMappingFlag = MAPVK_VK_TO_VSC;

	UINT iScanCodeRaw = MapVirtualKey((UINT)eVirtualKey, iMappingFlag);
	int iScanCode;
	if(iMappingFlag != MAPVK_VK_TO_CHAR)
	{
		static unsigned char ucState[256];
		GetKeyboardState(ucState);
#ifdef USE_UNICODE_STRINGS
		wchar_t pcsRes;
		memset(&pcsRes, 0, sizeof(wchar_t));
		int iRes = ToUnicode((UINT)eVirtualKey, iScanCodeRaw, ucState, &pcsRes, 1, 0);
		iScanCode = pcsRes;
#else
		WORD pcsRes;
		memset(&pcsRes, 0, sizeof(WORD));
		int iRes = ToAscii((UINT)eVirtualKey, iScanCodeRaw, ucState, &pcsRes, 0);
		iScanCode = pcsRes;
#endif
	}
	else
		iScanCode = iScanCodeRaw;

	//if(iScanCode == 0)
	{
		if(eVirtualKey == Keys::Up)
			iScanCode = SilentKeyUpArrow;
		else if(eVirtualKey == Keys::Down)
			iScanCode = SilentKeyDownArrow;
		else if(eVirtualKey == Keys::Right)
			iScanCode = SilentKeyRArrow;
		else if(eVirtualKey == Keys::Left)
			iScanCode = SilentKeyLArrow;
		else if(eVirtualKey == Keys::Tab)
			iScanCode = SilentKeyTab;
		else if(eVirtualKey == Keys::Enter)
		{
			if(bIsExtended)
				iScanCode = SilentKeyNumpadEnter;
			else
				iScanCode = SilentKeyCarriageReturn;
		}
		else if(eVirtualKey == Keys::Delete)
			iScanCode = SilentKeyDelete;
		else if(eVirtualKey == Keys::Back)
			iScanCode = SilentKeyBackspace;
		else if(eVirtualKey == Keys::Escape)
			iScanCode = SilentKeyEscape;
		else if(eVirtualKey == Keys::End)
			iScanCode = SilentKeyEnd;
		else if(eVirtualKey == Keys::Home)
			iScanCode = SilentKeyHome;
		else if(eVirtualKey == Keys::PageUp)
			iScanCode = SilentKeyPageUp;
		else if(eVirtualKey == Keys::PageDown)
			iScanCode = SilentKeyPageDown;
		else if(eVirtualKey == Keys::F1)
			iScanCode = SilentKeyF1;
		else if(eVirtualKey == Keys::F2)
			iScanCode = SilentKeyF2;
		else if(eVirtualKey == Keys::F3)
			iScanCode = SilentKeyF3;
		else if(eVirtualKey == Keys::F4)
			iScanCode = SilentKeyF4;
		else if(eVirtualKey == Keys::F5)
			iScanCode = SilentKeyF5;
		else if(eVirtualKey == Keys::F6)
			iScanCode = SilentKeyF6;
		else if(eVirtualKey == Keys::F7)
			iScanCode = SilentKeyF7;
		else if(eVirtualKey == Keys::F8)
			iScanCode = SilentKeyF8;
		else if(eVirtualKey == Keys::F9)
			iScanCode = SilentKeyF9;
		else if(eVirtualKey == Keys::F10)
			iScanCode = SilentKeyF10;
		else if(eVirtualKey == Keys::F11)
			iScanCode = SilentKeyF11;
		else if(eVirtualKey == Keys::F12)
			iScanCode = SilentKeyF12;
		else if(eVirtualKey == Keys::ControlKey || eVirtualKey == Keys::Control)
		{
			bIsAffectingControlOut = true;
			iScanCode = 0;
		}
		else if(eVirtualKey == Keys::Alt || eVirtualKey == Keys::Menu)
		{
			bIsAffectingAltOut = true;
			iScanCode = 0;
		}
		else if(eVirtualKey == Keys::Shift || eVirtualKey == Keys::ShiftKey)
		{
			bIsAffectingShiftOut = true;
			iScanCode = 0;
		}
	}

	if(iScanCode >= 0 && iScanCode <= 255 && ISALPHA((UNSIGNED_CHAR_TYPE)iScanCode))
	{
		if(bIsAffectingShiftOut)
			iScanCode = TOUPPER((char)iScanCode);
		else
			iScanCode = TOLOWER((char)iScanCode);
	}

	return iScanCode;
}
/*****************************************************************************/
inline gcroot<HyperUI::IBaseForm^> IBaseForm::getMainForm()
{
	return g_ptheMainForm;
	//return dynamic_cast<HyperUI::IBaseForm^>((System::Windows::Forms::Form^)g_ptheMainForm);
}
/*****************************************************************************/
inline HDC IBaseForm::getHDC()
{
	return myGlWindow->getHDC();
}
/*****************************************************************************
void IBaseForm::setCursor(CursorType eType) { myGlWindow->setCursor(eType); }
UNIQUEID_TYPE IBaseForm::pushCursor(CursorType eType) { return myGlWindow->pushCursor(eType); }
void IBaseForm::popCursor(UNIQUEID_TYPE idCursor) { myGlWindow->popCursor(idCursor); }
/*****************************************************************************/
inline IBaseForm::IBaseForm()
{
	initCommon();
}
/*****************************************************************************/
inline IBaseForm::IBaseForm(int iOptWidth, int iOptHeight, bool bThinFrame, bool bFixedSize, ResourceType eSourceCollection, int iMinWidth, int iMinHeight, const char* pcsTitle, int iCenterX, int iCenterY, bool bIsMainWindow, const char* pcsInitLayerToShow)
{
	initCommon();
	initializeUI(eSourceCollection, iOptWidth, iOptHeight, bThinFrame, bFixedSize, iMinWidth, iMinHeight, pcsTitle, iCenterX, iCenterY, bIsMainWindow, pcsInitLayerToShow);
}
/*****************************************************************************/
inline void IBaseForm::initCommon()
{
/*
	if(!g_ptheMainForm)
	{
		g_ptheMainForm = this;
	}
*/

	myPreventRendering = false;
	myMouseCoordsVector = new TTouchVector;
	myIsLButtonDown = false;
	myIsRButtonDown = false;
	myIsInOnActivated = false;
	myMoveLeft = myMoveRight = myMoveUp = myMoveDown = false;

#ifdef ALLOW_UI_EDITOR
	myEditorPanel = nullptr;
#endif
}
/*****************************************************************************/
inline IBaseForm::~IBaseForm()
{
	delete myMouseCoordsVector;
	myMouseCoordsVector = NULL;

	TabletManager::getInstance()->shutdown(false);

	destroyUI();
}
/*****************************************************************************/
inline bool IBaseForm::getIsInEditorMode() 
{ 
#ifdef ALLOW_UI_EDITOR
	return myEditorPanel && myEditorPanel->Visible; 
#else
	return false;
#endif
}
/*****************************************************************************/
inline void IBaseForm::hideEditor()
{
#ifdef ALLOW_UI_EDITOR
	UIEditor::getInstance()->onHide();
	if(myEditorPanel)
		myEditorPanel->Hide();
#endif
}
/*****************************************************************************/
#ifdef ALLOW_UI_EDITOR
EditorPanel^ IBaseForm::getEditorPanel()
{
	_ASSERT(myEditorPanel);
	return myEditorPanel;
}
#endif
/*****************************************************************************/
inline System::Void IBaseForm::OnPaint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e)
{
	Window* pWindow = getWindow();
	if(!myGlWindow || !pWindow)
		return;

	if(myPreventRendering)
		return;

	myGlWindow->makeCurrent();
	pWindow->renderToTexture();

	myGlWindow->preRender();

	pWindow->render();

	myGlWindow->postRender();

}
/*****************************************************************************/
inline System::Void IBaseForm::OnMouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
{
	if(myGlWindow)
		myGlWindow->makeCurrent();
	if(e->Button == System::Windows::Forms::MouseButtons::Right)
		myIsRButtonDown = true;
	else
		myIsLButtonDown = true;

#ifdef ALLOW_UI_EDITOR
	if(getIsInEditorMode())
	{
		UIEditor::getInstance()->onMouseDown(e->X, e->Y);
		return;
	}
#endif

	//	GameEngine::setIsTouchDown(true);
	myMouseCoordsVector->clear();
	STouchInfo rDummy;
	rDummy.myPoint.set(e->X, e->Y);
	myMouseCoordsVector->push_back(rDummy);
	if(getWindow())
		getWindow()->onTouchDown(*myMouseCoordsVector, myIsRButtonDown ? MouseButtonRight : MouseButtonLeft);
	OnMouseMove(sender, e);

}
/*****************************************************************************/
inline System::Void IBaseForm::OnMouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
{
	//ActivityManager::getInstance()->singleValueChanged();

	if(myGlWindow)
		myGlWindow->makeCurrent();
	// This happens if we move out and then come back and then release the button.
	if(!myIsLButtonDown && !myIsRButtonDown)
		return;

	if(getIsInEditorMode())
	{
#ifdef ALLOW_UI_EDITOR
		UIEditor::getInstance()->onMouseUp(e->X, e->Y);
		myIsLButtonDown = false;
		myIsRButtonDown = false;
#endif
		return;
	}


	//	GameEngine::setIsTouchDown(false);
	myMouseCoordsVector->clear();
	STouchInfo rDummy;
	rDummy.myPoint.set(e->X, e->Y);
	myMouseCoordsVector->push_back(rDummy);
	if(getWindow())
		getWindow()->onTouchUp(*myMouseCoordsVector);
	myIsLButtonDown = false;
	myIsRButtonDown = false;
	OnMouseMove(sender, e);	
}
/*****************************************************************************/
inline System::Void IBaseForm::OnMouseWheel(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
{
	if(!getWindow())
		return;

	SVector2D svMousePos(e->X, e->Y);
	FLOAT_TYPE fDelta = (FLOAT_TYPE)e->Delta/(FLOAT_TYPE)WHEEL_DELTA;

	if(getWindow()->getIsTabletButtonDown())
		getWindow()->onTabletMousePan(svMousePos, fDelta);
	else
		getWindow()->onMouseWheel(svMousePos, fDelta);


/*
	if(getWindow()->getIsTabletButtonDown() && g_pMainEngine->getLastActiveImageWindow())
	{
		// This means we're panning with the tablet pen
		g_pMainEngine->getLastActiveImageWindow()->onMousePan(svMousePos);
	}
	else
		getWindow()->onMouseWheel(svMousePos, fDelta);
*/
}
/*****************************************************************************/
inline System::Void IBaseForm::OnMouseLeave(System::Object^  sender, System::EventArgs^  e)
{
	if(myGlWindow)
		myGlWindow->makeCurrent();

	System::Windows::Forms::MouseEventArgs rEvent(System::Windows::Forms::MouseButtons::Right, 1, -100, -100, 0);
	if(myIsLButtonDown || myIsRButtonDown)
		OnMouseUp(this, %rEvent);
	if(getWindow())
		getWindow()->onTouchLeave();

	// This is to invoke hover over the out of window area.
	_ASSERT(!myIsLButtonDown && !myIsRButtonDown);
	OnMouseMove(this, %rEvent);
}
/*****************************************************************************/
inline System::Void IBaseForm::OnMouseMove(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
{
	if(myGlWindow)
		myGlWindow->makeCurrent();
	if(myIsLButtonDown && getIsInEditorMode())
	{
#ifdef ALLOW_UI_EDITOR
		UIEditor::getInstance()->onMouseMove(e->X, e->Y);
#endif
		return;
	}

	if(!getWindow())
		return;

	//g_svLastMousePos.set(e->X, e->Y);
	if(myIsLButtonDown || myIsRButtonDown)
	{
		myMouseCoordsVector->clear();
		STouchInfo rDummy;
		rDummy.myPoint.set(e->X, e->Y);
		myMouseCoordsVector->push_back(rDummy);
		getWindow()->onTouchMove(*myMouseCoordsVector);	
	}
	// We need this for brushe sizing now...
/*

	else if(myIsRButtonDown)
	{
		// NOthing for now
	}
*/
	else
	{
		myMouseCoordsVector->clear();
		STouchInfo rDummy;
		rDummy.myPoint.set(e->X, e->Y);
		myMouseCoordsVector->push_back(rDummy);
		getWindow()->onCursorGlide(*myMouseCoordsVector);
	}

	// Note that if we
	g_pMainWinProgram->DrawFrame();
}
/*****************************************************************************/
inline System::Void IBaseForm::OnKeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e)
{
	if(myGlWindow)
		myGlWindow->makeCurrent();

	bool bIsControlDown, bIsShiftDown, bIsAltDown;
	int iScanCode = mapVirtualKeyCode(e, bIsControlDown, bIsAltDown, bIsShiftDown);
	bool bHandled = false;
	if(getWindow())
		bHandled = getWindow()->onKeyDown(iScanCode, bIsControlDown, bIsAltDown, bIsShiftDown, false);
	if(bHandled)
		return;

	if(e->Control)
		return;

	if(e->KeyCode == Keys::A)
		myMoveLeft = true;
	else if(e->KeyCode == Keys::D)
		myMoveRight = true;
	else if(e->KeyCode == Keys::W)
		myMoveUp = true;
	else if(e->KeyCode == Keys::S)
		myMoveDown = true;

	if (e->Alt)
		e->Handled = true;  
}
/*****************************************************************************/
inline System::Void IBaseForm::OnActivated(System::Object^ sender, System::EventArgs^ e)
{
	if(myIsInOnActivated)
		return;

	myIsInOnActivated = true;
	Application::getInstance()->onSwitchedTo(myWindow);

	myIsInOnActivated = false;

	TabletManager::getInstance()->enableContext();
}
/*****************************************************************************/
inline System::Void IBaseForm::OnKeyUp(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e)
{
	if(myGlWindow)
		myGlWindow->makeCurrent();

#ifdef ALLOW_UI_EDITOR
	if(e->Control && e->KeyCode == Keys::E)
	{
		// Show the editor form
		if(myEditorPanel && myEditorPanel->Visible)
		{
			hideEditor();
			return;
		}
		else
		{
			if(!myEditorPanel)
				myEditorPanel = gcnew EditorPanel;			

			myEditorPanel->Location = Point(this->RectangleToScreen(this->DisplayRectangle).Right + 20, this->Location.Y + 5);
			myEditorPanel->onBeforeShown();
			myEditorPanel->Show(this);
		}
	}
#endif

#ifdef _DEBUG
/*
	if(e->KeyCode == Keys::F5)
	{
		myPreventRendering = true;
		ResourceManager::getInstance()->reloadAll(g_pMainEngine);
		getWindow()->getDrawingCache()->reloadAllTextures();
#ifdef ALLOW_UI_EDITOR
		UIEditor::getInstance()->onRealodAll();
#endif
		myPreventRendering = false;

		return;
	}
*/
#endif
	bool bIsControlDown, bIsShiftDown, bIsAltDown;
	int iScanCode = mapVirtualKeyCode(e, bIsControlDown, bIsAltDown, bIsShiftDown);

	bool bHandled = false;
	if(getWindow())
		bHandled = getWindow()->onKeyUp(iScanCode, bIsControlDown, bIsAltDown, bIsShiftDown, false);
	if(bHandled)
		return;

}
/*****************************************************************************/
inline HWND IBaseForm::initializeUI(ResourceType eSourceCollection, int iOptWidth, int iOptHeight, bool bThinFrame, bool bFixedSize, int iMinWidth, int iMinHeight, const char* pcsTitle, int iCenterX, int iCenterY, bool bIsMainWindow, const char* pcsInitLayerToShow)
{
	int iScreenScale = HyperCore::getScreenDensityScalingFactor(HyperUI::getMaxScreenScalingFactor());
	this->ClientSize = System::Drawing::Size(DEFAULT_WIDTH*iScreenScale, DEFAULT_HEIGHT*iScreenScale);

	if(bFixedSize)
	{
		if(bThinFrame)
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedToolWindow;
		else
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;

	}
	else
	{
		if(bThinFrame)
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::SizableToolWindow;
		else
			this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::Sizable;
	}

	this->ShowInTaskbar = true;

	this->BackColor = System::Drawing::Color::Black;

	myFormWrapper = new SBaseFormWrapper;
	myFormWrapper->myForm = this;

	FormWindowManager::getInstance()->onWindowCreated(myFormWrapper, bIsMainWindow);

	myGlWindow = gcnew OpenGLWindow(this, DEFAULT_WIDTH*iScreenScale, DEFAULT_HEIGHT*iScreenScale);
	myGlWindow->makeCurrent();
	myWindow = Application::getInstance()->allocateWindow(eSourceCollection, DEFAULT_WIDTH*iScreenScale, DEFAULT_HEIGHT*iScreenScale, bIsMainWindow);

	myWindow->setupGlView(DEFAULT_WIDTH*iScreenScale, DEFAULT_HEIGHT*iScreenScale);
	myWindow->finishPrelimInit(eSourceCollection, pcsInitLayerToShow);

#ifdef _DEBUG
/*
	string strTemp;
	this->enumerateAllPixelFormats(strTemp);
	FileUtils::saveToFile("db_gr_formats.txt", strTemp.c_str(), true);
*/
#endif


	if(pcsTitle)
		this->Text = gcnew System::String(pcsTitle);

	if(iMinWidth > 0 && iMinHeight > 0)
		this->MinimumSize = System::Drawing::Size(iMinWidth, iMinHeight);

	if(iOptWidth > 0 && iOptHeight > 0)
	{
		this->ClientSize = System::Drawing::Size(iOptWidth, iOptHeight);
		OnResize(nullptr, nullptr);
	}

	if(iCenterX >= 0 && iCenterY >= 0)
	{
		System::Drawing::Point rOrigin(iCenterX - this->ClientSize.Width/2, iCenterY - this->ClientSize.Height/2);

		if(rOrigin.X < 0)
			rOrigin.X = 0;
		if(rOrigin.Y < 0)
			rOrigin.Y = 0;

		// To avoid opening under the tool bar
		const FLOAT_TYPE fBottomPadding = 90;
		System::Windows::Forms::Screen^ pScreen = System::Windows::Forms::Screen::FromControl(this);

		if(rOrigin.X + this->ClientSize.Width > pScreen->Bounds.Size.Width)
			rOrigin.X = pScreen->Bounds.Size.Width - this->ClientSize.Width;
		if(rOrigin.Y + this->ClientSize.Height > pScreen->Bounds.Size.Height - fBottomPadding)
			rOrigin.Y = pScreen->Bounds.Size.Height - fBottomPadding - this->ClientSize.Height;

		this->Location = rOrigin;
	}

	initializeComponent();

	this->OnResize(nullptr, nullptr);

	HWND hWnd = (HWND)myGlWindow->Handle.ToPointer();
	TabletManager::getInstance()->initialize(hWnd, false);
	return hWnd;
}
/*****************************************************************************/
inline void IBaseForm::destroyUI()
{
	if(myGlWindow)
		myGlWindow->makeCurrent();

	FormWindowManager::getInstance()->onWindowDestroyed(myFormWrapper, true);

	// Now, destroy all other windows:
	if(this->getIsMainWindow())
	{
		FormWindowManager::Iterator fi;
		vector < gcroot<IBaseForm^> > rForms;
		for(fi = FormWindowManager::getInstance()->windowsBegin(); !fi.isEnd(); fi++)
			rForms.push_back(fi.getWindow()->myForm);
		int iCurr, iNum = rForms.size();
		for(iCurr = 0; iCurr < iNum; iCurr++)
			delete rForms[iCurr];
	}

 	delete myWindow;
 	myWindow = NULL;
 
	delete myGlWindow;
	myGlWindow = nullptr;
}
/*****************************************************************************/
inline void IBaseForm::initializeComponent()
{
	this->KeyPreview = true;

	this->KeyDown += gcnew System::Windows::Forms::KeyEventHandler(this, &IBaseForm::OnKeyDown);
	this->KeyUp += gcnew System::Windows::Forms::KeyEventHandler(this, &IBaseForm::OnKeyUp);
	this->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &IBaseForm::OnMouseDown);
	this->MouseLeave += gcnew System::EventHandler(this, &IBaseForm::OnMouseLeave);
	this->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &IBaseForm::OnMouseMove);
	this->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &IBaseForm::OnMouseUp);
	this->MouseWheel += gcnew System::Windows::Forms::MouseEventHandler(this, &IBaseForm::OnMouseWheel);
	this->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &IBaseForm::OnPaint);
	this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &IBaseForm::OnFormClosing);
	this->Resize += gcnew System::EventHandler(this, &IBaseForm::OnResize);
	this->Shown += gcnew System::EventHandler(this, &IBaseForm::OnShown);
	this->Activated += gcnew System::EventHandler(this, &IBaseForm::OnActivated);
	this->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &IBaseForm::OnDragDrop);
	this->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &IBaseForm::OnDragDropEnter);
	this->DragOver += gcnew System::Windows::Forms::DragEventHandler(this, &IBaseForm::OnDragOver);
	
	try
	{
		this->AllowDrop = true;
	}
	catch(System::Exception^ e)
	{
	}


#if defined(MAC_BUILD) || defined(UNIVERSAL_BINARY)
	this->MinimumSize = System::Drawing::Size(800, 600);
#endif
}
/*****************************************************************************/
inline bool IBaseForm::checkForClosure()
{
	Window* pWindow = getWindow();
	if(!pWindow)
		return false;
	if(pWindow->getShouldClose() && !pWindow->getHasCalledClose())
	{
		pWindow->setHasCalledClose(true);
		gLog("----------- Closing window -----------\n");
		IBaseForm^ pBaseForm = findFormByWindow(pWindow);
		if(pBaseForm)
			pBaseForm->Close();
		ELSE_ASSERT;
		gLog("----------- Close call done -----------\n");
		return true;
	}

	return false;
}
/*****************************************************************************/
inline System::Void IBaseForm::OnResize(System::Object^ sender, System::EventArgs^ e)
{
	// When minimizing, we get zero size. Ignore.
	if(this->ClientSize.Width <= 0 || this->ClientSize.Height <= 0)
		return;

	if(myGlWindow)
		myGlWindow->resize(this->ClientSize.Width, this->ClientSize.Height);
	if(myWindow)
		myWindow->onWindowSizeChanged(this->ClientSize.Width, this->ClientSize.Height);
}
/*****************************************************************************/
inline System::Void IBaseForm::OnTimerTick(System::Object^ sender, System::EventArgs ^e)
{
	if(myGlWindow)
		myGlWindow->makeCurrent();

	if(myWindow)
	{
		myWindow->onTimerTickBegin();
		myWindow->onTimerTickEnd();
		myWindow->checkForClosing();
	}
}
/*****************************************************************************/
inline System::Void IBaseForm::OnShown(System::Object^ sender, System::EventArgs^ e)
{
	TabletManager::getInstance()->enableContext();

	if(this->getIsMainWindow())
	{
		// Now, attempt to load the last used layout
		UILayoutManager::getInstance()->loadLayout(NULL);
	}
}
/*****************************************************************************/
inline System::Void IBaseForm::OnFormClosing(System::Object^ sender, System::Windows::Forms::FormClosingEventArgs^ e)
{
#ifdef IMAGE_VIEWER_MODE
	e->Cancel = false;
	return;
#endif

	// So we set the window flag telling it to close,
	// and then always cancel this event.
	if(!getWindow() || getWindow()->getHasBegunClosing())
	{
		e->Cancel = false;
		return;
	}

	getWindow()->beginClosing(ClosingWindow);
	e->Cancel = true;
}
/*****************************************************************************/
inline void IBaseForm::simulateKeyPress(System::Windows::Forms::Keys eKeyCode, bool bGenerateUpEvent)
{
	Window* pWindow = WindowManager::getInstance()->getLastActiveWindow();
	IBaseForm^ pResForm = findFormByWindow(pWindow);
	if(!pResForm)
		pResForm = getMainForm();

	KeyEventArgs eKey(eKeyCode);
	if(bGenerateUpEvent)
		pResForm->OnKeyUp(nullptr, %eKey);
	else
		pResForm->OnKeyDown(nullptr, %eKey);
}
/*****************************************************************************/
inline bool IBaseForm::convertDragArgumentsToFileNameList(System::Windows::Forms::DragEventArgs^ e, TStringVector& vecNamesOut)
{
	vecNamesOut.clear();

	//DataFormats::Format^ myFormat = DataFormats::GetFormat( "FileNameW" );
	//array<String^>^  eFormats = e->Data->GetFormats();
	cli::array<System::String^>^ pData = dynamic_cast<cli::array<System::String^>^>(e->Data->GetData(DataFormats::FileDrop));
	if(!pData)
		return false;

	// Otherwise, go through the list and convert strings
	string strTemp;
	int iCurr, iNum = pData->Length;
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		stringToSTLString(pData[iCurr], strTemp);
		vecNamesOut.push_back(strTemp);
	}

	return vecNamesOut.size() > 0;
}
/*****************************************************************************/
inline System::Void IBaseForm::OnDragDropEnter(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e) 
{
	TStringVector vecFiles;
	convertDragArgumentsToFileNameList(e, vecFiles);
	FormatsManager* pFormatsManager = getWindow()->getUIPlane()->getFormatsManager();
	if(pFormatsManager && pFormatsManager->canOpenAnyFileFrom(vecFiles, true))
		e->Effect = DragDropEffects::Copy;
	else
		e->Effect = DragDropEffects::None;
}
/*****************************************************************************/
inline System::Void IBaseForm::OnDragDrop(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e)
{
	TStringVector vecFiles;
	convertDragArgumentsToFileNameList(e, vecFiles);
	if(e->Effect == DragDropEffects::Copy || e->Effect == DragDropEffects::Move)
	{
		//UICoordinator::getInstance()->openFiles(vecFiles);
		System::Drawing::Point pt(e->X, e->Y);
		pt = PointToClient(pt);
		getWindow()->onDragDropFiles(vecFiles, pt.X, pt.Y);
	}
}
/*****************************************************************************/
inline System::Void IBaseForm::OnDragOver(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e)
{
	TStringVector vecFiles;
	convertDragArgumentsToFileNameList(e, vecFiles);

	FormatsManager* pFormatsManager = getWindow()->getUIPlane()->getFormatsManager();
	if(pFormatsManager && pFormatsManager->canOpenAnyFileFrom(vecFiles, true))
		e->Effect = DragDropEffects::Copy;
	else
		e->Effect = DragDropEffects::None;
}
/*****************************************************************************/
inline void IBaseForm::enumerateAllPixelFormats(string& strOut)
{
	PIXELFORMATDESCRIPTOR rTempFormat;
	rTempFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);

	string strTemp;
	HDC hDC = this->getHDC();
	int iCurr, iNumFormats = DescribePixelFormat(hDC, 1, 0, NULL);

	StringUtils::numberToString(iNumFormats, strTemp);
	strOut = "Num Formats:" + strTemp + "\n";
	strOut += "=====================================\n";

	for(iCurr = 1; iCurr <= iNumFormats; iCurr++)
	{
		StringUtils::numberToString(iCurr, strTemp);
		strOut += "Format Index: " + strTemp + "\n";
		DescribePixelFormat(hDC, iCurr, sizeof(PIXELFORMATDESCRIPTOR), &rTempFormat);
		getPixelFormatInfo(rTempFormat, strTemp);
		strOut += strTemp;
		strOut += "=====================================\n";
	}
}
/*****************************************************************************/

};


/*
void gPositionWindowCenter(Window* pWindow, int iCenterX, int iCenterY)
{
	IBaseForm^ pBaseForm = findFormByWindow(pWindow);
	if(!pBaseForm)	
		ASSERT_RETURN;

	System::Drawing::Point rOrigin(iCenterX - pBaseForm->ClientSize.Width/2, iCenterY - pBaseForm->ClientSize.Height/2);
	pBaseForm->Location = rOrigin;	
}


void gCloseWindow(Window* pWindow)
{
	IBaseForm^ pBaseForm = findFormByWindow(pWindow);
	if(pBaseForm)
		pBaseForm->Close();
	ELSE_ASSERT_RETURN;
}

UNIQUEID_TYPE gPushCursor(CursorType eType, Window* pParentWindow)  
{ 
	IBaseForm^ pBaseForm = findFormByWindow(pParentWindow);
	if(!pBaseForm)
	{
		_ASSERT(0);
		return -1;
	}
	else
		return pBaseForm->pushCursor(eType); 
}

void gPopCursor(UNIQUEID_TYPE idCursor, Window* pParentWindow) 
{  
	IBaseForm^ pBaseForm = findFormByWindow(pParentWindow);
	if(!pBaseForm)
		ASSERT_RETURN;
	pBaseForm->popCursor(idCursor); 
}
*/
#ifdef WIN_COMPILE_UNMANAGED
#pragma managed(pop)
#endif

