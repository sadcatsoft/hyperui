#pragma once


//#define NO_BORDER_WINDOW
#define RESIZE_BORDER_THICKNESS		3

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lp)                        ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lp)                        ((int)(short)HIWORD(lp))
#endif

//#include "ICursorManager.h"

//#include "EngineDefines.h"
//#include "IBaseForm.h"

#ifdef WIN_COMPILE_UNMANAGED
#pragma managed(push, on)
#endif

using namespace System::Windows::Forms;
//using namespace NTest;

namespace HyperUI
{

ref class IBaseForm;

/*****************************************************************************/
#ifdef USE_OPENGL2
class esContext
{
public:
	esContext() :
		nativeDisplay(0), nativeWin(0),
		eglDisplay(0), eglSurface(0), eglContext(0) 

		//		  nWindowWidth(0), nWindowHeight(0), nMouseX(0), nMouseY(0)
	{}

	~esContext() {}

	EGLNativeDisplayType nativeDisplay;
	EGLNativeWindowType nativeWin;
	EGLDisplay eglDisplay;
	EGLSurface eglSurface;
	EGLContext eglContext;


	/*
	int         nWindowWidth;
	int         nWindowHeight;
	int         nMouseX;
	int         nMouseY;
	*/
	// RenderState rs;
};

bool TestEGLError(HWND hWnd, char* pszLocation);

#endif
};
/*****************************************************************************/
#ifdef WIN_COMPILE_UNMANAGED
#pragma managed(push, on)
#endif
namespace HyperUI
{
/*****************************************************************************/
public ref class OpenGLWindow: public System::Windows::Forms::NativeWindow
{
public:
	OpenGLWindow(IBaseForm ^ parentForm, GLsizei iWidth, GLsizei iHeight);

	bool makeCurrent();
	void resize(int iW, int iH);
	void preRender();
	void postRender();

// 	void setCursor(CursorType eType) {	myCursorManager->setCursor(eType); 	}
// 	UNIQUEID_TYPE pushCursor(CursorType eType) {	return myCursorManager->pushCursor(eType); 	}
// 	void popCursor(UNIQUEID_TYPE idCursor) {	myCursorManager->popCursor(idCursor); 	}

	HDC getHDC() { return m_hDC; }

protected:

	~OpenGLWindow(System::Void);
	GLint MySetPixelFormat(HDC hdc);
	GLvoid ReSizeGLScene(GLsizei width, GLsizei height);

protected:

	void onButtonDown(int iX, int iY, MouseButtonType eButton, bool bFromTablet, FLOAT_TYPE fCurrPressure, int iTabletCursorId);
	void onMouseMove(int iX, int iY, bool bFromTablet, FLOAT_TYPE fCurrPressure);
	void onButtonUp(int iX, int iY, MouseButtonType eButton, bool bFromTablet, FLOAT_TYPE fCurrPressure, int iTabletCursorId);

	virtual void WndProc( Message % m ) override;

#ifdef USE_OPENGL2
	EGLBoolean SetupGL2(esContext &ctx);
	void DestroyNativeWin(EGLNativeDisplayType nativedisp, EGLNativeWindowType nativewin);
	void CloseNativeDisplay(EGLNativeDisplayType nativedisp);
	bool OpenNativeDisplay(EGLNativeDisplayType* nativedisp_out);
#endif

private:
	HDC m_hDC;

#ifdef USE_OPENGL2
	esContext *myGl2Context;
#else
	HGLRC m_hglrc;
#endif

	// This is now in Window.h
	//WinCursorManager *myCursorManager;
	
	IBaseForm ^ myParent;
	int myWidth,  myHeight;

	bool myIsTabletPenDown;
	bool myIsMouseDown;

	bool myIsMouseTracking;

	bool myDidRender;
};
/*****************************************************************************/
};

#include "OpenGLWindow.hpp"

#ifdef WIN_COMPILE_UNMANAGED
#pragma managed(pop)
#endif
