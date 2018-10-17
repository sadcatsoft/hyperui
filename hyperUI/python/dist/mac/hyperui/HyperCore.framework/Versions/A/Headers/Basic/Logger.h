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
#pragma once

#define DEFAULT_MAX_LINES		400

/*****************************************************************************/
enum LogLevelType
{
	LogLevelSystem		= 0x0001,
	LogLevelUser		= 0x0002,
	LogLevelError		= 0x0004,
};

#define LOG_LEVEL_ALL					(0xFFFFFFFF)
#define LOG_LEVEL_ALL_EXCEPT_USER		((LOG_LEVEL_ALL) & (~LogLevelUser))
/*****************************************************************************/
struct SLogItem
{
	SLogItem() { myLevel = LogLevelSystem; }
	string myValue;
	LogLevelType myLevel;
};
typedef list < SLogItem > TLogItemList;
/*****************************************************************************/
class HYPERCORE_API Logger
{
public:

	static void log(const char* format, ...);
	static void log(LogLevelType eLogLevel, const char* format, ...);
	static void getAsString(string& strOut, int iLogLevel = LOG_LEVEL_ALL_EXCEPT_USER, int iMaxRecordsToGet = DEFAULT_MAX_LINES);

private:

	static TLogItemList theLog;
	static Mutex theInternalsLock;
};
/*****************************************************************************/