#include "stdafx.h"

#if defined(EXPERIMENTAL_SORT_PERSP_SEQ_BY_DEPTH) || defined(EXPERIMENTAL_SORT_ORTHO_SEQ_BY_DEPTH)
	#define EXPERIM_ENABLE_RELATED_SEQ_SORTING
#endif
//#ifndef WIN32
//#include "GameView.h"
//extern GameView* g_pMainView;
//#endif

namespace HyperUI
{
#ifdef DIRECTX_PIPELINE
#include "D3DRenderer.h"
extern D3DRenderer* g_pDxRenderer;
#endif

// There are TWO of these - one in DrawingCache.cpp one in RenderUtils.cpp!!! Bad Oleg...
#define EXTRA_FONT_PIXEL_PADDING		0.0 // 2.0

SGenData DrawingCache::mySharedTrailData;
TIntVector DrawingCache::theTextLineLengths;

//TCachedSequenceAlphaMapMemCacher DrawingCache::mySequencesMapMemCacher(32);

/***********************************************************************************************************/
// DrawingCache
/***********************************************************************************************************/
DrawingCache::DrawingCache(Window* pWindow)
	: mySequencesMapMemCacher(32), mySequences(&mySequencesMapMemCacher)
{
	myParentWindow = pWindow;

	myNumFrameFlushes = 0;

	///myCacher =  new TCachedSeqCacher;
	myDeepCacher = new TCachedDeepSeqCacher;
	myDeepPerspCacher = new TCachedDeepSeqCacher;
	myFlushText = true;

	myTextCacher = new TextCacher(pWindow);
}
/***********************************************************************************************************/
DrawingCache::~DrawingCache()
{
// 	myCacher->clearAll();
// 	delete myCacher;

	myDeepCacher->clearAll();
	delete myDeepCacher;

	myDeepPerspCacher->clearAll();
	delete myDeepPerspCacher;

	delete myTextCacher;

	// Clear the map
	clearAndDeleteContainer(myAllSequences);
	mySequences.clear();

	

// 	TStringCachedSequenceMap::iterator mi;
// 	for(mi = mySequences.begin(); mi != mySequences.end(); mi++)
// 		delete mi->second;
// 	mySequences.clear();

	TStringCachedDeepSequenceMap::iterator mi2;
	for(mi2 = myDeepSequences.begin(); mi2 != myDeepSequences.end(); mi2++)
		delete mi2->second;
	myDeepSequences.clear();

	for(mi2 = myDeepPerspSequences.begin(); mi2 != myDeepPerspSequences.end(); mi2++)
		delete mi2->second;
	myDeepPerspSequences.clear();
}
/***********************************************************************************************************/
void DrawingCache::addSequenceToRender(CachedSequence* pSeq)
{
	int iRenderStage = pSeq->getRenderStage();
	myFullSequences.ensureCapacity(iRenderStage + 1);
	if(!myFullSequences[iRenderStage])
		myFullSequences[iRenderStage] = new TCachedSequences;
	myFullSequences[iRenderStage]->push_back(pSeq);
}
/***********************************************************************************************************/
void DrawingCache::addDeepSequenceToRender(CachedDeepSequence* pSeq, bool bPerspective, FLOAT_TYPE fDepth, int iVertOffset, FLOAT_TYPE fDepthShift, CachedSequence* pRelatedSeq)
{
	int iRenderStage = pSeq->getRenderStage();
	if(bPerspective)
	{
#ifdef EXPERIMENTAL_SORT_PERSP_SEQ_BY_DEPTH
		SPerspObjInstance rInst;
		rInst.mySeq = pSeq;
		rInst.myDepth = fDepth + fDepthShift - PERSP_FIXED_Y_OFFSET;
		rInst.myVertStartIndex = iVertOffset;
		rInst.myType = PerspObjDeepSeq;
		rInst.myRelatedSeqPtr = pRelatedSeq;
		mySortedDeepPerspSequences.insert(rInst);
#else
		myFullDeepPerspSequences[iRenderStage].push_back(pSeq);
#endif
	}
	else
	{
#ifdef EXPERIMENTAL_SORT_ORTHO_SEQ_BY_DEPTH
		SPerspObjInstance rInst;
		rInst.mySeq = pSeq;
		rInst.myDepth = fDepth + fDepthShift;
		rInst.myVertStartIndex = iVertOffset;
		rInst.myType = PerspObjDeepSeq;
		rInst.myRelatedSeqPtr = pRelatedSeq;
		mySortedDeepPerspSequences.insert(rInst);
#else
		myFullDeepSequences[iRenderStage].push_back(pSeq);
#endif
	}
}
/***********************************************************************************************************/
void DrawingCache::drawSampledTrail(SGenData &rSharedData, SCachedTrail* pTrail)
{
	// const char* pcsAnim = pTrail->mySampledAnimSeq->getName();
	int iCurrPoint;
	FLOAT_TYPE fAdditionalRotation = pTrail->myTrail->getAdditionalRotation();
	FLOAT_TYPE fAlpha, fDegAngle;
	SVector2D svDir, svCurr, svNext;
	FLOAT_TYPE fFrameAnim = 0.0;
	// See if we're doing an atlas
	bool bIsProgressFrameNum = false;
	if(pTrail->myTrail->getSampleAnimAtlasFrame() >= 0 && !pTrail->myAnimateFrame)
	{
		fFrameAnim = pTrail->myTrail->getSampleAnimAtlasFrame();
		bIsProgressFrameNum = true;
	}
	FLOAT_TYPE fPrevGoodAngle = 0.0;
	for(iCurrPoint = rSharedData.myNumLines - 1; iCurrPoint >= 0; iCurrPoint--)
	{
		svCurr.set(rSharedData.myVerts[iCurrPoint].x, rSharedData.myVerts[iCurrPoint].y);
#ifdef USE_OPENGL2
		fAlpha = rSharedData.myVerts[iCurrPoint].a;
#else
		fAlpha = ((FLOAT_TYPE)rSharedData.myVerts[iCurrPoint].a)/255.0;
#endif
		if(pTrail->myAnimateFrame)
		{
			fFrameAnim = 1.0 - (fAlpha) - 0.1;
			if(fFrameAnim < 0.0)
				fFrameAnim = 0.0;
		}
		fAlpha *= pTrail->myOpacity;

		// Now determine the direction
		svNext.set(rSharedData.myVerts[iCurrPoint + 1].x, rSharedData.myVerts[iCurrPoint + 1].y);
		svDir = svCurr - svNext;
		if(svDir.normalize() > FLOAT_EPSILON || iCurrPoint == rSharedData.myNumLines - 1)
		{
			fDegAngle = svDir.getAngleFromPositiveX() + fAdditionalRotation;
			fPrevGoodAngle = fDegAngle;
		}
		else
			fDegAngle = fPrevGoodAngle;

		// pTrail->mySampledAnimSeq->addSprite(svCurr.x, svCurr.y, fAlpha, fDegAngle, 1, fFrameAnim, true);
		pTrail->mySampledAnimSeq->addSprite(svCurr.x, svCurr.y, fAlpha, fDegAngle, pTrail->myScale, pTrail->myScale, fFrameAnim, true, true, NULL, bIsProgressFrameNum, NULL);
	}
}
/***********************************************************************************************************/
void DrawingCache::flush()
{
	myNumFrameFlushes++;

	SVector2D svScreenDims;
	myParentWindow->getSize(svScreenDims);

	int iStage, iSeq, iNumSeqs;

	int iTrail, iNumTrails = myTrails.size();
	if(iNumTrails > 0)
	{
		// Draw trails first
		myParentWindow->setBlendMode(BlendModeText);
		GraphicsUtils::enableFlatDrawingState(-1.0);

		SCachedTrail *pTrail;
		bool bHadAnySampledTrails = false;
		for(iTrail = 0; iTrail < iNumTrails; iTrail++)
		{
			pTrail = &myTrails[iTrail];
			if(pTrail->myTrail->getSampleFreq() > 0)
				bHadAnySampledTrails = true;
			else
			{
				pTrail->myTrail->generateLines(pTrail->myScroll, mySharedTrailData, pTrail->myOpacity, pTrail->myScale);
				RenderUtils::drawLines(myParentWindow, mySharedTrailData, pTrail->myTrail->getThickness(), false);
			}
		}
		GraphicsUtils::disableFlatDrawingState(-1.0);
		myParentWindow->setBlendMode(BlendModeNormal);

		// Now draw any sampled trails if we so choose
		if(bHadAnySampledTrails)
		{
			for(iTrail = 0; iTrail < iNumTrails; iTrail++)
			{
				pTrail = &myTrails[iTrail];
				if(pTrail->myTrail->getSampleFreq() > 0)
				{
					pTrail->myTrail->generateLines(pTrail->myScroll, mySharedTrailData, pTrail->myOpacity, pTrail->myScale);
					drawSampledTrail(mySharedTrailData, pTrail);
				}
			}
		}

		myTrails.clear();
	}
	myParentWindow->setBlendMode(BlendModeNormal);
	int iNumRenderStages = myFullSequences.getCurrentCapacity();
	for(iStage = 0; iStage < iNumRenderStages; iStage++)
	{
		if(!myFullSequences[iStage])
			continue;

		iNumSeqs =  myFullSequences[iStage]->size();
		for(iSeq = 0; iSeq < iNumSeqs; iSeq++)
		{
#ifdef EXPERIM_ENABLE_RELATED_SEQ_SORTING
			if((*myFullSequences[iStage])[iSeq]->getMarkRelated() == false)
#endif
			(*myFullSequences[iStage])[iSeq]->flush();
		}
		myFullSequences[iStage]->clear();
	}

#ifdef USE_FREETYPE_FONTS
	SCachedLetterInfo *pLetter;
	int iCurrLetter, iNumLetters = myLetterObjects.size();
	for(iCurrLetter = 0; iCurrLetter < iNumLetters; iCurrLetter++)
	{
		pLetter = &myLetterObjects[iCurrLetter];
		CachedFontInfo::renderCharTexture(pLetter->myTextureIndex, pLetter->myOrigin, pLetter->mySize, pLetter->myColor);
	}
	myLetterObjects.clear();
#endif

	bool bHasSetupDeepSeqMode = false;
	for(iStage = 0; iStage < MAX_DEEP_RENDER_STAGES; iStage++)
	{
		iNumSeqs = myFullDeepSequences[iStage].size();
		for(iSeq = 0; iSeq < iNumSeqs; iSeq++)
		{
			if(!bHasSetupDeepSeqMode)
			{
				myParentWindow->setup3DGlView(false, false, false, true);
				bHasSetupDeepSeqMode = true;
			}
			myFullDeepSequences[iStage][iSeq]->flush();
		}
		myFullDeepSequences[iStage].clear();
	}
//	if(bHasSetupDeepSeqMode)
//		GameEngine::setup2DGlView();

	int iDepthOp, iNumDepthOps = myDepthBufferOps.size();

	// This will need to change for mixed persp and not arcs.
	int iObj3d, iNumPerspArcs3D = myArcs3D.size();


#if defined(EXPERIMENTAL_SORT_PERSP_SEQ_BY_DEPTH) || defined(EXPERIMENTAL_SORT_ORTHO_SEQ_BY_DEPTH)
	// Draw any arcs we might have
	SCachedArc3D *pArc3D;
	for(iObj3d = 0; iObj3d < iNumPerspArcs3D; iObj3d++)
	{
		if(iObj3d == 0)
		{
#ifdef DIRECTX_PIPELINE
			DXSTAGE3
#else

			myParentWindow->setup3DGlView(false, true, false, false);
			//glAlphaFunc ( GL_GREATER, ARC_ALPHA_CUTOFF_VALUE_3D ) ;
			//glEnable(GL_ALPHA_TEST);
//            glDisable(GL_ALPHA_TEST);
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
#endif
			myParentWindow->allowLighting(true);
		}
		pArc3D = &myArcs3D[iObj3d];

		RenderUtils::renderArc3D(myParentWindow, pArc3D->myCenter.x, pArc3D->myCenter.y, pArc3D->myCenter.z, pArc3D->myRadius, pArc3D->myThickness, pArc3D->myColor, pArc3D->myStartAngle, pArc3D->myEndAngle, pArc3D->myTexture.c_str(), pArc3D->myTaper, pArc3D->myWrapTexture, false, pArc3D->myFlatOnGround);
	}
	if(iNumPerspArcs3D > 0)
	{
#ifdef DIRECTX_PIPELINE
		DXSTAGE3
#else
#ifndef USE_OPENGL2
		glAlphaFunc ( GL_GREATER, ALPHA_CUTOFF_VALUE_3D ) ;
#endif
#endif
		GraphicsUtils::disableAlphaTest();

//        glDisable(GL_BLEND);
		Window::disableLighting();
	}
	myArcs3D.clear();
#endif



	bHasSetupDeepSeqMode = false;
#if defined(EXPERIMENTAL_SORT_PERSP_SEQ_BY_DEPTH) || defined(EXPERIMENTAL_SORT_ORTHO_SEQ_BY_DEPTH)
	int iNumForcedAlpha = 0;

	// Now the perspective seqs
	SCachedObject3D *pObj3d;
	TSortedPerspInstances::iterator si;
	bool bIsLightningEnabled = false;
    bHasSetupDeepSeqMode = false;
	for(si = mySortedDeepPerspSequences.begin(); si != mySortedDeepPerspSequences.end(); si++)
	{
		if(!bHasSetupDeepSeqMode)
		{
			// Notice - no alpha necessary.
#ifdef RENDER_FIELD_WITH_PERSPECTIVE
			myParentWindow->setup3DGlView(false, true, false, false);
#else
			myParentWindow->setup3DGlView(false, false, false, false);
#endif
			// Note that this is here so that overlapping 2D objects
            // such as grass on lake do not block portions of each othe.r
            // It causes artifacts for other things such as walls, but they're
            // more tolerable.
#ifdef DIRECTX_PIPELINE
			DXSTAGE2
#else
            glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);
#endif
			bIsLightningEnabled = false;
			bHasSetupDeepSeqMode = true;
		}

		if((*si).myType == PerspObjDeepSeq)
		{
			if(bIsLightningEnabled)
			{
				Window::disableLighting();
				bIsLightningEnabled = false;
			}

#ifdef DIRECTX_PIPELINE
			DXSTAGE2
#else
			if((*si).mySeq->getForceAlphaTest() == true)
				GraphicsUtils::enableAlphaTest();
#endif

#ifdef _DEBUG
			if(strcmp((*si).mySeq->getName(), "oTrees") == 0)
			{
				int bp = 0;
			}
#endif

//            glDisable ( GL_ALPHA_TEST ) ;
  //          glDisable ( GL_DEPTH_TEST ) ;

			// Attempt to see if there are more textures here that can be flushed.
			TSortedPerspInstances::iterator fi = si;
			fi++;
			while(fi != mySortedDeepPerspSequences.end() && (*fi).mySeq == (*si).mySeq && (*si).myVertStartIndex + 6 == (*fi).myVertStartIndex)
				fi++;

			fi--;
			if(fi == si)
				(*si).mySeq->flush((*si).myVertStartIndex);
			else
			{	
				_ASSERT(fi != mySortedDeepPerspSequences.end());
				(*si).mySeq->flushRangeInclusive((*si).myVertStartIndex, (*fi).myVertStartIndex);
			}
#ifdef DIRECTX_PIPELINE
			DXSTAGE2
#else
			if((*si).mySeq->getForceAlphaTest() == true)
				GraphicsUtils::disableAlphaTest();
#endif
			si = fi;

		}
		else if((*si).myType == PerspObjObject3D && iNumDepthOps <= 0)
		{
			if(!bIsLightningEnabled)
			{
				myParentWindow->allowLighting(true);
				bIsLightningEnabled = true;
			}

#ifdef DIRECTX_PIPELINE
			DXSTAGE2
#else
			glDepthMask(GL_TRUE);
			glEnable(GL_DEPTH_TEST);
#endif

			pObj3d = const_cast<SCachedObject3D*>(&((*si).myObj3DInfo));
#ifdef _DEBUG
			if(pObj3d->myObjId == "oStoneWall")
			{
				int bp = 0;
			}
#endif
			RenderUtils::renderObject3D(myParentWindow, pObj3d->myObjId.c_str(), pObj3d->myCenter.x, pObj3d->myCenter.y, pObj3d->myCenter.z, pObj3d->myAlpha, pObj3d->myScale, pObj3d->myAnimProgress, pObj3d->myClipIndex, pObj3d->myRotations, pObj3d->myOverrideTexture.c_str(), false);

#ifdef DIRECTX_PIPELINE
			DXSTAGE2
#else
			glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);
#endif
		}

#ifdef EXPERIM_ENABLE_RELATED_SEQ_SORTING
		if((*si).myRelatedSeqPtr && (*si).myRelatedSeqPtr->getNeedsFlushing())
		{
			myParentWindow->setupGlView(svScreenDims.x, svScreenDims.y);
			GraphicsUtils::transformForGlOrtho(svScreenDims.x, svScreenDims.y);

#ifdef DIRECTX_PIPELINE
			DXSTAGE1
#else
			glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);
#endif
			GraphicsUtils::disableAlphaTest();

