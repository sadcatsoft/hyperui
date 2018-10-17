#pragma once

/*****************************************************************************/
class HYPERCORE_API DateUtils
{
public:
	static void timeToString(double fLeftOver, string& strOut, TimeSecondsType eSecondsOption, TimeHoursType eHoursType, bool bUseDigiClockFormat, bool bUseFullTimeWords, bool bUse24HourFormat);
	static void convertUTCTimeToString(UTC_TYPE dSecondsSince1970, string& strOut, TimeSecondsType eSecondsOption, TimeHoursType eHoursType, bool bUseDigiClockFormat, bool bUseFullTimeWords, bool bUse24HourFormat);
	static void convertUTCTimeToComponents(UTC_TYPE dSecondsSince1970, SDate& rDateOut);
	static void dateToString(int iDay, MonthType eMonth, int iYear, bool bUseShortMonthName, string& strOut);

	// Note that the month here is 1-based
	static int computeDateDifferenceBetweenTodayAnd(int iDay, int iMonth, int iYear);

	static int getCumulativeDaysFromYearStart(int iDay, int iMonth, int iYear);

	static UTC_TYPE dateToUTCTime(const SDate& rDate);

	static UTC_TYPE getCurrentUTCTime();

	// In seconds
	static double getCurrentTime();
};
/*****************************************************************************/
