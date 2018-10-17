#pragma once

/*****************************************************************************/
class HYPERUI_API GraphicsUtils
{
public:

	static GLint getMaxTextureSize();
	static void deleteTexture(TX_MAN_RETURN_TYPE texture);
	static void setDiffuseTexture(TX_MAN_RETURN_TYPE rTexture);
	static void generateTextures(int iNum, GLuint* textures);
	static void setTextureSamplingMode(TX_MAN_RETURN_TYPE pTexture, TextureSamplingType eNewType);
	static void drawImage(IWindowBase* pWindow, TX_MAN_RETURN_TYPE iTexture, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fW, FLOAT_TYPE fH, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, bool bPremultiplyAlpha, SUVSet& uvs, BlendModeType eBlendMode, SColor* pTextureCols = NULL);

#ifdef DIRECTX_PIPELINE
	static ID3D11ShaderResourceView* createTextureFrom(int xSize, int ySize, unsigned char* pStorage, bool bResampleNearest);
	static ID3D11ShaderResourceView* createTexture(bool bResampleNearest);
#else
	static TX_MAN_RETURN_TYPE createTextureFrom(int xSize, int ySize, unsigned char* pStorage, bool bResampleNearest);
	static TX_MAN_RETURN_TYPE createTexture(bool bResampleNearest);
#endif

	static void enableFlatDrawingState(FLOAT_TYPE fLineThickness);
	static void disableFlatDrawingState(FLOAT_TYPE fLineThickness);
	static void disableAlphaTest();
	static void enableAlphaTest();
	static void enableNormalization();
	static void disableNormalization();
	static void enableXorMode();
	static void disableXorMode();

	static void transformForGlOrtho(int iW, int iH);
	static void setAlphaTestCutoffFunc(unsigned int eFunc, FLOAT_TYPE fValue);

#if !defined(DIRECTX_PIPELINE) && !defined(USE_OPENGL2)
	static void perspectiveGl(FLOAT_TYPE fFovY, FLOAT_TYPE fAspect, FLOAT_TYPE fNear, FLOAT_TYPE fFar);
#endif

};
/*****************************************************************************/