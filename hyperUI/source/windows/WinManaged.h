#pragma once

#ifndef DIRECTX_PIPELINE
#include <vcclr.h>
#endif

namespace HyperUI
{
ref class IBaseForm;
IBaseForm^ findFormByWindow(Window* pWindow);
};