#pragma once

#ifdef WIN_COMPILE_UNMANAGED
#pragma managed(push, on)
#endif

#ifndef DIRECTX_PIPELINE
#include <vcclr.h>

namespace HyperUI
{
ref class OpenGLWindow;
/*****************************************************************************/
struct HYPERUI_API SBaseFormWrapper;
/*****************************************************************************/
public ref class IBaseForm: public System::Windows::Forms::Form
{
public:

	IBaseForm();
	IBaseForm(int iOptWidth, int iOptHeight, bool bThinFrame, bool bFixedSize, ResourceType eSourceCollection, int iMinWidth, int iMinHeight, const char* pcsTitle, int iCenterX, int iCenterY, bool bIsMainWindow, const char* pcsInitLayerToShow);
	virtual ~IBaseForm();

	virtual System::Void OnMouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
	virtual System::Void OnMouseUp(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
	virtual System::Void OnMouseMove(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
	virtual System::Void OnMouseLeave(System::Object^  sender, System::EventArgs^  e);
	virtual System::Void OnMouseWheel(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);

	virtual System::Void OnFormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e);
	virtual System::Void OnPaint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e);
	virtual System::Void OnTimerTick(System::Object^  sender, System::EventArgs ^e);
	virtual System::Void OnResize(System::Object^  sender, System::EventArgs^ e);

	virtual System::Void OnKeyDown(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e);
	virtual System::Void OnKeyUp(System::Object^  sender, System::Windows::Forms::KeyEventArgs^  e);

	virtual System::Void OnActivated(System::Object^  sender, System::EventArgs^  e);
	virtual System::Void OnShown(System::Object^  sender, System::EventArgs^  e);

	virtual System::Void OnDragDropEnter(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e);
	virtual System::Void OnDragDrop(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e);
	virtual System::Void OnDragOver(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e);

	inline UNIQUEID_TYPE getId() { return myId; }
	inline void setId(UNIQUEID_TYPE idValue) { myId = idValue; }

	int mapVirtualKeyCode(System::Windows::Forms::KeyEventArgs^ e, bool &bIsAffectingControlOut, bool &bIsAffectingAltOut, bool &bIsAffectingShiftOut);

	static gcroot<HyperUI::IBaseForm^> getMainForm();

	HDC getHDC();
	inline void setPreventRendering(bool bValue) { myPreventRendering = bValue; }

	void enumerateAllPixelFormats(string& strOut);

	// Editor
	void hideEditor();
	bool getIsInEditorMode();
#ifdef ALLOW_UI_EDITOR
	EditorPanel^ getEditorPanel();
#endif

// 	UNIQUEID_TYPE pushCursor(CursorType eType);
// 	void popCursor(UNIQUEID_TYPE idCursor);
	virtual Window* getWindow() { return myWindow; }
	inline bool getIsMainWindow() { return getWindow() ? getWindow()->getIsMainWindow() : false; }

	bool checkForClosure();

	OpenGLWindow^ getOpenGLWindow() { return myGlWindow; }

	static void simulateKeyPress(System::Windows::Forms::Keys eKeyCode, bool bGenerateUpEvent);

	virtual void onTabletButtonDown(int iTabletCursorId, FLOAT_TYPE fLastPenPressure) { }
	virtual void onTabletCursorMove(FLOAT_TYPE fLastPenPressure) { }
	virtual void onTabletButtonUp() { }
	virtual void setPenPressure(FLOAT_TYPE fLastPenPressure) { }

protected:

	HWND initializeUI(ResourceType eSourceCollection, int iOptWidth, int iOptHeight, bool bThinFrame, bool bFixedSize, int iMinWidth, int iMinHeight, const char* pcsTitle, int iCenterX, int iCenterY, bool bIsMainWindow, const char* pcsInitLayerToShow);
	void destroyUI();

	void initializeComponent();
	void initCommon();
	bool convertDragArgumentsToFileNameList(System::Windows::Forms::DragEventArgs^ e, TStringVector& vecNamesOut);

	Window* myWindow;
	SBaseFormWrapper *myFormWrapper;

protected:
	OpenGLWindow^ myGlWindow;
	TTouchVector *myMouseCoordsVector;
	bool myIsLButtonDown, myIsRButtonDown;
	bool myMoveLeft, myMoveRight, myMoveUp, myMoveDown;

	
	bool myPreventRendering;

#ifdef ALLOW_UI_EDITOR
	EditorPanel^ myEditorPanel;
#endif
private:

	UNIQUEID_TYPE myId;
	bool myIsInOnActivated;
};
/*****************************************************************************/
struct HYPERUI_API SBaseFormWrapper
{
	void setId(UNIQUEID_TYPE idValue)
	{
		myForm->setId(idValue); 
	}
	UNIQUEID_TYPE getId()
	{
		return myForm->getId();
	}

	gcroot<IBaseForm^> myForm;
};

};

typedef HyperUI::TWindowManager< HyperUI::SBaseFormWrapper > FormWindowManager;
#endif


#include "IBaseForm.hpp"


#ifdef WIN_COMPILE_UNMANAGED
#pragma managed(pop)
#endif
