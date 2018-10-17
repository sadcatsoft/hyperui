#include "stdafx.h"

namespace HyperUI
{

#ifdef DIRECTX_PIPELINE
#include "D3DRenderer.h"
extern D3DRenderer* g_pDxRenderer;
#endif

static
const GLfloat g_pPerspCacheVerts[] =
{
	-0.5f, 0.0, -0.5f,
	0.5f, 0.0f, -0.5f,
	-0.5f,  0.0f, 0.5f,
	0.5f,  0.0f, 0.5f,
};
/*

static
const GLfloat g_pPerspCacheVerts[] =
{
	-0.5f, 0.0, 0.0f,
	0.5f, 0.0f, 0.0f,
	-0.5f,  0.0f, 1.0f,
	0.5f,  0.0f, 1.0f,
};
*/
bool CachedDeepSequence::theInitializedPerspPoints = false;
int CachedDeepSequence::thePerspCallCount = 0;
SVector3D CachedDeepSequence::svTempVec[4];

/***********************************************************************************************************/
CachedDeepSequence::CachedDeepSequence(DrawingCache* pParent, bool bPerspective)
{
	myIsPerspective = bPerspective;

	myTexture = NULL;
	myIsUnloadable = false;
	myRenderStage = 1;
	myParentCache = pParent;

	myDisableAlphaTest = false;
	myVertices = NULL;
	myNumAllocVerts = 0;
	
	myCachedAnimProgress = -1;
	myNumTris = 0;
	myCallCount = 0;
	thePerspCallCount = 0;

	resetCache();
/*
	if(!theInitializedPerspPoints)
	{
		theInitializedPerspPoints = true;
		SVector2D svZVec(1.0, 0);
		svZVec.rotateCCWPrecise(-PERSP_CAMERA_ANGLE);

		g_pPerspCacheVerts[2] = g_pPerspCacheVerts[5] = -svZVec.x/2.0;
		g_pPerspCacheVerts[8] = g_pPerspCacheVerts[11] = svZVec.x/2.0;

		g_pPerspCacheVerts[1] = g_pPerspCacheVerts[4] = -svZVec.y/2.0;
		g_pPerspCacheVerts[7] = g_pPerspCacheVerts[10] = svZVec.y/2.0;
	}
*/	
}
/***********************************************************************************************************/
CachedDeepSequence::~CachedDeepSequence()
{
	
	if(myVertices)
		delete[] myVertices;
	myVertices = NULL;
	
	myNumAllocVerts = 0;
	
	myNumTris = 0;
	
}
/***********************************************************************************************************/
void CachedDeepSequence::onTimerTick(GTIME lTime)
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
void CachedDeepSequence::setAnimType(const char *pcsType)
{
	myAnimType = pcsType;

	myRenderStage = getTextureManager()->getTextureRenderStage(pcsType);
	myIsUnloadable = getTextureManager()->getIsTextureUnloadable(pcsType);
	myDisableAlphaTest = getTextureManager()->getDisableAlphaTest(pcsType);
	myForceAlphaTest = getTextureManager()->getForceAlphaTest(pcsType);

	myTexture = getTextureManager()->getTexture(pcsType);

}
/***********************************************************************************************************/
#ifdef CACHE_TEXTURE_INFO
void CachedDeepSequence::initCachedTexParms(FLOAT_TYPE fAnimProgress, bool bIsProgressAFrameNum)
{	
	_ASSERT(fAnimProgress != myCachedAnimProgress);
	if(bIsProgressAFrameNum)
		myCachedTexIndex = getTextureManager()->getTextureIndex(myAnimType.c_str(), (int)fAnimProgress, myCachedTexW, myCachedTexH, myCachedTexUVs, myCachedTexBlendMode, NULL);	
	else
		myCachedTexIndex = getTextureManager()->getTextureIndexFromProgress(myAnimType.c_str(), fAnimProgress, myCachedTexW, myCachedTexH, myCachedTexUVs, myCachedTexBlendMode, NULL);	
	myCachedAnimProgress = fAnimProgress;
}
#endif
/***********************************************************************************************************/
void CachedDeepSequence::flushRangeInclusive(int iVertextStartIndex, int iVertextEndIndex)
{
#ifdef CACHE_TEXTURE_INFO
	_ASSERT(myCachedAnimProgress >= 0.0);
	if(myParentCache->getWindow()->getBlendMode() != myCachedTexBlendMode)
		myParentCache->getWindow()->setBlendMode(myCachedTexBlendMode);

#ifdef DIRECTX_PIPELINE
	if(myNumTris > 0)
		myDxBuffer.setFromVertexArray(myVertices, myNumTris);
#endif


#ifdef DIRECTX_PIPELINE
	g_pDxRenderer->getD3dContext()->PSSetShaderResources(0, 1, myCachedTexIndex);
#else
	glBindTexture(GL_TEXTURE_2D, myCachedTexIndex);	
#endif

#else
	int iW, iH;
	GLuint iIndex;	
	SUVSet uvsOut;
	BlendModeType eBlendMode;
	iIndex = g_pTextureManager->getTextureIndex(myAnimType.c_str(), 0, iW, iH, uvsOut, eBlendMode);

	if(myParentCache->getWindow()->getBlendMode() != eBlendMode)
		myParentCache->getWindow()->setBlendMode(eBlendMode);

	glBindTexture(GL_TEXTURE_2D, iIndex);	
#endif

#ifdef DIRECTX_PIPELINE
	UINT stride = sizeof(BasicVertex);
	UINT offset = 0;
	g_pDxRenderer->getD3dContext()->IASetVertexBuffers(0, 1, myDxBuffer.getVertexBuffer(), &stride, &offset);
	g_pDxRenderer->getD3dContext()->IASetIndexBuffer(myDxBuffer.getIndexBuffer(), INDEX_BUFFER_FORMAT,	0);
	g_pDxRenderer->getD3dContext()->DrawIndexed(iVertextEndIndex - iVertextStartIndex + 3*2, iVertextStartIndex, 0);

#else

#ifdef USE_OPENGL2
	RenderUtils::drawBuffer<SVertex3DInfo>(&myVertices[0].x, SVertex3DInfo::getNumVertexComponents(), &myVertices[0].s0, &myVertices[0].r, NULL, iVertextEndIndex - iVertextStartIndex + 3*2, iVertextStartIndex);
#else
	OpenGLStateCleaner::doVertexPointerCall(3, GL_FLOAT, sizeof(SVertex3DInfo), &myVertices[0].x);
	OpenGLStateCleaner::doTexturePointerCall(2, GL_FLOAT, sizeof(SVertex3DInfo), &myVertices[0].s0);
	OpenGLStateCleaner::doColorPointerCall(4, GL_UNSIGNED_BYTE, sizeof(SVertex3DInfo), &myVertices[0].r);

	glDrawArrays(GL_TRIANGLES, iVertextStartIndex, iVertextEndIndex - iVertextStartIndex + 3*2);
#endif
#endif
	myCallCount = 0;
	thePerspCallCount = 0;
	myNumTris = 0;
}
/***********************************************************************************************************/
void CachedDeepSequence::flush(int iVertStartIndex)
{
	if(myNumTris <= 0 && iVertStartIndex < 0 )
		return;

#ifdef DIRECTX_PIPELINE
	if(myNumTris > 0)
		myDxBuffer.setFromVertexArray(myVertices, myNumTris);
#endif

	myCallCount = 0;
	thePerspCallCount = 0;
	
    
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

#ifdef DIRECTX_PIPELINE

	UINT stride = sizeof(BasicVertex);
	UINT offset = 0;

	int iNumToFlush = myNumTris*3;
	if(iVertStartIndex < 0)
		iVertStartIndex = 0;
	else
		iNumToFlush = 2*3;

	g_pDxRenderer->getD3dContext()->IASetVertexBuffers(0, 1, myDxBuffer.getVertexBuffer(), &stride, &offset);
	g_pDxRenderer->getD3dContext()->IASetIndexBuffer(myDxBuffer.getIndexBuffer(), INDEX_BUFFER_FORMAT,	0);
	g_pDxRenderer->getD3dContext()->DrawIndexed(iNumToFlush, iVertStartIndex, 0);

#else

	if(iVertStartIndex >= 0)
	{
#ifdef USE_OPENGL2
		RenderUtils::drawBuffer<SVertex3DInfo>(&myVertices[0].x, SVertex3DInfo::getNumVertexComponents(), &myVertices[0].s0, &myVertices[0].r, NULL, 2*3, iVertStartIndex);
#else
		// We're flushing an instance - two triangles
		//glVertexPointer(3, GL_SHORT, sizeof(SVertex3DInfo), &myVertices[0].x);
		OpenGLStateCleaner::doVertexPointerCall(3, GL_FLOAT, sizeof(SVertex3DInfo), &myVertices[0].x);
		OpenGLStateCleaner::doTexturePointerCall(2, GL_FLOAT, sizeof(SVertex3DInfo), &myVertices[0].s0);
		OpenGLStateCleaner::doColorPointerCall(4, GL_UNSIGNED_BYTE, sizeof(SVertex3DInfo), &myVertices[0].r);

//		glDrawArrays(GL_TRIANGLES, 0, 2*3);

		glDrawArrays(GL_TRIANGLES, iVertStartIndex, 2*3);
#endif
	}
	else
	{
#ifdef USE_OPENGL2
		RenderUtils::drawBuffer<SVertex3DInfo>(&myVertices[0].x, SVertex3DInfo::getNumVertexComponents(), &myVertices[0].s0, &myVertices[0].r, NULL, myNumTris*3);
#else
		// We're flushing it all, baby!
		//glVertexPointer(3, GL_SHORT, sizeof(SVertex3DInfo), &myVertices[0].x);
		OpenGLStateCleaner::doVertexPointerCall(3, GL_FLOAT, sizeof(SVertex3DInfo), &myVertices[0].x);
		OpenGLStateCleaner::doTexturePointerCall(2, GL_FLOAT, sizeof(SVertex3DInfo), &myVertices[0].s0);
		OpenGLStateCleaner::doColorPointerCall(4, GL_UNSIGNED_BYTE, sizeof(SVertex3DInfo), &myVertices[0].r);

		glDrawArrays(GL_TRIANGLES, 0, myNumTris*3);
#endif
	}
#endif
	
	myNumTris = 0;

}
/***********************************************************************************************************/
static
const GLfloat g_pCacheVerts[] =
{
	-0.5f, -0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	-0.5f,  0.5f, 0.0f,
	0.5f,  0.5f, 0.0f,
};

void CachedDeepSequence::addSprite(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScale, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha, bool bTestScreenIntersection, FLOAT_TYPE fZDepth)
{
	addSprite(fX, fY, fAlpha, fDegAngle, fScale, fScale, fAnimProgress, bPremultiplyAlpha, bTestScreenIntersection, NULL, false, NULL, fZDepth);	
}

void CachedDeepSequence::addSprite(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha, bool bTestScreenIntersection, SUVSet* pUVSet, bool bIsProgressAFrameNum, SExtraSeqDrawInfo3D* pExtraInfo, FLOAT_TYPE fZDepth, FLOAT_TYPE fPerspVertShift, FLOAT_TYPE fDepthShift)
{
	if(myTexture)
		myTexture->ensureLoaded();

	if(myTexture)
		myTexture->markUsed();

	if(fAnimProgress < 0 && bIsProgressAFrameNum)
		fAnimProgress = 0;

	const GLfloat* pVertArrayPtr = g_pCacheVerts;
	if(myIsPerspective)
	{
		fY *= -1;
		pVertArrayPtr = g_pPerspCacheVerts;
	}

	// NOTE: This is not what we use to sort in non-persp mode!
	if(fZDepth == FLOAT_TYPE_MAX)
	{
		if(myIsPerspective)
			fZDepth = -fY  / 10.0;
		else
			fZDepth = fY;
	}

	FLOAT_TYPE fFlatZUpShift = 0;
	if(myIsPerspective && pExtraInfo && pExtraInfo->myIsFlatOnGround)
	{
		fFlatZUpShift = 0.005*myCallCount;
		myCallCount++;
	}
	else if(myIsPerspective)
	{
		fY -= 0.005*thePerspCallCount;
		thePerspCallCount++;
	}
	
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
		initCachedTexParms(fAnimProgress, bIsProgressAFrameNum);
	
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
		SExtraSeqDrawInfo3D rOwnInfo;
		rOwnInfo = *pExtraInfo;
		if(pExtraInfo->myUseVertices)
		{
			// We need to use custom UVs?
			// Unsupported for now.
			_ASSERT(0);
		}
		else
		{
			if(myIsPerspective)
			{
				uvCustomSplit = *uvs;
				uvCustomSplit.myEndY = (uvs->myEndY - uvs->myStartY)*pExtraInfo->mySkewingPortion + uvs->myStartY;

				// This is the completely skewed portion
				// Negatve Y since we multiplied it by -1 above.
				rOwnInfo.mySkewingPortion = 1.0;
				this->addSprite(fX, -fY, fAlpha, fDegAngle, fScaleX, fScaleY*pExtraInfo->mySkewingPortion, fAnimProgress, 
					bPremultiplyAlpha, bTestScreenIntersection, &uvCustomSplit, 
					bIsProgressAFrameNum, &rOwnInfo, fZDepth, fH - fVertSize);

				// This is the untouched foundation.
				if(pExtraInfo->mySkewingPortion < 1.0)
				{
					uvCustomSplit = *uvs;
					
					//uvCustomSplit.myStartY = (uvs->myEndY - uvs->myStartY)*(1.0 - pExtraInfo->mySkewingPortion) + uvs->myStartY;
					uvCustomSplit.myStartY = (uvs->myEndY - uvs->myStartY)*(pExtraInfo->mySkewingPortion) + uvs->myStartY;

					rOwnInfo.mySkewingPortion = 0.0;
					rOwnInfo.mySkewingSize = 0.0;
					this->addSprite(fX, -fY, fAlpha, fDegAngle, fScaleX, fScaleY*(1.0 - pExtraInfo->mySkewingPortion), fAnimProgress, 
						bPremultiplyAlpha, bTestScreenIntersection, &uvCustomSplit, 
						// If we're drawing at the bottom point, the bottom remains unchanged
						bIsProgressAFrameNum, &rOwnInfo, fZDepth,  0); // -(fH - fVertSize)/2.0 + (fH - fVertSize)/2.0); // fVertSize/2.0
				}

			}
			else
			{
				uvCustomSplit = *uvs;
				uvCustomSplit.myEndY = (uvs->myEndY - uvs->myStartY)*pExtraInfo->mySkewingPortion + uvs->myStartY;

				// This is the completely skewed portion
				rOwnInfo.mySkewingPortion = 1.0;
				this->addSprite(fX, fY - (fH - fVertSize)/2.0, fAlpha, fDegAngle, fScaleX, fScaleY*pExtraInfo->mySkewingPortion, fAnimProgress, 
					bPremultiplyAlpha, bTestScreenIntersection, &uvCustomSplit, 
					bIsProgressAFrameNum, &rOwnInfo, fZDepth);

				// This is the untouched foundation.
				if(pExtraInfo->mySkewingPortion < 1.0)
				{
					uvCustomSplit = *uvs;
					uvCustomSplit.myStartY = (uvs->myEndY - uvs->myStartY)*(1.0 - pExtraInfo->mySkewingPortion) + uvs->myStartY;

					rOwnInfo.mySkewingPortion = 0.0;
					rOwnInfo.mySkewingSize = 0.0;
					this->addSprite(fX, fY + fVertSize/2.0, fAlpha, fDegAngle, fScaleX, fScaleY*(1.0 - pExtraInfo->mySkewingPortion), fAnimProgress, 
						bPremultiplyAlpha, bTestScreenIntersection, &uvCustomSplit, 
						bIsProgressAFrameNum, &rOwnInfo, fZDepth);
				}

			}
		}

		return;
	}
	
#ifdef	ENABLE_IMAGE_VIEW_CLIPPING
	if(bTestScreenIntersection && !myIsPerspective)
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

