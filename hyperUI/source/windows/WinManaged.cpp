#include "stdafx.h"
#include "WinManaged.h"
#include <windows/WinIncludes.h>



namespace HyperUI
{
ref class ApplicationRefreshLoop;
HYPERUI_API gcroot<ApplicationRefreshLoop^> g_pMainWinProgram = nullptr;
HYPERUI_API int g_iChosenFormatIndex = -1;
HYPERUI_API PIXELFORMATDESCRIPTOR g_rChosenFormat;

template < class ITEM_TYPE >
HYPERUI_API TWindowManager<ITEM_TYPE>* TWindowManager<ITEM_TYPE>::theInstance = NULL;

HYPERUI_API gcroot<IBaseForm^> g_ptheMainForm = nullptr;

template < class ID_CLASS >
UNIQUEID_TYPE GlobalIdGeneratorInternal<ID_CLASS>::myGeneratorLastId = 0;

/*****************************************************************************/
public ref class MainMessageFilter : public System::Windows::Forms::IMessageFilter
{
public:
	//[SecurityPermissionAttribute(SecurityAction::LinkDemand, Flags = SecurityPermissionFlag::UnmanagedCode)]
	virtual bool PreFilterMessage(Message% m)
	{
		if(m.Msg == WM_SYSKEYUP)
		{
			if(m.WParam.ToInt64() == VK_MENU)
			{
				// The reason we go to this extent is because if we
				// pass the message on, we will send the mouse leave
				// event, and we would like to ignore it. Moreover,
				// it will actually remove the focus from the window -
				// so instead, we simulate the press ourselves.
				//g_bIgnoreNextMouseLeaveEvent = true;
				IBaseForm::simulateKeyPress(Keys::Alt, true);
				return true;
			}
		}
		else if(m.Msg == WM_SYSKEYDOWN)
		{
			if(m.WParam.ToInt64() == VK_SPACE)
			{
				// And this guy here is to prevent the Alt+Spacebar 
				// combo from opening the system menu, since we use it
				// for special zoom override.
				IBaseForm::simulateKeyPress(Keys::Space, false);
				return true;
			}
		}
		return false;
	}
};
/*****************************************************************************/
void initializeOSSpecific()
{
	System::Windows::Forms::Application::AddMessageFilter(gcnew MainMessageFilter());
}
/*****************************************************************************/
HYPERUI_API void run()
{
	run(nullptr);
}
/*****************************************************************************/
HYPERUI_API void run(const SWindowParms& rParms)
{
	IBaseForm^ pForm = gcnew IBaseForm(rParms.myWidth, rParms.myHeight, rParms.myIsThinFrame, rParms.myIsFixedSize, rParms.myInitCollectionType, rParms.myMinWidth, rParms.myMinHeight, rParms.myTitle.c_str(), rParms.myCenterX, rParms.myCenterY, true, rParms.myInitLayerToShow.c_str());
	run(pForm);
}
/*****************************************************************************/
void getHardwareInfo(string& strOut)
{
	strOut = "";

#ifdef DIRECTX_PIPELINE
	DXSTAGE2
#else

	using namespace System::Management;
	try
	{

		unsigned __int64 lSize64;
		unsigned int lSize;
		string strTemp;
		System::String ^pTemp;
		System::String ^wquery = gcnew System::String("select * from Win32_Processor");
		ManagementObjectSearcher ^searcher1 = gcnew ManagementObjectSearcher(wquery);
		ManagementObjectCollection ^pColl = searcher1->Get();
		for each (ManagementObject^ mo in pColl)
		{
			pTemp = mo["Name"]->ToString();
			stringToSTLString(pTemp, strTemp);
			strOut += "CPU: ";
			strOut += strTemp;
			strOut += "\n";
		}

		wquery = gcnew System::String("select * from Win32_PhysicalMemory");
		searcher1 = gcnew ManagementObjectSearcher(wquery);
		pColl = searcher1->Get();
		strOut += "System RAM: ";
		lSize64 = 0;
		for each (ManagementObject^ mo in pColl)
		{
			lSize64 += safe_cast<unsigned __int64>(mo["Capacity"]);
		}
		lSize64 = lSize64/(1024*1024*1024);
		StringUtils::longNumberToString(lSize64, strTemp);
		strOut += strTemp + "Gb\n";

		wquery = gcnew System::String("select * from Win32_VideoController");
		searcher1 = gcnew ManagementObjectSearcher(wquery);
		pColl = searcher1->Get();
		for each (ManagementObject^ mo in pColl)
		{
			strOut += "GPU: ";
			pTemp = mo["Name"]->ToString();
			stringToSTLString(pTemp, strTemp);
			strOut += strTemp;

			lSize = safe_cast<unsigned int>(mo["AdapterRAM"]);
			lSize = lSize/(1024*1024);
			StringUtils::longNumberToString(lSize, strTemp);
			//strTemp = mo["AdapterRAM"]->ToString();
			//stringToSTLString(pTemp, strTemp);
			strOut += " RAM: " + strTemp + "Mb";

			pTemp = mo["DriverVersion"]->ToString();
			stringToSTLString(pTemp, strTemp);
			strOut += " Driver Version: " + strTemp;

			strOut += "\n";
		}

		wquery = gcnew System::String("select * from Win32_DesktopMonitor");
		searcher1 = gcnew ManagementObjectSearcher(wquery);
		pColl = searcher1->Get();
		for each (ManagementObject^ mo in pColl)
		{
			strOut += "Monitor: ";
			pTemp = mo["Name"]->ToString();
			stringToSTLString(pTemp, strTemp);
			strOut += strTemp;

			pTemp = mo["ScreenWidth"]->ToString();
			stringToSTLString(pTemp, strTemp);
			strOut += " Resolution: " + strTemp;

			pTemp = mo["ScreenHeight"]->ToString();
			stringToSTLString(pTemp, strTemp);
			strOut += "x" + strTemp;

			strOut += "\n";
		}

		const GLubyte* pcsVendor = glGetString(GL_VENDOR);
		strOut += "OpenGL Vendor: ";
		if(pcsVendor)
			strOut += (const char*)pcsVendor;
		else
			strOut += "Null";
		strOut += "\n";

	}
	catch(...)
	{

	}
#endif
}
/*****************************************************************************/
void getPixelFormatInfo(PIXELFORMATDESCRIPTOR &rFormatIn, string& strOut)
{
	string strTemp;
	strOut = "";

	strOut += "Color Bits: ";
	StringUtils::numberToString(rFormatIn.cColorBits, strTemp);
	strOut += strTemp + "\n";

	strOut += "Alpha Bits: ";
	StringUtils::numberToString(rFormatIn.cAlphaBits, strTemp);
	strOut += strTemp + "\n";

	strOut += "Stencil Bits: ";
	StringUtils::numberToString(rFormatIn.cStencilBits, strTemp);
	strOut += strTemp + "\n";

	strOut += "Depth Bits: ";
	StringUtils::numberToString(rFormatIn.cDepthBits, strTemp);
	strOut += strTemp + "\n";

	strOut += "Accum Bits: ";
	StringUtils::numberToString(rFormatIn.cAccumBits, strTemp);
	strOut += strTemp + "\n";

	strOut += "Accum Alpha Bits: ";
	StringUtils::numberToString(rFormatIn.cAccumAlphaBits, strTemp);
	strOut += strTemp + "\n";

	strOut += "Aux Buffers: ";
	StringUtils::numberToString(rFormatIn.cAuxBuffers, strTemp);
	strOut += strTemp + "\n";

	strOut += "Visible mask: ";
	StringUtils::numberToString(rFormatIn.dwVisibleMask, strTemp);
	strOut += strTemp + "\n";

	strOut += "Flags: ";
	StringUtils::numberToString(rFormatIn.dwFlags, strTemp);
	strOut += strTemp + "\n";
	strOut += "PFD_DOUBLEBUFFER: ";
	strOut += (rFormatIn.dwFlags & PFD_DOUBLEBUFFER) ? TRUE_VALUE : FALSE_VALUE;
	strOut += "\n";
	strOut += "PFD_DRAW_TO_WINDOW: ";
	strOut += (rFormatIn.dwFlags & PFD_DRAW_TO_WINDOW) ? TRUE_VALUE : FALSE_VALUE;
	strOut += "\n";
	strOut += "PFD_DRAW_TO_BITMAP: ";
	strOut += (rFormatIn.dwFlags & PFD_DRAW_TO_BITMAP) ? TRUE_VALUE : FALSE_VALUE;
	strOut += "\n";
	strOut += "PFD_SUPPORT_OPENGL: ";
	strOut += (rFormatIn.dwFlags & PFD_SUPPORT_OPENGL) ? TRUE_VALUE : FALSE_VALUE;
	strOut += "\n";
	strOut += "PFD_GENERIC_ACCELERATED: ";
	strOut += (rFormatIn.dwFlags & PFD_GENERIC_ACCELERATED) ? TRUE_VALUE : FALSE_VALUE;
	strOut += "\n";
	strOut += "PFD_GENERIC_FORMAT: ";
	strOut += (rFormatIn.dwFlags & PFD_GENERIC_FORMAT) ? TRUE_VALUE : FALSE_VALUE;
	strOut += "\n";
	strOut += "PFD_SUPPORT_GDI: ";
	strOut += (rFormatIn.dwFlags & PFD_SUPPORT_GDI) ? TRUE_VALUE : FALSE_VALUE;
	strOut += "\n";
}
/*****************************************************************************/
void getChosenPixelFormatDescription(string& strOut)
{
	getPixelFormatInfo(g_rChosenFormat, strOut);

	string strTemp;
	StringUtils::numberToString(g_iChosenFormatIndex, strTemp);
	strOut = "Chosen Format Index: " + strTemp + "\n" + strOut;
}
/*****************************************************************************/

/*****************************************************************************
HYPERUI_API void setMainForm(System::Windows::Forms::Form^ pForm)
{
	g_ptheMainForm = safe_cast<IBaseForm^>(pForm);
}
/*****************************************************************************/
/*****************************************************************************
void SBaseFormWrapper::setId(UNIQUEID_TYPE idValue) 
{ 
	myForm->setId(idValue); 
}
/*****************************************************************************
UNIQUEID_TYPE SBaseFormWrapper::getId()
{
	return myForm->getId();
}
/*****************************************************************************
IBaseForm^ findFormByWindow(Window* pWindow)
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
/*****************************************************************************/

}
