#pragma once

/*****************************************************************************/
struct HYPERCORE_API SSize2D
{
	SSize2D()
	{
		w = 0;
		h = 0;
	}

	SSize2D(int dw, int dh)
	{
		w = dw;
		h = dh;
	}

	void set(int dw, int dh)
	{
		w = dw;
		h = dh;		
	}

	int w, h;
};
/*****************************************************************************/
