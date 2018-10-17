#include "stdafx.h"
#include "OSXCore.h"

namespace HyperCore
{
/*****************************************************************************/
UTC_TYPE DateUtils::getCurrentUTCTime()
{
	NSDate* today = [NSDate date];
    return [today timeIntervalSince1970];
}
/*****************************************************************************/
double DateUtils::getCurrentTime()
{
	static double conversion = 0.0;
	
	if( conversion == 0.0 )
	{
		mach_timebase_info_data_t info = {0,0};
		kern_return_t err = mach_timebase_info(&info);
		if (!err) {
			conversion = 1e-9 * (double)info.numer/(double)info.denom;
		}
	}
    
	return conversion * (double)mach_absolute_time();
}
/*****************************************************************************/
UTC_TYPE DateUtils::dateToUTCTime(const SDate& rDate)
{
	NSTimeZone *tz = [NSTimeZone timeZoneWithAbbreviation:@FIXED_TIMEZONE];

	NSCalendar *calendar = [[[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar] autorelease];
    NSLocale* formatterLocale = [[[NSLocale alloc] initWithLocaleIdentifier:@"en_GB"] autorelease];
    [calendar setLocale:formatterLocale];
    [calendar setTimeZone:tz];

	NSDateComponents *components = [[[NSDateComponents alloc] init] autorelease];
	[components setYear:rDate.myYear];
	[components setMonth: (int)rDate.myMonth + 1];
	[components setDay:rDate.myDay];
	[components setHour:rDate.myHour];
	[components setMinute:rDate.myMinute];
	[components setSecond:rDate.mySecond];

	NSDate *loc_date = [calendar dateFromComponents:components];
	return [loc_date timeIntervalSince1970];
}
/*****************************************************************************/
void DateUtils::convertUTCTimeToString(UTC_TYPE dSecondsSince1970, string& strOut, TimeSecondsType eSecondsOption, TimeHoursType eHoursType, bool bUseDigiClockFormat, bool bUseFullTimeWords, bool bUse24HourFormat)
{
	// We need to convert the GMT date from the seconds from 1970 GMT to the local time zone
	// (explicitly specified) and then take the hours/minutes.
	int iHours, iMinutes, iSeconds;

    NSTimeZone *tz = [NSTimeZone timeZoneWithAbbreviation:@FIXED_TIMEZONE];
    //NSTimeZone *tz = [NSTimeZone timeZoneWithAbbreviation:@"EST"];
 

    NSDate* pTempDate = [NSDate dateWithTimeIntervalSince1970: dSecondsSince1970];
/*
    NSDateFormatter *df_target_zone = [[[NSDateFormatter alloc] init] autorelease];
    [df_target_zone setTimeZone:tz];
    [df_target_zone setDateFormat:@"yyyy-MM-dd hh:mm:ss a"];
    
    
    NSString *pTargetZoneString = [df_target_zone stringFromDate:pTempDate];
  */  
    /*
	NSDate* pTempDate = [NSDate dateWithTimeIntervalSince1970: dSecondsSince1970];
	NSTimeZone *tz = [NSTimeZone timeZoneWithAbbreviation:@FIXED_TIMEZONE];
	NSInteger seconds = [tz secondsFromGMTForDate: pTempDate];
	NSDate* pLocalDate =  [NSDate dateWithTimeInterval: seconds sinceDate: pTempDate];
*/
	NSCalendar *calendar = [[[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar] autorelease];
    NSLocale* formatterLocale = [[[NSLocale alloc] initWithLocaleIdentifier:@"en_GB"] autorelease];
    [calendar setLocale:formatterLocale];
    [calendar setTimeZone:tz];
    
	NSDateComponents *components = [calendar components:(NSHourCalendarUnit | NSMinuteCalendarUnit | NSSecondCalendarUnit | NSDayCalendarUnit | NSMonthCalendarUnit | NSYearCalendarUnit) fromDate:pTempDate];
	iHours = [components hour];
	iMinutes = [components minute];
	iSeconds = [components second];
    
    int iYears = [components year];
    int iMonth = [components month];
    int iDay = [components day];

	double dTotalSeconds = iSeconds + iMinutes*60 + iHours*60*60;
	DateUtils::timeToString(dTotalSeconds, strOut, eSecondsOption, eHoursType, bUseDigiClockFormat, bUseFullTimeWords, bUse24HourFormat);
}
/*****************************************************************************/
void DateUtils::convertUTCTimeToComponents(UTC_TYPE dSecondsSince1970, SDate& rDateOut)
{
	int iHours, iMinutes, iSeconds;

    NSTimeZone *tz = [NSTimeZone timeZoneWithAbbreviation:@FIXED_TIMEZONE];
    //NSTimeZone *tz = [NSTimeZone timeZoneWithAbbreviation:@"EST"];

	NSDate* pTempDate = [NSDate dateWithTimeIntervalSince1970: dSecondsSince1970];
	NSCalendar *calendar = [[[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar] autorelease];
    NSLocale* formatterLocale = [[[NSLocale alloc] initWithLocaleIdentifier:@"en_GB"] autorelease];
    [calendar setLocale:formatterLocale];
    [calendar setTimeZone:tz];
    
	NSDateComponents *components = [calendar components:(NSHourCalendarUnit | NSMinuteCalendarUnit | NSSecondCalendarUnit | NSDayCalendarUnit | NSMonthCalendarUnit | NSYearCalendarUnit) fromDate:pTempDate];
	rDateOut.myHour = [components hour];
	rDateOut.myMinute = [components minute];
	rDateOut.mySecond = [components second];
    
    rDateOut.myYear = [components year];
    rDateOut.myMonth = (MonthType)([components month] - 1);
    rDateOut.myDay = [components day];
}
/*****************************************************************************/
}
