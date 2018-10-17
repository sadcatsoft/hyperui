#pragma once
#include "stdafx.h"
//#include "WinManaged.h"
//#include <windows/WinIncludes.h>

#ifdef WIN_COMPILE_UNMANAGED
#pragma managed(push, on)
#endif

//#include "OpenGLWindow.h"

namespace HyperUI
{

//#define VISUAL_BORDER				RESIZE_BORDER_THICKNESS
#define VISUAL_BORDER				0

#ifndef NO_BORDER_WINDOW
#undef VISUAL_BORDER
#define VISUAL_BORDER	0
#endif

extern HYPERUI_API int g_iChosenFormatIndex;
extern HYPERUI_API PIXELFORMATDESCRIPTOR g_rChosenFormat;
/*****************************************************************************/
inline OpenGLWindow::OpenGLWindow(IBaseForm ^ parentForm, GLsizei iWidth, GLsizei iHeight)
{
	memset(&g_rChosenFormat, 0, sizeof(PIXELFORMATDESCRIPTOR));	

	myDidRender = false;
	myIsMouseDown = false;
	myIsTabletPenDown = false;
	//myCursorManager = new WinCursorManager;

	myParent = parentForm;
	CreateParams^ cp = gcnew CreateParams;

	// Set the position on the form
	cp->X = VISUAL_BORDER;
	cp->Y = VISUAL_BORDER;
	cp->Width = iWidth - VISUAL_BORDER*2;
	cp->Height = iHeight - VISUAL_BORDER*2;

	// Specify the form as the parent.
	cp->Parent = parentForm->Handle;

	// Create as a child of the specified parent
	// and make OpenGL compliant (no clipping)
	cp->Style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	// Create the actual window
	this->CreateHandle(cp);

	m_hDC = GetDC((HWND)this->Handle.ToPointer());
	if(m_hDC)
	{
#ifndef USE_OPENGL2
		ReSizeGLScene(iWidth, iHeight);
		MySetPixelFormat(m_hDC);
#endif		

#ifdef USE_OPENGL2
		myGl2Context = new esContext;
		SetupGL2(*myGl2Context);
		gInitShaders();
#endif

		ReSizeGLScene(iWidth, iHeight);
	}

	myIsMouseTracking = false;
}
/*****************************************************************************/
inline OpenGLWindow::~OpenGLWindow(System::Void)
{
	this->DestroyHandle();
#ifdef USE_OPENGL2
	delete myGl2Context;
#endif

// 	delete myCursorManager;
// 	myCursorManager = NULL;
}
/*****************************************************************************/
inline void OpenGLWindow::WndProc( Message % m )
{
	// Listen for messages that are sent to the button window. Some messages are sent
	// to the parent window instead of the button's window.
// 	int iMsg = m.Msg;
// 	if(iMsg != 512 && iMsg != 132 && iMsg != 32756 && iMsg != 32 && iMsg != 32757 && iMsg != 32752)
// 		gLog("Message: %d\n", m.Msg);
	switch ( m.Msg )
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		{
				DWORD dWord = m.LParam.ToInt32();
				int xPos = GET_X_LPARAM(dWord); 
				int yPos = GET_Y_LPARAM(dWord); 
				onButtonDown(xPos, yPos, m.Msg != WM_RBUTTONDOWN ? MouseButtonLeft : MouseButtonRight, false, 1.0, -1);
// 					System::Windows::Forms::MouseEventArgs e(m.Msg != WM_RBUTTONDOWN ? MouseButtons::Left : MouseButtons::Right, 1, xPos, yPos, 0);
// 					myParent->OnMouseDown(nullptr, %e);
		}
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		{
			DWORD dWord = m.LParam.ToInt32();
			int xPos = GET_X_LPARAM(dWord); 
			int yPos = GET_Y_LPARAM(dWord); 
			onButtonUp(xPos, yPos, m.Msg != WM_RBUTTONDOWN ? MouseButtonLeft : MouseButtonRight, false, 1.0, -1);
// 				System::Windows::Forms::MouseEventArgs e(m.Msg != WM_RBUTTONUP ? MouseButtons::Left : MouseButtons::Right, 1, xPos, yPos, 0);
// 				myParent->OnMouseUp(nullptr, %e);
		}
		break;
		
#ifdef NO_BORDER_WINDOW
	case WM_NCHITTEST:
		{
			m.Result = (System::IntPtr)HTTRANSPARENT;
/*
			DWORD dWord = m.LParam.ToInt32();
			POINT scrPoint;
			scrPoint.x = GET_X_LPARAM(dWord); 
			scrPoint.y = GET_Y_LPARAM(dWord); 
			ScreenToClient((HWND)this->Handle.ToPointer(), &scrPoint);

			RECT rcClient;
			GetClientRect((HWND)this->Handle.ToPointer(), &rcClient);

			if(scrPoint.y <= 40)
				m.Result = (System::IntPtr)HTCAPTION;
			else 
				m.Result = (System::IntPtr)HTCLIENT;
*/
		/*	else if(scrPoint.x <= RESIZE_BORDER_THICKNESS)
				m.Result = (System::IntPtr)HTLEFT;
			else if(scrPoint.x >= rcClient.right - RESIZE_BORDER_THICKNESS)
				m.Result = (System::IntPtr)HTRIGHT;
			else if(scrPoint.x >= 0 && scrPoint.x <= rcClient.right && scrPoint.y >= 0 && scrPoint.y <= rcClient.bottom)
				m.Result = (System::IntPtr)HTCLIENT;
			else
				m.Result = (System::IntPtr)HTNOWHERE;
*/
		}
		// DO NOT REPLACE WITH BRAKE!
		return;
#endif
		
	case WM_MOUSEMOVE:
		{
			{
				//gLog("Mouse move\n");
				if(!myIsMouseTracking)
				{
					TRACKMOUSEEVENT tme;
					tme.cbSize = sizeof(TRACKMOUSEEVENT);
					tme.dwFlags = TME_LEAVE;
					tme.hwndTrack = (HWND)this->Handle.ToPointer();
					if(TrackMouseEvent(&tme))
						myIsMouseTracking = true;
				}

				DWORD dWord = m.LParam.ToInt32();
				int xPos = GET_X_LPARAM(dWord); 
				int yPos = GET_Y_LPARAM(dWord); 
				onMouseMove(xPos, yPos, false, 1.0);
// 					System::Windows::Forms::MouseEventArgs e(MouseButtons::Left, 1, xPos, yPos, 0);
// 					myParent->OnMouseMove(nullptr, %e);
			}
		}
		break;	
	case WM_ERASEBKGND:
		{
			if(!myDidRender)
			{
				RECT rc;
				GetClientRect((HWND)this->Handle.ToPointer(),&rc);
				FillRect(m_hDC,&rc,(HBRUSH)GetStockObject(BLACK_BRUSH));
			}
		}
		break;
	case WM_MOUSELEAVE:
		{
			myIsMouseTracking = false;
			myParent->OnMouseLeave(nullptr, nullptr);
		}
		break;
	case WM_SETCURSOR:
		{
			DWORD dWord = m.LParam.ToInt32();
			if (LOWORD(dWord) == HTCLIENT)
			{
				myParent->getWindow()->restoreLastCursor();
				//m.Result = true;
			}						
		}
		break;

	case WT_PACKET:
		{
			PACKET pkt;
			if (gpWTPacket((HCTX)m.LParam.ToInt64(), m.WParam.ToInt64(), &pkt)) 
			{
// 					int xPos = pkt.pkX;
// 					int yPos = pkt.pkY;
				POINT scrPoint;
				scrPoint.x = pkt.pkX;
				scrPoint.y = pkt.pkY;
				ScreenToClient((HWND)this->Handle.ToPointer(), &scrPoint);
				int xPos = scrPoint.x;
				int yPos = scrPoint.y;
				int iCursor = pkt.pkCursor;
				int iButtonIndex = LOWORD(pkt.pkButtons);
				int iButtonPressType = HIWORD(pkt.pkButtons);

				Window* pParentWindow = this->myParent->getWindow();

				if(iButtonIndex == 0 && !pParentWindow->getIsTabletButtonDown())
				{
					FLOAT_TYPE fPressure = (FLOAT_TYPE)pkt.pkNormalPressure/getMaxPenPressure();
					if (iButtonPressType == TBN_UP) 
					{
						onButtonUp(xPos, yPos, MouseButtonLeft, true, fPressure, iCursor);
					}
					else if (iButtonPressType == TBN_DOWN) 
					{
						//						myIsTabletPenDown = true;
						gLog("Tablet Button down %d for w = %d\n", LOWORD(pkt.pkButtons), myWidth);
						onButtonDown(xPos, yPos, MouseButtonLeft, true, fPressure, iCursor);
						//gLog("Tablet Button down\n");
					}
					else if (iButtonPressType == TBN_NONE) 
					{
						if(myIsTabletPenDown)
						{
							//gLog("Raw pressure %d cursor = %d tangent = %d\n",pkt.pkNormalPressure, pkt.pkCursor, pkt.pkTangentPressure);
						}
						onMouseMove(xPos, yPos, true, fPressure);
					}
				}
				else if(iButtonIndex != 0)
				{
					// We need to tell our game engine so that we know
					// where events are coming from
					if(iButtonPressType == TBN_UP)
						pParentWindow->setIsTabletButtonDown(false);
					else if(iButtonPressType == TBN_DOWN)
						pParentWindow->setIsTabletButtonDown(true);
				}
			} // end if packet

		}
		break;
	
	case WM_ACTIVATEAPP:
		break;
		// Do something here in response to messages
		break;
	}
	NativeWindow::WndProc( m );
}
/*****************************************************************************/
inline void OpenGLWindow::onButtonDown(int iX, int iY, MouseButtonType eButton, bool bFromTablet, FLOAT_TYPE fCurrPressure, int iTabletCursorId)
{
	if((!bFromTablet && myIsTabletPenDown) || (bFromTablet && myIsMouseDown))
		return;

	//gLog("Button down isTablet = %d pressure = %g\n", bFromTablet, fCurrPressure);
	if(bFromTablet)
	{
		myIsTabletPenDown = true;
		myParent->setPenPressure(fCurrPressure);
		myParent->onTabletButtonDown(iTabletCursorId, fCurrPressure);
	}
	else
	{
		myParent->setPenPressure(1.0);
		myIsMouseDown = true;
	}
		
	System::Windows::Forms::MouseEventArgs e(eButton != MouseButtonRight ? MouseButtons::Left : MouseButtons::Right, 1, iX, iY, 0);
	myParent->OnMouseDown(nullptr, %e);
}
/*****************************************************************************/
inline void OpenGLWindow::onButtonUp(int iX, int iY, MouseButtonType eButton, bool bFromTablet, FLOAT_TYPE fCurrPressure, int iTabletCursorId)
{
	if((bFromTablet && !myIsTabletPenDown) || (!bFromTablet && !myIsMouseDown))
		return;

	//gLog("Button up isTablet = %d pressure = %g\n", bFromTablet, fCurrPressure);
	if(bFromTablet)
		myIsTabletPenDown = false;
	else
		myIsMouseDown = false;
	System::Windows::Forms::MouseEventArgs e(eButton != MouseButtonRight ? MouseButtons::Left : MouseButtons::Right, 1, iX, iY, 0);
	myParent->OnMouseUp(nullptr, %e);

	myParent->onTabletButtonUp();
}
/*****************************************************************************/
inline void OpenGLWindow::onMouseMove(int iX, int iY, bool bFromTablet, FLOAT_TYPE fCurrPressure)
{
	if((myIsTabletPenDown || myIsMouseDown) && ((bFromTablet && !myIsTabletPenDown) || (!bFromTablet && !myIsMouseDown)))
		return;

	if(myIsTabletPenDown)
		myParent->onTabletCursorMove(fCurrPressure);

/*
	if(myIsTabletPenDown || myIsMouseDown)
	{
		gLog("Mouse move isTablet = %d pressure = %g\n", myIsTabletPenDown, fCurrPressure);
	}*/

	System::Windows::Forms::MouseEventArgs e(MouseButtons::Left, 1, iX, iY, 0);
	myParent->OnMouseMove(nullptr, %e);
}
/*****************************************************************************/
inline void OpenGLWindow::resize(int iW, int iH)
{
	HWND hwnd = (HWND)this->Handle.ToPointer();
	iW -= VISUAL_BORDER*2;
	iH -= VISUAL_BORDER*2;
	MoveWindow(hwnd, VISUAL_BORDER, VISUAL_BORDER, iW, iH, true);

	makeCurrent();
	ReSizeGLScene(iW, iH);
}
/*****************************************************************************/
inline void OpenGLWindow::preRender(void)
{
	if(!makeCurrent())
		return;

	glClear(GL_COLOR_BUFFER_BIT);
	GraphicsUtils::transformForGlOrtho(myWidth, myHeight);
}
/*****************************************************************************/
inline void OpenGLWindow::postRender(void)
{
#ifdef USE_OPENGL2
	eglSwapBuffers(myGl2Context->eglDisplay, myGl2Context->eglSurface);
#else
	SwapBuffers(m_hDC);
#endif
	myDidRender = true;
}
/*****************************************************************************/
inline bool OpenGLWindow::makeCurrent()
{
#ifdef USE_OPENGL2
	eglMakeCurrent(myGl2Context->eglDisplay, myGl2Context->eglSurface, myGl2Context->eglSurface, myGl2Context->eglContext);
	if (!TestEGLError(NULL, "eglMakeCurrent"))
	{
		return false;
	}
	return true;
#else
	//gLog("Make current %x\n", m_hDC);
	if((wglMakeCurrent(m_hDC, m_hglrc)) == NULL)
	{
		//			MessageBox::Show("wglMakeCurrent Failed");
		return false;
	}
	return true;
#endif
}
/*****************************************************************************/
inline GLint OpenGLWindow::MySetPixelFormat(HDC hdc)
{
	PIXELFORMATDESCRIPTOR pfd = { 
		sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd 
		1,                                // version number 
		PFD_DRAW_TO_WINDOW |              // support window 
		PFD_SUPPORT_OPENGL |              // support OpenGL 
		PFD_DOUBLEBUFFER,                 // double buffered 
		PFD_TYPE_RGBA,                    // RGBA type 
		32,                               // 24-bit color depth 
		0, 0, 0, 0, 0, 0,                 // color bits ignored 
		8,                                // no alpha buffer 
		0,                                // shift bit ignored 
		0,                                // no accumulation buffer 
		0, 0, 0, 0,                       // accum bits ignored 
		32,                               // 32-bit z-buffer     
		8,                                // no stencil buffer 
		0,                                // no auxiliary buffer 
		PFD_MAIN_PLANE,                   // main layer 
		0,                                // reserved 
		0, 0, 0                           // layer masks ignored 
	}; 

	GLint  iPixelFormat; 

	// get the device context's best, available pixel format match 
	if((iPixelFormat = ChoosePixelFormat(hdc, &pfd)) == 0)
	{
		MessageBox::Show("ChoosePixelFormat Failed");
		return 0;
	}

	g_iChosenFormatIndex = iPixelFormat;
	g_rChosenFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	DescribePixelFormat(hdc, iPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &g_rChosenFormat);

	// make that match the device context's current pixel format 
	if(SetPixelFormat(hdc, iPixelFormat, &pfd) == FALSE)
	{
		MessageBox::Show("SetPixelFormat Failed");
		return 0;
	}

#ifdef USE_OPENGL2
#else

	if((m_hglrc = wglCreateContext(m_hDC)) == NULL)
	{
		MessageBox::Show("wglCreateContext Failed");
		return 0;
	}
#endif
	if(!makeCurrent())
		return 0;

	// 		if((wglMakeCurrent(m_hDC, m_hglrc)) == NULL)
	// 		{
	// 			MessageBox::Show("wglMakeCurrent Failed");
	// 			return 0;
	// 		}

	if(g_rChosenFormat.dwFlags & PFD_GENERIC_FORMAT)
		MessageBox::Show("Unable to find a hardware-accelerated graphics mode, and will likely run extremely slow as a result.\n\nPlease install the latest drivers for your video card and relaunch the application.", "Error");

	return 1;
}
/*****************************************************************************/
inline GLvoid OpenGLWindow::ReSizeGLScene(GLsizei width, GLsizei height)		// Resize and initialise the gl window
{
	if(!makeCurrent())
		return;
	if (height==0)										// Prevent A Divide By Zero By
	{
		height=1;										// Making Height Equal One
	}

	myWidth = width;
	myHeight = height;
	glViewport(0,0,width,height);						// Reset The Current Viewport
#ifndef USE_OPENGL2
	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
#endif
}
/*****************************************************************************/
#ifdef USE_OPENGL2
bool OpenGLWindow::OpenNativeDisplay(EGLNativeDisplayType* nativedisp_out)
{
	*nativedisp_out = (EGLNativeDisplayType) NULL;
	return true;
}
/*****************************************************************************/
void OpenGLWindow::CloseNativeDisplay(EGLNativeDisplayType nativedisp)
{
}
/*****************************************************************************/
void OpenGLWindow::DestroyNativeWin(EGLNativeDisplayType nativedisp, EGLNativeWindowType nativewin)
{
	this->DestroyHandle();
	// 		WINDOWINFO info;
	// 		GetWindowInfo((HWND) nativewin, &info);
	// 		DestroyWindow((HWND) nativewin);
	// 		UnregisterClass((LPCTSTR) info.atomWindowType, GetModuleHandle(NULL));
}
/*****************************************************************************/
EGLBoolean OpenGLWindow::SetupGL2(esContext &ctx)
{
	// Windows variables
	HWND				hWnd	= (HWND)this->Handle.ToPointer();
	HDC					hDC		= m_hDC = GetDC((HWND)this->Handle.ToPointer());

	// EGL variables
	EGLDisplay			eglDisplay	= 0;
	EGLSurface			eglSurface	= 0;
	EGLContext			eglContext	= 0;

	EGLConfig			eglConfig	= 0;
	EGLNativeWindowType	eglWindow	= 0;


	/*
	Step 0 - Create a EGLNativeWindowType that we can use for OpenGL ES output
	*/
#ifndef NO_GDI
	/*
	// Register the windows class
	WNDCLASS sWC;
	sWC.style = CS_HREDRAW | CS_VREDRAW;
	sWC.lpfnWndProc = WndProc;
	sWC.cbClsExtra = 0;
	sWC.cbWndExtra = 0;
	sWC.hInstance = hInstance;
	sWC.hIcon = 0;
	sWC.hCursor = 0;
	sWC.lpszMenuName = 0;
	sWC.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	sWC.lpszClassName = WINDOW_CLASS;
	unsigned int nWidth = WINDOW_WIDTH;
	unsigned int nHeight = WINDOW_HEIGHT;

	ATOM registerClass = RegisterClass(&sWC);
	if (!registerClass)
	{
	MessageBox(0, _T("Failed to register the window class"), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}
	*/
#if defined(UNDER_CE)
	// Get the monitor dimensions
	{
		HMONITOR	hMonitor;
		BOOL		bRet;
		POINT		p;
		MONITORINFO sMInfo;

		p.x			= 0;
		p.y			= 0;
		hMonitor	= MonitorFromPoint(p, MONITOR_DEFAULTTOPRIMARY);
		sMInfo.cbSize = sizeof(sMInfo);
		bRet = GetMonitorInfo(hMonitor, &sMInfo);
		if (!bRet)
		{
			//Forms::MessageBox(0, _T("Failed to get monitor info"), _T("Error"), MB_OK|MB_ICONEXCLAMATION);
			goto cleanup;
		}

		nWidth = sMInfo.rcMonitor.right - sMInfo.rcMonitor.left;
		nHeight = sMInfo.rcMonitor.bottom - sMInfo.rcMonitor.top;
	}
#endif

	eglWindow = hWnd;

	/*
	// Create the eglWindow
	RECT	sRect;
	SetRect(&sRect, 0, 0, nWidth, nHeight);
	AdjustWindowRectEx(&sRect, WS_CAPTION | WS_SYSMENU, false, 0);
	hWnd = CreateWindow( WINDOW_CLASS, _T("HelloTriangle"), WS_VISIBLE | WS_SYSMENU,
	0, 0, nWidth, nHeight, NULL, NULL, hInstance, NULL);
	eglWindow = hWnd;

	// Get the associated device context
	hDC = GetDC(hWnd);
	if (!hDC)
	{
	MessageBox(0, _T("Failed to create the device context"), _T("Error"), MB_OK|MB_ICONEXCLAMATION);
	goto cleanup;
	}
	*/
#endif
	/*
	Step 1 - Get the default display.
	EGL uses the concept of a "display" which in most environments
	corresponds to a single physical screen. Since we usually want
	to draw to the main screen or only have a single screen to begin
	with, we let EGL pick the default display.
	Querying other displays is platform specific.
	*/
	eglDisplay = eglGetDisplay(hDC);

	if(eglDisplay == EGL_NO_DISPLAY)
		eglDisplay = eglGetDisplay((EGLNativeDisplayType) EGL_DEFAULT_DISPLAY);
	/*
	Step 2 - Initialize EGL.
	EGL has to be initialized with the display obtained in the
	previous step. We cannot use other EGL functions except
	eglGetDisplay and eglGetError before eglInitialize has been
	called.
	If we're not interested in the EGL version number we can just
	pass NULL for the second and third parameters.
	*/
	EGLint iMajorVersion, iMinorVersion;
	if (!eglInitialize(eglDisplay, &iMajorVersion, &iMinorVersion))
	{
#ifndef NO_GDI
		//Windows::Forms::MessageBox("eglInitialize() failed.", "Error", MB_OK|MB_ICONEXCLAMATION);
#endif
		goto cleanup;
	}

	gLog("Initialized GLES, version = %d.%d\n", iMajorVersion, iMinorVersion);

	/*
	Step 3 - Make OpenGL ES the current API.
	EGL provides ways to set up OpenGL ES and OpenVG contexts
	(and possibly other graphics APIs in the future), so we need
	to specify the "current API".
	*/
	eglBindAPI(EGL_OPENGL_ES_API);
	if (!TestEGLError(hWnd, "eglBindAPI"))
	{
		goto cleanup;
	}

	/*
	Step 4 - Specify the required configuration attributes.
	An EGL "configuration" describes the pixel format and type of
	surfaces that can be used for drawing.
	For now we just want to use the default Windows surface,
	i.e. it will be visible on screen. The list
	has to contain key/value pairs, terminated with EGL_NONE.
	*/
	const EGLint pi32ConfigAttribs[] =
	{
 		EGL_LEVEL,				0,
 		EGL_SURFACE_TYPE,		EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES2_BIT,
		EGL_NATIVE_RENDERABLE,	EGL_FALSE,
		EGL_DEPTH_SIZE,			EGL_DONT_CARE,
 		EGL_BLUE_SIZE,			8,  
 		EGL_GREEN_SIZE,			8,  
 		EGL_RED_SIZE,			8,  
		EGL_NONE
	};

	/*
	Step 5 - Find a config that matches all requirements.
	eglChooseConfig provides a list of all available configurations
	that meet or exceed the requirements given as the second
	argument. In most cases we just want the first config that meets
	all criteria, so we can limit the number of configs returned to 1.
	*/
	EGLint iConfigs;
	if (!eglChooseConfig(eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &iConfigs) || (iConfigs != 1))
	{
#ifndef NO_GDI
		//Windows::Forms::MessageBox(0, "eglChooseConfig() failed.", "Error", MB_OK|MB_ICONEXCLAMATION);
#endif
		TestEGLError(hWnd, "eglChooseConfig");
		goto cleanup;
	}

	/*
	Step 6 - Create a surface to draw to.
	Use the config picked in the previous step and the native window
	handle when available to create a window surface. A window surface
	is one that will be visible on screen inside the native display (or
	fullscreen if there is no windowing system).
	Pixmaps and pbuffers are surfaces which only exist in off-screen
	memory.
	*/
	eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, eglWindow, NULL);

	if(eglSurface == EGL_NO_SURFACE)
	{
		eglGetError(); // Clear error
		eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, NULL, NULL);
	}

	if (!TestEGLError(hWnd, "eglCreateWindowSurface"))
	{
		goto cleanup;
	}

	/*
	Step 7 - Create a context.
	EGL has to create a context for OpenGL ES. Our OpenGL ES resources
	like textures will only be valid inside this context
	(or shared contexts)
	*/
	EGLint ai32ContextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
	eglContext = eglCreateContext(eglDisplay, eglConfig, NULL, ai32ContextAttribs);
	if (!TestEGLError(hWnd, "eglCreateContext"))
	{
		goto cleanup;
	}

	/*
	Step 8 - Bind the context to the current thread and use our
	window surface for drawing and reading.
	Contexts are bound to a thread. This means you don't have to
	worry about other threads and processes interfering with your
	OpenGL ES application.
	We need to specify a surface that will be the target of all
	subsequent drawing operations, and one that will be the source
	of read operations. They can be the same surface.
	*/
	eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
	if (!TestEGLError(hWnd, "eglMakeCurrent"))
	{
		goto cleanup;
	}

	myGl2Context->eglDisplay = eglDisplay;
	myGl2Context->eglSurface = eglSurface;
	myGl2Context->eglContext = eglContext;

	return true;