	int iVStart, iCStart, iUVStart;
	getArrayIndicesFromTriIdx(myNumTris, iVStart, iCStart, iUVStart);

	if(myNumTris == 0)
		myParentCache->addDeepSequenceToRender(this, myIsPerspective, myIsPerspective ? fY*-1 : fZDepth, 0, fDepthShift, pExtraInfo ? pExtraInfo->myRelatedSeq : NULL);
	else // if(myIsPerspective)
		myParentCache->addDeepSequenceToRender(this, myIsPerspective, myIsPerspective ? fY*-1 : fZDepth, iVStart, fDepthShift, pExtraInfo ? pExtraInfo->myRelatedSeq : NULL);

	int iVert;
	SVector2D svTemp2DVec;	
	SVector2D svRelPivot;

	if(pExtraInfo && pExtraInfo->myVerticalAxisAngle != 0)
	{
		svRelPivot.x = pExtraInfo->myRotPivot.x/fW;
		svRelPivot.y = pExtraInfo->myRotPivot.y/fH;
	}

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
			
			svTemp2DVec.set(pExtraInfo->myVerts[iVert].x, pExtraInfo->myVerts[iVert].y);

			// NOTE: This assumes the first two are the top!
			if(fSkewAmount != 0.0 && iVert < 2)
				svTempVec[iVert].x += fSkewAmount;

