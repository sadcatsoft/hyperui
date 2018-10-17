/*****************************************************************************

Disclaimer: This software is supplied to you by Sad Cat Software
("Sad Cat") in consideration of your agreement to the following terms, and 
your use, installation, modification or redistribution of this Sad Cat software
constitutes acceptance of these terms.  If you do not agree with these terms,
please do not use, install, modify or redistribute this Sad Cat software.

This software is provided "as is". Sad Cat Software makes no warranties, 
express or implied, including without limitation the implied warranties
of non-infringement, merchantability and fitness for a particular
purpose, regarding Sad Cat's software or its use and operation alone
or in combination with other hardware or software products.

In no event shall Sad Cat Software be liable for any special, indirect,
incidental, or consequential damages (including, but not limited to, 
procurement of substitute goods or services; loss of use, data, or profits;
or business interruption) arising in any way out of the use, reproduction,
modification and/or distribution of Sad Cat's software however caused and
whether under theory of contract, tort (including negligence), strict
liability or otherwise, even if Sad Cat Software has been advised of the
possibility of such damage.

Copyright (C) 2012, Sad Cat Software. All Rights Reserved.

*****************************************************************************/
#include "stdafx.h"

#define ENABLE_LOGGING
#define MAX_LOG_RECORDS		5000

namespace HyperCore
{
TLogItemList Logger::theLog;
Mutex Logger::theInternalsLock;
/*****************************************************************************/
void Logger::getAsString(string& strOut, int iLogLevel, int iMaxRecordsToGet)
{
	strOut = "";

	if(iMaxRecordsToGet < 0)
		iMaxRecordsToGet = MAX_LOG_RECORDS;

#ifdef ENABLE_LOGGING
	theInternalsLock.lock();
	TLogItemList::iterator li;
	int iNumFilteredLines = 0;
	for(li = theLog.end(), li--; li != theLog.begin() && iNumFilteredLines < iMaxRecordsToGet; li--)
	{
		if((*li).myLevel & iLogLevel)
			iNumFilteredLines++;
	}

	for(; li != theLog.end(); li++)
	{
		if((*li).myLevel & iLogLevel)
			strOut += (*li).myValue;
	}
	theInternalsLock.unlock();
#endif
}
/*****************************************************************************/
void Logger::log(const char* format, ...)
{
#ifdef ENABLE_LOGGING
	va_list argptr;

	char* pcsFinalPtr = NULL;
	char pcsMessage[2048];
	char *pcsDynMessage = NULL;

	va_start(argptr, format);
#ifdef WINDOWS
	int iNeededBufferLen = _vscprintf(format, argptr);
#else
	int iNeededBufferLen = vsnprintf(NULL, NULL, format, argptr);
#endif
	va_end(argptr);

	if(iNeededBufferLen < 2047)
	{
		va_start(argptr, format);
		vsprintf(pcsMessage, format, argptr);
		va_end(argptr);
		pcsFinalPtr = pcsMessage;
	}
	else
	{
		pcsDynMessage = new char[iNeededBufferLen + 2];
		va_start(argptr, format);
		vsprintf(pcsDynMessage, format, argptr);
		va_end(argptr);

		pcsFinalPtr = pcsDynMessage;
	}

	int iLen = strlen(pcsFinalPtr);
	if(iLen == 0)
    {
        if(pcsDynMessage)
            delete[] pcsDynMessage;
		return;
    }
    
	SLogItem rItem;
	DateUtils::convertUTCTimeToString(DateUtils::getCurrentUTCTime(), rItem.myValue, TimeSecondsAlwaysPrint, TimeHoursAlwaysPrint, true, false, true);

	// Quick hack for selection
	if(format && format[0] == '|')
		rItem.myValue = "|" + rItem.myValue + ":|";
	else
		rItem.myValue += ": ";

	rItem.myValue += pcsFinalPtr;
	if(pcsFinalPtr[iLen - 1] != '\n')
		rItem.myValue += "\n";

	theInternalsLock.lock();

	theLog.push_back(rItem);
	while(theLog.size() > MAX_LOG_RECORDS)
		theLog.pop_front();

	theInternalsLock.unlock();

	StringUtils::replaceAllInstances(STR_LIT("%"), STR_LIT("%%"), rItem.myValue);
	gLog(rItem.myValue.c_str());

	if(pcsDynMessage)
		delete[] pcsDynMessage;
#endif

}
/*****************************************************************************/
void Logger::log(LogLevelType eLogLevel, const char* format, ...)
{
#ifdef ENABLE_LOGGING
	va_list argptr;

	char* pcsFinalPtr = NULL;
	char pcsMessage[2048];
	char *pcsDynMessage = NULL;

	va_start(argptr, format);
#ifdef WINDOWS
	int iNeededBufferLen = _vscprintf(format, argptr);
#else
	int iNeededBufferLen = vsnprintf(NULL, NULL, format, argptr);
#endif
	va_end(argptr);

	if(iNeededBufferLen < 2047)
	{
		va_start(argptr, format);
		vsprintf(pcsMessage, format, argptr);
		va_end(argptr);
		pcsFinalPtr = pcsMessage;
	}
	else
	{
		pcsDynMessage = new char[iNeededBufferLen + 2];
		va_start(argptr, format);
		vsprintf(pcsDynMessage, format, argptr);
		va_end(argptr);

		pcsFinalPtr = pcsDynMessage;
	}

	int iLen = strlen(pcsFinalPtr);
	if(iLen == 0)
    {
        if(pcsDynMessage)
            delete[] pcsDynMessage;
		return;
    }

	SLogItem rItem;
	rItem.myLevel = eLogLevel;
	DateUtils::convertUTCTimeToString(DateUtils::getCurrentUTCTime(), rItem.myValue, TimeSecondsAlwaysPrint, TimeHoursAlwaysPrint, true, false, true);
	
	// Quick hack for selection
	if(format && format[0] == '|')
		rItem.myValue = "|" + rItem.myValue + ":|";
	else
		rItem.myValue += ": ";

	rItem.myValue += pcsFinalPtr;
	if(pcsFinalPtr[iLen - 1] != '\n')
		rItem.myValue += "\n";

	theInternalsLock.lock();


	theLog.push_back(rItem);
	while(theLog.size() > MAX_LOG_RECORDS)
		theLog.pop_front();

	theInternalsLock.unlock();

	StringUtils::replaceAllInstances(STR_LIT("%"), STR_LIT("%%"), rItem.myValue);
	gLog(rItem.myValue.c_str());

	if(pcsDynMessage)
		delete[] pcsDynMessage;
#endif
}
/*****************************************************************************/
};