#include "stdafx.h"

namespace HyperUI
{
#ifndef MAC_BUILD
string CachedObject3D::theCommonString;
string CachedObject3D::theCommonString2;
#endif

// In seconds. This is really short since
// currently we only use one clip at a time, and they
// take up a lot of memory.
#define CLIP_UNLOAD_TIME	0.5

/********************************************************************************************/
CachedObject3D::CachedObject3D()
{
	myDefaultScale.set(1,1,1);
}
/********************************************************************************************/
CachedObject3D::~CachedObject3D()
{
	clear();
}
/********************************************************************************************/
void CachedObject3D::clear(void)
{
	int iCurr, iNum = myClips.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
		delete myClips[iCurr];
	myClips.clear();
}
/********************************************************************************************/
bool CachedObject3D::loadFromItem(ResourceItem* pItem, Window* pWindow)
{
	string strFileName;

	myTypeName = pItem->getStringProp(PropertyId);

	if(pItem->doesPropertyExist(PropertyObj3dDiffuseMults))
		pItem->getAsColor(PropertyObj3dDiffuseMults, myDiffuseMults);
	else
		myDiffuseMults.set(-1,1,1,1);
		//myDiffuseMults.set(1,1,1,1);

	if(pItem->doesPropertyExist(PropertyObj3dAmbientMults))
		pItem->getAsColor(PropertyObj3dAmbientMults, myAmbientMults);
	else
		myAmbientMults.set(-1,1,1,1);
		//myAmbientMults.set(1,1,1,1);

	if(pItem->doesPropertyExist(PropertyObj3dSpecularMults))
		pItem->getAsColor(PropertyObj3dSpecularMults, mySpecular);
	else
		mySpecular.set(-1,1,1,1);

	if(pItem->doesPropertyExist(PropertyObj3dSpecularPower))
		mySpecularPower = pItem->getNumProp(PropertyObj3dSpecularPower);
	else
		mySpecularPower = -1;

	if(pItem->doesPropertyExist(PropertyFile))
		strFileName = pItem->getStringProp(PropertyFile);

	bool bFlipU = pItem->getBoolProp(PropertyObj3dFlipU);
	bool bFlipV = pItem->getBoolProp(PropertyObj3dFlipV);
	bool bFlipAllTris = pItem->getBoolProp(PropertyObj3dFlipAllTris);

	if(pItem->doesPropertyExist(PropertyObj3dDefaultScale))
		pItem->getAsVector3(PropertyObj3dDefaultScale, myDefaultScale);
	else
		myDefaultScale.set(1,1,1);

	clear();

	bool bAutoCenter = pItem->getBoolProp(PropertyObj3dAutoCenter);

	// Now, lets see if we have multiple clips or just one piece of geometry.
	bool bRes = true;
	CachedClip3D* pNewClip;
/*
	if(pItem->doesPropertyExist(PropertyObj3dClipList))
	{
		// We've got a list of clips
		ResourceCollection* pClipsColl = pEngine->getResourceManager()->getCollection(ResourcePrelimClips3D);
		int iCurrClip, iNumClips = pItem->getEnumPropCount(PropertyObj3dClipList);
		const char* pcsClipType;
		ResourceItem* pClipItem;
		for(iCurrClip = 0; iCurrClip < iNumClips; iCurrClip++)
		{
			pcsClipType = pItem->getEnumPropValue(PropertyObj3dClipList, iCurrClip);
			// Find it in the clips library
			pClipItem = pClipsColl->findItemByType(pcsClipType);

			// Now get the file name
			strFileName = pClipItem->getStringProp(PropertyAnimFilename);
			pNewClip = new CachedClip3D;
			bRes = pNewClip->loadFromObjFile(strFileName.c_str(), pEngine, bAutoCenter, bFlipU, bFlipV, pClipItem);
			_ASSERT(bRes);
			myClips.push_back(pNewClip);
		}

		bRes = true;
	}
	else
	{
		_ASSERT(strFileName.length() >= 0);

		// Just to the property
		pNewClip = new CachedClip3D;
		bRes = pNewClip->loadFromObjFile(strFileName.c_str(), pEngine, bAutoCenter, bFlipU, bFlipV, NULL);
		myClips.push_back(pNewClip);
	}

	*/

	// Now we do things differently. There must always be one base file, and then the clips are read
	// from clp files which only store vertices and normals, nothing else.
	CachedClip3D* pMainClip;
	pNewClip = new CachedClip3D;
	// bRes = pNewClip->loadFromObjFile(strFileName.c_str(), pEngine, bAutoCenter, bFlipU, bFlipV, NULL, myDefaultScale);
	pNewClip->setLoadInfo(false, strFileName.c_str(), pWindow, bAutoCenter, bFlipU, bFlipV, NULL, myDefaultScale, NULL, bFlipAllTris);
	myClips.push_back(pNewClip);

	pMainClip = pNewClip;

	myNormalMapOverride = 0;
	if(pItem->doesPropertyExist(PropertyObj3dNormalMap))
		myNormalMapOverride = pWindow->getTextureManager()->getTextureIndex(pItem->getStringProp(PropertyObj3dNormalMap));

	mySpecularMapOverride = 0;
	if(pItem->doesPropertyExist(PropertyObj3dSpecularMap))
		mySpecularMapOverride = pWindow->getTextureManager()->getTextureIndex(pItem->getStringProp(PropertyObj3dSpecularMap));

	// Now, do the rest of the clips, if any
	if(pItem->doesPropertyExist(PropertyObj3dClipList))
	{
		// We've got a list of clips
		ResourceCollection* pClipsColl = ResourceManager::getInstance()->getCollection(ResourceClips3D);
		int iCurrClip, iNumClips = pItem->getEnumPropCount(PropertyObj3dClipList);
		const char* pcsClipType;
		ResourceItem* pClipItem;
		for(iCurrClip = 0; iCurrClip < iNumClips; iCurrClip++)
		{
			pcsClipType = pItem->getEnumPropValue(PropertyObj3dClipList, iCurrClip);
			// Find it in the clips library
			pClipItem = pClipsColl->getItemById(pcsClipType);

			// Now get the file name
			strFileName = pClipItem->getStringProp(PropertyFile);
			pNewClip = new CachedClip3D;
			//bRes = pNewClip->loadFromClipFile(strFileName.c_str(), pEngine, bAutoCenter, bFlipU, bFlipV, pClipItem, pMainClip);
			//_ASSERT(bRes);
			pNewClip->setLoadInfo(true, strFileName.c_str(), pWindow, bAutoCenter, bFlipU, bFlipV, pClipItem, myDefaultScale, pMainClip, bFlipAllTris);
			myClips.push_back(pNewClip);
		}

		bRes = true;
	}

	return bRes;

/*

	myDefaultScale = svDefScale;

	clear();

	// Iterate over frames to see how many we have. We should have at least one.
	int iCurrFrame;
	char pcsBuff[1024];
	CachedFrame3D* pNewFrame;
	for(iCurrFrame = 0; true; iCurrFrame++)
	{
		// See if it exists
		sprintf(pcsBuff, "%s%.4d.obj", pcsNameToken, iCurrFrame + 1);
		strFileName = pcsBuff;

#ifdef WIN32
		strFileName = "graphics3d\\" + strFileName;
#endif


		if(gDoesFileExist(strFileName.c_str(), false) == false)
		{
			if(iCurrFrame == 0)
			{
				// No files at all! Did we forget to 
				// add 0001 to the end?
				_ASSERT(0);
			}
			break;
		}

		// The file exists. Read it.
		pNewFrame = new CachedFrame3D;
		pNewFrame->loadFromObjFile(strFileName.c_str(), pEngine, bFlipU, bFlipV);
		myFrames.push_back(pNewFrame);
	};

	_ASSERT(myFrames.size() > 0);

	// Do it here since all frames have to be centered by the same location.
	if(bAutoCenter)
		centerOnOrigin();

	return true;
	*/
}
/********************************************************************************************/
CachedFrame3D* CachedObject3D::getFrameForProgress(FLOAT_TYPE fProgress, int iClipIndex)
{
	// See how many frames we have.
	if(iClipIndex < 0 || iClipIndex >= (int)myClips.size())
		return NULL;

	CachedClip3D* pClip = myClips[iClipIndex];
	return pClip->getFrameForProgress(fProgress);
}
/********************************************************************************************/
CachedClip3D* CachedObject3D::findClip(const char* pcsName, int* iClipIndexOut)
{
	int iCurr, iNum = myClips.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		if(strcmp(myClips[iCurr]->getTypeName(), pcsName) == 0)
		{
			if(iClipIndexOut)
				*iClipIndexOut = iCurr;
			return myClips[iCurr];
		}
	}

