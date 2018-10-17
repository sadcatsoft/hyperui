#pragma once

#ifdef WIN_COMPILE_UNMANAGED
#pragma managed(push, on)
#endif

using namespace System;
using namespace System::Drawing;
using namespace System::Globalization;
using namespace System::Runtime::InteropServices;
using namespace System::Security;
using namespace System::Threading;
using namespace System::Windows::Forms;

namespace HyperUI
{
/*****************************************************************************/
public ref class ApplicationRefreshLoop
{
public:

	void Run(Form^ pForm)
	{
		System::Windows::Forms::Application::Idle += gcnew  EventHandler(this, &ApplicationRefreshLoop::onApplicationIdle);
		System::Windows::Forms::Application::Run(pForm);
	}

	void onApplicationIdle(Object^ sender, EventArgs^ e)
	{
		MSG rMsg;

		while (!PeekMessage(&rMsg, NULL, 0, 0, 0))
		{
			//UpdateFrame();
			DrawFrame();
		}
	}

	/*
	void onMainFormLoad(Object^ sender, EventArgs^ e)
	{
		myMainForm->OnPaint(nullptr, nullptr);
	}
	*/
	ApplicationRefreshLoop()
	{
		myLastFrameTime = DateTime::Now;
		//myMainForm = gcnew NTest::Form1();
		//myMainForm->Load += gcnew EventHandler(this, &MyGameProgram::onMainFormLoad);
	}

	void DrawFrame()
	{
#ifndef RUN_AS_FAST_AS_POSSIBLE
		Thread::Sleep(1); // give the CPU a break
#endif

		DateTime now = DateTime::Now;

		// You can throttle the maximum FPS here
#ifndef RUN_AS_FAST_AS_POSSIBLE
		if (now - myLastFrameTime >= TimeSpan::FromMilliseconds(1000.0/GAME_FRAMERATE))
#endif
		{
			bool bIsActive = ActivityManager::getInstance()->getIsActive();

			IBaseForm^ pBaseForm;
			FormWindowManager::Iterator fi;
			for(fi = FormWindowManager::getInstance()->windowsBegin(); !fi.isEnd(); fi++)
			{
				pBaseForm = fi.getWindow()->myForm;
				pBaseForm->OnTimerTick(nullptr, nullptr);
				if(bIsActive)
					pBaseForm->OnPaint(nullptr, nullptr);
			}

			// Run a second time to check for closures
			for(fi = FormWindowManager::getInstance()->windowsBegin(); !fi.isEnd(); )
			{
				if(pBaseForm->checkForClosure())
				{
					// A window has closed. Our iterator is invalid now.
					// Start all over.
					fi = FormWindowManager::getInstance()->windowsBegin();
				}
				else
					fi++;
			}
			myLastFrameTime = now;
			ActivityManager::getInstance()->onScreenRedrawn();
		}
		//GameProgram::UpdateFrame();
	}

// public:
// 	NTest::Form1 ^myMainForm;

private:
	DateTime myLastFrameTime;
};
/*****************************************************************************/
};
#ifdef WIN_COMPILE_UNMANAGED
#pragma managed(pop)
#endif
