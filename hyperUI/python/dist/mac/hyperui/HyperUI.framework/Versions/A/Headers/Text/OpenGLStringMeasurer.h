#pragma once

/*****************************************************************************/
class HYPERUI_API OpenGLStringMeasurer : public IStringMeasurer
{
public:
	virtual ~OpenGLStringMeasurer() { }
	static OpenGLStringMeasurer* getInstance()
	{
		if(!theInstance)
			theInstance = new OpenGLStringMeasurer;
		return theInstance;
	}

	virtual void measureString(Window* pWindow, const char* pcsString, const char* pcsFont, FLOAT_TYPE fPixelSize, Material* pMaterial, FLOAT_TYPE fKerning, FLOAT_TYPE fLineSpacing, SVector2D& svSizeOut);
	virtual FLOAT_TYPE getLineHeight(Window* pWindow, const char* pcsFont, FLOAT_TYPE fPixelSize, Material* pMaterial, FLOAT_TYPE fKerning, FLOAT_TYPE fLineSpacing);

protected:
	OpenGLStringMeasurer() { }

private:
	static OpenGLStringMeasurer *theInstance;
};
/*****************************************************************************/
