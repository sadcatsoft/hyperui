#include "stdafx.h"

#ifdef DIRECTX_PIPELINE
#include "D3DRenderer.h"
extern D3DRenderer* g_pDxRenderer;
#endif

#define NUM_SPRITE_VERTS		4

namespace HyperUI
{
// TODO: Do this better
#ifdef USE_OPENGL2
static GLfloat g_refSpriteVertices[] =
{
	-0.5f, -0.5f, 0.0f, 1.0f,
	0.5f, -0.5f, 0.0f, 1.0f,
	-0.5f,  0.5f, 0.0f, 1.0f,
	0.5f,  0.5f, 0.0f, 1.0f,
};

static GLfloat g_finalSpriteVertices[] =
{
	-0.5f, -0.5f, 0.0f, 1.0f,
	0.5f, -0.5f, 0.0f, 1.0f,
	-0.5f,  0.5f, 0.0f, 1.0f,
	0.5f,  0.5f, 0.0f, 1.0f,
};

static GLfloat g_refSpriteVertices3D[] =
{
	-0.5f, -0.5f, 0.0, 1.0f,
	0.5f, -0.5f, 0.0, 1.0f,
	-0.5f,  0.5f, 0.0, 1.0f,
	0.5f,  0.5f, 0.0, 1.0f,
};

static GLfloat g_finalSpriteVertices3D[] =
{
	-0.5f, -0.5f, 0.0, 1.0f,
	0.5f, -0.5f, 0.0, 1.0f,
	-0.5f,  0.5f, 0.0, 1.0f,
	0.5f,  0.5f, 0.0, 1.0f,
};
#else
static GLfloat g_refSpriteVertices[] =
{
	-0.5f, -0.5f,
	0.5f, -0.5f,
	-0.5f,  0.5f,
	0.5f,  0.5f,
};

static GLfloat g_finalSpriteVertices[] =
{
	-0.5f, -0.5f,
	0.5f, -0.5f,
	-0.5f,  0.5f,
	0.5f,  0.5f,
};

static GLfloat g_refSpriteVertices3D[] =
{
-0.5f, -0.5f, 0.0,
0.5f, -0.5f, 0.0,
-0.5f,  0.5f, 0.0,
0.5f,  0.5f, 0.0,
};

static GLfloat g_finalSpriteVertices3D[] =
{
-0.5f, -0.5f, 0.0,
0.5f, -0.5f, 0.0,
-0.5f,  0.5f, 0.0,
0.5f,  0.5f, 0.0,
};
#endif


static
const GLshort g_spriteTexcoords[] = {
	0, 0,
	1, 0,
	0, 1,
	1, 1,
};

static
GLfloat g_NonConstSpriteTexcoords[] = {
	0, 0,
	1, 0,
	0, 1,
	1, 1,
};

static
GLfloat g_spriteColors[] = {
	1,1,1,0.5,
	1,1,1,0.5,
	1,1,1,0.5,
	1,1,1,0.5,
};

static const
GLfloat g_solidSpriteColors[] = {
	1,1,1,1,
	1,1,1,1,
	1,1,1,1,
	1,1,1,1,
};


/*****************************************************************************/
void GraphicsUtils::generateTextures(int iNum, GLuint* textures)
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else
#ifdef USE_OPENGL2
	glActiveTexture(GL_TEXTURE0);
#endif
	glGenTextures(iNum, textures);
#endif
}
/*****************************************************************************/
GLint GraphicsUtils::getMaxTextureSize()
{
#ifdef DIRECTX_PIPELINE
	return D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
#else
	GLint texSize = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
	return texSize;
#endif
}
/*****************************************************************************/
void GraphicsUtils::deleteTexture(TX_MAN_RETURN_TYPE texture)
{
#ifdef DIRECTX_PIPELINE
	if(texture && *texture)
	{
		(*texture)->Release();
		(*texture) = NULL;
	}
#else
	glDeleteTextures(1, &texture);
#endif
}
/*****************************************************************************/
#ifdef DIRECTX_PIPELINE
ID3D11ShaderResourceView* GraphicsUtils::createTextureFrom(int xSize, int ySize, unsigned char* pStorage, bool bResampleNearest)
#else
TX_MAN_RETURN_TYPE GraphicsUtils::createTextureFrom(int xSize, int ySize, unsigned char* pStorage, bool bResampleNearest)
#endif
{
#ifdef DIRECTX_PIPELINE

	// Create a texture description from information known a priori about the data.
	// Generalized texture loading code can be found in the Resource Loading sample.
	// DXGI_FORMAT_R8G8B8A8_TYPELESS
	D3D11_TEXTURE2D_DESC textureDesc = {0};
	textureDesc.Width = xSize;
	textureDesc.Height = ySize;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	textureDesc.Usage = D3D11_USAGE_DYNAMIC;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	// 	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0; /// D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;

							   // Most textures contain more than one MIP level.  For simplicity, this sample uses only one.
	textureDesc.MipLevels = 1;

	// As this will not be a texture array, this parameter is ignored.
	textureDesc.ArraySize = 1;

	// Don't use multi-sampling.
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;

	// Allow the texture to be bound as a shader resource.
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; //  | D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;

	if(pStorage)
	{
		D3D11_SUBRESOURCE_DATA textureSubresourceData = { 0 };
		//memset(pStorage, 255, sizeof(unsigned char)*xSize*ySize*4);
		textureSubresourceData.pSysMem = pStorage;

		// Specify the size of a row in bytes, known a priori about the texture data.
		textureSubresourceData.SysMemPitch = xSize*4;

		// As this is not a texture array or 3D texture, this parameter is ignored.
		textureSubresourceData.SysMemSlicePitch = 0;

		DX::ThrowIfFailed(
			g_pDxRenderer->getD3dDevice()->CreateTexture2D(&textureDesc, &textureSubresourceData, &texture)
		);
	}
	else
	{

		DX::ThrowIfFailed(
			g_pDxRenderer->getD3dDevice()->CreateTexture2D(&textureDesc, NULL, &texture)
		);
	}

	// Once the texture is created, we must create a shader resource view of it
	// so that shaders may use it.  In general, the view description will match
	// the texture description.
	D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc;
	ZeroMemory(&textureViewDesc, sizeof(textureViewDesc));
	textureViewDesc.Format = textureDesc.Format;
	textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
	textureViewDesc.Texture2D.MostDetailedMip = 0;

	ID3D11ShaderResourceView* resTexture = NULL;
	DX::ThrowIfFailed(
		g_pDxRenderer->getD3dDevice()->CreateShaderResourceView(texture.Get(), &textureViewDesc, &resTexture)
	);
	//resTexture = &pActualTexturePtr;

#else

	TX_MAN_RETURN_TYPE resTexture = NULL;

	GraphicsUtils::generateTextures(1, &resTexture);
	GraphicsUtils::setDiffuseTexture(resTexture);

	if(bResampleNearest)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	if(pStorage)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, xSize, ySize, 0, GL_RGBA, GL_UNSIGNED_BYTE, pStorage);