			_ASSERT((*si).myRelatedSeqPtr->getMarkRelated());

			(*si).myRelatedSeqPtr->flush();

#ifdef RENDER_FIELD_WITH_PERSPECTIVE
			myParentWindow->setup3DGlView(false, true, false, false);
#else
			myParentWindow->setup3DGlView(false, false, false, false);
#endif
#ifdef DIRECTX_PIPELINE
			DXSTAGE1
#else
			glDisable(GL_DEPTH_TEST);
			glDepthMask(GL_FALSE);
#endif
			bIsLightningEnabled = false;
			bHasSetupDeepSeqMode = true;
		}
#endif
		//(*si).mySeq->flush(-1);
	}

	mySortedDeepPerspSequences.clear();



#else
	for(iStage = 0; iStage < MAX_DEEP_RENDER_STAGES; iStage++)
	{
		bool bHadNoAlpha = false;
		iNumSeqs = myFullDeepPerspSequences[iStage].size();
		for(iSeq = 0; iSeq < iNumSeqs; iSeq++)
		{
			if(!bHasSetupDeepSeqMode)
			{
				// TODO: Set the last alpha to false when we'll have sorting.
				myParentWindow->setup3DGlView(false, true, false, true);
				//glDisable ( GL_ALPHA_TEST ) ;
				bHasSetupDeepSeqMode = true;
			}

			if(myFullDeepPerspSequences[iStage][iSeq]->getDisableAlphaTest() == false)
				myFullDeepPerspSequences[iStage][iSeq]->flush();
			else
				bHadNoAlpha = true;
		}
		// Now render all those without alpha test
		if(bHadNoAlpha)
		{
			glDisable(GL_ALPHA_TEST) ;
			for(iSeq = 0; iSeq < iNumSeqs; iSeq++)
			{
				if(!bHasSetupDeepSeqMode)
				{
					myParentWindow->setup3DGlView(false, true, false, false);
					bHasSetupDeepSeqMode = true;
				}

				if(myFullDeepPerspSequences[iStage][iSeq]->getDisableAlphaTest() == true)
					myFullDeepPerspSequences[iStage][iSeq]->flush();
			}
			glEnable ( GL_ALPHA_TEST );
		}

		myFullDeepPerspSequences[iStage].clear();
	}
#endif

	if(bHasSetupDeepSeqMode)
	{
		myParentWindow->setupGlView(svScreenDims.x, svScreenDims.y);
		SVector2D svScreenDims;
		myParentWindow->getSize(svScreenDims);
		GraphicsUtils::transformForGlOrtho(svScreenDims.x, svScreenDims.y);
	}

	// Now, draw any rectangles we might have
	SCachedRectangle *pRect;
	int iRect, iNumRects = myRectangles.size();
	if(iNumRects > 0)
	{
		GraphicsUtils::enableFlatDrawingState(-1.0);
		myParentWindow->setBlendMode(BlendModeNormal);
		for(iRect = 0; iRect < iNumRects; iRect++)
		{
			pRect = &(myRectangles[iRect]);
			if(pRect->myLineThickness <= 0)
				RenderUtils::fillRectangle(pRect->myRect.x, pRect->myRect.y, pRect->myRect.w, pRect->myRect.h, pRect->myColor, false);
			else
				RenderUtils::drawRectangle(myParentWindow, pRect->myRect.x, pRect->myRect.y, pRect->myRect.w, pRect->myRect.h, pRect->myColor, pRect->myLineThickness, false);
		}
		myRectangles.clear();

		GraphicsUtils::disableFlatDrawingState(-1.0);
	}

	SCachedCustomObject2D *pCustObj2D;
	int iCust2DObj, iNum2DObjs = myCachedCustomObjects2D.size();
	if(iNum2DObjs > 0)
	{
		GraphicsUtils::enableFlatDrawingState(-1.0);
		myParentWindow->setBlendMode(BlendModeText);
		for(iCust2DObj = 0; iCust2DObj < iNum2DObjs; iCust2DObj++)
		{
			pCustObj2D = &(myCachedCustomObjects2D[iCust2DObj]);
			RenderUtils::renderCustomObject2D(myParentWindow, pCustObj2D->myVerts, pCustObj2D->myNumVerts, false);
		}
		myCachedCustomObjects2D.clear();

		GraphicsUtils::disableFlatDrawingState(-1.0);
	}


	// Gradient rects
	SCachedGradRectangle *pGradRect;
	int iGradRect, iNumGradRects = myGradRectangles.size();
	if(iNumGradRects > 0)
	{
		GraphicsUtils::enableFlatDrawingState(-1.0);
		myParentWindow->setBlendMode(BlendModeNormal);
		for(iGradRect = 0; iGradRect < iNumGradRects; iGradRect++)
		{
			pGradRect = &(myGradRectangles[iGradRect]);
			RenderUtils::renderGradientRectangle(pGradRect->myRect.x, pGradRect->myRect.y, pGradRect->myRect.w, pGradRect->myRect.h, pGradRect->myGradient);
		}
		myGradRectangles.clear();
		GraphicsUtils::disableFlatDrawingState(-1.0);
	}


	// Draw the arcs
	SCachedArc *pArc;
	int iArc, iNumArcs = myArcs.size();
	if(iNumArcs > 0)
	{
//		glEnableFlatDrawingState(-1.0);
//		myParentWindow->setBlendMode(BlendModeNormal);
		for(iArc = 0; iArc < iNumArcs; iArc++)
		{
			pArc = &(myArcs[iArc]);
			RenderUtils::renderArc(myParentWindow, pArc->myCenter.x, pArc->myCenter.y, pArc->myRadius, pArc->myThickness,
				pArc->myColor, pArc->myStartAngle, pArc->myEndAngle, pArc->myTexture.length() > 0 ? pArc->myTexture.c_str() : NULL, pArc->myTaper, pArc->myWrapTexture, true, pArc->myRadius2);
		}
		myArcs.clear();

//		glDisableFlatDrawingState(-1.0);
	}

	int iLine, iNumLines = myLines.size();
#ifdef _DEBUG
	// If we render too many lines per frame, certain cards/drivers
	// will begin to randomly render flickering ghost geometry.
	// Mostly notably the 870M on my Razer when the text tool is
	// invoked...
//	_ASSERT(iNumLines < 50);
#endif
	if(iNumLines > 0)
	{
		SCachedLine* pLine;
		GraphicsUtils::enableFlatDrawingState(-1.0);
		myParentWindow->setBlendMode(BlendModeNormal);
		myCachedPoints.clear();
		myCachedPoints.resize(2);
		for(iLine = 0; iLine < iNumLines; iLine++)
		{
			pLine = &(myLines[iLine]);
			myCachedPoints[0] = pLine->myStart;
			myCachedPoints[1] = pLine->myEnd;
			RenderUtils::drawLines(myParentWindow, myCachedPoints, pLine->myColor, pLine->myThickness, false);
		}
		myLines.clear();
		GraphicsUtils::disableFlatDrawingState(-1.0);
	}

	iNumLines = myCachedTexturedLines.size();
	if(iNumLines > 0)
	{

		STexturedLine *pTexturedLine;
		for(iLine = 0; iLine < iNumLines; iLine++)
		{
			pTexturedLine = &myCachedTexturedLines[iLine];
			RenderUtils::renderTexturedPath(myParentWindow, *pTexturedLine->myPoints, pTexturedLine->myThickness,
				pTexturedLine->myTextureName.c_str(), pTexturedLine->myOpacity, pTexturedLine->myStretchTexture,
				pTexturedLine->myMiter, pTexturedLine->myMaxKinkMult,
				pTexturedLine->myFillColor.r == FLOAT_TYPE_MAX ? NULL : &pTexturedLine->myFillColor,
				pTexturedLine->myTextureLengthMult, pTexturedLine->myInitUOffset);
		}
		myCachedTexturedLines.clear();
	}
#ifdef ALLOW_3D
	// Draw 3D objects
	int iNumCustom3D = myCachedCustomObjects3D.size();
	int iNum3d = myCachedObjects3D.size();
	int iNumPersp3d = myCachedPerspObjects3D.size();

	bool bNeedToResetViewFinal = false;

	if(iNum3d > 0 || iNumCustom3D > 0)
	{
		myParentWindow->setup3DGlView(true, false, false, false);

		SCachedObject3D *pObj3d = NULL;
		for(iObj3d = 0; iObj3d < iNum3d; iObj3d++)
		{
			pObj3d = &myCachedObjects3D[iObj3d];
			RenderUtils::renderObject3D(myParentWindow, pObj3d->myObjId.c_str(), pObj3d->myCenter.x, pObj3d->myCenter.y, pObj3d->myCenter.z, pObj3d->myAlpha, pObj3d->myScale, pObj3d->myAnimProgress, pObj3d->myClipIndex, pObj3d->myRotations, pObj3d->myOverrideTexture.c_str(), false);
		}
		myCachedObjects3D.clear();

		SCachedCustomObject3D *pObjCustom3d;
		for(iObj3d = 0; iObj3d < iNumCustom3D; iObj3d++)
		{
			pObjCustom3d = &myCachedCustomObjects3D[iObj3d];
			RenderUtils::renderCustomObject3D(myParentWindow, pObjCustom3d->myTextureSeq, pObjCustom3d->myCenter.x, pObjCustom3d->myCenter.y, pObjCustom3d->myElevation, pObjCustom3d->myAlpha, pObjCustom3d->myScale,
				pObjCustom3d->myVertexData, pObjCustom3d->myNumVerts, pObj3d->myRotations, pObjCustom3d->myDiffuseMults, pObjCustom3d->myAmbMults);
		}
		myCachedCustomObjects3D.clear();

		bNeedToResetViewFinal = true;
//		GameEngine::setup2DGlView();
	}


#if defined(EXPERIMENTAL_SORT_PERSP_SEQ_BY_DEPTH) || defined(EXPERIMENTAL_SORT_ORTHO_SEQ_BY_DEPTH)
	if(iNumDepthOps > 0) //  || iNumPerspArcs3D > 0)
#else
	if(iNumPersp3d > 0 || iNumPerspArcs3D > 0)
