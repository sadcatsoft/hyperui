#include "stdafx.h"

namespace HyperUI
{
/*****************************************************************************/
CachedPart3D::CachedPart3D()
{
	myVertexCapacity = 0;
	myNumVerts = 0;
	myData = NULL;
	myTexture = NULL;
#ifdef ENABLE_VBOS
	myVBOHandle = 0;
#endif
	myNormalTexture = NULL;
	mySpecularTexture = NULL;
}
/*****************************************************************************/
CachedPart3D::~CachedPart3D()
{
	clear();
}
/*****************************************************************************/
void CachedPart3D::clear(void)
{
	if(myData)
		delete[] myData;
	myData = NULL;
	myNumVerts = 0;
	myVertexCapacity = 0;
	myTexture = NULL;
	myNormalTexture = NULL;
	mySpecularTexture = NULL;
}
/*****************************************************************************/
void CachedPart3D::ensureNumVertices(int iNumVerts, bool bPreserveContents)
{
	if(iNumVerts <= myVertexCapacity)
		return;

	if(bPreserveContents)
	{
		SVertex3D* pOldData = myData;
		myData = new SVertex3D[iNumVerts];
		if(pOldData && myNumVerts > 0)
			memcpy(myData, pOldData, sizeof(SVertex3D)*myNumVerts);
		memset(myData + myNumVerts, 0, sizeof(SVertex3D)*(iNumVerts - myNumVerts));
		delete[] pOldData;
	}
	else
	{
		if(myData)
			delete[] myData;
		myData = new SVertex3D[iNumVerts];
	}

	myVertexCapacity = iNumVerts;
}
/*****************************************************************************/
void CachedPart3D::initFromFaces(TObjFaceInfos& rFaces, SMaterialInfo* pMaterial, Window* pWindow, 
								 bool bFlipU, bool bFlipV, SVector3D& svDefaultScale, bool bFlipAllTris)
{

	// Now will in the faces
	this->ensureNumVertices(rFaces.size()*3, false);

	TextureManager *pTextureManager = pWindow->getTextureManager();
	myMaterial.reset();

	SVector3D svFlipSigns(1,1,1);
	if(svDefaultScale.x < 0)
		svFlipSigns.x = -1;
	if(svDefaultScale.y < 0)
		svFlipSigns.y = -1;
	if(svDefaultScale.z < 0)
		svFlipSigns.z = -1;

	SObjFaceInfo* pInfo;
	int iCurr, iNum = rFaces.size();
	int iComp;
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pInfo = &rFaces[iCurr];
		for(iComp = 0; iComp < 3; iComp++)
		{
			myData[iCurr*3 + iComp].x = pInfo->myVertIndex[iComp].x*svDefaultScale.x;
			myData[iCurr*3 + iComp].y = pInfo->myVertIndex[iComp].y*svDefaultScale.y;
			myData[iCurr*3 + iComp].z = pInfo->myVertIndex[iComp].z*svDefaultScale.z;
#ifdef USE_OPENGL2
			myData[iCurr*3 + iComp].w = 1.0;
#endif

			if(bFlipU)
				myData[iCurr*3 + iComp].u = 1.0 - pInfo->myTxIndex[iComp].x;
			else
				myData[iCurr*3 + iComp].u = pInfo->myTxIndex[iComp].x;
			if(bFlipV)
				myData[iCurr*3 + iComp].v = 1.0 - pInfo->myTxIndex[iComp].y;
			else
				myData[iCurr*3 + iComp].v = pInfo->myTxIndex[iComp].y;
			
			myData[iCurr*3 + iComp].nx = pInfo->myNormIndex[iComp].x*svFlipSigns.x;
			myData[iCurr*3 + iComp].ny = pInfo->myNormIndex[iComp].y*svFlipSigns.y;
			myData[iCurr*3 + iComp].nz = pInfo->myNormIndex[iComp].z*svFlipSigns.z;

		}
// getPolyOrientation
// Reverse

/*
		SVertex3D svTempVert;
		svTempVert = myData[iCurr*3 + 0];
myData[iCurr*3 + 0] = myData[iCurr*3 + 2];
myData[iCurr*3 + 2] = svTempVert;
*/

	}

	myNumVerts = rFaces.size()*3;