#endif

	return resTexture;
}
/*****************************************************************************/
#ifdef DIRECTX_PIPELINE
ID3D11ShaderResourceView* GraphicsUtils::createTexture(bool bResampleNearest)
#else
TX_MAN_RETURN_TYPE GraphicsUtils::createTexture(bool bResampleNearest)
#endif
{
	return GraphicsUtils::createTextureFrom(32, 32, NULL, bResampleNearest);
}
/*****************************************************************************/
void GraphicsUtils::setDiffuseTexture(TX_MAN_RETURN_TYPE rTexture)
{
#ifdef DIRECTX_PIPELINE
	if(rTexture)
		g_pDxRenderer->getD3dContext()->PSSetShaderResources(0, 1, rTexture);
#else
#ifdef USE_OPENGL2
	glActiveTexture(GL_TEXTURE0);
#endif
	glBindTexture(GL_TEXTURE_2D, rTexture);
#endif
}
/*****************************************************************************/
void GraphicsUtils::setTextureSamplingMode(TX_MAN_RETURN_TYPE pTexture, TextureSamplingType eNewType)
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else
	glBindTexture(GL_TEXTURE_2D, pTexture);
	if(eNewType == TextureSamplingNearestNeighbour)
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	}
	else if(eNewType == TextureSamplingLinear)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	ELSE_ASSERT;
