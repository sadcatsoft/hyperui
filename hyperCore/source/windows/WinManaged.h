#pragma once

namespace HyperCore
{
#ifdef DIRECTX_PIPELINE
void stringToSTLString(Platform::String^ pStringIn, string& rStringOut);
#else
void stringToSTLString(System::String^ pStringIn, string& rStringOut);
#endif
};