#endif
	{
		bool bDisableDepthClearing = false;
		if(iNumDepthOps > 0)
		{

			if(bNeedToResetViewFinal)
			{
				myParentWindow->setupGlView(svScreenDims.x, svScreenDims.y);
				SVector2D svScreenDims;
				myParentWindow->getSize(svScreenDims);
				GraphicsUtils::transformForGlOrtho(svScreenDims.x, svScreenDims.y);
			}

//			glDisableClientState(GL_NORMAL_ARRAY);
//			glEnableClientState(GL_COLOR_ARRAY);
#ifdef DIRECTX_PIPELINE
			DXSTAGE3
#else

			gClearDepth(1.0f);
			glEnable(GL_DEPTH_TEST);
			glDepthMask(GL_TRUE);

			glClear(GL_DEPTH_BUFFER_BIT);
			GraphicsUtils::disableAlphaTest();


			glDepthFunc(GL_ALWAYS);


			// Apply depth ops
			SCachedRectangle *pDepthOp;
			SColor scolDummy(1,1,1,0.0);
			for(iDepthOp = 0; iDepthOp < iNumDepthOps; iDepthOp++)
			{
				pDepthOp = &myDepthBufferOps[iDepthOp];
				RenderUtils::fillRectangleAtDepth(pDepthOp->myRect.x, pDepthOp->myRect.y, pDepthOp->myRect.w, pDepthOp->myRect.h, scolDummy, pDepthOp->myLineThickness, true);
			}


			myDepthBufferOps.clear();
			glDepthMask(GL_FALSE);

			glDepthFunc(GL_LEQUAL);

#endif

//			glEnableClientState(GL_NORMAL_ARRAY);
//			glDisableClientState(GL_COLOR_ARRAY);


			bDisableDepthClearing = true;
		}


		myParentWindow->setup3DGlView(true, true, bDisableDepthClearing, false);

#if defined(EXPERIMENTAL_SORT_PERSP_SEQ_BY_DEPTH) || defined(EXPERIMENTAL_SORT_ORTHO_SEQ_BY_DEPTH)
		if(iNumDepthOps > 0)
#endif
		{


			SCachedObject3D *pObj3d;
			for(iObj3d = 0; iObj3d < iNumPersp3d; iObj3d++)
			{
				pObj3d = &myCachedPerspObjects3D[iObj3d];
				RenderUtils::renderObject3D(myParentWindow, pObj3d->myObjId.c_str(), pObj3d->myCenter.x, pObj3d->myCenter.y, pObj3d->myCenter.z, pObj3d->myAlpha, pObj3d->myScale, pObj3d->myAnimProgress, pObj3d->myClipIndex, pObj3d->myRotations, pObj3d->myOverrideTexture.c_str(), false);
			}
		}

#if !defined(EXPERIMENTAL_SORT_PERSP_SEQ_BY_DEPTH) && !defined(EXPERIMENTAL_SORT_ORTHO_SEQ_BY_DEPTH)
		// Draw any arcs we might have
		SCachedArc3D *pArc3D;
		for(iObj3d = 0; iObj3d < iNumPerspArcs3D; iObj3d++)
		{
			if(iObj3d == 0)
			{
#ifdef DIRECTX_PIPELINE
				DXSTAGE2
#else
				glDisable(GL_DEPTH_TEST);
				glAlphaFunc ( GL_GREATER, ARC_ALPHA_CUTOFF_VALUE_3D ) ;
				glEnable(GL_ALPHA_TEST);
//				glEnableClientState(GL_COLOR_ARRAY);
#endif
			}
			pArc3D = &myArcs3D[iObj3d];
			RenderUtils::renderArc3D(pArc3D->myCenter.x, pArc3D->myCenter.y, pArc3D->myCenter.z, pArc3D->myRadius, pArc3D->myThickness, pArc3D->myColor, pArc3D->myStartAngle, pArc3D->myEndAngle, pArc3D->myTexture.c_str(), pArc3D->myTaper, pArc3D->myWrapTexture, false, pArc3D->myFlatOnGround);
		}
		if(iNumPerspArcs3D > 0)
		{
#ifdef DIRECTX_PIPELINE
			DXSTAGE3
#else
			glEnable(GL_DEPTH_TEST);
			glAlphaFunc ( GL_GREATER, ALPHA_CUTOFF_VALUE_3D ) ;
			glDisable(GL_ALPHA_TEST);
//			glDisableClientState(GL_COLOR_ARRAY);
#endif
		}
		myArcs3D.clear();
#endif

		bNeedToResetViewFinal = true;
	}

	myCachedPerspObjects3D.clear();

	if(bNeedToResetViewFinal)
	{
		myParentWindow->setupGlView(svScreenDims.x, svScreenDims.y);
		SVector2D svScreenDims;
		myParentWindow->getSize(svScreenDims);
		GraphicsUtils::transformForGlOrtho(svScreenDims.x, svScreenDims.y);
	}

#endif

	// Finally, draw the text objects
	if(myFlushText)
	{
#if !defined(WIN32) && !defined(MAC_BUILD)
//#ifndef WIN32
#ifdef USE_OPENGL2
		SVector2D svScreenDims;
		GameEngine::getScreenDims(svScreenDims);
		RenderStateManager::getInstance()->resetForCachedText(svScreenDims.x, svScreenDims.y);
#else
		glPushMatrix();
		glLoadIdentity();
#endif

#endif
		TCachedTextObjectCacher* pTextObjectCacher = myParentWindow->getTextObjectCacher();

		bool bNeedReflushing = false;
		SCachedTextObject* pTextObj;
		int iText, iNumTexts = myTextObjects.size();
		for(iText = 0; iText < iNumTexts; iText++)
		{
			pTextObj = myTextObjects[iText];
			bNeedReflushing |= this->drawCachedText(pTextObj);
		}
		for(iText = 0; iText < iNumTexts; iText++)
		{
			// Tell the cacher that we're done with this object
			pTextObjectCacher->deleteObject(myTextObjects[iText]);
		}
		myTextObjects.clear();

#ifdef DIRECTX_PIPELINE
		g_pDxRenderer->setCachedRenderingTextMode(false);
#endif

#if !defined(WIN32) && !defined(MAC_BUILD)
//#ifndef WIN32
#ifndef USE_OPENGL2
		glPopMatrix();
#else
		RenderStateManager::getInstance()->resetForOrtho(svScreenDims.x, svScreenDims.y, ORTHO_DEPTH_LIMIT);
#endif
#endif
		// Because some text may not be cachable (such as small nubmers)
		// we may need to reflush again, or the buffer may be cleared by
		// something and we'll end up never drawing it...
		if(bNeedReflushing)
			flush();
	}
	/*
#ifndef WIN32
	int iObj, iNumObjs = myCachedGameViewObjects.size();
	if(iNumObjs > 0)
	{
		myParentWindow->setBlendMode(BlendModeText);

		for(iObj = 0; iObj < iNumObjs; iObj++)
			gDrawSpecialObject(myCachedGameViewObjects[iObj]);
		///		[g_pMainView drawSpecialObject:myCachedGameViewObjects[iObj]];
		myCachedGameViewObjects.clear();
		myParentWindow->setBlendMode(BlendModeNormal);
	}
#endif
	 */
}
/***********************************************************************************************************/
void DrawingCache::addText(const char* pcsText, const char* pcsFontName, int iFontSize, FLOAT_TYPE fX, FLOAT_TYPE fY,
						   const SColor &scolText, HorAlignType eHorAlign, VertAlignType eVertAlign, FLOAT_TYPE fExtraKerning,
						   SVector2D* svStringMeasurementsOut, const SColor* scolShadow, SVector2D* svShadowOffset, FLOAT_TYPE fW,
						   FLOAT_TYPE fScale, bool bResampleNearest, SColor* scolHighlightCol, BlendModeType eBlendMode,
						   bool bForceDrawingIfNotCached)
{
#if defined(_DEBUG) && !defined(USE_FREETYPE_FONTS)
	_ASSERT(g_pMainEngine->findResourceItem(ResourceRasterFonts, pcsFontName));
#endif

#ifdef _DEBUG
	if(IS_STRING_EQUAL(pcsText, "px"))
	{
		int bp = 0;
	}
#endif

	// SCachedTextObject rDummy;
    SCachedTextObject *pDummy = myParentWindow->getTextObjectCacher()->getNewObject();
	pDummy->myCenter.set(fX, fY);
	pDummy->myColor = scolText;
	if(scolHighlightCol)
		pDummy->myBaseInfo.myHighlightColor = *scolHighlightCol;
	else
		pDummy->myBaseInfo.myHighlightColor = scolText;

	pDummy->myBlendMode =  eBlendMode;
	pDummy->myText = pcsText;
	pDummy->myHorAlign = eHorAlign;
	pDummy->myVertAlign = eVertAlign;
	pDummy->myScale = fScale;
	pDummy->myBaseInfo.myResampleNearest = bResampleNearest;
	pDummy->myBaseInfo.myFont = pcsFontName;
	pDummy->myBaseInfo.myFontSize = iFontSize;
	pDummy->myBaseInfo.myWidth = fW;
	pDummy->myBaseInfo.myExtraKerning = fExtraKerning;
	pDummy->myDidRenderDuringStencil = RenderUtils::isInsideStencil();
	pDummy->myForceDrawingIfUncached = bForceDrawingIfNotCached;
	if(scolShadow && svShadowOffset && ((int)svShadowOffset->x != 0 || (int)svShadowOffset->y != 0))
	{
		pDummy->myShadowColor = *scolShadow;
		pDummy->myShadowOffset = *svShadowOffset;
		pDummy->myHasShadow = true;
	}
	else
		pDummy->myHasShadow = false;

	myTextObjects.push_back(pDummy);
}
/***********************************************************************************************************/
#ifdef USE_FREETYPE_FONTS
void DrawingCache::addCachedLetter(TX_MAN_RETURN_TYPE rTextureIndex, const SVector2D& svOrigin, const SVector2D& svSize, const SColor& scolColor)
{
	SCachedLetterInfo rInfo;
	rInfo.myTextureIndex = rTextureIndex;
	rInfo.myOrigin = svOrigin;
	rInfo.mySize = svSize;
	rInfo.myColor = scolColor;
	myLetterObjects.push_back(rInfo);
}
#endif
/***********************************************************************************************************/
void DrawingCache::addOrMeasureText(const char* pcsText, const char* pcsFontName, int iFontSize, FLOAT_TYPE fX, FLOAT_TYPE fY, SColor &scolText,
						   HorAlignType eHorAlign, VertAlignType eVertAlign, FLOAT_TYPE fExtraKerning, SVector2D* svStringMeasurementsOut, 
						   SColor* scolShadow, SVector2D* svShadowOffset,
						   FLOAT_TYPE fW, bool bForceDrawUncached, SColor* scolHighlightCol, FLOAT_TYPE fScale, bool bDrawNowIfForced)
{
	if(svShadowOffset && svShadowOffset->x == 0 && svShadowOffset->y == 0)
		svShadowOffset = NULL;

	if(!bForceDrawUncached)
	{

		this->addText(pcsText, pcsFontName, iFontSize, fX, fY, scolText,
			eHorAlign, eVertAlign, fExtraKerning, svStringMeasurementsOut,
			scolShadow, svShadowOffset, fW, fScale,
			true, scolHighlightCol);
		// Don't return, since otherwise we're going to blink.
		if(!bDrawNowIfForced)
			return;
	}

	RenderUtils::drawOrMeasureText(this, pcsText, pcsFontName, iFontSize, fX, fY, scolText, eHorAlign, eVertAlign, fExtraKerning, svStringMeasurementsOut, scolShadow, svShadowOffset, fW, scolHighlightCol, fScale);

#if 0

// TODO: The code below has been moved to render utils. Remove it.
	// Find the right sequence
	ResourceCollection* pFonts = ResourceManager::getInstance()->getCollection(ResourceRasterFonts);
	ResourceItem* pItem = pFonts->findItemByType(pcsFontName);
	if(!pItem)
	{
		// Could not find the right font
		_ASSERT(0);
		return;
	}

	if(fProgress > 1.0)
		fProgress = 1.0;

	SVector2D svAlignmentOffsets(0,0);
	if(svStringMeasurementsOut)
	{
		svStringMeasurementsOut->set(0,0);
	}

	if(!svStringMeasurementsOut && (eHorAlign != HorAlignLeft || eVertAlign != VertAlignTop))
	{
		// Not the default alignment. Measure it.
		SVector2D svTextSize;
		this->addOrMeasureText(pcsText, pcsFontName, fX, fY, scolText, eHorAlign, eVertAlign, fExtraKerning, &svTextSize, scolShadow,
			svShadowOffset, fW, true);

		svTextSize *= fScale;

		// See what the offset is
		if(eHorAlign == HorAlignRight)
			svAlignmentOffsets.x = -svTextSize.x;
		else if(eHorAlign == HorAlignCenter)
			svAlignmentOffsets.x = -svTextSize.x/2.0;
		else
			svAlignmentOffsets.x = 0.0;

		if(eVertAlign == VertAlignBottom)
			svAlignmentOffsets.y = -svTextSize.y;
		else if(eVertAlign == VertAlignCenter)
			svAlignmentOffsets.y = -svTextSize.y/2.0;
		else
			svAlignmentOffsets.y = 0.0;
	}

	FLOAT_TYPE fCharWidth, fCharUVWidth;
	SExtraSeqDrawInfo rExtraInfo, rExtraShadowInfo;
	myCachedString2 = pItem->getStringProp(PropertyFontChars);
	int iCurr, iNum;
	char cChar;
	int iCharIdx;

	FLOAT_TYPE fSpaceWidth = pItem->getNumericEnumPropValue(PropertyFontCharWidths, myCachedString2.find('a'))*fScale;
	FLOAT_TYPE fLineUVWidth = pItem->getNumProp(PropertyFontUVHeight);
	FLOAT_TYPE fLinePixHeight = pItem->getNumProp(PropertyFontHeight)*fScale;
	FLOAT_TYPE fKerning = pItem->getNumProp(PropertyFontKerning)*fScale;
	FLOAT_TYPE fExraLineSpacing = pItem->getNumProp(PropertyFontExtraLineSpacing)*fScale;
	fKerning += fExtraKerning*fScale;


	// We're  forced to insert newlines into text here if we're passed in a max width
	if(fW > 0.0)
	{
		iNum = strlen(pcsText);

		char* pcsEvil = const_cast<char*>(pcsText);

		//pcsEvil[2] = 't';


		FLOAT_TYPE fCurrXPos = 0;
//		myCachedString = pcsText;
		int iPrevSpacePosition = -1;
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
			cChar = pcsText[iCurr];
			if(cChar == '\n' || cChar == '^')
			{
				fCurrXPos = 0;
				iPrevSpacePosition = -1;
				continue;
			}

			if(fCurrXPos > fW*fScale)
			{
				// We need to replace the previous space with a newline.

				// We need to insert a newline *before* the previous
				// character, since it already exceeded the limit.
				// And possibly step back.
				// And we need to do it
				fCurrXPos = 0;
				if(iPrevSpacePosition >= 0)
				{
					pcsEvil[iPrevSpacePosition] = '\n';
					iCurr = iPrevSpacePosition;
				}
				iPrevSpacePosition = -1;
				continue;
//				myCachedString.insert(iCurr - 1, "\n");
			}

			if(cChar == ' ')
				iPrevSpacePosition = iCurr;

			// Find it in the string
			iCharIdx = myCachedString2.find(cChar);

			if(iCharIdx < 0)
			{
				// Treat as space, skip.
				fCharWidth = fSpaceWidth;
				fCurrXPos += fCharWidth/2.0;
			}
			else
			{
				// Get all the metrics, print it:
				fCharWidth = pItem->getNumericEnumPropValue(PropertyFontCharWidths, iCharIdx)*fScale;
				fCurrXPos += fCharWidth/2.0;
			}
			fCurrXPos += fCharWidth/2.0 + fKerning;
		}


///		pcsText = myCachedString.c_str();
	}

	// Now, go letter by letter, computing the right coordinates on the spot
	iNum = strlen(pcsText);

	int iLastNewlinePos = -1;
	int iLineLen;
	for(iCurr = 0; iCurr <= iNum; iCurr++)
	{
		if(pcsText[iCurr] == '\n' || pcsText[iCurr] == '^' || (iLastNewlinePos >= 0 && pcsText[iCurr] == 0))
		{
			// Newline!
			if(iLastNewlinePos < 0)
				theTextLineLengths.clear();

			iLineLen = iCurr - iLastNewlinePos - 1;
			theTextLineLengths.push_back(iLineLen);
			iLastNewlinePos = iCurr;
		}
	}

	int iNextStopDrawingPos = iNum;
	if(iLastNewlinePos < 0)
	{
		iNum = ((FLOAT_TYPE)iNum)*fProgress;
		iNextStopDrawingPos = iNum;
	}
	else
		iNextStopDrawingPos = ((FLOAT_TYPE)theTextLineLengths[0])*fProgress;

	const char* pcsFontAnim = pItem->getStringProp(PropertyObjAnim);
	CachedSequence* pFontSeq = this->getCachedSequence(pcsFontAnim, NULL);

	int iFontFileW, iFontFileH;
	getImageSize(pcsFontAnim, iFontFileW, iFontFileH);
	FLOAT_TYPE fSingleUPixel = 0.0;
	if(iFontFileW)
		fSingleUPixel = EXTRA_FONT_PIXEL_PADDING/(FLOAT_TYPE)iFontFileW;