#endif
}
/*****************************************************************************/
#ifdef DIRECTX_PIPELINE
AdhocDxBuffer* g_pDrawImageBuffer = NULL;
#endif
void GraphicsUtils::drawImage(IWindowBase* pWindow, TX_MAN_RETURN_TYPE iTexture, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fW, FLOAT_TYPE fH, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, bool bPremultiplyAlpha, SUVSet& uvs, BlendModeType eBlendMode, SColor* pTextureCols)
{
	if(fAlpha < FLOAT_EPSILON)
		return;

	int iCurrIdx;

#ifndef LINUX
	if(pWindow->getBlendMode() != eBlendMode)
#endif
		pWindow->setBlendMode(eBlendMode);

	const GLfloat* pColorPointer = NULL;
#ifdef DIRECTX_PIPELINE
	g_pDxRenderer->getD3dContext()->PSSetShaderResources(0, 1, iTexture);
#else
	glBindTexture(GL_TEXTURE_2D, iTexture);
#endif

	int iOffset = 0;
	int iColOffset = 0;
	bool bDidPush = false;


	FLOAT_TYPE fColVal = 1.0;
	if(bPremultiplyAlpha)
		fColVal = fAlpha;

	FLOAT_TYPE iColRVal = (fColVal);
	FLOAT_TYPE iColGVal = iColRVal;
	FLOAT_TYPE iColBVal = iColRVal;
	FLOAT_TYPE iAlpha = (fAlpha);

	if(pTextureCols)
	{
		// Use supplied color
		iColRVal = (fColVal*pTextureCols->r);
		iColGVal = (fColVal*pTextureCols->g);
		iColBVal = (fColVal*pTextureCols->b);
	}

	if(fabs(fDegAngle) > FLOAT_EPSILON)
	{
#ifdef DIRECTX_PIPELINE
		SVector2D svTemp;
#else
#ifdef USE_OPENGL2
		SMatrix4 rMatrix;
		rMatrix.appendTranslation(fX, fY, 0.0);
		rMatrix.appendZRotation(-fDegAngle);
#else

		glPushMatrix();
		glTranslatef(fX, fY, 0.0);
		glRotatef(-fDegAngle, 0.0f, 0.0f, 1.0f);
#endif
		bDidPush = true;
#endif
		for(iCurrIdx = 0; iCurrIdx < NUM_SPRITE_VERTS; iCurrIdx++)
		{
			g_finalSpriteVertices[iOffset] = g_refSpriteVertices[iOffset] * (FLOAT_TYPE)fW;
			g_finalSpriteVertices[iOffset + 1] = g_refSpriteVertices[iOffset + 1] * (FLOAT_TYPE)fH;

#ifdef DIRECTX_PIPELINE
			svTemp.x = g_finalSpriteVertices[iOffset];
			svTemp.y = g_finalSpriteVertices[iOffset + 1];
			svTemp.rotateCCW(fDegAngle);
			svTemp.x += fX;
			svTemp.y += fY;

			g_finalSpriteVertices[iOffset] = svTemp.x;
			g_finalSpriteVertices[iOffset + 1] = svTemp.y;
#endif

			g_spriteColors[iColOffset + 0] = iColRVal;
			g_spriteColors[iColOffset + 1] = iColGVal;
			g_spriteColors[iColOffset + 2] = iColBVal;
			g_spriteColors[iColOffset + 3] = fAlpha;

			iOffset += g_iNumVertexComponents2D;
			iColOffset += 4;
		}

		pColorPointer = g_spriteColors;
#if !defined(USE_OPENGL2) && !defined(DIRECTX_PIPELINE)
		OpenGLStateCleaner::doColorPointerCall(4, GL_FLOAT, 0, g_spriteColors);
#endif
	}
	else
	{
		if(fAlpha == 1.0 && !bPremultiplyAlpha)
		{
			for(iCurrIdx = 0; iCurrIdx < NUM_SPRITE_VERTS; iCurrIdx++)
			{
				g_finalSpriteVertices[iOffset] = g_refSpriteVertices[iOffset] * (FLOAT_TYPE)fW + fX;
				g_finalSpriteVertices[iOffset + 1] = g_refSpriteVertices[iOffset + 1] * (FLOAT_TYPE)fH + fY;
				iOffset += g_iNumVertexComponents2D;
			}

			pColorPointer = g_solidSpriteColors;
#if !defined(USE_OPENGL2) && !defined(DIRECTX_PIPELINE)
			OpenGLStateCleaner::doColorPointerCall(4, GL_FLOAT, 0, g_solidSpriteColors);
#endif
		}
		else
		{
			for(iCurrIdx = 0; iCurrIdx < NUM_SPRITE_VERTS; iCurrIdx++)
			{
				g_finalSpriteVertices[iOffset] = g_refSpriteVertices[iOffset] * (FLOAT_TYPE)fW + fX;
				g_finalSpriteVertices[iOffset + 1] = g_refSpriteVertices[iOffset + 1] * (FLOAT_TYPE)fH + fY;
				g_spriteColors[iColOffset + 0] = iColRVal;
				g_spriteColors[iColOffset + 1] = iColGVal;
				g_spriteColors[iColOffset + 2] = iColBVal;
				g_spriteColors[iColOffset + 3] = fAlpha;

				iOffset += g_iNumVertexComponents2D;
				iColOffset += 4;
			}

			pColorPointer = g_spriteColors;
#if !defined(USE_OPENGL2) && !defined(DIRECTX_PIPELINE)
			OpenGLStateCleaner::doColorPointerCall(4, GL_FLOAT, 0, g_spriteColors);
#endif
		}

	}

	// Cast to int:
	iOffset = 0;
	for(iCurrIdx = 0; iCurrIdx < NUM_SPRITE_VERTS; iCurrIdx++)
	{
		g_finalSpriteVertices[iOffset] = (int)g_finalSpriteVertices[iOffset];
		g_finalSpriteVertices[iOffset + 1] = (int)g_finalSpriteVertices[iOffset + 1];
		iOffset += 2;
	}

#if !defined(USE_OPENGL2) && !defined(DIRECTX_PIPELINE)
	OpenGLStateCleaner::doVertexPointerCall(2, GL_FLOAT, 0, g_finalSpriteVertices);
#endif

	// Simple, const UVs.
	//	glTexCoordPointer(2, GL_SHORT, 0, g_spriteTexcoords);

	// New, custom UVs
	g_NonConstSpriteTexcoords[0] = uvs.myStartX;
	g_NonConstSpriteTexcoords[1] = uvs.myStartY;
	g_NonConstSpriteTexcoords[2] = uvs.myEndX;
	g_NonConstSpriteTexcoords[3] = uvs.myStartY;
	g_NonConstSpriteTexcoords[4] = uvs.myStartX;
	g_NonConstSpriteTexcoords[5] = uvs.myEndY;
	g_NonConstSpriteTexcoords[6] = uvs.myEndX;
	g_NonConstSpriteTexcoords[7] = uvs.myEndY;

#ifndef DIRECTX_PIPELINE

#ifdef USE_OPENGL2
	RenderUtils::drawBuffer(g_finalSpriteVertices, (float*)g_NonConstSpriteTexcoords, pColorPointer, 4, GL_TRIANGLE_STRIP);
#else
	OpenGLStateCleaner::doTexturePointerCall(2, GL_FLOAT, 0, g_NonConstSpriteTexcoords);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif

#else
	if(!g_pDrawImageBuffer)
		g_pDrawImageBuffer = new AdhocDxBuffer;
	g_pDrawImageBuffer->setFrom2DArrays(g_finalSpriteVertices, g_NonConstSpriteTexcoords, pColorPointer, 4);

	g_pDxRenderer->getD3dContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	UINT stride = sizeof(BasicVertex);
	UINT offset = 0;
	g_pDxRenderer->getD3dContext()->IASetVertexBuffers(0, 1, g_pDrawImageBuffer->getVertexBuffer(), &stride, &offset);
	g_pDxRenderer->getD3dContext()->IASetIndexBuffer(g_pDrawImageBuffer->getIndexBuffer(), INDEX_BUFFER_FORMAT,	0);
	g_pDxRenderer->getD3dContext()->DrawIndexed(4, 0, 0);

	g_pDxRenderer->getD3dContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
#endif

#ifndef DIRECTX_PIPELINE
	if(bDidPush)
	{
#ifdef USE_OPENGL2
		RenderStateManager::getInstance()->resetModelMatrix();
#else
		glPopMatrix();
#endif
	}
#endif
}
/*****************************************************************************/
void GraphicsUtils::enableFlatDrawingState(FLOAT_TYPE fLineThickness)
{
#ifdef DIRECTX_PIPELINE
	// Do nothing
#else

#ifndef USE_OPENGL2
	glDisable(GL_TEXTURE_2D);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif

	if(fLineThickness > 0)
	{
		OpenGLStateCleaner::doLineWidth(fLineThickness);
		if(fLineThickness == 1.0)
		{
#ifndef USE_OPENGL2
			glEnable(GL_LINE_SMOOTH);
#endif
		}
	}
#endif
}
/*****************************************************************************/
void GraphicsUtils::disableFlatDrawingState(FLOAT_TYPE fLineThickness)
{
#ifdef DIRECTX_PIPELINE
	// Do nothing
#else
	if(fLineThickness == 1.0)
	{
#ifndef USE_OPENGL2
		glDisable(GL_LINE_SMOOTH);
#endif
	}

#ifndef USE_OPENGL2
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);
#endif

