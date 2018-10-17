#pragma once

struct SDate
{
	SDate() { reset(); }

	void reset()
	{
		myYear = 1970;
		myDay = 1;
		myMonth = MonthJanuary;

		myHour = 0;
		myMinute = 0;
		mySecond = 0;
	}

	int myYear, myDay;
	MonthType myMonth;
	int myHour, myMinute, mySecond;
};