			// Rotate 
			if(fDegAngle != 0.0)
				svTemp2DVec.rotateCCW(fDegAngle);

			svTempVec[iVert].x = svTemp2DVec.x;
			svTempVec[iVert].y = svTemp2DVec.y;
			svTempVec[iVert].z = pExtraInfo->myVerts[iVert].z;


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
		if(myIsPerspective)
		{
			SVector3D svExtraOffsets;
			if(pExtraInfo)
				svExtraOffsets = pExtraInfo->myPerspExtraOffset;

			for(iVert = 0; iVert < 4; iVert++)
			{

				svTemp2DVec.set(pVertArrayPtr[iVert*3]*fW, pVertArrayPtr[iVert*3 + 2]*fH);

				// NOTE: Assumes the first two verts are the top.
				if(fSkewAmount != 0.0 && iVert >= 2)
					svTemp2DVec.x += fSkewAmount;

				// Rotate 
				if(fDegAngle != 0.0)
					svTemp2DVec.rotateCCW(-fDegAngle);

				svTempVec[iVert].x = svTemp2DVec.x + svExtraOffsets.x;
				svTempVec[iVert].y = 0.0 + svExtraOffsets.y;
				// The last term, fH/2.0, is to shift this up so the bottom touches the ground.
				svTempVec[iVert].z = svTemp2DVec.y + fPerspVertShift + fH/2.0 + svExtraOffsets.z;

				if(pExtraInfo && pExtraInfo->myVerticalAxisAngle != 0)
				{
					// Rotate around our z-axis
					svTemp2DVec.set(svTempVec[iVert].x - svRelPivot.x,  svTempVec[iVert].y - svRelPivot.y);
					svTemp2DVec.rotateCCW(pExtraInfo->myVerticalAxisAngle);
					svTempVec[iVert].x = svTemp2DVec.x + svRelPivot.x;
					svTempVec[iVert].y = svTemp2DVec.y + svRelPivot.y;
				}

				svTemp2DVec.set(svTempVec[iVert].y,  svTempVec[iVert].z);

				if(pExtraInfo && pExtraInfo->myIsFlatOnGround)
				{
					svTemp2DVec.rotateCCW(90);
					// X is really Y, so...
					svTemp2DVec.x -= fH/2.0;
				}
				else
				{
#if !defined(DEBUG_PERSP_TOP_DOWN_VIEW)
				svTemp2DVec.rotateCCW(90 + PERSP_CAMERA_ANGLE);
#endif
				}

				svTempVec[iVert].y = svTemp2DVec.x + PERSP_FIXED_Y_OFFSET;
				svTempVec[iVert].z = svTemp2DVec.y + PERSP_FIXED_Z_OFFSET + fFlatZUpShift;

				// Move to position
				svTempVec[iVert].x += fX;
				svTempVec[iVert].y += fY;
			}
		}
		else
		{
			for(iVert = 0; iVert < 4; iVert++)
			{
				svTemp2DVec.set(pVertArrayPtr[iVert*3]*fW, pVertArrayPtr[iVert*3 + 1]*fH);

				// NOTE: Assumes the first two verts are the top.
				if(fSkewAmount != 0.0 && iVert < 2)
					svTemp2DVec.x += fSkewAmount;
		
				// Rotate 
				if(fDegAngle != 0.0)
					svTemp2DVec.rotateCCW(fDegAngle);

				svTempVec[iVert].x = svTemp2DVec.x;
				svTempVec[iVert].y = svTemp2DVec.y;
				svTempVec[iVert].z = pVertArrayPtr[iVert*3 + 2];

				if(pExtraInfo && pExtraInfo->myVerticalAxisAngle != 0)
				{
					// Rotate around our y-axis since in non-persp, it's up.
					svTemp2DVec.set(svTempVec[iVert].x - svRelPivot.x,  svTempVec[iVert].z - svRelPivot.y);
					svTemp2DVec.rotateCCW(pExtraInfo->myVerticalAxisAngle);
					svTempVec[iVert].x = svTemp2DVec.x + svRelPivot.x;
					svTempVec[iVert].z = svTemp2DVec.y + svRelPivot.y;
				}

				// Note that if we ever actually use z-depth buffer in ortho,
				// this will cause me to puzzle over why the clipping is incorrect
				// forever...
				if(myIsPerspective)
					svTempVec[iVert].z += fZDepth;

				// Move to position
				svTempVec[iVert].x += fX;
				svTempVec[iVert].y += fY;


			}
		}
		
#ifdef CACHE_TEXTURE_INFO
		if(myIsPerspective)
		{
			myVertices[iVStart].s0 = uvs->myStartX;
			myVertices[iVStart].t0 = uvs->myEndY;
			myVertices[iVStart + 1].s0 = uvs->myEndX;
			myVertices[iVStart + 1].t0 = uvs->myEndY;
			myVertices[iVStart + 2].s0 = uvs->myStartX;
			myVertices[iVStart + 2].t0 = uvs->myStartY;
		}
		else
		{
			myVertices[iVStart].s0 = uvs->myStartX;
			myVertices[iVStart].t0 = uvs->myStartY;
			myVertices[iVStart + 1].s0 = uvs->myEndX;
			myVertices[iVStart + 1].t0 = uvs->myStartY;
			myVertices[iVStart + 2].s0 = uvs->myStartX;
			myVertices[iVStart + 2].t0 = uvs->myEndY;
		}
#else
		myUVs[iUVStart + 0] = uvs->myStartX;
		myUVs[iUVStart + 1] = uvs->myStartY;
		myUVs[iUVStart + 2] = uvs->myEndX;
		myUVs[iUVStart + 3] = uvs->myStartY;
		myUVs[iUVStart + 4] = uvs->myStartX;
		myUVs[iUVStart + 5] = uvs->myEndY;
#endif

		
#ifdef CACHE_TEXTURE_INFO

