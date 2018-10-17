#include "stdafx.h"

// #define VS_DEBUG_CLIP_LOADING

namespace HyperUI
{
#ifndef MAC_BUILD
string CachedClip3D::theCommonString;
string CachedClip3D::theCommonString2;
#endif
/********************************************************************************************/
CachedClip3D::CachedClip3D()
{
	myIsLoaded = false;	

	myLoadFromClip = false;
	myWindow = NULL;
	myAutoCenter = false;
	myFlipAllTris = false;
	myFlipU = false;
	myFlipV = false;
	myClipItem = NULL;
	myMainClip = NULL;
	myDuration = -1;

	myLastUseTime = 0;
}
/********************************************************************************************/
CachedClip3D::~CachedClip3D()
{
	clear();
}
/********************************************************************************************/
void CachedClip3D::clear(void)
{
	unloadData();
	myName = "";
	myDuration = -1;
}
/********************************************************************************************/
void CachedClip3D::setLoadInfo(bool bLoadFromClip, const char* pcsNameToken, Window* pWindow, bool bAutoCenter, bool bFlipU, bool bFlipV, ResourceItem* pClipItem, SVector3D& svDefaultScale, CachedClip3D* pMainClip, bool bFlipAllTris)
{
	myLoadFromClip = bLoadFromClip;
	myFileName = pcsNameToken;
	myWindow = pWindow;
	myAutoCenter = bAutoCenter;
	myFlipU = bFlipU;
	myFlipV = bFlipV;
	myFlipAllTris = bFlipAllTris;
	myClipItem = pClipItem;
	myDefaultScale = svDefaultScale;
	myMainClip = pMainClip;
}
/********************************************************************************************/
bool CachedClip3D::loadFromClipFile(const char* pcsNameToken, Window* pWindow, bool bAutoCenter, bool bFlipU, bool bFlipV, ResourceItem* pClipItem, CachedClip3D* pMainClip)
{
	int iBytesRead;
	char* pData;

	string strFileName;
	strFileName = pcsNameToken; 
	strFileName += ".clp";

#ifdef WIN32
	strFileName = "graphics3d\\" + strFileName;
#endif

	_ASSERT(pClipItem);

	myDuration = pClipItem->getNumProp(PropertyObj3dClipDuration);
	_ASSERT(myDuration > 0);
	myName = pClipItem->getStringProp(PropertyId);

	pData = FileUtils::loadFromFileBinary(strFileName.c_str(), FileSourcePackageDir, iBytesRead);
	_ASSERT(pData && iBytesRead > 0);
	if(!pData || iBytesRead <= 0)
		return false;

	// The first number is the number of frames
	int iCurrFrame;
	int iNumFrames;
//	int iNumVerts;
	int iMemPos = 0;

	CachedFrame3D *pNewFrame;
	memcpy(&iNumFrames, pData + iMemPos, sizeof(int));
	iMemPos += sizeof(int);

	CachedFrame3D* pMainFrame = pMainClip->getFrame(0);
	for(iCurrFrame = 0; iCurrFrame < iNumFrames; iCurrFrame++)
	{
		pNewFrame = new CachedFrame3D;
		pNewFrame->loadFromClipFile(pData, iMemPos, pMainFrame);
		myFrames.push_back(pNewFrame);
	}

	delete[] pData;

	myIsLoaded = true;
	return true;
}
/********************************************************************************************/
bool CachedClip3D::loadFromObjFile(const char* pcsNameToken, Window* pWindow, bool bAutoCenter, bool bFlipU, bool bFlipV, ResourceItem* pClipItem, SVector3D& svDefaultScale, bool bFlipAllTris)
{
	if(!pcsNameToken)
		return false;

	string strFileName;

	clear();

	// When adding anything here, make sure it's copied in CachedClip3D::loadFromClipFile();
	if(pClipItem)
	{
		myDuration = pClipItem->getNumProp(PropertyObj3dClipDuration);
		_ASSERT(myDuration > 0);
		myName = pClipItem->getStringProp(PropertyId);
	}
	else
	{
		myDuration = 0;
		myName = "";
	}

	// Iterate over frames to see how many we have. We should have at least one.
	int iCurrFrame;
	char pcsBuff[1024];
	CachedFrame3D* pNewFrame;
	for(iCurrFrame = 0; true; iCurrFrame++)
	{
		// See if it exists
		sprintf(pcsBuff, "%s%.4d", pcsNameToken, iCurrFrame + 1);
		strFileName = pcsBuff;

#ifdef ALLOW_ENCRYPTION
		strFileName += ".ebj";
#else
		strFileName += ".obj";
#endif

#ifdef WIN32
		strFileName = "graphics3d\\" + strFileName;
#endif


		if(FileUtils::doesFileExist(strFileName.c_str(), FileSourcePackageDir) == false)
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
		pNewFrame->loadFromObjFile(strFileName.c_str(), pWindow, bFlipU, bFlipV, svDefaultScale, bFlipAllTris);
		myFrames.push_back(pNewFrame);
	};

	_ASSERT(myFrames.size() > 0);

	myIsLoaded = true;

	// Do it here since all frames have to be centered by the same location.
	if(bAutoCenter)
		centerOnOrigin();

	return true;
}
/********************************************************************************************/
void CachedClip3D::centerOnOrigin(void)
{
	_ASSERT(myIsLoaded);
	SBBox3D sbbOwnBox;
	this->getBBox(sbbOwnBox);

	SVector3D svTranslations;
	svTranslations = (sbbOwnBox.myMax + sbbOwnBox.myMin)*0.5;

	// Now, translate all the frames
	int iCurr, iNum = myFrames.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		myFrames[iCurr]->translate(-svTranslations.x, -svTranslations.y, -svTranslations.z);
	}
}
/********************************************************************************************/
void CachedClip3D::getBBox(SBBox3D& bbox_out)
{
	_ASSERT(myIsLoaded);
	SBBox3D bbox_temp;
	bbox_out.reset();
	if(myFrames.size() <= 0)
		return;

	myFrames[0]->getBBox(bbox_out);
}
/********************************************************************************************/
CachedFrame3D* CachedClip3D::getFrameForProgress(FLOAT_TYPE fProgress)
{
	if(!myIsLoaded)
		ensureIsLoaded();

	// See how many frames we have.
	int iNumFrames = myFrames.size();
	int iFrame = ((FLOAT_TYPE)(iNumFrames - 1))*fProgress;
//	if(iFrame >= iNumFrames)
//		iFrame = iNumFrames - 1;	
	myLastUseTime =  Application::getInstance()->getGlobalTime(ClockUniversal);
	return myFrames[iFrame];
}
/********************************************************************************************/
int CachedClip3D::getDataMemUsage(void)
{
	int iRes = 0;
	int iCurr, iNum = myFrames.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		iRes += myFrames[iCurr]->getDataMemUsage();
		iRes += sizeof(int);
	}

	return iRes;
}
/********************************************************************************************/
void CachedClip3D::ensureIsLoaded(void)
{
	if(myIsLoaded)
		return;

#ifdef VS_DEBUG_CLIP_LOADING
#ifdef _DEBUG
	char pcsBuff[1024];
	sprintf(pcsBuff,  "Loaded 3D CLIP: %s\n", myFileName.c_str());
	OutputDebugString(pcsBuff);
#endif
#endif

	if(myLoadFromClip)
	{
		loadFromClipFile(myFileName.c_str(), myWindow, myAutoCenter, myFlipU, myFlipV, myClipItem, myMainClip);
	}
	else
	{
		loadFromObjFile(myFileName.c_str(), myWindow, myAutoCenter, myFlipU, myFlipV, myClipItem, myDefaultScale, myFlipAllTris);
	}

	myLastUseTime =  Application::getInstance()->getGlobalTime(ClockUniversal);
}
/********************************************************************************************/
void CachedClip3D::unloadData(void)
{
#ifdef VS_DEBUG_CLIP_LOADING
#ifdef _DEBUG
	char pcsBuff[1024];
	sprintf(pcsBuff,  "Unloaded 3D CLIP: %s\n", myFileName.c_str());
	OutputDebugString(pcsBuff);
#endif
#endif

	int iCurr, iNum = myFrames.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
		delete myFrames[iCurr];
	myFrames.clear();

	myLastUseTime = 0;
	myIsLoaded = false;
}
/********************************************************************************************/
FLOAT_TYPE CachedClip3D::getDuration(void)
{
/*
	if(!myIsLoaded)
		ensureIsLoaded();

	myLastUseTime =  Application::getInstance()->getGlobalTime(ClockUniversal);
	*/
	_ASSERT(myClipItem || !myLoadFromClip);
	if(myDuration < 0)
	{
		if(myClipItem)
			myDuration = myClipItem->getNumProp(PropertyObj3dClipDuration);
		else
			myDuration = 0;
	}

	return myDuration;
}
/********************************************************************************************/
const char* CachedClip3D::getTypeName(void)
{
//	if(!myIsLoaded)
//		ensureIsLoaded();
//	myLastUseTime =  Application::getInstance()->getGlobalTime(ClockUniversal);
	_ASSERT(myClipItem || !myLoadFromClip);
	if(myName.length() <= 0)
	{
		if(myClipItem)
			myName = myClipItem->getStringProp(PropertyId);
		else
			myName = "";
	}
	return myName.c_str();
}
/********************************************************************************************/
int CachedClip3D::getNumFrames(void) 
{ 
	if(!myIsLoaded)
		ensureIsLoaded();
	myLastUseTime =  Application::getInstance()->getGlobalTime(ClockUniversal);
	return myFrames.size(); 
}
/********************************************************************************************/
CachedFrame3D* CachedClip3D::getFrame(int iIndex) 
{ 
	if(!myIsLoaded)
		ensureIsLoaded();
	myLastUseTime =  Application::getInstance()->getGlobalTime(ClockUniversal);

	return myFrames[iIndex]; 
}
/********************************************************************************************/
};