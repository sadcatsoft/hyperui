#include "stdafx.h"

/*****************************************************************************/
UTC_TYPE DateUtils::getCurrentUTCTime()
{
	time_t rawtime;
	time ( &rawtime );
	return rawtime;
}
/*****************************************************************************/
UTC_TYPE DateUtils::dateToUTCTime(const SDate& rDate)
{
	tm rTime;
	memset(&rTime, 0, sizeof(tm));
	rTime.tm_year = rDate.myYear - 1900;
	rTime.tm_mon = (int)rDate.myMonth;
	rTime.tm_mday = rDate.myDay;
	rTime.tm_hour = rDate.myHour;
	rTime.tm_min = rDate.myMinute;
	rTime.tm_sec = rDate.mySecond;
	time_t givenDate = mktime (&rTime );
	return givenDate;
}
/*****************************************************************************/
void DateUtils::convertUTCTimeToString(UTC_TYPE dSecondsSince1970, string& strOut, TimeSecondsType eSecondsOption, TimeHoursType eHoursType,
					bool bUseDigiClockFormat, bool bUseFullTimeWords, bool bUse24HourFormat)
{
	SDate rDateOut;
	DateUtils::convertUTCTimeToComponents(dSecondsSince1970, rDateOut);

	int iHours = rDateOut.myHour;
	int iMinutes = rDateOut.myMinute;
	int iSeconds = rDateOut.mySecond;

	double dTotalSeconds = iSeconds + iMinutes*60 + iHours*60*60;
	DateUtils::timeToString(dTotalSeconds, strOut, eSecondsOption, eHoursType, bUseDigiClockFormat, bUseFullTimeWords, bUse24HourFormat);
}
/*****************************************************************************/
void DateUtils::convertUTCTimeToComponents(UTC_TYPE dSecondsSince1970, SDate& rDateOut)
{
	time_t rawtime = dSecondsSince1970;
	struct tm * ptm;
	ptm = gmtime(&rawtime);

	rDateOut.reset();
	rDateOut.myDay = ptm->tm_mday;
	rDateOut.myMonth = (MonthType)ptm->tm_mon;
	rDateOut.myYear = ptm->tm_year + 1900;
	rDateOut.myHour = ptm->tm_hour;
	rDateOut.myMinute = ptm->tm_min;
	rDateOut.mySecond = ptm->tm_sec;

	//free(ptm);
}
/*****************************************************************************/
int DateUtils::computeDateDifferenceBetweenTodayAnd(int iDay, int iMonth, int iYear)
{
	tm rTime;
	memset(&rTime, 0, sizeof(tm));
	rTime.tm_year = iYear - 1900;
	rTime.tm_mon = (int)iMonth;
	rTime.tm_mday = iDay;
	time_t givenDate = mktime (&rTime );

	time_t rawtime;
	time (&rawtime);
	return difftime(rawtime, givenDate);
}
/*****************************************************************************/