	if(svDefaultScale.x < 0 || svDefaultScale.y < 0 || svDefaultScale.z < 0)
	{
		// We may need to flip orientation of some triangles. Oh well...
		SVector3D svCenter;
		SVertex3D svTempVert;
		HyperCore::OrientationType eOrient;
		this->computeCentroid(svCenter);

		for(iCurr = 0; iCurr < myNumVerts; iCurr+=3)
		{
			eOrient = getPolyOrientation(&myData[iCurr], svCenter);
			if(eOrient == OrientationCW)
			{
				svTempVert = myData[iCurr + 0];
				myData[iCurr + 0] = myData[iCurr + 2];
				myData[iCurr + 2] = svTempVert;
			}
		}
	}

	if(bFlipAllTris)
	{
		// We may need to flip orientation of all triangles
		SVertex3D svTempVert;
		for(iCurr = 0; iCurr < myNumVerts; iCurr+=3)
		{
			{
				svTempVert = myData[iCurr + 0];
				myData[iCurr + 0] = myData[iCurr + 2];
				myData[iCurr + 2] = svTempVert;
			}
		}
	}


	// If we have a material, do something with the texture.
	// Note that we'll probably end up storing a pointer to the cached texture object or something.
	myCachedTextureIndex = 0;
	myCachedNormalTextureIndex = 0;
	myCachedSpecularTextureIndex = 0;
	myCachedBlendMode = BlendModeNormal;
	if(pMaterial)
	{
		myTexture = loadTextureInto(pTextureManager, pMaterial->myTexture, myTextureName, myCachedTextureIndex, &myCachedBlendMode);
#ifdef _DEBUG
		pcsDebugTextureName = myTextureName.c_str();
#endif
		// Load the normal texture, if any
		myNormalTexture = loadTextureInto(pTextureManager, pMaterial->myNormalTexture, myNormalTextureName, myCachedNormalTextureIndex, NULL);

		// Load the specular texture, if any
		mySpecularTexture = loadTextureInto(pTextureManager, pMaterial->mySpecularTexture, mySpecularTextureName, myCachedSpecularTextureIndex, NULL);

		// Add other material parameters
		myMaterial = *pMaterial;
	}

#ifdef ENABLE_NORMAL_MAPPING
	computeTangetsAndBitangents();
#endif
}
/*****************************************************************************/
void CachedPart3D::getBBox(SBBox3D& bbox_out)
{
	bbox_out.reset();

	SVector3D svPt;
	int iCurr;
	for(iCurr = 0; iCurr < myNumVerts; iCurr++)
	{
		svPt.set(myData[iCurr].x, myData[iCurr].y, myData[iCurr].z);
		bbox_out.addPoint(svPt);
	}
}
/*****************************************************************************/
void CachedPart3D::translate(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fZ)
{
	int iCurr;
	for(iCurr = 0; iCurr < myNumVerts; iCurr++)
	{
		myData[iCurr].x += fX;
		myData[iCurr].y += fY;
		myData[iCurr].z += fZ;
	}
}
/*****************************************************************************/
int CachedPart3D::getDataMemUsage(void)
{
	return sizeof(SVertex3D)*myNumVerts + sizeof(int);
}
/*****************************************************************************/
void CachedPart3D::copyInto(Window* pWindow, SObjInfo3D& rInfo, int iStartVertex, int iNumVerts)
{
	if(iStartVertex + iNumVerts > myNumVerts)
		iNumVerts = myNumVerts - iStartVertex;

	rInfo.copyAndRetainFrom(myData + iStartVertex, iNumVerts);

	int iAtlasFrame = -1;
	if(myTexture)
		rInfo.myTexture = pWindow->getDrawingCache()->getCachedSequence(myTexture->getName(), &iAtlasFrame);
	else
		rInfo.myTexture = NULL;
	_ASSERT(iAtlasFrame < 0);
}
/*****************************************************************************/
void CachedPart3D::copyFrom(CachedPart3D* pOther)
{
	int iOtherVerts = pOther->getNumVerts();
	ensureNumVertices(iOtherVerts, false);
	myNumVerts = iOtherVerts;
	memcpy(myData, pOther->getData(), sizeof(SVertex3D)*iOtherVerts);

#ifdef ENABLE_VBOS
	myVBOHandle = pOther->myVBOHandle;
#endif
	myName = pOther->getName();
	myCachedTextureIndex = pOther->myCachedTextureIndex;
	myCachedBlendMode = pOther->myCachedBlendMode;
	myTexture = pOther->myTexture;
	myTextureName = pOther->myTextureName;
#ifdef _DEBUG
	pcsDebugTextureName = myTextureName.c_str();
#endif

	myCachedNormalTextureIndex = pOther->myCachedNormalTextureIndex;
	myNormalTexture = pOther->myNormalTexture;
	myNormalTextureName = pOther->myNormalTextureName;

	myCachedSpecularTextureIndex = pOther->myCachedSpecularTextureIndex;
	mySpecularTexture = pOther->mySpecularTexture;
	mySpecularTextureName = pOther->mySpecularTextureName;

	myMaterial = pOther->myMaterial;
}
/*****************************************************************************/
bool CachedPart3D::loadFromClipFile(char* pData, int& iMemPos)
{
	int iNumVerts;

	memcpy(&iNumVerts, pData + iMemPos, sizeof(int));
	iMemPos += sizeof(int);

	ensureNumVertices(iNumVerts, true);

	// If this fails, we're trying to read an obj seq with
	// a varying number of vertices. The architecture would
	// support this, but since we're not saving UVs to our
	// sequence files, this assumes they're constant point
	// count. This is also why we can't simply resize.
	_ASSERT(iNumVerts <= myVertexCapacity);

	int iCurrVert;
	for(iCurrVert = 0; iCurrVert < iNumVerts; iCurrVert++)
	{
		memcpy(myData + iCurrVert, pData + iMemPos, sizeof(SVertex3DSaveCore));
		iMemPos += sizeof(SVertex3DSaveCore);
		/*
		SVector3D svTemp;
		SVertex3D* pCurrPtr = (myData + iCurrVert);
		svTemp.x = pCurrPtr->nx;
		svTemp.y = pCurrPtr->ny;
		svTemp.z = pCurrPtr->nz;
		svTemp.normalize();
		svTemp = svTemp * 40.0;
		pCurrPtr->nx = svTemp.x;
		pCurrPtr->ny = svTemp.y;
		pCurrPtr->nz = svTemp.z;
		 */
	}

	myNumVerts = iNumVerts;

	// Copy all
//	memcpy(myData, pData + iMemPos, sizeof(SVertex3D)*iNumVerts);
//	iMemPos += sizeof(SVertex3D)*iNumVerts;

	return true;
}
/*****************************************************************************/
HyperCore::OrientationType CachedPart3D::getPolyOrientation(SVertex3D* pData, SVector3D& svCenter)
{
	SVertex3D* svVert = pData;

	// Area is positive if we the polygin is CCW when looked at from the side pointed to by normals.

	FLOAT_TYPE area = 0;
//	FLOAT_TYPE an, ax, ay, az;  // abs value of normal and its coords
//	int   coord;           // coord to ignore: 1=x, 2=y, 3=z
	int   i; // , j, k;         // loop indices
//	int   safe_i, safe_j, safe_k;         // loop indices
	
	SVector3D svPolyNormal;

	SVector3D svVerts[3], svSum;
	svVerts[0].set(pData[0].x, pData[0].y, pData[0].z);
	svVerts[1].set(pData[1].x, pData[1].y, pData[1].z);
	svVerts[2].set(pData[2].x, pData[2].y, pData[2].z);

	SVector3D svAvgVert;
	svAvgVert = (svVerts[0] + svVerts[1] + svVerts[2])*(1.0/3.0);

/*
	svPolyNormal.x = svAvgVert.x - svCenter.x;
	svPolyNormal.y = svAvgVert.y - svCenter.y;
	svPolyNormal.z = svAvgVert.z - svCenter.z;
	svPolyNormal.normalize();
*/
	
	svPolyNormal.x = (pData[0].nx + pData[1].nx + pData[2].nx)/3.0;
	svPolyNormal.y = (pData[0].ny + pData[1].ny + pData[2].ny)/3.0;
	svPolyNormal.z = (pData[0].nz + pData[1].nz + pData[2].nz)/3.0;

	for(i = 0; i < 3; i++)
	{
		svSum = svSum + svVerts[i].cross(svVerts[(i + 1)%3]);
	}

	FLOAT_TYPE fRes = svSum.dot(svPolyNormal);
	if(fRes > 0)
		return OrientationCCW;
	else
		return OrientationCW;

#if 0
	// select largest abs coordinate to ignore for projection
	ax = (svPolyNormal.x>0 ? svPolyNormal.x : -svPolyNormal.x);     // abs x-coord
	ay = (svPolyNormal.y>0 ? svPolyNormal.y : -svPolyNormal.y);     // abs y-coord
	az = (svPolyNormal.z>0 ? svPolyNormal.z : -svPolyNormal.z);     // abs z-coord

	int n = 3;

	coord = 3;                     // ignore z-coord
	if (ax > ay) 
	{
		if (ax > az) coord = 1;    // ignore x-coord
	}
	else if (ay > az) coord = 2;   // ignore y-coord

	// compute area of the 2D projection
	for (i=1, j=2, k=0; i<=n; i++, j++, k++)
	{
		safe_i = i%3;
		safe_j = i%3;
		safe_k = i%3;
		switch (coord) 
		{
		case 1:
			area += (svVert[safe_i].y * (svVert[safe_j].z - svVert[safe_k].z));
			continue;
		case 2:
			area += (svVert[safe_i].x * (svVert[safe_j].z - svVert[safe_k].z));
			continue;
		case 3:
			area += (svVert[safe_i].x * (svVert[safe_j].y - svVert[safe_k].y));
			continue;
		}
	}
/*
	// scale to get area before projection
	an = F_SQRT( ax*ax + ay*ay + az*az);  // length of normal vector
	switch (coord) {
	case 1:
		area *= (an / (2*ax));
		break;
	case 2:
		area *= (an / (2*ay));
		break;
	case 3:
		area *= (an / (2*az));
	}
	return area;
	*/
/*
	SVector2D svTemp;

	int i,j;
	double dTotalArea=0.0;

	for (i = 0; i < 3; i++) 
	{
		j = (i+1)%iNumVertices;
		dTotalArea += rPoints[i].x * rPoints[j].y;
		dTotalArea -= rPoints[i].y * rPoints[j].x;
	}
	if(dTotalArea>=0)
		return OrientationClockwise;
	else
		return OrientationCounterClockwise;
		*/
#endif
}
/*****************************************************************************/
void CachedPart3D::computeCentroid(SVector3D& svCenterOut)
{
	svCenterOut.set(0,0,0);
	int iCurr;
	for(iCurr = 0; iCurr < myNumVerts; iCurr++)
	{
		svCenterOut.x += myData[iCurr].x;
		svCenterOut.y += myData[iCurr].y;
		svCenterOut.z += myData[iCurr].z;
	}

	svCenterOut = svCenterOut*(1.0/(FLOAT_TYPE)myNumVerts);
}
/*****************************************************************************/
void CachedPart3D::ensureOrientation(SVector3D& svModelCenter)
{

	SVertex3D svTempVert;
	HyperCore::OrientationType eOrient;

	int iCurr;
	for(iCurr = 0; iCurr < myNumVerts; iCurr+=3)
	{
		eOrient = getPolyOrientation(&myData[iCurr], svModelCenter);
		if(eOrient == OrientationCW)
		{
			svTempVert = myData[iCurr + 0];
			myData[iCurr + 0] = myData[iCurr + 2];
			myData[iCurr + 2] = svTempVert;
		}
	}

}
/*****************************************************************************/
void CachedPart3D::ensureTexturesLoaded(TextureManager* pTextureManager)
{
	myTexture = ensureTextureLoadedInternal(pTextureManager, myTexture, myTextureName, myCachedTextureIndex, &myCachedBlendMode);
	myNormalTexture = ensureTextureLoadedInternal(pTextureManager, myNormalTexture, myNormalTextureName, myCachedNormalTextureIndex, NULL);
	mySpecularTexture = ensureTextureLoadedInternal(pTextureManager, mySpecularTexture, mySpecularTextureName, myCachedSpecularTextureIndex, NULL);
}
/*****************************************************************************/
FLOAT_TYPE CachedPart3D::getTotalArea(int iStartVertex, int iNumVerts)
{
	if(iNumVerts <= 0)
		iNumVerts = myNumVerts;

	if(iStartVertex + iNumVerts > myNumVerts)
		iNumVerts = myNumVerts - iStartVertex;

	FLOAT_TYPE fTotalArea = 0;
	int iVert;
	SVector3D svPoints[3];
	for(iVert = iStartVertex; iVert < iStartVertex + iNumVerts; iVert += 3)
	{
		svPoints[0].set(myData[iVert].x, myData[iVert].y, myData[iVert].z);
		svPoints[1].set(myData[iVert + 1].x, myData[iVert + 1].y, myData[iVert + 1].z);
		svPoints[2].set(myData[iVert + 2].x, myData[iVert + 2].y, myData[iVert + 2].z);
#ifdef _DEBUG
		FLOAT_TYPE ftemp = fabs(GeometryUtils3D::computeTriangleArea(svPoints[0], svPoints[1], svPoints[2]));
		_ASSERT(isValidNumber(ftemp));
#endif
		fTotalArea += fabs(GeometryUtils3D::computeTriangleArea(svPoints[0], svPoints[1], svPoints[2]));
	}

	return fTotalArea;
}
/*****************************************************************************/
void CachedPart3D::computeTangetsAndBitangents()
{
#ifdef ENABLE_NORMAL_MAPPING
	// First, shove all the triangles into the cacher
	TriangleCacher rTriCacher;
	int iVert;
	for(iVert = 0; iVert < myNumVerts; iVert += 3)
		rTriCacher.addTriangle(myData[iVert], myData[iVert + 1], myData[iVert + 2]);
	
	// Now, go over them again, getting the resultant triangles for every vert.
	// Notice that we do a lot more work here than necessary...

	STriCacherElement* pTri;
	TTriangleResMap::iterator mi;
	TTriangleResMap rTrisOut;
	SVector3D svTangent, svBitangent;
	SVector3D svAccumTangent, svAccumBitangent;
	int iTriCount;
	SVector3D svNormal;
	for(iVert = 0; iVert < myNumVerts; iVert++)
	{
		rTriCacher.findTrianglesForVertex(myData[iVert].x, myData[iVert].y, myData[iVert].z, rTrisOut);
		_ASSERT(rTrisOut.size() > 0);

		// Now, go over all tris
		iTriCount = 0;
		svAccumTangent.set(0,0,0);
		svAccumBitangent.set(0,0,0);
		svNormal.set(myData[iVert].nx, myData[iVert].ny, myData[iVert].nz);
		for(mi = rTrisOut.begin(); mi != rTrisOut.end(); mi++)
		{
			pTri = &mi->second;
			GeometryUtils3D::computeTangentAndBiTangent(pTri->myTriangle.myPoints, svNormal, pTri->myTriangle.myU, pTri->myTriangle.myV, svTangent, svBitangent);

			svAccumBitangent += svBitangent;
			svAccumTangent += svTangent;
			iTriCount++;
		}

		if(iTriCount > 0)
		{
			svAccumTangent *= 1.0/(FLOAT_TYPE)iTriCount;
			svAccumBitangent *= 1.0/(FLOAT_TYPE)iTriCount;
		}

		myData[iVert].tangent_x = svAccumTangent.x;
		myData[iVert].tangent_y = svAccumTangent.y;
		myData[iVert].tangent_z = svAccumTangent.z;

		myData[iVert].bitangent_x = svAccumBitangent.x;
		myData[iVert].bitangent_y = svAccumBitangent.y;
		myData[iVert].bitangent_z = svAccumBitangent.z;
	}
#else
	_ASSERT(0);
#endif
}
/*****************************************************************************/
TextureAnimSequence* CachedPart3D::loadTextureInto(TextureManager *pTextureManager, string& strInputPath, string& strTextureNameOut, TX_MAN_RETURN_TYPE& rCachedIndexOut, BlendModeType* eBlendModeOut)
{
	strTextureNameOut = "";

	if(strInputPath.length() == 0)
		return NULL;

	TextureAnimSequence* pRes = NULL;
	string strNoExtFilename;

	int iDotPos = strInputPath.rfind(".");
	if(iDotPos != string::npos)
		strNoExtFilename = strInputPath.substr(0, iDotPos);
	else
		strNoExtFilename = strInputPath;

	// The last four numbers must be digits
	_ASSERT(strNoExtFilename.length() > 4);

#ifdef _DEBUG
	const char* pcsName = strNoExtFilename.c_str();
#endif
	_ASSERT(strNoExtFilename[strNoExtFilename.length() - 1] == '1');
	_ASSERT(strNoExtFilename[strNoExtFilename.length() - 2] == '0');
	_ASSERT(strNoExtFilename[strNoExtFilename.length() - 3] == '0');
	_ASSERT(strNoExtFilename[strNoExtFilename.length() - 4] == '0');
		
	strNoExtFilename = strNoExtFilename.substr(0, strNoExtFilename.length() - 4);

	// Now, convert to relative path
	int iSlashPos = strNoExtFilename.rfind("\\");
	if(iSlashPos != string::npos)
	{
		strNoExtFilename = strNoExtFilename.substr(iSlashPos + 1);
	}

	// Also, take out any digits we may have at the front for resolution.
	if(isdigit(strNoExtFilename[0]))
	{
		int iDigitPos, iLen = strNoExtFilename.length();
		for(iDigitPos = 0; iDigitPos < iLen && isdigit(strNoExtFilename[iDigitPos]); iDigitPos++);

		if(iDigitPos < iLen)
		{
			_ASSERT(iDigitPos + 1 < iLen);
			_ASSERT(strNoExtFilename[iDigitPos] == '_');
			strNoExtFilename = strNoExtFilename.substr(iDigitPos + 1);
		}
	}

	// Find our texture type name from animations. It must have been loaded already.
	ResourceCollection* pColl = ResourceManager::getInstance()->getCollection(ResourceAnimations);
	ResourceItem* pTextureDef = pColl->findItemWithPropValue(PropertyFile, strNoExtFilename.c_str(), false);

	if(!pTextureDef)
	{
		// Forgot to add texture to the .txt files, or model in prelim while
		// the texture is not.
		// Note that the texture name is caps-sensitive.
		_ASSERT(0);
	}
	else
	{
		// We need the type name of this texture
		strTextureNameOut = pTextureDef->getStringProp(PropertyId);
		_ASSERT(strTextureNameOut.length() > 0);

		int iW, iH;
		SUVSet uvsOut;
		BlendModeType eBlend;
		rCachedIndexOut = pTextureManager->getTextureIndex(strTextureNameOut.c_str(), 1, iW, iH, uvsOut, eBlend);	
		if(eBlendModeOut)
			*eBlendModeOut = eBlend;
		pRes = pTextureManager->getTexture(strTextureNameOut.c_str());
	}		

	return pRes;
}
/*****************************************************************************/
TextureAnimSequence* CachedPart3D::ensureTextureLoadedInternal(TextureManager *pTextureManager, TextureAnimSequence *pCurrTexture, string& strTextureName, TX_MAN_RETURN_TYPE& rCachedIndexOut, BlendModeType* eBlendModeOut)
{
	if(strTextureName.length() <= 0)
		return NULL;

	if(!pCurrTexture)
		pCurrTexture = pTextureManager->getTexture(strTextureName.c_str());

	if(!pCurrTexture)
		return NULL;

	pCurrTexture->ensureLoaded();
	pCurrTexture->markUsed();

	// Note that we have to overwrite our cached texture index, since it probably changed.
	int iW, iH;
	SUVSet uvsOut;
	BlendModeType eBlend;
	rCachedIndexOut = pTextureManager->getTextureIndex(pCurrTexture->getName(), 1, iW, iH, uvsOut, eBlend);	
	if(eBlendModeOut)
		*eBlendModeOut = eBlend;

	return pCurrTexture;
}
/*****************************************************************************/
void CachedPart3D::createVBOOjbect()
{
#ifdef ENABLE_VBOS
	_ASSERT(myVBOHandle == 0);

	glGenBuffers(1, &myVBOHandle);
	_ASSERT(myVBOHandle > 0);
	glBindBuffer(GL_ARRAY_BUFFER, myVBOHandle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(SVertex3D)*myNumVerts, &myData[0].x, GL_STATIC_DRAW);

#endif
}
/*****************************************************************************/
void CachedPart3D::destroyVBOObject()
{
#ifdef ENABLE_VBOS
	 glDeleteBuffers(1, &myVBOHandle);
	 myVBOHandle = 0;
#endif
}
/*****************************************************************************/
};