//	FLOAT_TYPE fSingleUPixel = 0.0;


	// pItem->getNumericEnumPropValue(PropertyFontCharWidths, );

	// If we can do progress for lines, we can do the drawing in the same loop.
	// Scan through the string once, tracking line lengths?



	rExtraInfo.myPolyColor = scolText;
	rExtraInfo.myH = fLinePixHeight;

	if(scolShadow && svShadowOffset)
	{
		rExtraShadowInfo = rExtraInfo;
		rExtraShadowInfo.myPolyColor = *scolShadow;
	}

	SUVSet rUVSet;
	FLOAT_TYPE iX = fX + svAlignmentOffsets.x, iY = fY + fLinePixHeight/2 + svAlignmentOffsets.y;
	int iCurrLineCount = 0;
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		cChar = pcsText[iCurr];

		if(cChar == '\n' || cChar == '^')
		{
			if(svStringMeasurementsOut && iX > svStringMeasurementsOut->x)
				svStringMeasurementsOut->x = iX;

			iCurrLineCount++;
			iNextStopDrawingPos = iCurr + ((FLOAT_TYPE)theTextLineLengths[iCurrLineCount])*fProgress + 1;
			iX = fX + svAlignmentOffsets.x;
			iY += fLinePixHeight + fExraLineSpacing;
			continue;
		}
		else if(iCurr >= iNextStopDrawingPos)
			continue;

		// Find it in the string
		iCharIdx = myCachedString2.find(cChar);

		if(iCharIdx < 0)
		{
			// Treat as space, skip.
			fCharWidth = fSpaceWidth;
			iX += fCharWidth/2.0;
		}
		else
		{
			// Get all the metrics, print it:
			fCharWidth = pItem->getNumericEnumPropValue(PropertyFontCharWidths, iCharIdx)*fScale;
			fCharUVWidth = pItem->getNumericEnumPropValue(PropertyFontCharUVWidths, iCharIdx);

			iX += fCharWidth/2.0;

			rUVSet.myStartX = pItem->getNumericEnumPropValue(PropertyFontCharXs, iCharIdx) - fSingleUPixel;
			rUVSet.myStartY = pItem->getNumericEnumPropValue(PropertyFontCharYs, iCharIdx);
			rUVSet.myEndX = rUVSet.myStartX + fCharUVWidth + fSingleUPixel*2.0;
			rUVSet.myEndY = rUVSet.myStartY + fLineUVWidth;
			rUVSet.myApplyToSize = true;

			rExtraInfo.myW = fCharWidth + (int)(EXTRA_FONT_PIXEL_PADDING*2.0);

			if(!svStringMeasurementsOut)
			{
				// Render the shadow if any
				if(svShadowOffset && scolShadow)
				{
					rExtraShadowInfo.myW = rExtraInfo.myW;
					pFontSeq->addSprite(iX + svShadowOffset->x, iY + svShadowOffset->y,
										scolShadow->alpha, // Alpha
										0.0, fScale, fScale,
										1.0, //  anim progress
										true, true, &rUVSet, false, &rExtraShadowInfo);
				}

				pFontSeq->addSprite(iX, iY,
					scolText.alpha, // Alpha
					0.0, fScale, fScale,
					1.0, //  anim progress
					true, true, &rUVSet, false, &rExtraInfo);
			}
		}


		iX += fCharWidth/2.0 + fKerning;

	}

	if(svStringMeasurementsOut)
	{
		if(iX > svStringMeasurementsOut->x)
			svStringMeasurementsOut->x = iX;

		svStringMeasurementsOut->x -= fX;
		// Line spacing does not apply to the first line.
		svStringMeasurementsOut->y = (fLinePixHeight + fExraLineSpacing)*(iCurrLineCount) + fLinePixHeight;

		// Increase by the shadow distance
		if(svShadowOffset && scolShadow)
		{
			svStringMeasurementsOut->x += svShadowOffset->x;
			svStringMeasurementsOut->y += svShadowOffset->y;
		}
	}
