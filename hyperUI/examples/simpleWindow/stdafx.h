#pragma once

#ifdef _WIN32
	#include <windows.h>
	#include "HyperUI.h"
#else
	#import <HyperCore/HyperCore.h>
	#import <HyperUI/HyperUI.h>
#endif

using namespace HyperUI;

void initialize();