cleanup:
	eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglTerminate(eglDisplay);

	return false;

	/*
	EGLBoolean bsuccess;

	// create native window
	EGLNativeDisplayType nativeDisplay;
	if(!OpenNativeDisplay(&nativeDisplay))
	{
	printf("Could not get open native display\n");
	return GL_FALSE;
	}

	// get egl display handle
	EGLDisplay eglDisplay;
	eglDisplay = eglGetDisplay(nativeDisplay);
	if(eglDisplay == EGL_NO_DISPLAY)
	{
	printf("Could not get EGL display\n");
	CloseNativeDisplay(nativeDisplay);
	return GL_FALSE;
	}
	ctx.eglDisplay = eglDisplay;

	// Initialize the display
	EGLint major = 0;
	EGLint minor = 0;
	bsuccess = eglInitialize(eglDisplay, &major, &minor);
	if (!bsuccess)
	{
	printf("Could not initialize EGL display\n");
	CloseNativeDisplay(nativeDisplay);
	return GL_FALSE;
	}
	//if (major < 1 || minor < 4)
	if (major < 1 || minor < 3)
	{
	// Does not support EGL 1.4
	printf("System does not support at least EGL 1.4\n");
	CloseNativeDisplay(nativeDisplay);
	return GL_FALSE;
	}

	// Obtain the first configuration with a depth buffer
	EGLint attrs[] = { EGL_DEPTH_SIZE, 16, EGL_NONE };
	EGLint numConfig =0;
	EGLConfig eglConfig = 0;
	bsuccess = eglChooseConfig(eglDisplay, attrs, &eglConfig, 1, &numConfig);
	if (!bsuccess)
	{
	printf("Could not find valid EGL config\n");
	CloseNativeDisplay(nativeDisplay);
	return GL_FALSE;
	}

	// Get the native visual id
	int nativeVid;
	if (!eglGetConfigAttrib(eglDisplay, eglConfig, EGL_NATIVE_VISUAL_ID, &nativeVid))
	{
	printf("Could not get native visual id\n");
	CloseNativeDisplay(nativeDisplay);
	return GL_FALSE;
	}

	EGLNativeWindowType nativeWin = (EGLNativeWindowType)((HWND)this->Handle.ToPointer());
	//  		if(!CreateNativeWin(nativeDisplay, 640, 480, nativeVid, &nativeWin))
	//  		{
	//  			printf("Could not create window\n");
	//  			CloseNativeDisplay(nativeDisplay);
	//  			return GL_FALSE;
	//  		}

	// Create a surface for the main window
	EGLSurface eglSurface;
	eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, nativeWin, NULL);
	if (eglSurface == EGL_NO_SURFACE)
	{
	printf("Could not create EGL surface\n");
	DestroyNativeWin(nativeDisplay, nativeWin);
	CloseNativeDisplay(nativeDisplay);
	return GL_FALSE;
	}
	ctx.eglSurface = eglSurface;

	// Create an OpenGL ES context
	EGLContext eglContext;
	eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, NULL);
	if (eglContext == EGL_NO_CONTEXT)
	{
	printf("Could not create EGL context\n");
	DestroyNativeWin(nativeDisplay, nativeWin);
	CloseNativeDisplay(nativeDisplay);
	return GL_FALSE;
	}

	// Make the context and surface current
	bsuccess = eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
	if(!bsuccess)
	{
	printf("Could not activate EGL context\n");
	DestroyNativeWin(nativeDisplay, nativeWin);
	CloseNativeDisplay(nativeDisplay);
	return GL_FALSE;
	}

	ctx.nativeDisplay = nativeDisplay;
	ctx.nativeWin = nativeWin;
	return GL_TRUE;
	*/
}
/*****************************************************************************/
bool TestEGLError(HWND hWnd, char* pszLocation)
{
	/*
	eglGetError returns the last error that has happened using egl,
	not the status of the last called function. The user has to
	check after every single egl call or at least once every frame.
	*/
	EGLint iErr = eglGetError();
	if (iErr != EGL_SUCCESS)
	{
#ifndef NO_GDI
		//		TCHAR pszStr[256];
		//		_stprintf(pszStr, _T("%s failed (%d).\n"), pszLocation, iErr);
		//		MessageBox(hWnd, pszStr, _T("Error"), MB_OK|MB_ICONEXCLAMATION);
#endif
		gLog("EGL ERROR: %d at %s\n", iErr, pszLocation);
		return false;
	}

	return true;
}
/*****************************************************************************/
#endif
};