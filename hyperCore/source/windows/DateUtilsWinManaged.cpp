#include "stdafx.h"
#include "WinManaged.h"

#ifdef WINDOWS
#pragma managed(pop)
#pragma managed(push, on)
#endif

#ifdef DIRECTX_PIPELINE
using namespace Windows::Globalization;
using namespace Windows::Globalization::DateTimeFormatting;
using namespace Windows::Foundation;
#else
#ifdef WIN32
using namespace System;
#endif
#endif

namespace HyperCore
{
#ifdef WIN32
/*****************************************************************************/
#ifdef DIRECTX_PIPELINE
void setToOrigin(Windows::Globalization::Calendar% rCalOrigin)
{
	rCalOrigin.Minute = 0;
	rCalOrigin.Hour = 0;
	rCalOrigin.Second = 0;
	rCalOrigin.Nanosecond = 0;
	rCalOrigin.Year = 1970;
	rCalOrigin.Month = 1;
	rCalOrigin.Day = 1;
}
#endif
/*****************************************************************************/
UTC_TYPE DateUtils::getCurrentUTCTime()
{
#ifdef DIRECTX_PIPELINE
	Windows::Globalization::Calendar rCalNow;
	rCalNow.SetToNow();

	Windows::Globalization::Calendar rCalOrigin;
	setToOrigin(rCalOrigin);
	return rCalNow.GetDateTime().UniversalTime - rCalOrigin.GetDateTime().UniversalTime;
#else
	return (DateTime::Now - DateTime(1970, 1, 1)).TotalSeconds;
#endif
}
/*****************************************************************************/
UTC_TYPE DateUtils::dateToUTCTime(const SDate& rDate)
{
#ifdef DIRECTX_PIPELINE
	Windows::Globalization::Calendar rCalNow;
	rCalNow.Minute = rDate.myMinute;
	rCalNow.Hour = rDate.myHour;
	rCalNow.Second = rDate.mySecond;
	rCalNow.Nanosecond = 0;
	rCalNow.Year = rDate.myYear;
	rCalNow.Month = rDate.myMonth + 1;
	rCalNow.Day = rDate.myDay;

	Windows::Globalization::Calendar rCalOrigin;
	setToOrigin(rCalOrigin);

	return rCalNow.GetDateTime().UniversalTime - rCalOrigin.GetDateTime().UniversalTime;
#else
	DateTime origin(rDate.myYear, rDate.myMonth + 1, rDate.myDay, rDate.myHour, rDate.myMinute, rDate.mySecond);
	return (origin - DateTime(1970, 1, 1)).TotalSeconds;
#endif
}
/*****************************************************************************/
void DateUtils::convertUTCTimeToString(UTC_TYPE dSecondsSince1970, string& strOut, TimeSecondsType eSecondsOption, TimeHoursType eHoursType, 
					bool bUseDigiClockFormat, bool bUseFullTimeWords, bool bUse24HourFormat)
{
	// We need to convert the GMT date from the seconds from 1970 GMT to the local time zone
	// (explicitly specified) and then take the hours/minutes.
	int iHours, iMinutes, iSeconds;
#ifdef DIRECTX_PIPELINE
	Windows::Globalization::Calendar origin;
	setToOrigin(origin);
	origin.AddSeconds(dSecondsSince1970);
#else
	DateTime origin(1970, 1, 1, 0, 0, 0, 0);
	origin = origin.AddSeconds(dSecondsSince1970);
#endif
	iHours = origin.Hour;
	iMinutes = origin.Minute;
	iSeconds = origin.Second;

	double dTotalSeconds = iSeconds + iMinutes*60 + iHours*60*60;
	DateUtils::timeToString(dTotalSeconds, strOut, eSecondsOption, eHoursType, bUseDigiClockFormat, bUseFullTimeWords, bUse24HourFormat);
}
/*****************************************************************************/
void DateUtils::convertUTCTimeToComponents(UTC_TYPE dSecondsSince1970, SDate& rDateOut)
{
#ifdef DIRECTX_PIPELINE
	Windows::Globalization::Calendar origin;
	setToOrigin(origin);
	origin.AddSeconds(dSecondsSince1970);
#else
	DateTime origin(1970, 1, 1, 0, 0, 0, 0);
	origin = origin.AddSeconds(dSecondsSince1970);
#endif

	rDateOut.reset();

	rDateOut.myYear = origin.Year;
	rDateOut.myMonth = (MonthType)(origin.Month - 1);
	rDateOut.myDay = origin.Day;

	rDateOut.myHour = origin.Hour;
	rDateOut.myMinute = origin.Minute;
	rDateOut.mySecond = origin.Second;
}
/*****************************************************************************/
int DateUtils::computeDateDifferenceBetweenTodayAnd(int iDay, int iMonth, int iYear)
{
	_ASSERT(iMonth >= 1 && iMonth <= 12);
#ifdef DIRECTX_PIPELINE
	Windows::Globalization::Calendar rCalNow;
	rCalNow.SetToNow();

	Windows::Globalization::Calendar rCalThen;
	rCalThen.Minute = 0;
	rCalThen.Hour = 0;
	rCalThen.Second = 0;
	rCalThen.Nanosecond = 0;
	rCalThen.Year = iYear;
	rCalThen.Month = iMonth;
	rCalThen.Day = iDay;

	// The interval is apparently stored in 100-nanosecond intervals, and we need it in days
	return (rCalNow.GetDateTime().UniversalTime - rCalThen.GetDateTime().UniversalTime)/864000000000L;

#else
	DateTime dateNow = DateTime::Now;
	DateTime dateStart(iYear, iMonth, iDay);

	System::TimeSpan diff1 = dateNow.Subtract(dateStart);
	double dValue = diff1.TotalDays;
	return dValue;
#endif
}
/*****************************************************************************/
#endif
};