		if(myIsPerspective)
		{
			myVertices[iVStart + 3].s0 = uvs->myEndX;
			myVertices[iVStart + 3].t0 = uvs->myEndY;
			myVertices[iVStart + 1 + 3].s0 = uvs->myStartX;
			myVertices[iVStart + 1 + 3].t0 = uvs->myStartY;
			myVertices[iVStart + 2 + 3].s0 = uvs->myEndX;
			myVertices[iVStart + 2 + 3].t0 = uvs->myStartY;
		}
		else
		{
			myVertices[iVStart + 3].s0 = uvs->myEndX;
			myVertices[iVStart + 3].t0 = uvs->myStartY;
			myVertices[iVStart + 1 + 3].s0 = uvs->myStartX;
			myVertices[iVStart + 1 + 3].t0 = uvs->myEndY;
			myVertices[iVStart + 2 + 3].s0 = uvs->myEndX;
			myVertices[iVStart + 2 + 3].t0 = uvs->myEndY;
		}
		
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
		myVertices[iVStart + iVert].z = svTempVec[iVert].z;
#ifdef USE_OPENGL2
		myVertices[iVStart + iVert].w = 1.0;
#endif

#ifdef _DEBUG
		// Test for validity
		_ASSERT(isValidNumber(myVertices[iVStart + iVert].x));
		_ASSERT(isValidNumber(myVertices[iVStart + iVert].y));
		_ASSERT(isValidNumber(myVertices[iVStart + iVert].z));
#endif

#ifdef USE_OPENGL2
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
		myVertices[iVStart + iVert + 3].z = svTempVec[iVert + 1].z;
#ifdef USE_OPENGL2
		myVertices[iVStart + iVert + 3].w = 1.0;
#endif


#ifdef _DEBUG
		// Test for validity
		_ASSERT(isValidNumber(myVertices[iVStart + iVert + 3].x));
		_ASSERT(isValidNumber(myVertices[iVStart + iVert + 3].y));
		_ASSERT(isValidNumber(myVertices[iVStart + iVert + 3].z));
#endif

#ifdef USE_OPENGL2
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
	SVertex3DInfo svTemp;
	svTemp = myVertices[iVStart + 2 + 3];
	myVertices[iVStart + 2 + 3] = myVertices[iVStart + 0 + 3];
	myVertices[iVStart + 0 + 3] = svTemp;
#endif

	myNumTris += 2;

}
/***********************************************************************************************************/
TextureManager* CachedDeepSequence::getTextureManager()
{
	return myParentCache->getTextureManager();
}
/***********************************************************************************************************/
inline 
void CachedDeepSequence::getArrayIndicesFromTriIdx(int iTriIdx, int& iVertIdxOut, int& iColIdxOut, int& iUVIdxOut)
{
	// For interleaved only:
	iVertIdxOut = iTriIdx*3;
}
/***********************************************************************************************************/
void CachedDeepSequence::ensureCapacity(int iNumTris)
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
		SVertex3DInfo* pOldArray = NULL;
		
		// Re-alloc the vertices.
		if(myNumAllocVerts > 0)
			pOldArray = myVertices;
		
		// Allocate the new array
		myVertices = new SVertex3DInfo[iNewVStart];
		
		// Copy the old data there
		if(myNumAllocVerts > 0)
		{
			memcpy(myVertices, pOldArray, sizeof(SVertexInfo)*myNumAllocVerts);
			delete[] pOldArray;
		}
		myNumAllocVerts = iNewVStart;
		pOldArray = NULL;	
	}	
}
/***********************************************************************************************************/
};