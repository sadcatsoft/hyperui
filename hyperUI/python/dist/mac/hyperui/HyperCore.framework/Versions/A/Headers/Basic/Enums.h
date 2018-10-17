#pragma once

/*****************************************************************************/
enum BezierPointCurveType
{
	BezierPointCurveTied = 0,		// Has control points that move together 
	BezierPointCurveSeparate,		// Has control points that move separately
	BezierPointCurveSharp,			// Sharp linear corner
};

const char* const g_pcsBezierPointCurveTypeStrings[] = 
{
	"bezier",
	"bezierCorner",
	"corner",
	0
};
/*****************************************************************************/
enum OrientationType
{
	OrientationUnknown = 0,
	OrientationCW,
	OrientationCCW
};
/*****************************************************************************/
enum SideType
{
	SideLeft = 0,
	SideRight,
	SideTop,
	SideBottom,
	SideCenter,

	SideLastPlaceholder
};

const char* const g_pcsSideStrings[SideLastPlaceholder + 1] =
{
	"Left",
	"Right",
	"Top",
	"Bottom",
	"Center",
	0,
};
/*****************************************************************************/
enum UnitType
{
	UnitPixels = 0,
	UnitCentimeters,
	UnitMillimeters,
	UnitInches,
	UnitPoints,
	UnitPercent,

	UnitLastPlaceholder
};

const char* const g_pcsUnitSuffixes[UnitLastPlaceholder + 1] =
{
	"px",
	"cm",
	"mm",
	"in",
	"pt",
	"%",

	0
};
/*****************************************************************************/
enum MonthType
{
	MonthJanuary = 0,
	MonthFebruary,
	MonthMarch,
	MonthApril,
	MonthMay,
	MonthJune,
	MonthJuly,
	MonthAugust,
	MonthSeptember,
	MonthOctober,
	MonthNovember,
	MonthDecember,
};

const char* const g_pcsShortMonthNames[MonthDecember + 1] =
{
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec"
};

const char* const g_pcsLongMonthNames[MonthDecember + 1] =
{
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December"
};

const int g_iMonthDurations[MonthDecember + 1] =
{
	31,
	28,
	31,
	30,
	31,
	30,
	31,
	31,
	30,
	31,
	30,
	31
};
/*****************************************************************************/
enum TimeSecondsType
{
	TimeSecondsPrintWhenNonZero = 0,
	TimeSecondsAlwaysPrint,
	TimeSecondsNeverPrint
};
/*****************************************************************************/
enum TimeHoursType
{
	TimeHoursPrintWhenNonZero = 0,
	TimeHoursAlwaysPrint
};
/*****************************************************************************/
