#pragma once

/*****************************************************************************/
enum ColorSource
{
	ColorSourceHue,
	ColorSourceSaturation,
	ColorSourceBrightness,
	ColorSourceRed,
	ColorSourceGreen,
	ColorSourceBlue,
	ColorSourceNone
};
/*****************************************************************************/
class HYPERUI_API ColorPickerColorGetter : public IVertexColorGetter
{
public:
	ColorPickerColorGetter()
	{
		myHorColorSource = ColorSourceNone;
		myVertColorSource = ColorSourceNone;
		myBaseHSB[0] = myBaseHSB[1] = myBaseHSB[2] = 1.0;
		myBaseColor.set(1, 1, 1, 1);
	}
	ColorPickerColorGetter(ColorSource eHorColorSource, ColorSource eVertColorSource)
	{
		myHorColorSource = eHorColorSource;
		myVertColorSource = eVertColorSource;
		myBaseHSB[0] = myBaseHSB[1] = myBaseHSB[2] = 1.0;
		myBaseColor.set(1, 1, 1, 1);
	}
	virtual ~ColorPickerColorGetter() { }

	inline void setHorColorSource(ColorSource eSource) { myHorColorSource = eSource; }
	inline void setVertColorSource(ColorSource eSource) { myVertColorSource = eSource; }
	inline void setColorSources(ColorSource eHorSource, ColorSource eVertSource) { myHorColorSource = eHorSource; myVertColorSource = eVertSource; }

	inline void setBaseColor(const SColor& scolBase) { scolBase.toHSB(myBaseHSB[0], myBaseHSB[1], myBaseHSB[2]); myBaseColor = scolBase; }

	virtual void getVertexColor(FLOAT_TYPE fUParm, FLOAT_TYPE fVParm, SColor& scolOut)
	{
		_ASSERT(fUParm >= 0.0 && fUParm <= 1.001);
		_ASSERT(fVParm >= 0.0 && fVParm <= 1.001);

		if(myHorColorSource == ColorSourceHue || myHorColorSource == ColorSourceSaturation || myHorColorSource == ColorSourceBrightness 
			|| myVertColorSource == ColorSourceHue || myVertColorSource == ColorSourceSaturation || myVertColorSource == ColorSourceBrightness)
		{
			// HSB model
			FLOAT_TYPE fH = myBaseHSB[0], fS = myBaseHSB[1], fB = myBaseHSB[2];
			if(myHorColorSource == ColorSourceHue)
				fH = fUParm;
			else if(myHorColorSource == ColorSourceSaturation)
				fS = fUParm;
			else if(myHorColorSource == ColorSourceNone)
			{	}
			ELSE_ASSERT;

			if(myVertColorSource == ColorSourceSaturation)
				fS = 1.0 - fVParm;
			else if(myVertColorSource == ColorSourceBrightness)
				fB = 1.0 - fVParm;
			else if(myVertColorSource == ColorSourceHue)
				fH = fVParm;
			else if(myVertColorSource == ColorSourceNone)
			{	}
			ELSE_ASSERT;

			if(fH >= 1.0)
				fH -= 1.0;
			scolOut.fromHSB(fH, fS, fB);
		}
		else
		{
			// RGB model
			scolOut = myBaseColor;
			if(myHorColorSource == ColorSourceBlue)
				scolOut.b = fUParm;
			else if(myHorColorSource == ColorSourceRed)
				scolOut.r = fUParm;
			
			if(myVertColorSource == ColorSourceRed)
				scolOut.r = 1.0 - fVParm;
			else if(myVertColorSource == ColorSourceGreen)
				scolOut.g = 1.0 - fVParm;
			else if(myVertColorSource == ColorSourceBlue)
				scolOut.b = 1.0 - fVParm;
		}
	}

	void getInterpsFromBaseColor(FLOAT_TYPE& fUParmOut, FLOAT_TYPE& fVParmOut)
	{
		fUParmOut = fVParmOut = 0.0;
		if(myHorColorSource == ColorSourceHue || myHorColorSource == ColorSourceSaturation || myHorColorSource == ColorSourceBrightness 
			|| myVertColorSource == ColorSourceHue || myVertColorSource == ColorSourceSaturation || myVertColorSource == ColorSourceBrightness)
		{
			// HSB model
			FLOAT_TYPE fH = myBaseHSB[0], fS = myBaseHSB[1], fB = myBaseHSB[2];
			if(myHorColorSource == ColorSourceHue)
				fUParmOut = fH;
			else if(myHorColorSource == ColorSourceSaturation)
				fUParmOut = fS;
			else if(myHorColorSource == ColorSourceNone)
			{	}
			ELSE_ASSERT;

			if(myVertColorSource == ColorSourceSaturation)
				fVParmOut = 1.0 - fS;
			else if(myVertColorSource == ColorSourceBrightness)
				fVParmOut = 1.0 - fB;
			else if(myVertColorSource == ColorSourceHue)
				fVParmOut = fH;
			else if(myVertColorSource == ColorSourceNone)
			{	}
			ELSE_ASSERT;

		}
		else
		{
			// RGB model
			if(myHorColorSource == ColorSourceBlue)
				fUParmOut = myBaseColor.b;
			else if(myHorColorSource == ColorSourceRed)
				fUParmOut = myBaseColor.r;

			if(myVertColorSource == ColorSourceRed)
				fVParmOut = 1.0 - myBaseColor.r;
			else if(myVertColorSource == ColorSourceGreen)
				fVParmOut = 1.0 - myBaseColor.g;
			else if(myVertColorSource == ColorSourceBlue)
				fVParmOut = 1.0 - myBaseColor.b;
		}
	}

private:

	ColorSource myHorColorSource;	
	ColorSource myVertColorSource;

	FLOAT_TYPE myBaseHSB[3];
	SColor myBaseColor;
};
/*****************************************************************************/