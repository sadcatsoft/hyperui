#include "stdafx.h"

#ifdef DIRECTX_PIPELINE
#include "D3DRenderer.h"
extern D3DRenderer* g_pDxRenderer;
#endif

namespace HyperUI
{
/***********************************************************************************************************/
CachedSequence::CachedSequence(DrawingCache* pParent)
{
	myTexture = NULL;
	myIsUnloadable = false;
	myIsMarkedRelated = false;
//	myLastUsedTime = 0;
	myRenderStage = 1;
	myParentCache = pParent;
	//    myAnimType = AnimationLastPlaceholder;
	
	myVertices = NULL;
	myNumAllocVerts = 0;

	mySubtileMap = NULL;
	mySubtileMapWidth = mySubtileMapHeight = 0;

	
	/*
	 myColors = NULL;
	 myVerts = NULL;
	 myUVs = NULL;
	 myNumAllocColors = myNumAllocVerts = myNumAllocUVs = 0;
	 */
	myCachedAnimProgress = -1;
	myNumTris = 0;

	resetCache();
}
/***********************************************************************************************************/
CachedSequence::~CachedSequence()
{
	/*
	 if(myColors)
	 delete[] myColors;
	 myColors = NULL;
	 
	 if(myVerts)
	 delete[] myVerts;
	 myVerts = NULL;
	 
	 if(myUVs)
	 delete[] myUVs;
	 myUVs = NULL;
	 
	 myNumAllocColors = myNumAllocVerts = myNumAllocUVs = 0;
	 */

// 	if(myTexture)
// 		myTexture->unloadTexture();
	
	if(myVertices)
		delete[] myVertices;
	myVertices = NULL;
	
	myNumAllocVerts = 0;
	
	myNumTris = 0;
	
}
/***********************************************************************************************************/
void CachedSequence::onTimerTick(GTIME lTime)
{
	if(myTexture && myIsUnloadable)
	{
		// Unload it. We probably need a pointer to the main texture to see if it isn't
		// already unloaded and to actually tell it to do so.
		if(myTexture->shouldUnload())
			myParentCache->unloadTexture(myAnimType.c_str());
	}
}
/***********************************************************************************************************/
void CachedSequence::setAnimType(const char *pcsType)
{
	myAnimType = pcsType;

	myRenderStage = getTextureManager()->getTextureRenderStage(pcsType);
	myIsUnloadable = getTextureManager()->getIsTextureUnloadable(pcsType);

	myTexture = getTextureManager()->getTexture(pcsType);

	// See if we have a subtile map, and if so, store a pointer to it.
	// We need to find an anim item, which may be in the prelim anims
	// or regular anims.
	mySubtileMap = getTextureManager()->getSubtileMap(pcsType, mySubtileMapWidth, mySubtileMapHeight);

//	_ASSERT(myTexture);

	/*	
	 // Brutal hack
	 if(myAnimType == ROOM_SHADOW_TYPE)
	 myRenderStage = RenderStagePost;
	 // And another. For the door lights.
	 else if(myAnimType.length() == 2 && isdigit(myAnimType[0]) && isdigit(myAnimType[1]))
	 {
	 int iIntType = atoi(myAnimType.c_str());
	 if(iIntType >= 45 && iIntType <= 56)
	 myRenderStage = RenderStagePostPost;
	 }
	 */
}
/***********************************************************************************************************/
#ifdef CACHE_TEXTURE_INFO
void CachedSequence::initCachedTexParms(FLOAT_TYPE fAnimProgress, bool bIsProgressAFrameNum, TextureAnimSequence *pOptExistingPointer)
{	
	_ASSERT(fAnimProgress != myCachedAnimProgress || myCachedTexIndex == 0);
	if(bIsProgressAFrameNum)
		myCachedTexIndex = getTextureManager()->getTextureIndex(myAnimType.c_str(), (int)fAnimProgress, myCachedTexW, myCachedTexH, myCachedTexUVs, myCachedTexBlendMode, pOptExistingPointer);	
	else
		myCachedTexIndex = getTextureManager()->getTextureIndexFromProgress(myAnimType.c_str(), fAnimProgress, myCachedTexW, myCachedTexH, myCachedTexUVs, myCachedTexBlendMode, pOptExistingPointer);	
	myCachedAnimProgress = fAnimProgress;
}
#endif
/***********************************************************************************************************/
void CachedSequence::flush(void)
{
#ifdef _DEBUG
	if(myAnimType == "particlesAddAtlas")
	{
		int bp = 0;
	}
#endif

	myIsMarkedRelated = false;
#ifdef HARDCORE_LINUX_DEBUG_MODE
	static int g_iDbCounter = 0;
	bool bTrack = false;
	if(myAnimType == "ftMedSmallAnim")
	{
		g_iDbCounter++;
		if(g_iDbCounter == 10)
			bTrack = true;
	}
	static int g_iDbCounter2 = 0;
	if(myAnimType == "toolIcons")
	{
		g_iDbCounter2++;
		if(g_iDbCounter2 == 10)
			bTrack = true;
	}


	if(bTrack)
	{
		GLboolean blendEnabled;
		glGetBooleanv(GL_BLEND, &blendEnabled);
		GLint blendSrc;
		GLint blendDst;
		glGetIntegerv(GL_BLEND_SRC, &blendSrc);
		glGetIntegerv(GL_BLEND_DST, &blendDst);

		GLboolean vEnabled, tEnabled, cEnabled, nEnabled;
		glGetBooleanv(GL_VERTEX_ARRAY, &vEnabled);
		glGetBooleanv(GL_TEXTURE_COORD_ARRAY, &tEnabled);
		glGetBooleanv(GL_COLOR_ARRAY, &cEnabled);		
		glGetBooleanv(GL_NORMAL_ARRAY, &nEnabled);		

		GLboolean  depthEnabled, alphaTestEnabled, lightingEnabled, cullEnabled, stenEnabled, colmatEnabled, scissmatEnabled;
		glGetBooleanv(GL_DEPTH_TEST, &depthEnabled);
		glGetBooleanv(GL_ALPHA_TEST, &alphaTestEnabled);
		glGetBooleanv(GL_LIGHTING, &lightingEnabled);
		glGetBooleanv(GL_CULL_FACE, &cullEnabled);
		glGetBooleanv(GL_STENCIL_TEST, &stenEnabled);
		glGetBooleanv(GL_COLOR_MATERIAL, &colmatEnabled);
		glGetBooleanv(GL_SCISSOR_TEST, &scissmatEnabled);
		
		BlendModeType eCurrBlendmode = myParentCache->getWindow()->getBlendMode();
		Logger::log("CSFLUSH: seq = %s num = %d currBM = %d cachedBM = %d texIdx = %d", myAnimType.c_str(), myNumTris, eCurrBlendmode, myCachedTexBlendMode, myCachedTexIndex);
		Logger::log("CSFLUSH: glstate: blendEn = %d srcBlend = %d dstBlend = %d vEn = %d tEn = %d cEn = %d nEn = %d depthEn = %d alphaTEn = %d lightEn = %d culLEn = %d stenEn = %d colmatEn = %d scissEn = %d", blendEnabled, blendSrc, blendDst, vEnabled, tEnabled, cEnabled, nEnabled, depthEnabled, alphaTestEnabled, lightingEnabled, cullEnabled, stenEnabled, colmatEnabled, scissmatEnabled);

		if(myNumTris > 0)
		{
			Logger::log("CSFLUSH: tri = (%g, %g) (%g, %g) (%g, %g) tex = (%g, %g) (%g, %g) (%g, %g) col = (%d, %d, %d, %d) (%d, %d, %d, %d) (%d, %d, %d, %d)", myVertices[0].x, myVertices[0].y,  
				myVertices[1].x, myVertices[1].y, myVertices[2].x, myVertices[2].y, 
				myVertices[0].s0, myVertices[0].t0, myVertices[1].s0, myVertices[1].t0, myVertices[2].s0, myVertices[2].t0,
				myVertices[0].r, myVertices[0].g, myVertices[0].b, myVertices[0].a, 
				myVertices[1].r, myVertices[1].g, myVertices[1].b, myVertices[1].a, 
				myVertices[2].r, myVertices[2].g, myVertices[2].b, myVertices[2].a);
			
		}
	}
#endif

	if(myNumTris <= 0)
		return;

#ifdef DIRECTX_PIPELINE
	myDxBuffer.setFromVertexArray(myVertices, myNumTris);
#endif

	
#ifdef CACHE_TEXTURE_INFO
	_ASSERT(myCachedAnimProgress >= 0.0);
#ifndef LINUX
	if(myParentCache->getWindow()->getBlendMode() != myCachedTexBlendMode)
#endif
		myParentCache->getWindow()->setBlendMode(myCachedTexBlendMode);
	
#ifdef DIRECTX_PIPELINE
	g_pDxRenderer->getD3dContext()->PSSetShaderResources(0, 1, myCachedTexIndex);
#else
	glBindTexture(GL_TEXTURE_2D, myCachedTexIndex);	
#endif
#else


#ifdef DIRECTX_PIPELINE
	AHTUNG
#endif
	int iW, iH;
	GLuint iIndex;	
	SUVSet uvsOut;
	BlendModeType eBlendMode;
	iIndex = getTextureManager()->getTextureIndex(myAnimType.c_str(), 0, iW, iH, uvsOut, eBlendMode);
	
#ifndef LINUX
	if(myParentCache->getWindow()->getBlendMode() != eBlendMode)
#endif
		myParentCache->getWindow()->setBlendMode(eBlendMode);
	
	glBindTexture(GL_TEXTURE_2D, iIndex);	
#endif
	/*
	 glColorPointer(4, GL_FLOAT, 0, myColors);
	 glVertexPointer(2, GL_FLOAT, 0, myVerts);
	 glTexCoordPointer(2, GL_FLOAT, 0, myUVs);	
	 glDrawArrays(GL_TRIANGLES, 0, myNumTris*3);
	 */
	
#ifdef DIRECTX_PIPELINE

	UINT stride = sizeof(BasicVertex);
	UINT offset = 0;
	g_pDxRenderer->getD3dContext()->IASetVertexBuffers(0, 1, myDxBuffer.getVertexBuffer(), &stride, &offset);
	g_pDxRenderer->getD3dContext()->IASetIndexBuffer(myDxBuffer.getIndexBuffer(), INDEX_BUFFER_FORMAT,	0);
	g_pDxRenderer->getD3dContext()->DrawIndexed(myNumTris*3, 0, 0);

#else

#ifdef USE_OPENGL2
	RenderUtils::drawBuffer<SVertexInfo>(&myVertices[0].x, SVertexInfo::getNumVertexComponents(), &myVertices[0].s0, &myVertices[0].r, NULL, myNumTris*3);
#else
	//	glVertexPointer(2, GL_FLOAT, sizeof(SVertexInfo), &myVertices[0].x);
#ifdef USE_INT_OPENGL_POSITIONS
	OpenGLStateCleaner::doVertexPointerCall(2, GL_SHORT, sizeof(SVertexInfo), &myVertices[0].x);
#else
	OpenGLStateCleaner::doVertexPointerCall(2, GL_FLOAT, sizeof(SVertexInfo), &myVertices[0].x);
#endif
	OpenGLStateCleaner::doTexturePointerCall(2, GL_FLOAT, sizeof(SVertexInfo), &myVertices[0].s0);
	OpenGLStateCleaner::doColorPointerCall(4, GL_UNSIGNED_BYTE, sizeof(SVertexInfo), &myVertices[0].r);
	
#ifdef _DEBUG
	if(myNumTris <= 2)
	{
		//gLog("OPENGL: Flushing only %d triangles for %s\n", myNumTris, myAnimType.c_str());
		int bp = 0;
	}
#endif

	glDrawArrays(GL_TRIANGLES, 0, myNumTris*3);
#endif

#endif
	myNumTris = 0;

}
/***********************************************************************************************************/
static
const GLfloat g_pCacheVerts[] =
{
-0.5f, -0.5f,
0.5f, -0.5f,
-0.5f,  0.5f,
0.5f,  0.5f,
};
static
const GLfloat g_pCacheUVs[] =
{
0, 0,
1, 0,
0, 1,
1, 1,
};

void CachedSequence::addSprite(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScale, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha, bool bTestScreenIntersection)
{
	addSprite(fX, fY, fAlpha, fDegAngle, fScale, fScale, fAnimProgress, bPremultiplyAlpha, bTestScreenIntersection);	
}

void CachedSequence::addSprite(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha, bool bTestScreenIntersection, const SUVSet* pUVSet, bool bIsProgressAFrameNum, const SExtraSeqDrawInfo* pExtraInfo)
{
	// myLastUsedTime = GameEngine::getGlobalTime(ClockUniversal); 
	if(myTexture)
		myTexture->ensureLoaded();

	if(myTexture)
		myTexture->markUsed();

	if(fAnimProgress < 0 && bIsProgressAFrameNum)
		fAnimProgress = 0;
	
	// Ensure we have enough indices
	ensureCapacity(myNumTris + 2);
	
	FLOAT_TYPE fW, fH;
	const SUVSet *uvs;
#ifdef DIRECTX_PIPELINE
	SUVSet spare_uvs;
#endif

#ifdef CACHE_TEXTURE_INFO
	
	// See if we have to re-cache our animation if the animation progress is different
	FLOAT_TYPE fAnimProgDiff = fAnimProgress - myCachedAnimProgress;
	if(myCachedTexIndex == 0 || !(fAnimProgDiff >= -FLOAT_EPSILON && fAnimProgDiff <= FLOAT_EPSILON))
		initCachedTexParms(fAnimProgress, bIsProgressAFrameNum, myTexture);
	
	if(pUVSet)
		uvs = pUVSet;
	else
		uvs = &myCachedTexUVs;
	
	if(fScaleX != 1.0 || fScaleY != 1.0)
	{
		fW = myCachedTexW*fScaleX;
		fH = myCachedTexH*fScaleY;

#ifdef DIRECTX_PIPELINE
		if(fScaleX < 0 || fScaleY < 0)
		{
			FLOAT_TYPE fTempCoord;
			spare_uvs = *uvs;	
			uvs = &spare_uvs;

			if(fScaleX < 0)
			{
				fW = fabs(fW);
				fTempCoord = spare_uvs.myStartX;
				spare_uvs.myStartX = spare_uvs.myEndX;
				spare_uvs.myEndX = fTempCoord;
			}

			if(fScaleY < 0)
			{
				fH = fabs(fH);
				fTempCoord = spare_uvs.myStartY;
				spare_uvs.myStartY = spare_uvs.myEndY;
				spare_uvs.myEndY = fTempCoord;
			}
		}
#endif

	}
	else
	{
		fW = myCachedTexW;
		fH = myCachedTexH;
	}
	
	if(pExtraInfo)
	{
		if(pExtraInfo->myW > 0)
			fW = pExtraInfo->myW;
		if(pExtraInfo->myH > 0)
			fH = pExtraInfo->myH;
	}
	
#else
	int iW, iH;
	GLuint iIndex;	
	BlendModeType eBlendMode;
	if(bIsProgressAFrameNum)
		iIndex = getTextureManager()->getTextureIndex(myAnimType.c_str(), (int)fAnimProgress, iW, iH, uvs, eBlendMode);
	else
		iIndex = getTextureManager()->getTextureIndexFromProgress(myAnimType.c_str(), fAnimProgress, iW, iH, uvs, eBlendMode);
	
	if(pUVSet)
		uvs = *pUVSet;
	
	if(fScaleX != 1.0 || fScaleY != 1.0)
	{
		fW = iW*fScaleX;
		fH = iH*fScaleY;
	}
	else
	{
		fW = iW;
		fH = iH;
	}
#endif

	if(pExtraInfo && pExtraInfo->mySkewingPortion > 0.0 && pExtraInfo->mySkewingPortion < 1.0)
	{
		// In this case we need to split the bitmap into two calls
		FLOAT_TYPE fVertSize = fH*pExtraInfo->mySkewingPortion;
		
		SUVSet uvCustomSplit;
		SExtraSeqDrawInfo rOwnInfo;
		rOwnInfo = *pExtraInfo;
		if(pExtraInfo->myUseVertices)
		{
			// We need to use custom UVs?
			// Unsupported for now.
			_ASSERT(0);
		}
		else
		{
			uvCustomSplit = *uvs;
			uvCustomSplit.myEndY = (uvs->myEndY - uvs->myStartY)*pExtraInfo->mySkewingPortion + uvs->myStartY;

			// This is the completely skewed portion
			rOwnInfo.mySkewingPortion = 1.0;
			this->addSprite(fX, fY - (fH - fVertSize)/2.0, fAlpha, fDegAngle, fScaleX, fScaleY*pExtraInfo->mySkewingPortion, fAnimProgress, 
				bPremultiplyAlpha, bTestScreenIntersection, &uvCustomSplit, 
				bIsProgressAFrameNum, &rOwnInfo);

			// This is the untouched foundation.
			if(pExtraInfo->mySkewingPortion < 1.0)
			{
				uvCustomSplit = *uvs;
				uvCustomSplit.myStartY = uvs->myEndY - (uvs->myEndY - uvs->myStartY)*(1.0 - pExtraInfo->mySkewingPortion);

				rOwnInfo.mySkewingPortion = 0.0;
				rOwnInfo.mySkewingSize = 0.0;
				this->addSprite(fX, fY + fVertSize/2.0, fAlpha, fDegAngle, fScaleX, fScaleY*(1.0 - pExtraInfo->mySkewingPortion), fAnimProgress, 
					bPremultiplyAlpha, bTestScreenIntersection, &uvCustomSplit, 
					bIsProgressAFrameNum, &rOwnInfo);
			}
		}

		return;
	}

	
#ifdef	ENABLE_IMAGE_VIEW_CLIPPING
	if(bTestScreenIntersection)
	{
		SRect2D srScreenRect;
		myParentCache->getWindow()->getWindowRectRelative(srScreenRect);

		FLOAT_TYPE fMaxSize = max(fW, fH);
		// NOTE: This does not account for rotations!
		// SRect2D srSpriteRect(fX - fW/2.0, fY - fH/2.0, fW, fH);
		// This one does, though, but is really convervative.
		SRect2D srSpriteRect(fX - fMaxSize/2.0, fY - fMaxSize/2.0, fMaxSize, fMaxSize);
	
		if(!srScreenRect.doesIntersect(srSpriteRect))
			return;

		// Now, test for the scissor rect:
		SRect2D srScissorRect;
		if(RenderUtils::getCurrentScissorRectangle(srScissorRect))
		{
			if(!srScissorRect.doesIntersect(srSpriteRect))
				return;
		}
	} 
#endif
	
	FLOAT_TYPE fColVal = 1.0;
#ifndef DIRECTX_PIPELINE
	if(bPremultiplyAlpha)
		fColVal = fAlpha;
#endif

#ifdef USE_OPENGL2
	SColor scolVertexColors;
	scolVertexColors.set(fColVal, fColVal, fColVal, fAlpha);
	if(pExtraInfo)
	{
		scolVertexColors.r = fColVal*pExtraInfo->myPolyColor.r;
		scolVertexColors.g = fColVal*pExtraInfo->myPolyColor.g;
		scolVertexColors.b = fColVal*pExtraInfo->myPolyColor.b;
	}
#else

	short iColRVal = (short)(fColVal*(FLOAT_TYPE)255.0);
	short iColGVal = iColRVal;
	short iColBVal = iColRVal;
	short iAlpha = (short)(fAlpha*(FLOAT_TYPE)255.0);
	
	if(pExtraInfo)
	{
		// Use supplied color
		iColRVal = (short)(fColVal*255.0*pExtraInfo->myPolyColor.r);
		iColGVal = (short)(fColVal*255.0*pExtraInfo->myPolyColor.g);
		iColBVal = (short)(fColVal*255.0*pExtraInfo->myPolyColor.b);
	}
#endif

	if(myNumTris == 0)
		myParentCache->addSequenceToRender(this);
	
	int iVStart, iCStart, iUVStart;
	getArrayIndicesFromTriIdx(myNumTris, iVStart, iCStart, iUVStart);

	int iVert;

	FLOAT_TYPE fSkewAmount = 0;
	if(pExtraInfo && pExtraInfo->mySkewingSize != 0.0)
	{
		_ASSERT(pExtraInfo->mySkewingPortion <= 0.0 || 
			pExtraInfo->mySkewingPortion >= 1.0);
		fSkewAmount = pExtraInfo->mySkewingSize*fW;
	}

	if(pExtraInfo && pExtraInfo->myUseVertices)
	{
		for(iVert = 0; iVert < 4; iVert++)
		{
			svTempVec[iVert].x = pExtraInfo->myVerts[iVert].x;
			svTempVec[iVert].y = pExtraInfo->myVerts[iVert].y;

			// NOTE: This assumes the first two are the top!
			if(fSkewAmount != 0.0 && iVert < 2)
				svTempVec[iVert].x += fSkewAmount;

			// Rotate 
			if(fDegAngle != 0.0)
				svTempVec[iVert].rotateCCW(fDegAngle);

			// Move to position
			svTempVec[iVert].x += fX;
			svTempVec[iVert].y += fY;
		}

		myVertices[iVStart].s0 = pExtraInfo->myCustomUVs[0].x;
		myVertices[iVStart].t0 = pExtraInfo->myCustomUVs[0].y;
		myVertices[iVStart + 1].s0 = pExtraInfo->myCustomUVs[1].x;
		myVertices[iVStart + 1].t0 = pExtraInfo->myCustomUVs[1].y;
		myVertices[iVStart + 2].s0 = pExtraInfo->myCustomUVs[2].x;
		myVertices[iVStart + 2].t0 = pExtraInfo->myCustomUVs[2].x;

		myVertices[iVStart + 3].s0 = pExtraInfo->myCustomUVs[1].x;
		myVertices[iVStart + 3].t0 = pExtraInfo->myCustomUVs[1].y;
		myVertices[iVStart + 1 + 3].s0 = pExtraInfo->myCustomUVs[2].x;
		myVertices[iVStart + 1 + 3].t0 = pExtraInfo->myCustomUVs[2].y;
		myVertices[iVStart + 2 + 3].s0 = pExtraInfo->myCustomUVs[3].x;
		myVertices[iVStart + 2 + 3].t0 = pExtraInfo->myCustomUVs[3].y;
	}
	else
	{
		
		// Transform the points
		for(iVert = 0; iVert < 4; iVert++)
		{
			svTempVec[iVert].x = g_pCacheVerts[iVert*2]*fW;
			svTempVec[iVert].y = g_pCacheVerts[iVert*2 + 1]*fH;

			// NOTE: Assumes the first two verts are the top.
			if(fSkewAmount != 0.0 && iVert < 2)
				svTempVec[iVert].x += fSkewAmount;
			
			// Rotate 
			if(fDegAngle != 0.0)
				svTempVec[iVert].rotateCCW(fDegAngle);
			
			// Move to position
			svTempVec[iVert].x += fX;
			svTempVec[iVert].y += fY;
		}
		
		
#ifdef CACHE_TEXTURE_INFO
		myVertices[iVStart].s0 = uvs->myStartX;
		myVertices[iVStart].t0 = uvs->myStartY;
		myVertices[iVStart + 1].s0 = uvs->myEndX;
		myVertices[iVStart + 1].t0 = uvs->myStartY;
		myVertices[iVStart + 2].s0 = uvs->myStartX;
		myVertices[iVStart + 2].t0 = uvs->myEndY;

#else
		myUVs[iUVStart + 0] = uvs->myStartX;
		myUVs[iUVStart + 1] = uvs->myStartY;
		myUVs[iUVStart + 2] = uvs->myEndX;
		myUVs[iUVStart + 3] = uvs->myStartY;
		myUVs[iUVStart + 4] = uvs->myStartX;
		myUVs[iUVStart + 5] = uvs->myEndY;
#endif

		
#ifdef CACHE_TEXTURE_INFO
		myVertices[iVStart + 3].s0 = uvs->myEndX;
		myVertices[iVStart + 3].t0 = uvs->myStartY;
		myVertices[iVStart + 1 + 3].s0 = uvs->myStartX;
		myVertices[iVStart + 1 + 3].t0 = uvs->myEndY;
		myVertices[iVStart + 2 + 3].s0 = uvs->myEndX;
		myVertices[iVStart + 2 + 3].t0 = uvs->myEndY;
		
#else
		myUVs[iUVStart + 6] = uvs->myEndX;
		myUVs[iUVStart + 7] = uvs->myStartY;
		myUVs[iUVStart + 8] = uvs->myStartX;
		myUVs[iUVStart + 9] = uvs->myEndY;
		myUVs[iUVStart + 10] = uvs->myEndX;
		myUVs[iUVStart + 11] = uvs->myEndY;
#endif
	} // end if no extra info

	// Write to arrays
	for(iVert = 0; iVert < 3; iVert++)
	{
		myVertices[iVStart + iVert].x = svTempVec[iVert].x;
		myVertices[iVStart + iVert].y = svTempVec[iVert].y;

#ifdef USE_OPENGL2
		myVertices[iVStart + iVert].z = 0.0;
		myVertices[iVStart + iVert].w = 1.0;

		myVertices[iVStart + iVert].r = scolVertexColors.r;
		myVertices[iVStart + iVert].g = scolVertexColors.g;
		myVertices[iVStart + iVert].b = scolVertexColors.b;
		myVertices[iVStart + iVert].a = scolVertexColors.alpha;

#else
		myVertices[iVStart + iVert].r = iColRVal;
		myVertices[iVStart + iVert].g = iColGVal;
		myVertices[iVStart + iVert].b = iColBVal;
		myVertices[iVStart + iVert].a = iAlpha;
#endif
	}


	for(iVert = 0; iVert < 3; iVert++)
	{
		myVertices[iVStart + iVert + 3].x = svTempVec[iVert + 1].x;
		myVertices[iVStart + iVert + 3].y = svTempVec[iVert + 1].y;

#ifdef USE_OPENGL2
		myVertices[iVStart + iVert + 3].z = 0.0;
		myVertices[iVStart + iVert + 3].w = 1.0;

		myVertices[iVStart + iVert + 3].r = scolVertexColors.r;
		myVertices[iVStart + iVert + 3].g = scolVertexColors.g;
		myVertices[iVStart + iVert + 3].b = scolVertexColors.b;
		myVertices[iVStart + iVert + 3].a = scolVertexColors.alpha;
#else
		myVertices[iVStart + iVert + 3].r = iColRVal;
		myVertices[iVStart + iVert + 3].g = iColGVal;
		myVertices[iVStart + iVert + 3].b = iColBVal;
		myVertices[iVStart + iVert + 3].a = iAlpha;
#endif
	}

#ifdef DIRECTX_PIPELINE
	// Swap the last triangle to get the right 
	// winding order
	SVertexInfo svTemp;
	svTemp = myVertices[iVStart + 2 + 3];
	myVertices[iVStart + 2 + 3] = myVertices[iVStart + 0 + 3];
	myVertices[iVStart + 0 + 3] = svTemp;
#endif

	myNumTris += 2;

}
/***********************************************************************************************************/
inline 
void CachedSequence::getArrayIndicesFromTriIdx(int iTriIdx, int& iVertIdxOut, int& iColIdxOut, int& iUVIdxOut)
{	/*
 iVertIdxOut = iTriIdx*2*3;
 iColIdxOut = iTriIdx*4*3;
 iUVIdxOut = iTriIdx*2*3;
 */
	// For interleaved only:
	iVertIdxOut = iTriIdx*3;
}
/***********************************************************************************************************/
void CachedSequence::ensureCapacity(int iNumTris)
{
	int iVStart, iCStart, iUVStart;
	getArrayIndicesFromTriIdx(iNumTris, iVStart, iCStart, iUVStart); 
	
	int iNewVStart, iNewCStart, iNewUVStart;
	int iNewNumTris = myNumTris * 2;
	if(iNewNumTris < iNumTris)
		iNewNumTris = iNumTris;
	getArrayIndicesFromTriIdx(iNewNumTris, iNewVStart, iNewCStart, iNewUVStart); 
	
	if(iVStart > myNumAllocVerts)
	{
		SVertexInfo* pOldArray = NULL;
		
		// Re-alloc the vertices.
		if(myNumAllocVerts > 0)
			pOldArray = myVertices;
		
		// Allocate the new array
		myVertices = new SVertexInfo[iNewVStart];
		
		// Copy the old data there
		if(myNumAllocVerts > 0)
			memcpy(myVertices, pOldArray, sizeof(SVertexInfo)*myNumAllocVerts);

		delete[] pOldArray;
		myNumAllocVerts = iNewVStart;
		pOldArray = NULL;	
	}	
	/*	
	 GLfloat *pTempArray = NULL;  
	 
	 // Now, see if we need to re-alloc the array.
	 if(iVStart > myNumAllocVerts)
	 {
	 // Re-alloc the vertices.
	 if(myNumAllocVerts > 0)
	 pTempArray = myVerts;
	 
	 // Allocate the new array
	 myVerts = new GLfloat[iNewVStart];
	 
	 // Copy the old data there
	 if(myNumAllocVerts > 0)
	 {
	 memcpy(myVerts, pTempArray, sizeof(GLfloat)*myNumAllocVerts);
	 delete[] pTempArray;
	 }
	 myNumAllocVerts = iNewVStart;
	 pTempArray = NULL;	
	 }
	 
	 if(iUVStart > myNumAllocUVs)
	 {
	 // Re-alloc the vertices.
	 if(myNumAllocUVs > 0)
	 pTempArray = myUVs;
	 
	 // Allocate the new array
	 myUVs = new GLfloat[iNewUVStart];
	 
	 // Copy the old data there
	 if(myNumAllocUVs > 0)
	 {
	 memcpy(myUVs, pTempArray, sizeof(GLfloat)*myNumAllocUVs);
	 delete[] pTempArray;
	 }
	 myNumAllocUVs = iNewUVStart;
	 pTempArray = NULL;	
	 }
	 
	 if(iCStart > myNumAllocColors)
	 {
	 // Re-alloc the vertices.
	 if(myNumAllocColors > 0)
	 pTempArray = myColors;
	 
	 // Allocate the new array
	 myColors = new GLfloat[iNewCStart];
	 
	 // Copy the old data there
	 if(myNumAllocColors > 0)
	 {
	 memcpy(myColors, pTempArray, sizeof(GLfloat)*myNumAllocColors);
	 delete[] pTempArray;
	 }
	 myNumAllocColors = iNewCStart;
	 pTempArray = NULL;	
	 }
	 
	 */
}
/***********************************************************************************************************/
void CachedSequence::resetCache(void) 
{ 
	myCachedAnimProgress = -2.0; 
	myCachedTexIndex = 0; 
} 
/*****************************************************************************/
TextureManager* CachedSequence::getTextureManager()
{
	return myParentCache->getTextureManager();
}
/***********************************************************************************************************/
};