#endif
}
/***********************************************************************************************************/
void DrawingCache::getImageSize(const char *pcsType, int& iW, int &iH)
{
	SUVSet dummy;
	BlendModeType eBlendMode;
	getTextureManager()->getTextureIndex(pcsType, 0, iW, iH, dummy, eBlendMode);
}
/***********************************************************************************************************/
CachedSequence* DrawingCache::getCachedSequence(const char* pcsType, int* iAtlasFrameOut)
{
	// See if this is a reference to a specific frame. Assumes one or two digits.
	// Eeeeeevil. But fast.
	int iKilledPos = -1;
	char* pcsActualType = const_cast<char*>(pcsType);

	if(iAtlasFrameOut)
		iKilledPos = StringUtils::findFrameNum(pcsActualType, *iAtlasFrameOut);

	// Try to find an entry:
/*
	CachedSequence* pSeq = myCacher->getCached(pcsActualType);
	if(!pSeq)
	{
		TStringCachedSequenceMap::iterator mi;
		myCachedString = pcsActualType;
		mi = mySequences.find(myCachedString);
		if(mi != mySequences.end())
		{
			pSeq = mi->second;
			myCacher->addToCache(pcsActualType, pSeq);
		}
	}
*/

	CachedSequence** pFindRes = mySequences.findSimple(pcsActualType);
	CachedSequence* pSeq = pFindRes ? *pFindRes : NULL;
	if(!pSeq)
	{
		// Insert a new entry and set its type:
		pSeq = new CachedSequence(this);
		///myCachedString = pcsActualType;
		///mySequences[myCachedString] = pSeq;
		///myCacher->addToCache(pcsActualType, pSeq);
		mySequences.insert(pcsActualType, pSeq);
		myAllSequences.push_back(pSeq);

		pSeq->setAnimType(pcsActualType);
	}

	if(iKilledPos >= 0)
	{
		pcsActualType[iKilledPos] = ':';
	}

	return pSeq;
}
/***********************************************************************************************************/
CachedDeepSequence* DrawingCache::getCachedDeepSequence(const char* pcsType, int* iAtlasFrameOut, bool bPerspective)
{
	// See if this is a reference to a specific frame. Assumes one or two digits.
	// Eeeeeevil. But fast.
	int iKilledPos = -1;
	char* pcsActualType = const_cast<char*>(pcsType);

	if(iAtlasFrameOut)
		iKilledPos = StringUtils::findFrameNum(pcsActualType, *iAtlasFrameOut);

	// Try to find an entry:
	CachedDeepSequence* pSeq = NULL;

	if(bPerspective)
		pSeq = myDeepPerspCacher->getCached(pcsActualType);
	else
		pSeq = myDeepCacher->getCached(pcsActualType);

	if(!pSeq)
	{
		TStringCachedDeepSequenceMap::iterator end_it;
		TStringCachedDeepSequenceMap::iterator mi;
		myCachedString = pcsActualType;
		if(bPerspective)
		{
			mi = myDeepPerspSequences.find(myCachedString);
			end_it = myDeepPerspSequences.end();
		}
		else
		{
			mi = myDeepSequences.find(myCachedString);
			end_it = myDeepSequences.end();
		}
		if(mi != end_it)
		{
			pSeq = mi->second;
			if(bPerspective)
				myDeepPerspCacher->addToCache(pcsActualType, pSeq);
			else
				myDeepCacher->addToCache(pcsActualType, pSeq);
		}
	}

	if(!pSeq)
	{
		// Insert a new entry and set its type:
		pSeq = new CachedDeepSequence(this, bPerspective);
		myCachedString = pcsActualType;
		if(bPerspective)
		{
			myDeepPerspSequences[myCachedString] = pSeq;
			myDeepPerspCacher->addToCache(pcsActualType, pSeq);
		}
		else
		{
			myDeepSequences[myCachedString] = pSeq;
			myDeepCacher->addToCache(pcsActualType, pSeq);
		}

		pSeq->setAnimType(pcsActualType);
	}

	if(iKilledPos >= 0)
	{
		pcsActualType[iKilledPos] = ':';
	}

	return pSeq;
}
/***********************************************************************************************************/
void DrawingCache::addPathTrail(PathTrail* pTrail, const SVector2D& svScroll, FLOAT_TYPE fOpacity, FLOAT_TYPE fScale, CachedSequence* pcsSampledAnimSeq, bool bAnimateFrame)
{
	if(!pTrail->getIsEnabled())
		return;

	SCachedTrail rTrail;
	rTrail.mySampledAnimSeq = pcsSampledAnimSeq;
	rTrail.myTrail = pTrail;
	rTrail.myScroll = svScroll;
	rTrail.myAnimateFrame = bAnimateFrame;
	rTrail.myOpacity = fOpacity;
	rTrail.myScale = fScale;
	myTrails.push_back(rTrail);
}
/***********************************************************************************************************/
void DrawingCache::addDepthBufferOp(FLOAT_TYPE fXStart, FLOAT_TYPE fYStart, FLOAT_TYPE fW, FLOAT_TYPE fH, FLOAT_TYPE fDepthValue)
{
	SCachedRectangle rOp;
	rOp.myRect.x = fXStart;
	rOp.myRect.y = fYStart;
	rOp.myRect.w = fW;
	rOp.myRect.h = fH;
	rOp.myLineThickness = fDepthValue;

	myDepthBufferOps.push_back(rOp);
}
/***********************************************************************************************************/
void DrawingCache::addTexturedLineCollection(PolylineCollection& rColl, const char* pcsTexture, FLOAT_TYPE fAlpha, FLOAT_TYPE fThickness, bool bStretchTexture, bool bMiter, FLOAT_TYPE fMaxKinkMult, FLOAT_TYPE fTextureLengthMult, FLOAT_TYPE fInitUOffset)
{
	int iCurr, iNum = rColl.getNumLines();
	for(iCurr = 0; iCurr < iNum; iCurr++)
		addTexturedLine(rColl.getLine(iCurr), pcsTexture, fAlpha, fThickness, bStretchTexture, bMiter, fMaxKinkMult, fTextureLengthMult, fInitUOffset);
}
/***********************************************************************************************************/
void DrawingCache::addTexturedLine(TPointVector* pPoints, const char* pcsTexture, FLOAT_TYPE fAlpha, FLOAT_TYPE fThickness, bool bStretchTexture, bool bMiter, FLOAT_TYPE fMaxKinkMult, FLOAT_TYPE fTextureLengthMult, FLOAT_TYPE fInitUOffset, const SColor* pOptFillColor)
{
	STexturedLine rLine;
	rLine.myPoints = pPoints;
	rLine.myThickness = fThickness;
	rLine.myTextureName = pcsTexture;
	rLine.myOpacity = fAlpha;
	rLine.myMaxKinkMult = fMaxKinkMult;
	rLine.myStretchTexture = bStretchTexture;
	rLine.myMiter = bMiter;
	rLine.myTextureLengthMult = fTextureLengthMult;
	rLine.myInitUOffset = fInitUOffset;

	if(pOptFillColor)
		rLine.myFillColor = *pOptFillColor;
	else
		rLine.myFillColor.r = FLOAT_TYPE_MAX;

	myCachedTexturedLines.push_back(rLine);
}
/***********************************************************************************************************/
bool DrawingCache::addRectangle(const SRect2D& srRect, const SColor& rColor, FLOAT_TYPE fLineThickness)
{
	return addRectangle(srRect.x, srRect.y, srRect.w, srRect.h, rColor, fLineThickness);
}
/***********************************************************************************************************/
void DrawingCache::addGradientRectangle(const SRect2D& srRect, const SGradient& sGrad)
{
	SCachedGradRectangle rRect;
	rRect.myRect = srRect;
	rRect.myGradient = sGrad;
	myGradRectangles.push_back(rRect);
}
/***********************************************************************************************************/
bool DrawingCache::addRectangle(FLOAT_TYPE fStartX, FLOAT_TYPE fStartY, FLOAT_TYPE fW, FLOAT_TYPE fH, const SColor& rColor, FLOAT_TYPE fLineThickness)
{
	// NOTE: On some NVidia cards, excessive numbers of rectangles on the screen cause random flickering.
	// This happens primarily when drawing backgrounds for image cells in tables & grids. So
	// we test to see if we're in scissors rect, and if we are, we whether we need rendering at all.
	// Now, test for the scissor rect:
	SCachedRectangle rRect;
	rRect.myRect.x = fStartX;
	rRect.myRect.y = fStartY;
	rRect.myRect.w = fW;
	rRect.myRect.h = fH;

	if(!RenderUtils::doesIntersectScissorRect(rRect.myRect))
		return false;

	rRect.myColor = rColor;
	rRect.myLineThickness = fLineThickness;
	myRectangles.push_back(rRect);
	return true;
}
/***********************************************************************************************************
void DrawingCache::addEllipse(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fRadius, FLOAT_TYPE fThickness, SColor& scolFill, FLOAT_TYPE fSolidDashAngle, FLOAT_TYPE fEmptyDashAngle, const char* pcsTexture)
{
	FINISH - ENSURE the geometr and renderer konw about 0 angles.
}
/***********************************************************************************************************
void DrawingCache::addCircularProgress(FLOAT_TYPE fProgress, const char* pcsTexture, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fRadius, FLOAT_TYPE fOpacity, FLOAT_TYPE fStartAngle = 90.0, FLOAT_TYPE fAngleSpan = 360.0, bool bPremultiplyAlpha = false, ::OrientationType eOrientation = OrientationCW)
{

}
/***********************************************************************************************************/
void DrawingCache::addArc3D(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fZ, FLOAT_TYPE fRadius, FLOAT_TYPE fThickness, SColor& scolFill, FLOAT_TYPE fStartAngle, FLOAT_TYPE fEndAngle, const char* pcsTexture, FLOAT_TYPE fTaper, bool bWrapTexture, bool bIsPerspective, bool bFlatOnGround)
{
	if(bIsPerspective)
		fY *= -1.0;

	SCachedArc3D rArc;
	rArc.myCenter.set(fX, fY, fZ);
	rArc.myColor = scolFill;
	rArc.myWrapTexture = bWrapTexture;
	rArc.myFlatOnGround = bFlatOnGround;
	rArc.myStartAngle = fStartAngle;
	rArc.myEndAngle = fEndAngle;
	rArc.myTaper = fTaper;
	rArc.myRadius = fRadius;
	rArc.myThickness = fThickness;
	rArc.myIsPerspective = bIsPerspective;
	if(pcsTexture)
		rArc.myTexture = pcsTexture;
	myArcs3D.push_back(rArc);

	// Non-perspective ones not rendered yet when flushing.
	_ASSERT(bIsPerspective);

}
/***********************************************************************************************************/
void DrawingCache::addArc(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fRadius, FLOAT_TYPE fThickness, const SColor& scolFill, FLOAT_TYPE fStartAngle, FLOAT_TYPE fEndAngle, const char* pcsTexture, FLOAT_TYPE fTaper, bool bWrapTexture, FLOAT_TYPE fRadius2)
{
	SCachedArc rArc;
	rArc.myCenter.set(fX, fY);
	rArc.myColor = scolFill;
	rArc.myWrapTexture = bWrapTexture;
	rArc.myStartAngle = fStartAngle;
	rArc.myEndAngle = fEndAngle;
	rArc.myTaper = fTaper;
	rArc.myRadius = fRadius;
	rArc.myRadius2 = fRadius2;
	rArc.myThickness = fThickness;
	if(pcsTexture)
		rArc.myTexture = pcsTexture;
	myArcs.push_back(rArc);
}
/***********************************************************************************************************/
void DrawingCache::addLine(FLOAT_TYPE fStartX, FLOAT_TYPE fStartY, FLOAT_TYPE fEndX, FLOAT_TYPE fEndY, const SColor& col, FLOAT_TYPE fThickness)
{
	SCachedLine rLine;
	rLine.myStart.set(fStartX, fStartY);
	rLine.myEnd.set(fEndX, fEndY);
	rLine.myColor = col;
	rLine.myThickness = fThickness;
	myLines.push_back(rLine);
}
/***********************************************************************************************************/
void DrawingCache::addDeepSprite(const char *pcsType, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE  fScale, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha, FLOAT_TYPE fZDepth, bool bPerspective)
{
	addDeepSprite(pcsType, fX, fY, fAlpha, fDegAngle, fScale, fScale, fAnimProgress, bPremultiplyAlpha, NULL, false, fZDepth, bPerspective);
}
void DrawingCache::addDeepSprite(const char *pcsType, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha, SUVSet* pUVSet, bool bIsProgressAFrameNum, FLOAT_TYPE fZDepth, bool bPerspective)
{
	if(fAlpha < FLOAT_EPSILON)
		return;

	int iAtlasFrame;
	CachedDeepSequence* pSeq = getCachedDeepSequence(pcsType, &iAtlasFrame, bPerspective);
	if(iAtlasFrame >= 0)
		fAnimProgress = iAtlasFrame;
	pSeq->addSprite(fX, fY, fAlpha, fDegAngle, fScaleX, fScaleY, fAnimProgress, bPremultiplyAlpha, true, pUVSet, (iAtlasFrame >= 0 || bIsProgressAFrameNum), NULL, fZDepth);
}
/***********************************************************************************************************/
void DrawingCache::addSprite(const char *pcsType, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE  fScale, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha, const SColor* pExtraColor)
{
	addSprite(pcsType, fX, fY, fAlpha, fDegAngle, fScale, fScale, fAnimProgress, bPremultiplyAlpha, NULL, false, pExtraColor);
}
/***********************************************************************************************************/
void DrawingCache::addSprite(const char *pcsType, FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fAlpha, FLOAT_TYPE fDegAngle, FLOAT_TYPE fScaleX, FLOAT_TYPE fScaleY, FLOAT_TYPE fAnimProgress, bool bPremultiplyAlpha, SUVSet* pUVSet, bool bIsProgressAFrameNum, const SColor* pExtraColor)
{
	if(fAlpha < FLOAT_EPSILON)
		return;

	int iAtlasFrame;
	CachedSequence* pSeq = getCachedSequence(pcsType, &iAtlasFrame);
	if(iAtlasFrame >= 0)
		fAnimProgress = iAtlasFrame;

	SExtraSeqDrawInfo rInfo;
	SExtraSeqDrawInfo* pExtraInfo = NULL;
	if(pExtraColor)
	{
		rInfo.myPolyColor = *pExtraColor;
		pExtraInfo = &rInfo;
	}
	pSeq->addSprite(fX, fY, fAlpha, fDegAngle, fScaleX, fScaleY, fAnimProgress, bPremultiplyAlpha, true, pUVSet, (iAtlasFrame >= 0 || bIsProgressAFrameNum), pExtraInfo);
}
/***********************************************************************************************************/
void DrawingCache::addScalableButton(const char *pcsType, FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fW, FLOAT_TYPE fH, FLOAT_TYPE fAlpha, SColor* pExtraColor)
{
	int iCornerW, iCornerH;
	getTextureManager()->getCornerSize(pcsType, iCornerW, iCornerH);
	addScalableButton(pcsType, fCenterX, fCenterY, fW, fH, fAlpha, iCornerW, iCornerH, pExtraColor);
}
/***********************************************************************************************************/
void DrawingCache::addScalableButton(const char *pcsType, FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fW, FLOAT_TYPE fH, FLOAT_TYPE fAlpha, FLOAT_TYPE fCornerXSize, FLOAT_TYPE fCornerYSize, SColor* pExtraColor)
{
	// This breaks apart a single bitmap into 7 other pieces.
	_ASSERT(fCornerXSize > 0);
	_ASSERT(fCornerYSize > 0);

	SExtraSeqDrawInfo rInfo;
	SExtraSeqDrawInfo* pExtraInfo = NULL;
	if(pExtraColor)
	{
		rInfo.myPolyColor = *pExtraColor;
		pExtraInfo = &rInfo;
	}

	// We need to find out the real dims of hte bitmap. And by that, we mean the real width
	// and height.
	int iRealW, iRealH;
	getTextureManager()->getTextureRealDims(pcsType, iRealW, iRealH);

	int iFileW = getTextureManager()->getFileWidth(pcsType);
	int iFileH = getTextureManager()->getFileHeight(pcsType);

	// Now, see how large the corners are in terms of UVs.
	int iAtlasFrameOut;
	CachedSequence* pSeq = this->getCachedSequence(pcsType, &iAtlasFrameOut);
	// Albums for these guys are not supported.
	_ASSERT(iAtlasFrameOut < 0);

	// Assume it's centered.
	SVector2D svUVStart, svUVSize;
	svUVStart.x = (FLOAT_TYPE)(iFileW - iRealW)*0.5/(FLOAT_TYPE)iFileW;
	svUVStart.y = (FLOAT_TYPE)(iFileH - iRealH)*0.5/(FLOAT_TYPE)iFileH;

	svUVSize.x = (FLOAT_TYPE)iRealW/(FLOAT_TYPE)iFileW;
	svUVSize.y = (FLOAT_TYPE)iRealH/(FLOAT_TYPE)iFileH;

	SVector2D svCornerUVSize;
	svCornerUVSize.x = fCornerXSize/(FLOAT_TYPE)iFileW;
	svCornerUVSize.y = fCornerYSize/(FLOAT_TYPE)iFileH;

	SUVSet uvSet;
	SVector2D svCenter;
	// Now, actually add the bitmaps.

	// The corners always stay the same.
	// Top left
	uvSet.myStartX = svUVStart.x;
	uvSet.myStartY = svUVStart.y;
	uvSet.myEndX = uvSet.myStartX + svCornerUVSize.x;
	uvSet.myEndY = uvSet.myStartY + svCornerUVSize.y;
	svCenter.x = fCenterX - fW/2.0 + fCornerXSize/2.0;
	svCenter.y = fCenterY - fH/2.0 + fCornerYSize/2.0;
	//svCenter.castToInt();
	pSeq->addSprite(svCenter.x, svCenter.y, fAlpha, 0, svCornerUVSize.x, svCornerUVSize.y, 0.0, true, true, &uvSet, false, pExtraInfo);

	// Top right
	uvSet.myStartX = svUVStart.x + svUVSize.x - svCornerUVSize.x;
	uvSet.myStartY = svUVStart.y;
	uvSet.myEndX = uvSet.myStartX + svCornerUVSize.x;
	uvSet.myEndY = uvSet.myStartY + svCornerUVSize.y;
	svCenter.x = fCenterX + fW/2.0 - fCornerXSize/2.0;
	svCenter.y = fCenterY - fH/2.0 + fCornerYSize/2.0;
	//svCenter.castToInt();
	pSeq->addSprite(svCenter.x, svCenter.y, fAlpha, 0, svCornerUVSize.x, svCornerUVSize.y, 0.0, true, true, &uvSet, false, pExtraInfo);

	// Bottom left
	uvSet.myStartX = svUVStart.x;
	uvSet.myStartY = svUVStart.y + svUVSize.y - svCornerUVSize.y;
	uvSet.myEndX = uvSet.myStartX + svCornerUVSize.x;
	uvSet.myEndY = uvSet.myStartY + svCornerUVSize.y;
	svCenter.x = fCenterX - fW/2.0 + fCornerXSize/2.0;
	svCenter.y = fCenterY + fH/2.0 - fCornerYSize/2.0;
	//svCenter.castToInt();
	pSeq->addSprite(svCenter.x, svCenter.y, fAlpha, 0, svCornerUVSize.x, svCornerUVSize.y, 0.0, true, true, &uvSet, false, pExtraInfo);

	// Bottom right
	uvSet.myStartX = svUVStart.x + svUVSize.x - svCornerUVSize.x;
	uvSet.myStartY = svUVStart.y + svUVSize.y - svCornerUVSize.y;
	uvSet.myEndX = uvSet.myStartX + svCornerUVSize.x;
	uvSet.myEndY = uvSet.myStartY + svCornerUVSize.y;
	svCenter.x = fCenterX + fW/2.0 - fCornerXSize/2.0;
	svCenter.y = fCenterY + fH/2.0 - fCornerYSize/2.0;
	//svCenter.castToInt();
	pSeq->addSprite(svCenter.x, svCenter.y, fAlpha, 0, svCornerUVSize.x, svCornerUVSize.y, 0.0, true, true, &uvSet, false, pExtraInfo);

	// Now, draw the sides. We can just stretch

	FLOAT_TYPE fSideHorScale, fSideVertScale;
	//fSideHorScale = (fW - fCornerXSize*2.0)/((svUVSize.x - svCornerUVSize.x*2.0)*(FLOAT_TYPE)iFileW);
	//fSideVertScale = (fH - fCornerYSize*2.0)/((svUVSize.y - svCornerUVSize.y*2.0)*(FLOAT_TYPE)iFileH);
	fSideHorScale = (fW - fCornerXSize*2.0)/((FLOAT_TYPE)iFileW);
	fSideVertScale = (fH - fCornerYSize*2.0)/((FLOAT_TYPE)iFileH);

	// Top
	uvSet.myStartX = svUVStart.x + svCornerUVSize.x;
	uvSet.myStartY = svUVStart.y;
	uvSet.myEndX = uvSet.myStartX + svUVSize.x - svCornerUVSize.x*2;
	uvSet.myEndY = uvSet.myStartY + svCornerUVSize.y;
	svCenter.x = fCenterX;
	svCenter.y = fCenterY - fH/2.0 + fCornerYSize/2.0;
	//svCenter.castToInt();
	pSeq->addSprite(svCenter.x, svCenter.y, fAlpha, 0, fSideHorScale, svCornerUVSize.y, 0.0, true, true, &uvSet, false, pExtraInfo);

	// Bottom
	uvSet.myStartX = svUVStart.x + svCornerUVSize.x;
	uvSet.myStartY = svUVStart.y + svUVSize.y - svCornerUVSize.y;
	uvSet.myEndX = uvSet.myStartX + svUVSize.x - svCornerUVSize.x*2;
	uvSet.myEndY = uvSet.myStartY + svCornerUVSize.y;
	svCenter.x = fCenterX;
	svCenter.y = fCenterY + fH/2.0 - fCornerYSize/2.0;
	//svCenter.castToInt();
	pSeq->addSprite(svCenter.x, svCenter.y, fAlpha, 0, fSideHorScale, svCornerUVSize.y, 0.0, true, true, &uvSet, false, pExtraInfo);

	// Left
	uvSet.myStartX = svUVStart.x;
	uvSet.myStartY = svUVStart.y + svCornerUVSize.y;
	uvSet.myEndX = uvSet.myStartX + svCornerUVSize.x;
	uvSet.myEndY = uvSet.myStartY + svUVSize.y - svCornerUVSize.y*2;
	svCenter.x = fCenterX - fW/2.0 + fCornerXSize/2.0;
	svCenter.y = fCenterY;
	//svCenter.castToInt();
	pSeq->addSprite(svCenter.x, svCenter.y, fAlpha, 0, svCornerUVSize.x, fSideVertScale, 0.0, true, true, &uvSet, false, pExtraInfo);

	// Right
	uvSet.myStartX = svUVStart.x + svUVSize.x - svCornerUVSize.x;
	uvSet.myStartY = svUVStart.y + svCornerUVSize.y;
	uvSet.myEndX = uvSet.myStartX + svCornerUVSize.x;
	uvSet.myEndY = uvSet.myStartY + svUVSize.y - svCornerUVSize.y*2;
	svCenter.x = fCenterX + fW/2.0 - fCornerXSize/2.0;
	svCenter.y = fCenterY;
	//svCenter.castToInt();
	pSeq->addSprite(svCenter.x, svCenter.y, fAlpha, 0, svCornerUVSize.x, fSideVertScale, 0.0, true, true, &uvSet, false, pExtraInfo);

	// Finally, fill the inner part with tiled images of the center
	uvSet.myStartX = svUVStart.x + svCornerUVSize.x;
	uvSet.myStartY = svUVStart.y + svCornerUVSize.y;
	uvSet.myEndX = uvSet.myStartX + svUVSize.x - svCornerUVSize.x*2;
	uvSet.myEndY = uvSet.myStartY + svUVSize.y - svCornerUVSize.y*2;

	SVector2D svInnerActualSize, svInnerUVScale;

	svInnerUVScale.x = (svUVSize.x - svCornerUVSize.x*2.0);
	svInnerUVScale.y = (svUVSize.y - svCornerUVSize.y*2.0);
	svInnerActualSize.x = (svInnerUVScale.x*(FLOAT_TYPE)iFileW);
	svInnerActualSize.y = (svInnerUVScale.y*(FLOAT_TYPE)iFileH);

	int iNumYIters = 0, iNumXIters = 0;

	int iX = fCenterX + fCornerXSize - fW/2.0;
	int iY = fCenterY + fCornerYSize - fH/2.0;
	for(iY = fCenterY + fCornerYSize - fH/2.0; svInnerActualSize.y > 0 && iY + svInnerActualSize.y <= fCenterY + fH/2.0 - fCornerYSize; iY += svInnerActualSize.y)
	{
		iNumXIters = 0;
		for(iX = fCenterX + fCornerXSize - fW/2.0; svInnerActualSize.x > 0 && iX + svInnerActualSize.x <= fCenterX + fW/2.0 - fCornerXSize; iX += svInnerActualSize.x)
		{
			// Add the texture to the cache
			svCenter.x = iX + svInnerActualSize.x/2.0;
			svCenter.y = iY + svInnerActualSize.y/2.0;
			//svCenter.castToInt();
			pSeq->addSprite(svCenter.x, svCenter.y, fAlpha, 0, svInnerUVScale.x, svInnerUVScale.y, 0.0, true, true, &uvSet, false, pExtraInfo);
			iNumXIters++;
		}
		iNumYIters++;
	}

	FLOAT_TYPE fDistLeft, fDistLeft2;
	int iX2, iY2;
	SVector2D svPieceUVScale, svPieceActualSize;

	fDistLeft = (fCenterX + fW/2.0 - fCornerXSize) - iX;
	fDistLeft2 = (fCenterY + fH/2.0 - fCornerYSize) - iY;

	if(iNumYIters == 0)
	{
		// We're thinner than our bitmap. Special case.
		uvSet.myStartX = svUVStart.x + svCornerUVSize.x;
		uvSet.myStartY = svUVStart.y + svCornerUVSize.y;
		uvSet.myEndX = uvSet.myStartX + svUVSize.x - svCornerUVSize.x*2;
		uvSet.myEndY = uvSet.myStartY + fDistLeft2/(FLOAT_TYPE)iFileH;

		svPieceUVScale.x = uvSet.myEndX - uvSet.myStartX;
		svPieceUVScale.y = uvSet.myEndY - uvSet.myStartY;
		svPieceActualSize.x = (svPieceUVScale.x*(FLOAT_TYPE)iFileW);
		svPieceActualSize.y = (svPieceUVScale.y*(FLOAT_TYPE)iFileH);

		iNumXIters = 0;
		for(; svInnerActualSize.x > 0 && iX + svInnerActualSize.x <= fCenterX + fW/2.0 - fCornerXSize; iX += svInnerActualSize.x)
		{
			svCenter.x = iX + svInnerActualSize.x/2.0;
			svCenter.y = iY + svPieceActualSize.y/2.0;
			//svCenter.castToInt();
			pSeq->addSprite(svCenter.x, svCenter.y, fAlpha, 0, svInnerUVScale.x, svPieceUVScale.y, 0.0, true, true, &uvSet, false, pExtraInfo);
			iNumXIters++;
		}
	}

	if(iNumXIters == 0)
	{
		uvSet.myStartX = svUVStart.x + svCornerUVSize.x;
		uvSet.myStartY = svUVStart.y + svCornerUVSize.y;
		uvSet.myEndX = uvSet.myStartX + fDistLeft/(FLOAT_TYPE)iFileW;
		uvSet.myEndY = uvSet.myStartY + svUVSize.y - svCornerUVSize.y*2;

		svPieceUVScale.x = uvSet.myEndX - uvSet.myStartX;
		svPieceUVScale.y = uvSet.myEndY - uvSet.myStartY;
		svPieceActualSize.x = (svPieceUVScale.x*(FLOAT_TYPE)iFileW);
		svPieceActualSize.y = (svPieceUVScale.y*(FLOAT_TYPE)iFileH);

		iNumYIters = 0;
		for(; svInnerActualSize.y > 0 && iY + svInnerActualSize.y <= fCenterY + fH/2.0 - fCornerYSize; iY += svInnerActualSize.y)
		{
			svCenter.x = iX + svPieceActualSize.x/2.0;
			svCenter.y = iY + svInnerActualSize.y/2.0;
			//svCenter.castToInt();
			pSeq->addSprite(svCenter.x, svCenter.y, fAlpha, 0, svPieceUVScale.x, svInnerUVScale.y, 0.0, true, true, &uvSet, false, pExtraInfo);
			iNumYIters++;
		}
	}

	fDistLeft = (fCenterX + fW/2.0 - fCornerXSize) - iX;
	fDistLeft2 = (fCenterY + fH/2.0 - fCornerYSize) - iY;

	if(fDistLeft > FLOAT_EPSILON)
	{
		// Now, fill in any leftover pieces

		// First, the last column of missing x-pieces
		uvSet.myStartX = svUVStart.x + svCornerUVSize.x;
		uvSet.myStartY = svUVStart.y + svCornerUVSize.y;
		uvSet.myEndX = uvSet.myStartX + fDistLeft/(FLOAT_TYPE)iFileW;
		uvSet.myEndY = uvSet.myStartY + svUVSize.y - svCornerUVSize.y*2;

		svPieceUVScale.x = uvSet.myEndX - uvSet.myStartX;
		svPieceUVScale.y = uvSet.myEndY - uvSet.myStartY;
		svPieceActualSize.x = (svPieceUVScale.x*(FLOAT_TYPE)iFileW);
		svPieceActualSize.y = (svPieceUVScale.y*(FLOAT_TYPE)iFileH);

		for(iY2 = fCenterY + fCornerYSize - fH/2.0; svInnerActualSize.y > 0 && iY2 + svInnerActualSize.y <= fCenterY + fH/2.0 - fCornerYSize; iY2 += svInnerActualSize.y)
		{
			svCenter.x = iX + svPieceActualSize.x/2.0;
			svCenter.y = iY2 + svInnerActualSize.y/2.0;
			//svCenter.castToInt();
			pSeq->addSprite(svCenter.x, svCenter.y, fAlpha, 0, svPieceUVScale.x, svInnerUVScale.y, 0.0, true, true, &uvSet, false, pExtraInfo);
		}

	}

	// Last row of missing y-pieces
	if(fDistLeft2 > FLOAT_EPSILON && iNumYIters > 0)
	{
		uvSet.myStartX = svUVStart.x + svCornerUVSize.x;
		uvSet.myStartY = svUVStart.y + svCornerUVSize.y;
		uvSet.myEndX = uvSet.myStartX + svUVSize.x - svCornerUVSize.x*2;
		uvSet.myEndY = uvSet.myStartY + fDistLeft2/(FLOAT_TYPE)iFileH;

		svPieceUVScale.x = uvSet.myEndX - uvSet.myStartX;
		svPieceUVScale.y = uvSet.myEndY - uvSet.myStartY;
		svPieceActualSize.x = (svPieceUVScale.x*(FLOAT_TYPE)iFileW);
		svPieceActualSize.y = (svPieceUVScale.y*(FLOAT_TYPE)iFileH);

		for(iX2 = fCenterX + fCornerXSize - fW/2.0; svInnerActualSize.x > 0 && iX2 + svInnerActualSize.x <= fCenterX + fW/2.0 - fCornerXSize; iX2 += svInnerActualSize.x)
		{
			svCenter.x = iX2 + svInnerActualSize.x/2.0;
			svCenter.y = iY + svPieceActualSize.y/2.0;
			//svCenter.castToInt();
			pSeq->addSprite(svCenter.x, svCenter.y, fAlpha, 0, svInnerUVScale.x, svPieceUVScale.y, 0.0, true, true, &uvSet, false, pExtraInfo);
		}
	}
	// Finally, the last corner piece...
	if(fDistLeft > FLOAT_EPSILON && fDistLeft2 > FLOAT_EPSILON)
	{
		uvSet.myStartX = svUVStart.x + svCornerUVSize.x;
		uvSet.myStartY = svUVStart.y + svCornerUVSize.y;
		uvSet.myEndX = uvSet.myStartX + fDistLeft/(FLOAT_TYPE)iFileW;
		uvSet.myEndY = uvSet.myStartY + fDistLeft2/(FLOAT_TYPE)iFileH;

		svPieceUVScale.x = uvSet.myEndX - uvSet.myStartX;
		svPieceUVScale.y = uvSet.myEndY - uvSet.myStartY;
		svPieceActualSize.x = (svPieceUVScale.x*(FLOAT_TYPE)iFileW);
		svPieceActualSize.y = (svPieceUVScale.y*(FLOAT_TYPE)iFileH);

		svCenter.x = iX + svPieceActualSize.x/2.0;
		svCenter.y = iY + svPieceActualSize.y/2.0;
		//svCenter.castToInt();
		pSeq->addSprite(svCenter.x, svCenter.y, fAlpha, 0, svPieceUVScale.x, svPieceUVScale.y, 0.0, true, true, &uvSet, false, pExtraInfo);
	}
}
/*****************************************************************************/
void DrawingCache::addScalableButton(const char *pcsType, const SRect2D& srRect, FLOAT_TYPE fAlpha, SColor* pExtraColor)
{
	addScalableButton(pcsType, srRect.center().x, srRect.center().y, srRect.w, srRect.h, fAlpha, pExtraColor);
}
/***********************************************************************************************************/
void DrawingCache::addCustomObject2D(SVertexInfo* pVerts, int iNumVerts)
{
	SCachedCustomObject2D rDummy;
	rDummy.myVerts = pVerts;
	rDummy.myNumVerts = iNumVerts;
	myCachedCustomObjects2D.push_back(rDummy);
}
/***********************************************************************************************************/
void DrawingCache::addCustomObject3D(FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fCenterZ,  FLOAT_TYPE fAlpha, FLOAT_TYPE fScale, CachedSequence* pTexture, SVertex3D* pVertexData, int iNumVerts, SColor& scolDiffuseMults, SColor& scolAmbMults, SVector3D* pRotations)
{
#ifdef ALLOW_3D
	SCachedCustomObject3D rDummy;
	rDummy.myCenter.set(fCenterX, fCenterY);
	rDummy.myAlpha = fAlpha;
	rDummy.myScale = fScale;
	rDummy.myElevation = fCenterZ;

	rDummy.myDiffuseMults = scolDiffuseMults;
	rDummy.myAmbMults = scolAmbMults;
	rDummy.myTextureSeq = pTexture;
	rDummy.myNumVerts = iNumVerts;
	// Note: just a pointer...
	rDummy.myVertexData = pVertexData;
	if(pRotations)
		rDummy.myRotations = *pRotations;

	myCachedCustomObjects3D.push_back(rDummy);
#endif
}
/***********************************************************************************************************/
void DrawingCache::addPerspObject3D(const char* pcsObjType, FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fCenterZ, FLOAT_TYPE fAlpha, FLOAT_TYPE fScale, FLOAT_TYPE fAnimProgress, int iClipIndex, SVector3D* pRotations, const char* pcsOverrideTextureAnim)
{
#ifdef ALLOW_3D

	SCachedObject3D* pTarget;


#if defined(EXPERIMENTAL_SORT_PERSP_SEQ_BY_DEPTH)
	SPerspObjInstance rInstance;
	pTarget = &rInstance.myObj3DInfo;
	rInstance.myType = PerspObjObject3D;
	rInstance.mySeq = NULL;
	rInstance.myDepth = fCenterY;
#else
	SCachedObject3D rDummy;
	pTarget = &rDummy;
#endif

	pTarget->myObjId = pcsObjType;
	pTarget->myCenter.set(fCenterX, fCenterY, fCenterZ);
	if(pRotations)
		pTarget->myRotations = *pRotations;
	else
		pTarget->myRotations.set(0,0,0);

	pTarget->myAlpha = fAlpha;
	pTarget->myScale = fScale;

	if(pcsOverrideTextureAnim)
		pTarget->myOverrideTexture = pcsOverrideTextureAnim;
	else
		pTarget->myOverrideTexture = "";

	pTarget->myAnimProgress = fAnimProgress;
	pTarget->myClipIndex = iClipIndex;
	if(pTarget->myClipIndex < 0)
		pTarget->myClipIndex = 0;

#if defined(EXPERIMENTAL_SORT_PERSP_SEQ_BY_DEPTH) || defined(EXPERIMENTAL_SORT_ORTHO_SEQ_BY_DEPTH)
	mySortedDeepPerspSequences.insert(rInstance);
#endif

	myCachedPerspObjects3D.push_back(*pTarget);

#else
	// 3D Disabled!
	_ASSERT(0);
#endif

}
/***********************************************************************************************************/
void DrawingCache::addObject3D(const char* pcsObjType, FLOAT_TYPE fCenterX, FLOAT_TYPE fCenterY, FLOAT_TYPE fAlpha, FLOAT_TYPE fScale, FLOAT_TYPE fAnimProgress, int iClipIndex, SVector3D* pRotations, const char* pcsOverrideTextureAnim)
{
#ifdef ALLOW_3D
	SCachedObject3D rDummy;
	rDummy.myObjId = pcsObjType;
	rDummy.myCenter.set(fCenterX, fCenterY, 0);
#ifndef RENDER_FIELD_WITH_PERSPECTIVE
	rDummy.myCenter.y *= -1;
#endif
	if(pRotations)
		rDummy.myRotations = *pRotations;
	else
		rDummy.myRotations.set(0,0,0);

	rDummy.myAlpha = fAlpha;
	rDummy.myScale = fScale;

	if(pcsOverrideTextureAnim)
		rDummy.myOverrideTexture = pcsOverrideTextureAnim;
	else
		rDummy.myOverrideTexture = "";

	rDummy.myAnimProgress = fAnimProgress;
	rDummy.myClipIndex = iClipIndex;
	if(rDummy.myClipIndex < 0)
		rDummy.myClipIndex = 0;

	myCachedObjects3D.push_back(rDummy);
#else
	// 3D Disabled!
	_ASSERT(0);
#endif
}
/***********************************************************************************************************/
bool DrawingCache::drawCachedText(SCachedTextObject* pTextObj)
{
	if(pTextObj->myText.length() == 0)
		return false;

#ifndef ENABLE_TEXT_CACHING
	RenderUtils::drawText(this, pTextObj->myText.c_str(), *pTextObj);
	return true;
#endif

#ifdef _DEBUG
	if(pTextObj->myText == "0.00")
	{
		int bp = 0;
	}
#endif

	// Now, draw it with the right params
	SVector2D svTextDims, svBitmapDims;
	TX_MAN_RETURN_TYPE iTexIndex = myTextCacher->getTextureFor(pTextObj->myText, pTextObj->myBaseInfo, svTextDims, svBitmapDims, false);

	if(pTextObj->myScale != 1.0)
	{
		svBitmapDims *= pTextObj->myScale;
		svTextDims *= pTextObj->myScale;
	}

	if(iTexIndex == 0)
	{
		myTextCacher->queueForCreation(pTextObj->myText.c_str(), pTextObj->myBaseInfo, pTextObj->myDidRenderDuringStencil);
		// Semi-hack: if we render these forcibly when one is over the boundary of a stencil
		// buffer (originally) these will briefly flash on-screen. So for large textures,
		// we don't draw them, since they're likely to cut across the stencil boundary.
		// Since we don't have the bitmap dims at this point, just go by the text width.
		// Better solution: don't render these if we're in the stencil.
		if(!pTextObj->myDidRenderDuringStencil || pTextObj->myForceDrawingIfUncached)
		{

#ifdef DIRECTX_PIPELINE
			g_pDxRenderer->setCachedRenderingTextMode(false);
#endif
			RenderUtils::drawText(this, pTextObj->myText.c_str(), *pTextObj);
			return true;
		}
		else
			return false;
	}

	/*
	ResourceCollection* pFonts = ResourceManager::getInstance()->getCollection(ResourceRasterFonts);
	ResourceItem* pItem = pFonts->getItemById(pTextObj->myBaseInfo.myFont.c_str());
	if(!pItem)
	{
		// Could not find the right font
		_ASSERT(0);
		return false;
	}
	*/

#ifdef DIRECTX_PIPELINE
	g_pDxRenderer->setCachedRenderingTextMode(true);
#endif


	SVector2D svFinalDrawPos;
	svFinalDrawPos = pTextObj->myCenter;
//#ifndef WIN32
#if !defined(WIN32) && !defined(MAC_BUILD)
#ifndef ORIENTATION_LANDSCAPE
	TEST
	// Won't work with left orientations since we need
	// to make copy func be aware of the different rotation.
	_ASSERT(0);
#endif
	/*
	 glRotatef(90.0, 0.0, 0.0, 1.0);
	 glTranslatef(backingHeight, -backingWidth, 0.0);
	 glScalef(-1, 1, 1);
	 */
	SVector2D svScreen;
	myParentWindow->getSize(svScreen);

	svFinalDrawPos.x *= -1;

	svFinalDrawPos.x += svScreen.x;
	svFinalDrawPos.y -= svScreen.y;

	FLOAT_TYPE fTemp = svFinalDrawPos.x;
	svFinalDrawPos.x = -svFinalDrawPos.y;
	svFinalDrawPos.y = fTemp;

	if(pTextObj->myHorAlign == HorAlignLeft)
	{
		svFinalDrawPos.y -= svBitmapDims.y*0.5;
	}
	else if(pTextObj->myHorAlign == HorAlignCenter)
	{
		svFinalDrawPos.y -= (svBitmapDims.y - svTextDims.x)/2.0;
	}
	else if(pTextObj->myHorAlign == HorAlignRight)
	{
		svFinalDrawPos.y += svBitmapDims.y*0.5;
		svFinalDrawPos.y -= (svBitmapDims.y - svTextDims.x);
	}


	if(pTextObj->myVertAlign == VertAlignTop)
	{
		svFinalDrawPos.x -= svBitmapDims.x*0.5;

	}
	else if(pTextObj->myVertAlign == VertAlignCenter)
		svFinalDrawPos.x -= (svBitmapDims.x - svTextDims.y)/2.0;
	else if(pTextObj->myVertAlign == VertAlignBottom)
	{
		svFinalDrawPos.x += svBitmapDims.x*0.5;
		svFinalDrawPos.x -= (svBitmapDims.x - svTextDims.y);
	}

#else

	if(pTextObj->myHorAlign == HorAlignLeft)
	{
		svFinalDrawPos.x += svBitmapDims.x*0.5;
	}
	else if(pTextObj->myHorAlign == HorAlignCenter)
	{
		svFinalDrawPos.x += (svBitmapDims.x - svTextDims.x)/2.0;
	}
	else if(pTextObj->myHorAlign == HorAlignRight)
	{
		svFinalDrawPos.x -= svBitmapDims.x*0.5;
		svFinalDrawPos.x += (svBitmapDims.x - svTextDims.x);
	}


	if(pTextObj->myVertAlign == VertAlignTop)
	{
		svFinalDrawPos.y += svBitmapDims.y*0.5;

	}
	else if(pTextObj->myVertAlign == VertAlignCenter)
		svFinalDrawPos.y += (svBitmapDims.y - svTextDims.y)/2.0;
	else if(pTextObj->myVertAlign == VertAlignBottom)
	{
		svFinalDrawPos.y -= svBitmapDims.y*0.5;
		svFinalDrawPos.y += (svBitmapDims.y - svTextDims.y);
	}
#endif
	SUVSet uvsOut;

#if defined(WIN32) && defined(USE_OPENGL2)
	// Flip vertically for Win OpenGl2
	uvsOut.myEndY = 0.0;
	uvsOut.myStartY = 1.0;
#endif

	const FLOAT_TYPE fDegAngle = 0.0;

	// Draw shadow, if any
	if(pTextObj->myHasShadow)
	{
//#ifdef WIN32
#if defined(WIN32) || defined(MAC_BUILD)
		GraphicsUtils::drawImage(myParentWindow, iTexIndex, svFinalDrawPos.x + pTextObj->myShadowOffset.x, svFinalDrawPos.y + pTextObj->myShadowOffset.y, svBitmapDims.x, svBitmapDims.y, pTextObj->myShadowColor.alpha, fDegAngle, true, uvsOut, pTextObj->myBlendMode, &pTextObj->myShadowColor);
#else
		GraphicsUtils::drawImage(myParentWindow, iTexIndex, svFinalDrawPos.x - pTextObj->myShadowOffset.y, svFinalDrawPos.y - pTextObj->myShadowOffset.x, svBitmapDims.x, svBitmapDims.y, pTextObj->myShadowColor.alpha, fDegAngle, true, uvsOut, pTextObj->myBlendMode, &pTextObj->myShadowColor);
#endif
	}
	GraphicsUtils::drawImage(myParentWindow, iTexIndex, svFinalDrawPos.x, svFinalDrawPos.y, svBitmapDims.x, svBitmapDims.y, pTextObj->myColor.alpha, fDegAngle, true, uvsOut, pTextObj->myBlendMode, &pTextObj->myColor);

#ifdef _DEBUG
// SColor scolTest(1,0,0,1);
// glDrawRectangle(myParentWindow, svFinalDrawPos.x - svBitmapDims.x/2, svFinalDrawPos.y - svBitmapDims.y/2, svBitmapDims.x, svBitmapDims.y, scolTest, 1, true);
#endif
	return false;
}
/***********************************************************************************************************/
void DrawingCache::fillArea(const char* pcsTextureName, const SRect2D& srRect, FLOAT_TYPE fAlpha, FLOAT_TYPE fAreaScale, FLOAT_TYPE fBitmapScale)
{
	fillArea(pcsTextureName, srRect.x, srRect.y, srRect.w, srRect.h, fAlpha, fAreaScale, fBitmapScale);
}
/***********************************************************************************************************/
void DrawingCache::fillArea(const char* pcsTextureName, int iStartX, int iStartY, int iAreaW, int iAreaH, FLOAT_TYPE fAlpha, FLOAT_TYPE fAreaScale, FLOAT_TYPE fBitmapScale)
{
//	if(bUseHardRegionCutoff)
		//RenderUtils::beginScissorRectangle(iStartX, iStartY, iAreaW*fAreaScale, iAreaH*fAreaScale, this);

#ifdef _DEBUG
//#define DEBUG_FILL_REGIONS
#endif

	// Find out texture's real dims for this.
	int iAtlasFrame;
	CachedSequence *pSeq = this->getCachedSequence(pcsTextureName, &iAtlasFrame);
	// Atlases not supported for this. Though it's trivial - we just look at the
	// frame size instead of the file's real size.
	_ASSERT(iAtlasFrame < 0);

	int iFileW = getTextureManager()->getFileWidth(pcsTextureName);
	int iFileH = getTextureManager()->getFileHeight(pcsTextureName);

	int iRealW, iRealH;
	getTextureManager()->getTextureRealDims(pcsTextureName, iRealW, iRealH);
	// If this assert fails, you have an image set to fill an area, but haven't specified
	// its real (as opposed to bitmap) size.
	_ASSERT(iRealW > 0 && iRealH > 0);

	if(iRealW <= 0 || iRealH <= 0)
	{
		// These are to prevent us from an infinite cycle
		iRealW = iAreaW;
		iRealH = iAreaH;

	}

#ifdef DEBUG_FILL_REGIONS
	string strTempDbString;
	int iDbPieceCount = 1;
	const char* const pcsDbPieceFont = "ftMainFontSmall";
	SColor scolDbPieceColor(1,1,1,1);
	fAlpha *= 0.5;
#endif

	SVector2D svUVStart, svUVSize;
	svUVStart.x = (FLOAT_TYPE)(iFileW - iRealW)*0.5/(FLOAT_TYPE)iFileW;
	svUVStart.y = (FLOAT_TYPE)(iFileH - iRealH)*0.5/(FLOAT_TYPE)iFileH;
	svUVSize.x = (FLOAT_TYPE)iRealW/(FLOAT_TYPE)iFileW;
	svUVSize.y = (FLOAT_TYPE)iRealH/(FLOAT_TYPE)iFileH;

	iAreaW = (FLOAT_TYPE)iAreaW*fAreaScale;
	iAreaH = (FLOAT_TYPE)iAreaH*fAreaScale;

	iRealW = (FLOAT_TYPE)iRealW*fBitmapScale;
	iRealH = (FLOAT_TYPE)iRealH*fBitmapScale;

	iFileW = (FLOAT_TYPE)iFileW*fBitmapScale;
	iFileH = (FLOAT_TYPE)iFileH*fBitmapScale;

	// Now, we're finally set.
	int iX = iStartX, iY;
	for(iY = iStartY; iY <= iStartY + iAreaH - iRealH; iY += iRealH)
	{
		for(iX = iStartX; iX <= iStartX + iAreaW - iRealW; iX += iRealW)
		{
			// Add the texture to the cache
			pSeq->addSprite(iX + iRealW/2.0, iY + iRealH/2.0, fAlpha, 0, fBitmapScale, 0.0, true);
#ifdef DEBUG_FILL_REGIONS
			StringUtils::numberToString(iDbPieceCount, strTempDbString);
			this->addText(strTempDbString.c_str(), pcsDbPieceFont, iX + iRealW/2.0, iY + iRealH/2.0, scolDbPieceColor);
			iDbPieceCount++;
#endif
		}
	}

	// Now, we need to fill any remaining pieces on the right, at the bottom, and the last piece 
	// in the bottom-right corner.
	int iOldX = iX - iRealW;
	int iOldY = iY - iRealH;

	bool bNoItersX = (iX == iStartX);
	bool bNoItersY = (iY == iStartY);
	if(bNoItersX)
		iOldX = iStartX;
	else
		iOldX = iX - iRealW;

	if(bNoItersY)
		iOldY = iStartY;
	else
		iOldY = iY - iRealH;

	SVector2D svOldOffsets;
	FLOAT_TYPE fLeftOverSizeX, fLeftOverSizeY;
	fLeftOverSizeX = (iStartX + iAreaW) - (iOldX + iRealW);
	if(bNoItersX)
		fLeftOverSizeX = iAreaW;
	else
		svOldOffsets.x = iRealW;
	fLeftOverSizeY = (iStartY + iAreaH) - (iOldY + iRealH);
	if(bNoItersY)
		fLeftOverSizeY = iAreaH;
	else
		svOldOffsets.y = iRealH;

	SVector2D svExtraPieceUVSize(svUVSize.x, svUVSize.y);
	SVector2D svExtraPieceScale(1, 1);
/*
	if(iAreaW < iFileW)
	{
		// In this case,
		svExtraPieceScale.x = (FLOAT_TYPE)iAreaW/(FLOAT_TYPE)iFileW;
		svExtraPieceUVSize.x *= (FLOAT_TYPE)iAreaW/(FLOAT_TYPE)iRealW;
	}

	if(iAreaH < iFileH)
	{
		// In this case,
		svExtraPieceScale.y = (FLOAT_TYPE)iAreaH/(FLOAT_TYPE)iFileH;
		svExtraPieceUVSize.y *= (FLOAT_TYPE)iAreaH/(FLOAT_TYPE)iRealH;
	}
*/
	if((iAreaW - fLeftOverSizeX) < iFileW)
	{
		// In this case,
		svExtraPieceScale.x = (FLOAT_TYPE)(iAreaW - fLeftOverSizeX)/(FLOAT_TYPE)iFileW;
		svExtraPieceUVSize.x *= (FLOAT_TYPE)(iAreaW - fLeftOverSizeX)/(FLOAT_TYPE)iRealW;
	}

	if((iAreaH - fLeftOverSizeY) < iFileH)
	{
		// In this case,
		svExtraPieceScale.y = (FLOAT_TYPE)(iAreaH - fLeftOverSizeY)/(FLOAT_TYPE)iFileH;
		svExtraPieceUVSize.y *= (FLOAT_TYPE)(iAreaH - fLeftOverSizeY)/(FLOAT_TYPE)iRealH;
	}


	// Compute teh UVs
	FLOAT_TYPE fPieceCenter;
	FLOAT_TYPE fPieceScale;
	SUVSet rUVSet;

	rUVSet.myStartX = svUVStart.x;
	rUVSet.myStartY = svUVStart.y;

	// This is the right edge of stuff
	if(fLeftOverSizeX >= 1.0)
	{
		fPieceScale = fLeftOverSizeX/(FLOAT_TYPE)iFileW;

		fPieceCenter = iOldX + svOldOffsets.x + fLeftOverSizeX/2.0;

		rUVSet.myEndX = rUVSet.myStartX + svUVSize.x*fPieceScale;
		rUVSet.myEndY = rUVSet.myStartY + svExtraPieceUVSize.y;

		// Do this *after* uv calcs
		fPieceScale *= fBitmapScale;		
		for(iY = iStartY; iY <= iStartY + iAreaH - iRealH; iY += iRealH)
		{
			pSeq->addSprite(fPieceCenter, iY + iRealH/2.0, fAlpha, 0, fPieceScale, svExtraPieceScale.y*fBitmapScale, 0.0, true, true, &rUVSet);
#ifdef DEBUG_FILL_REGIONS
			StringUtils::numberToString(iDbPieceCount, strTempDbString);
			this->addText(strTempDbString.c_str(), pcsDbPieceFont, fPieceCenter, iY + iRealH/2.0, scolDbPieceColor);
			iDbPieceCount++;
#endif
		}
	}

	// This is the bottom edge of stuff
	if(fLeftOverSizeY >= 1.0)
	{
		fPieceScale = fLeftOverSizeY/(FLOAT_TYPE)iFileH;

		fPieceCenter = iOldY + svOldOffsets.y + fLeftOverSizeY/2.0;

		rUVSet.myEndX = rUVSet.myStartX + svExtraPieceUVSize.x;
		rUVSet.myEndY = rUVSet.myStartY + svUVSize.y*fPieceScale;

		// Do this *after* uv calcs
		fPieceScale *= fBitmapScale;
		for(iX = iStartX; iX <= iStartX + iAreaW - iRealW; iX += iRealW)
		{
			pSeq->addSprite(iX + iRealW/2.0, fPieceCenter, fAlpha, 0, svExtraPieceScale.x*fBitmapScale, fPieceScale, 0.0, true, true, &rUVSet);
#ifdef DEBUG_FILL_REGIONS
			StringUtils::numberToString(iDbPieceCount, strTempDbString);
			this->addText(strTempDbString.c_str(), pcsDbPieceFont, iX + iRealW/2.0, fPieceCenter, scolDbPieceColor);
			iDbPieceCount++;
#endif
		}
	}

	// This is the last bottom-left corner piece
	if(fLeftOverSizeX >= 1.0 && fLeftOverSizeY >= 1.0) // && !bNoItersX && !bNoItersY)
	{
		SVector2D svPieceScales;
		svPieceScales.x = fLeftOverSizeX/(FLOAT_TYPE)iFileW;
		svPieceScales.y = fLeftOverSizeY/(FLOAT_TYPE)iFileH;

		rUVSet.myEndX = rUVSet.myStartX + svUVSize.x*svPieceScales.x;
		rUVSet.myEndY = rUVSet.myStartY + svUVSize.y*svPieceScales.y;

		SVector2D svPieceCenter;
		svPieceCenter.x = iOldX + svOldOffsets.x + fLeftOverSizeX/2.0;
		svPieceCenter.y = iOldY + svOldOffsets.y + fLeftOverSizeY/2.0;

		svPieceScales *= fBitmapScale;
		pSeq->addSprite(svPieceCenter.x, svPieceCenter.y, fAlpha, 0, svPieceScales.x, svPieceScales.y, 0.0, true, true, &rUVSet);

#ifdef DEBUG_FILL_REGIONS
		StringUtils::numberToString(iDbPieceCount, strTempDbString);
		this->addText(strTempDbString.c_str(), pcsDbPieceFont, svPieceCenter.x, svPieceCenter.y, scolDbPieceColor);
		iDbPieceCount++;
#endif
	}

	// Restore the scissor region if we cut it off.
	// Note that what we should really do is ask for the region
	// before us in the beginning and restore it here; however, because
	// the querying will slowdown on some cards, we just assume this is
	// the only place we use scissors, and restore the full viewport.
	//if(bUseHardRegionCutoff)
		//RenderUtils::endScissorRectangle(this);
}
/***********************************************************************************************************/
CachedDeepSequence* DrawingCache::getDeepSequenceFromRegularSequence(CachedSequence* pSeq, bool bDoPerspective)
{
	const char *pcsName = pSeq->getName();
	return this->getCachedDeepSequence(pcsName, NULL, bDoPerspective);
}
/***********************************************************************************************************/
void DrawingCache::onTimerTick(void)
{
#ifdef ENABLE_TEXTURE_UNLOADING
	GTIME lTime = Application::getInstance()->getGlobalTime(ClockUniversal);
	if(lTime % 8 != 0)
		return;

	//TStringCachedSequenceMap::iterator mi;
	//for(mi = mySequences.begin(); mi != mySequences.end(); mi++)
	//	mi->second->onTimerTick(lTime);
	int iCurrSeq, iNumSeqs = myAllSequences.size();
	for(iCurrSeq = 0; iCurrSeq < iNumSeqs; iCurrSeq++)
		myAllSequences[iCurrSeq]->onTimerTick(lTime);

	TStringCachedDeepSequenceMap::iterator mi2;
	for(mi2 = myDeepSequences.begin(); mi2 != myDeepSequences.end(); mi2++)
	{
		mi2->second->onTimerTick(lTime);
	}

	for(mi2 = myDeepPerspSequences.begin(); mi2 != myDeepPerspSequences.end(); mi2++)
	{
		mi2->second->onTimerTick(lTime);
	}

#endif
}
/***********************************************************************************************************/
void DrawingCache::reloadAllTextures()
{
	getTextureManager()->reloadAll();

// 	TStringCachedSequenceMap::iterator mi;
// 	for(mi = mySequences.begin(); mi != mySequences.end(); mi++)
// 	{
// 		mi->second->resetCache();
// 	}
	int iCurrSeq, iNumSeqs = myAllSequences.size();
	for(iCurrSeq = 0; iCurrSeq < iNumSeqs; iCurrSeq++)
		myAllSequences[iCurrSeq]->resetCache();

	TStringCachedDeepSequenceMap::iterator mi2;
	for(mi2 = myDeepSequences.begin(); mi2 != myDeepSequences.end(); mi2++)
	{
		mi2->second->resetCache();
	}

	for(mi2 = myDeepPerspSequences.begin(); mi2 != myDeepPerspSequences.end(); mi2++)
	{
		mi2->second->resetCache();
	}

}
/***********************************************************************************************************/
void DrawingCache::unloadTexture(const char* pcsAnimName)
{
	// Since we can potentially share one texture between the regular and the deep draw sequence,
	// when we unload it, we must tell both about it and reset their caches.

	int iUnloadCount = 0;

	TStringCachedSequenceMap::iterator mi;
	myCachedString = pcsAnimName;
	CachedSequence **pResult = mySequences.findSimple(pcsAnimName);
	if(pResult)
	{
		if((*pResult)->getTexture())
			(*pResult)->getTexture()->unloadTexture();
		ELSE_ASSERT;
		(*pResult)->resetCache();
		iUnloadCount++;
	}

/*
	mi = mySequences.find(myCachedString);
	if(mi != mySequences.end())
	{
		mi->second->getTexture()->unloadTexture();
		mi->second->resetCache();
		iUnloadCount++;
	}
*/
/*
	int iCurrSeq, iNumSeqs = myAllSequences.size();
	for(iCurrSeq = 0; iCurrSeq < iNumSeqs; iCurrSeq++)
	{
		if(myAllSequences[iCurrSeq]->getTexture())
			myAllSequences[iCurrSeq]->getTexture()->unloadTexture();
		myAllSequences[iCurrSeq]->resetCache();
		iUnloadCount++;
	}
*/

	TStringCachedDeepSequenceMap::iterator mi2;
	mi2 = myDeepPerspSequences.find(myCachedString);
	if(mi2 != myDeepPerspSequences.end())
	{
		if(mi2->second->getTexture())
			mi2->second->getTexture()->unloadTexture();
		mi2->second->resetCache();
		iUnloadCount++;
	}

	mi2 = myDeepSequences.find(myCachedString);
	if(mi2 != myDeepSequences.end())
	{
		if(mi2->second->getTexture())
			mi2->second->getTexture()->unloadTexture();
		mi2->second->resetCache();
		iUnloadCount++;
	}

	_ASSERT(iUnloadCount > 0);
}
/*****************************************************************************/
TextureManager* DrawingCache::getTextureManager()
{
	if(!myParentWindow)
		ASSERT_RETURN_NULL;
	return myParentWindow->getTextureManager();
}
/***********************************************************************************************************/
void DrawingCache::unloadAllUnloadableTextures()
{
	getTextureManager()->unloadAll();

	int iCurrSeq, iNumSeqs = myAllSequences.size();
	for(iCurrSeq = 0; iCurrSeq < iNumSeqs; iCurrSeq++)
		myAllSequences[iCurrSeq]->resetCache();

	TStringCachedDeepSequenceMap::iterator mi2;
	for(mi2 = myDeepSequences.begin(); mi2 != myDeepSequences.end(); mi2++)
		mi2->second->resetCache();

	for(mi2 = myDeepPerspSequences.begin(); mi2 != myDeepPerspSequences.end(); mi2++)
		mi2->second->resetCache();
}
/********************************************************************************************/
};