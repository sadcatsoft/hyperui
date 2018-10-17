#pragma once

/*****************************************************************************/
struct HYPERCORE_API SUVSet
{
	SUVSet() 
	{ 
		reset(); 
	}

	inline void reset() 
	{ 
		myStartX = myStartY = 0; 
		myEndX = myEndY = 1.0; 
		myApplyToSize = false;
	}

	FLOAT_TYPE myStartX, myStartY, myEndX, myEndY;
	bool myApplyToSize;
};
/*****************************************************************************/