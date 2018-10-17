#include "stdafx.h"
#include "OSXCore.h"

namespace HyperCore
{
extern STRING_TYPE g_strSharedPathEnvVar;
/*****************************************************************************/
void gLog(const char* format, ...)
{
#if defined(_DEBUG) || defined(_DEBUG) || defined(DEBUG_MODE)
	char pcsMessage[2048];
	va_list argptr;
	va_start(argptr, format);
	vsprintf(pcsMessage, format, argptr);
    va_end(argptr);
	NSLog([NSString stringWithCString:pcsMessage]);
#endif
}
/*****************************************************************************/
extern int g_iScreenScalingFactor;
int getScreenDensityScalingFactor(int iOptMaxFactor)
{
	if(g_iScreenScalingFactor == 0)
	{
		int iRes = 1;
#ifdef IOS_BUILD
            iRes = [[UIScreen mainScreen] scale];
#else
			if([[NSScreen mainScreen] respondsToSelector:@selector(backingScaleFactor)])
				iRes = [[NSScreen mainScreen] backingScaleFactor];
#endif

		gLog("Screen Density Final: %d\n", iRes);
		g_iScreenScalingFactor = iRes;

		if(iOptMaxFactor > 0 && g_iScreenScalingFactor > iOptMaxFactor)
			g_iScreenScalingFactor = iOptMaxFactor;
	}
	return g_iScreenScalingFactor;
}
/*****************************************************************************/
void getAppStartupPath(string& strOut)
{
	NSBundle* pBundle = [NSBundle mainBundle];
	strOut = getCStringSafe([pBundle resourcePath]);
}
/*****************************************************************************/
bool getEnvVariable(const char* pcsVarName, string& strResOut)
{
	strResOut = "";
	const char *pcsRes = getenv(pcsVarName);
	if(pcsRes)
		strResOut = pcsRes;
	return strResOut.length() > 0;
}
/*****************************************************************************/
const char* getCStringSafe(NSString* pStringIn)
{
	if(!pStringIn)
		return "";
	const char* pcsString = [pStringIn cStringUsingEncoding:NSASCIIStringEncoding];
	if(pcsString)
		return pcsString;
	else
		return "";
}
/*****************************************************************************/
NSString* getDocumentsDir()
{
#ifdef MAC_BUILD
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);	
	NSString *documentsDirectoryRaw = [paths objectAtIndex:0];
	documentsDirectoryRaw = [documentsDirectoryRaw stringByAppendingString:@"/"];
	NSString *documentsDirectory = [documentsDirectoryRaw stringByAppendingString:[[NSBundle mainBundle] bundleIdentifier]];
	
	NSError *pError = nil;
	[[NSFileManager defaultManager] createDirectoryAtPath:documentsDirectory withIntermediateDirectories:YES attributes:nil error:&pError];
#else
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);	
	NSString *documentsDirectory = [paths objectAtIndex:0];
#endif
	
	return documentsDirectory;
}
/*****************************************************************************/
void gStartOpeningPath(const char* pcsPath)
{
	STRING_TYPE strCommonPath;
	getCommonAppDataPath(strCommonPath);
    if(StringUtils::doesStartWith(pcsPath, strCommonPath.c_str()))
		return;
	
    if(StringUtils::doesStartWith(pcsPath, ResourceManager::getFullAppPath()))
		return;

#ifdef IOS_BUILD
#else
	NSURL* pUrl = BookmarkManager::getInstance()->getBookmark(pcsPath);
	if(!pUrl)
		return;
	bool bResult = [pUrl startAccessingSecurityScopedResource];
#endif
}
/*****************************************************************************/
void gEndOpeningPath(const char* pcsPath)
{
	STRING_TYPE strCommonPath;
	getCommonAppDataPath(strCommonPath);
    if(StringUtils::doesStartWith(pcsPath, strCommonPath.c_str()))
		return;

    if(StringUtils::doesStartWith(pcsPath, ResourceManager::getFullAppPath()))
		return;

#ifdef IOS_BUILD
#else
	NSURL* pUrl = BookmarkManager::getInstance()->getBookmark(pcsPath);
	if(!pUrl)
		return;
	[pUrl stopAccessingSecurityScopedResource];
#endif
}
/*****************************************************************************/
void getCommonAppDataPath(string& strPath)
{
	if(Environment::getInstance()->getDoesVarExist(g_strSharedPathEnvVar.c_str()))
	{
		strPath = Environment::getInstance()->getStringVar(g_strSharedPathEnvVar.c_str());
		return;
	}

	NSString *documentsDirectory = getDocumentsDir();
	strPath = getCStringSafe(documentsDirectory);
}
/*****************************************************************************/
};
