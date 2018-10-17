#include "stdafx.h"

namespace HyperCore
{
/*****************************************************************************/
void DateUtils::timeToString(double fLeftOver, string& strOut, TimeSecondsType eSecondsOption, TimeHoursType eHoursType, bool bUseDigiClockFormat, bool bUseFullTimeWords, bool bUse24HourFormat)
{
	int fMonths = (int)(fLeftOver/(60.0*60.0*24.0*30.42) );
	fLeftOver = fLeftOver - fMonths*(60.0*60.0*24.0*30.42);

	int fDays = (int)(fLeftOver/(60.0*60.0*24.0) );
	fLeftOver = fLeftOver - fDays*(60.0*60.0*24.0);

	int fHours = (int)(fLeftOver/(60.0*60.0) );
	fLeftOver = fLeftOver - fHours*(60.0*60.0);

	int fMinutes = (int)(fLeftOver/(60.0) );
	fLeftOver = fLeftOver - fMinutes*(60.0);

	int fSeconds = fLeftOver;
	strOut = "";
	char pcsBuff[32];

	// Compose the string
	if(bUseDigiClockFormat)
	{
		// 00:00:00 format style
		if(fMonths > 0)
		{
			sprintf(pcsBuff, "%.2d", fMonths);
			strOut += pcsBuff;
		}
		if(fDays > 0)
		{
			if(strOut.length() > 0)
				strOut += ":";
			sprintf(pcsBuff, "%.2d", fDays);
			strOut += pcsBuff;
		}

		if( (fHours > 0 && eHoursType == TimeHoursPrintWhenNonZero) || eHoursType == TimeHoursAlwaysPrint)
		{
			int iActualPrintHorus = fHours;
			if(!bUse24HourFormat && iActualPrintHorus > 12)
				iActualPrintHorus -= 12;

			if(strOut.length() > 0)
				strOut += ":";
			if(bUse24HourFormat)
				sprintf(pcsBuff, "%.2d", iActualPrintHorus);
			else
				sprintf(pcsBuff, "%d", iActualPrintHorus);
			strOut += pcsBuff;
		}

		if(strOut.length() > 0)
		{
			strOut += ":";
			sprintf(pcsBuff, "%.2d", fMinutes);
		}
		else
			sprintf(pcsBuff, "%d", fMinutes);
		strOut += pcsBuff;


		// Always include seconds in this format
		if(eSecondsOption == TimeSecondsAlwaysPrint || (eSecondsOption == TimeSecondsPrintWhenNonZero && (int)fSeconds > 0))
		{
			if(strOut.length() > 0)
				strOut += ":";
			sprintf(pcsBuff, "%.2d", fSeconds);
			strOut += pcsBuff;
		}

		if(!bUse24HourFormat)
		{
			if(fHours < 12)
				strOut += "am";
			else
				strOut += "pm";
		}

	}
	else
	{
		if(fMonths > 0)
		{
			if(bUseFullTimeWords)
			{
				if(fMonths != 1)
					sprintf(pcsBuff, "%d months", fMonths);
				else
					sprintf(pcsBuff, "%d month", fMonths);
			}
			else
				sprintf(pcsBuff, "%dM", fMonths);
			strOut += pcsBuff;
		}

		if(fDays > 0)
		{
			if(strOut.length() > 0)
				strOut += " ";
			if(bUseFullTimeWords)
			{
				if(fDays != 1)
					sprintf(pcsBuff, "%d days", fDays);
				else
					sprintf(pcsBuff, "%d day", fDays);
			}
			else
				sprintf(pcsBuff, "%dd", fDays);
			strOut += pcsBuff;
		}

		if( (fHours > 0 && eHoursType == TimeHoursPrintWhenNonZero) || eHoursType == TimeHoursAlwaysPrint)
		{
			if(strOut.length() > 0)
				strOut += " ";
			if(bUseFullTimeWords)
			{
				if(fHours != 1)
					sprintf(pcsBuff, "%d hours", fHours);
				else
					sprintf(pcsBuff, "%d hour", fHours);
			}
			else
				sprintf(pcsBuff, "%dh", fHours);
			strOut += pcsBuff;
		}

		if(fMinutes > 0)
		{
			if(strOut.length() > 0)
				strOut += " ";
			if(bUseFullTimeWords)
			{
				if(fMinutes != 1)
					sprintf(pcsBuff, "%d minutes", fMinutes);
				else
					sprintf(pcsBuff, "%d minute", fMinutes);
			}
			else
				sprintf(pcsBuff, "%dm", fMinutes);
			strOut += pcsBuff;
		}

		if(strOut.length() <= 0 || eSecondsOption == TimeSecondsAlwaysPrint || (eSecondsOption == TimeSecondsPrintWhenNonZero && (int)fSeconds > 0) )
		{
			if(strOut.length() > 0)
				strOut += " ";
			if(bUseFullTimeWords)
			{
				if(fSeconds != 1)
					sprintf(pcsBuff, "%d seconds", fSeconds);
				else
					sprintf(pcsBuff, "%d second", fSeconds);
			}
			else
				sprintf(pcsBuff, "%ds", fSeconds);
			strOut += pcsBuff;
		}
	}
}
/*****************************************************************************/
#if defined(WIN32) || defined(LINUX)
double DateUtils::getCurrentTime()
{
	return (double)clock()/(double)CLOCKS_PER_SEC;
}
#endif
/*****************************************************************************/
void DateUtils::dateToString(int iDay, MonthType eMonth, int iYear, bool bUseShortMonthName, string& strOut)
{
	strOut = bUseShortMonthName ? g_pcsShortMonthNames[eMonth] : g_pcsLongMonthNames[eMonth];

	char pcsBuff[128];
	sprintf(pcsBuff, " %d, ", iDay);
	strOut += pcsBuff;

	sprintf(pcsBuff, "%d", iYear);
	strOut += pcsBuff;
}
/*****************************************************************************/
int DateUtils::getCumulativeDaysFromYearStart(int iDay, int iMonth, int iYear)
{
	SDate rFullDate;
	rFullDate.myYear = iYear;
	rFullDate.myMonth = (MonthType)iMonth;
	rFullDate.myDay = iDay;
	UTC_TYPE lCurrDate = DateUtils::dateToUTCTime(rFullDate);

	SDate rYearBeginDate;
	rYearBeginDate.myYear = iYear;
	UTC_TYPE lYearBeginDate = DateUtils::dateToUTCTime(rYearBeginDate);
	int iCumulDays = (lCurrDate - lYearBeginDate)/(SECONDS_PER_DAY);
	return iCumulDays;
}
/*****************************************************************************/
};