#endif
}
/*****************************************************************************/
void GraphicsUtils::enableAlphaTest()
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE2
#else
#ifdef USE_OPENGL2
	GL2FIXME
#else
	glEnable (GL_ALPHA_TEST) ;
#endif
#endif
}
/*****************************************************************************/
void GraphicsUtils::disableAlphaTest()
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE2
#else
#ifdef USE_OPENGL2
	GL2FIXME
#else
	glDisable (GL_ALPHA_TEST) ;
#endif
#endif
}
/*****************************************************************************/
void GraphicsUtils::enableNormalization()
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else
#ifdef USE_OPENGL2
	GL2FIXME
#else
	glEnable( GL_NORMALIZE );
#endif
#endif
}
/*****************************************************************************/
void GraphicsUtils::disableNormalization()
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else
#ifdef USE_OPENGL2
	GL2FIXME
#else
	glDisable( GL_NORMALIZE );
#endif
#endif
}
/*****************************************************************************/
void GraphicsUtils::enableXorMode()
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else
#ifdef USE_OPENGL2
	GL2FIXME
#else
	glEnable(GL_COLOR_LOGIC_OP); 
	glLogicOp(GL_XOR);
#endif
#endif

}
/*****************************************************************************/
void GraphicsUtils::disableXorMode()
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else
#ifdef USE_OPENGL2
	GL2FIXME