	if(iClipIndexOut)
		*iClipIndexOut = 0;
	return NULL;
}
/********************************************************************************************/
void CachedObject3D::onTimerTick(void)
{
	// See if we can get rid of any clips we have.
	GTIME lUniTime = Application::getInstance()->getGlobalTime(ClockUniversal);

	// No need to check too often
	if(lUniTime % 8 != 0)
		return;

	GTIME lClipUseTime;

	CachedClip3D* pClip;
	int iCurr, iNum = myClips.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pClip = myClips[iCurr];
		if(pClip->getIsLoaded() == false)
			continue;

		lClipUseTime = pClip->getLastUseTime();
		if(lClipUseTime + Application::secondsToTicks(CLIP_UNLOAD_TIME) < lUniTime)
			pClip->unloadData();
	}
}
/********************************************************************************************/
void CachedObject3D::getDiffuseMults(CachedPart3D* pPart, SColor& scolOut) 
{ 
	if(myDiffuseMults.r >= 0)
		scolOut = myDiffuseMults; 
	else
		pPart->getDiffuseMults(scolOut);
}
/********************************************************************************************/
void CachedObject3D::getAmbientMults(CachedPart3D* pPart, SColor& scolOut) 
{
	if(myAmbientMults.r >= 0)
		scolOut = myAmbientMults; 
	else
		pPart->getAmbientMults(scolOut);
}
/********************************************************************************************/
void CachedObject3D::getSpecularParms(CachedPart3D* pPart, SColor& scolOut, FLOAT_TYPE& fSpecPowerOut)
{
	if(mySpecular.r >= 0)
		scolOut = mySpecular; 
	else
		pPart->getSpecularMults(scolOut);

	if(mySpecularPower >= 1.0)
		fSpecPowerOut = mySpecularPower;
	else
		fSpecPowerOut = pPart->getSpecularPower();
}
/********************************************************************************************/
};