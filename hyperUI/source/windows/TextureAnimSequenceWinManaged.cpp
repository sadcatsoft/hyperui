/*
*  TextureManager.cpp
*  SpaceX
*
*  Created by Oleg Samus on 12/01/09.
*  Copyright 2009 __MyCompanyName__. All rights reserved.
*
*/

#include "stdafx.h"

#ifdef _DEBUG
	#include <iostream>
	#include <fstream>
#endif

#ifdef DIRECTX_PIPELINE
#include "D3DRenderer.h"
extern D3DRenderer* g_pDxRenderer;
#endif

namespace HyperUI
{
/********************************************************************************************/
#ifdef WINDOWS

#ifdef DIRECTX_PIPELINE

using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::UI::Core;

//void TextureAnimSequence::initDxTexture(const char* pcsName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& spriteTexture, bool bResampleNearest)
void TextureAnimSequence::initDxTexture(const char* pcsName, ID3D11ShaderResourceView** spriteTexture, bool bResampleNearest)
{
	IWICImagingFactory2* pWicFactory = g_pDxRenderer->getWicFactory();

	wchar_t* pcsWideName = StringUtils::asciiToWide(pcsName);

	myWidth = myHeight = 32;

	ComPtr<IWICBitmapDecoder> decoder;
	try
	{
		//		DX::ThrowIfFailed(
		HRESULT hr = 
			pWicFactory->CreateDecoderFromFilename(
			pcsWideName,
			NULL,  
			GENERIC_READ,
			WICDecodeMetadataCacheOnDemand,
			&decoder
			);

		if (FAILED(hr))
		{
			return;
		}
		//			);

	}
	catch (...)
	{
		// This may happen if the texture is missing, which is ok.
		StringUtils::freeWideText(pcsWideName);
		return;
	}

	StringUtils::freeWideText(pcsWideName);

	ComPtr<IWICBitmapFrameDecode> frame;
	DX::ThrowIfFailed(
		decoder->GetFrame(0, &frame)
		);

	ComPtr<IWICFormatConverter> converter;
	DX::ThrowIfFailed(
		pWicFactory->CreateFormatConverter(&converter)
		);

	DX::ThrowIfFailed(
		converter->Initialize(
		frame.Get(),
		//GUID_WICPixelFormat32bppRGBA,
		GUID_WICPixelFormat32bppPRGBA,
		//GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		NULL,
		0.0f,
		WICBitmapPaletteTypeCustom  // premultiplied RGBA has no paletting, so this is ignored
		)
		);

	UINT width;
	UINT height;
	DX::ThrowIfFailed(
		converter->GetSize(&width, &height)
		);

	// Create the bitmap from the image frame.
	HRESULT hr;
	IWICBitmap *pIBitmap = NULL;
	hr = pWicFactory->CreateBitmapFromSource(
		frame.Get(),          // Create a bitmap from the image frame
		WICBitmapCacheOnDemand,  // Cache metadata when needed
		&pIBitmap);              // Pointer to the bitmap



	BYTE *pv = NULL;
	UINT cbStride = 0;
	WICRect rcLock = { 0, 0, width, height };


	IWICBitmapLock *pLock = NULL;

	hr = pIBitmap->Lock(&rcLock, WICBitmapLockRead, &pLock);

	UINT cbBufferSize = 0;

	hr = pLock->GetStride(&cbStride);
	hr = pLock->GetDataPointer(&cbBufferSize, &pv);

	pLock->Release();	

	myWidth = width;
	myHeight = height;


	//Windows::System::Drawing::Drawing2D::Bitmap ^rTempBitmap;
	D3D11_SUBRESOURCE_DATA textureSubresourceData = {0};
	//textureSubresourceData.pSysMem = textureData->Data;
	textureSubresourceData.pSysMem = pv;



	// Specify the size of a row in bytes, known a priori about the texture data.
	textureSubresourceData.SysMemPitch = cbStride;

	// As this is not a texture array or 3D texture, this parameter is ignored.
	textureSubresourceData.SysMemSlicePitch = 0;

	// Create a texture description from information known a priori about the data.
	// Generalized texture loading code can be found in the Resource Loading sample.
	// DXGI_FORMAT_R8G8B8A8_TYPELESS
	D3D11_TEXTURE2D_DESC textureDesc = {0};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;	
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Most textures contain more than one MIP level.  For simplicity, this sample uses only one.
	textureDesc.MipLevels = 1;

	// As this will not be a texture array, this parameter is ignored.
	textureDesc.ArraySize = 1;

	// Don't use multi-sampling.
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;

	// Allow the texture to be bound as a shader resource.
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	ComPtr<ID3D11Texture2D> texture;
	DX::ThrowIfFailed(
		g_pDxRenderer->getD3dDevice()->CreateTexture2D(
		&textureDesc,
		&textureSubresourceData,
		&texture
		)
		);

	// Once the texture is created, we must create a shader resource view of it
	// so that shaders may use it.  In general, the view description will match
	// the texture description.
	D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc;
	ZeroMemory(&textureViewDesc, sizeof(textureViewDesc));
	textureViewDesc.Format = textureDesc.Format;
	textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
	textureViewDesc.Texture2D.MostDetailedMip = 0;


	DX::ThrowIfFailed(
		g_pDxRenderer->getD3dDevice()->CreateShaderResourceView(
		texture.Get(),
		&textureViewDesc,
		spriteTexture
		)
		);


	pIBitmap->Release();
}
#else


#ifdef WINDOWS
#pragma managed(push, on)
#endif

using namespace System;
using namespace System::Windows;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;
GLuint initGLTexture2(const char* pcsName, GLuint spriteTexture, bool bResampleNearest, int& iWidthOut, int& iHeightOut)
{
	try
	{
		if(FileUtils::doesFileExist(pcsName, FileSourceFullPath))
	{

	   Bitmap rBitmap(gcnew System::String(pcsName));

	Drawing::Rectangle rect = Drawing::Rectangle(0,0,rBitmap.Width,rBitmap.Height);
	System::Drawing::Imaging::BitmapData^ pBmpData = rBitmap.LockBits(rect, System::Drawing::Imaging::ImageLockMode::ReadWrite, rBitmap.PixelFormat );
	int iScanlineW = pBmpData->Stride;
	IntPtr ptr = pBmpData->Scan0;
	unsigned char* pRawData = (unsigned char*)(ptr.ToPointer());

	long lSize = rBitmap.Height*iScanlineW;
	//unsigned char* pCopy = new unsigned char[lSize];
	//	memcpy(pCopy, pRawData, sizeof(unsigned char)*lSize);

	int r,g,b;
	float fAlpha;
	int iX, iY, iBmpOffset = 0;

	int iBitmapHeight = rBitmap.Height;
	int iBitmapWidth = rBitmap.Width;

	int iArea = iBitmapWidth*iBitmapHeight;

#ifdef WINDOWS
	//#pragma omp parallel for private(fAlpha, r,g,b, iX, iBmpOffset) firstprivate(iScanlineW, iBitmapWidth) schedule(static) if(iArea > MIN_OPENMP_BITMAP_AREA)
#endif
	for(iY = 0; iY < iBitmapHeight; iY++)
	{
		for(iX = 0; iX < iBitmapWidth; iX++)
		{
			iBmpOffset = iY*iScanlineW + iX*4;

			// Premult the alpha
			fAlpha = (float)pRawData[iBmpOffset + 3]/255.0;
			r = (int)((float)pRawData[iBmpOffset + 2]*fAlpha);
			g = (int)((float)pRawData[iBmpOffset + 1]*fAlpha);
			b = (int)((float)pRawData[iBmpOffset + 0]*fAlpha);
			pRawData[iBmpOffset + 0] = r;
			pRawData[iBmpOffset + 1] = g;
			pRawData[iBmpOffset + 2] = b;
		}
	}

	iWidthOut = rBitmap.Width;
	iHeightOut = rBitmap.Height;

	//gLog("TEXTURE LOAD: %s\n", pcsName);

	glBindTexture(GL_TEXTURE_2D, spriteTexture);
	// Speidfy a 2D texture image, provideing the a pointer to the image data in memory
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rBitmap.Width, rBitmap.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pRawData); // pCopy

	if(bResampleNearest)
	{
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	rBitmap.UnlockBits(pBmpData);
	///delete[] pCopy;
	}
	}
	catch (...)
	{
		// Could not load the file. Did you forget the 0001 at the end?
		// We currently have some files we attempt to load that don't exist, and
		// which we don't actually use.
		// _ASSERT(0);
	}


	return spriteTexture;
}
#ifdef WINDOWS
#pragma managed(pop)
#endif

GLuint TextureAnimSequence::initGLTexture(const char* pcsName, GLuint spriteTexture, bool bResampleNearest)
{
    return initGLTexture2(pcsName, spriteTexture, bResampleNearest, myWidth, myHeight);
}
#endif // if not dx 11

#endif // if windows

/********************************************************************************************/
};