#else
	glLogicOp(GL_COPY);
	glDisable(GL_COLOR_LOGIC_OP);
#endif
#endif
}
/*****************************************************************************/
void GraphicsUtils::transformForGlOrtho(int iW, int iH)
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else

#ifdef USE_OPENGL2
	RenderStateManager::getInstance()->resetForOrtho(iW, iH, ORTHO_DEPTH_LIMIT);
#else
	glLoadIdentity();

	// Isn't this the same on windows, since we just reverse the width and height?
	glTranslatef(0, iH, 0.0);
	glScalef(1, -1, 1);
#endif

#endif
/*

#ifdef ORIENTATION_LANDSCAPE
	///		glRotatef(90.0, 0.0, 0.0, 1.0);
	//		glTranslatef(myHeight, -myWidth, 0.0);
	//		glScalef(-1, 1, 1);
	//		glTranslatef(myWidth, myHeight, 0.0);
	glTranslatef(0, iH, 0.0);
	glScalef(1, -1, 1);
#else
	Not tested
		glRotatef(-90.0, 0.0, 0.0, 1.0);
	glScalef(-1, 1, 1);
#endif
	*/
}
/*****************************************************************************/
void GraphicsUtils::setAlphaTestCutoffFunc(unsigned int eFunc, FLOAT_TYPE fValue)
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else
#ifdef USE_OPENGL2
	GL2FIXME
#else
	glAlphaFunc ( eFunc, fValue ) ;
#endif
#endif
}
/*****************************************************************************/
#if !defined(DIRECTX_PIPELINE) && !defined(USE_OPENGL2)
void GraphicsUtils::perspectiveGl(FLOAT_TYPE fFovY, FLOAT_TYPE fAspect, FLOAT_TYPE fNear, FLOAT_TYPE fFar)
{
	// May need to cache these to avoid recomputing them, since we set this every frame.
	double fW, fH;
#if defined(WIN32) || defined(MAC_BUILD) || defined(LINUX)
	fH = tan( fFovY / 360.0 * M_PI ) * fNear;
	fW = fH * fAspect;
	glFrustum( -fW, fW, -fH, fH, fNear, fFar );
#else
	//	fH = tan( fFovY / 360.0 * pi ) * fNear;
	//	fW = fH * 1.0/fAspect;

	fH = tan( fFovY / 360.0 * M_PI ) * fNear;
	fW = fH * fAspect;

	glFrustumf( -fH, fH, -fW, fW, fNear, fFar );

	//	fH = tan( fFovY / 360.0 * pi ) * fNear;
	//	fW = fH * fAspect;
	//	glFrustumf( -fW, fW, -fH, fH, fNear, fFar );

#endif
}
#endif
/*****************************************************************************/
};
