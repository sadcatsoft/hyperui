#include "stdafx.h"

namespace HyperUI
{
#ifndef MAC_BUILD
string CachedFrame3D::theCommonString;
string CachedFrame3D::theCommonString2;
string CachedFrame3D::theCommonString3;
#endif
/********************************************************************************************/
CachedFrame3D::CachedFrame3D()
{

}
/********************************************************************************************/
CachedFrame3D::~CachedFrame3D()
{
	clear();
}
/********************************************************************************************/
void CachedFrame3D::clear(void)
{
	int iCurr, iNum = myParts.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
		delete myParts[iCurr];
	myParts.clear();
}
/********************************************************************************************/
int CachedFrame3D::getNumParts(void)
{
	return myParts.size();
}
/********************************************************************************************/
CachedPart3D* CachedFrame3D::getPart(int iPart)
{
	if(iPart >= 0 && iPart < (int)myParts.size())
		return myParts[iPart];
	else
		return NULL;
}
/********************************************************************************************/
CachedPart3D* CachedFrame3D::addPart(const char* pcsName)
{
	CachedPart3D* pPart = new CachedPart3D;
	if(pcsName && strlen(pcsName) > 0)
		pPart->setName(pcsName);
	myParts.push_back(pPart);
	return pPart;
}
/********************************************************************************************/
CachedPart3D* CachedFrame3D::addPartFromFaces(const char* pcsName, TObjFaceInfos& rFaces, SMaterialInfo* pMaterial, Window* pWindow, bool bFlipU, bool bFlipV, SVector3D& svDefaultScale, bool bFlipAllTris)
{
	CachedPart3D* pPart = addPart(pcsName);
	pPart->initFromFaces(rFaces, pMaterial, pWindow, bFlipU, bFlipV, svDefaultScale, bFlipAllTris);
	return pPart;
}
/********************************************************************************************/
SMaterialInfo* CachedFrame3D::findMaterial(const char* pcsName)
{
	TStringMatInfoMap::iterator mi = myMaterials.find(pcsName);
	if(mi != myMaterials.end())
		return &mi->second;
	else
		return NULL;
}
/********************************************************************************************/
bool CachedFrame3D::loadMaterials(const char* pcsName)
{
	theCommonString3 = pcsName;
	// Find the dot, replace the next character with an "e" 
	// if we have encryption
#ifdef ALLOW_ENCRYPTION
	int iDotPos = theCommonString3.rfind(".");
	_ASSERT(iDotPos != string::npos);
	if(iDotPos != string::npos)
	{
		theCommonString3 = theCommonString3.substr(0, iDotPos + 1) + "e" + theCommonString3.substr(iDotPos + 2);
	}
#endif

#ifdef ALLOW_ENCRYPTION
	bool bEncrypted = true;
#else
	bool bEncrypted = false;
#endif
	FileUtils::loadFromFile(theCommonString3.c_str(), theCommonString2, FileSourcePackageDir);
	if(theCommonString2.length() <= 0)
		return false;

	/*
#ifdef ALLOW_ENCRYPTION
	// Decrypt
	theCommonString3 = theCommonString2;
	StringCoder::decode(theCommonString3, theCommonString2);
#endif
	*/

	TCharPtrVector  rLines, rSingleLine;
	//TokenizeString(theCommonString2, "\r\n", rLines);
	TokenizeUtils::tokenizeStringToCharPtrsInPlace(theCommonString2, "\r\n", rLines);
	int iCurrLine, iNumLines = rLines.size();
	string strCurrMatName;
	SMaterialInfo rInfo;
	for(iCurrLine = 0; iCurrLine < iNumLines; iCurrLine++)
	{
		//TokenizeString(rLines[iCurrLine], " \r\t/", rSingleLine);
		TokenizeUtils::tokenizeStringToCharPtrsInPlace(rLines[iCurrLine], " \r\t/", rSingleLine);


		// Unrecognzied or empty line
		if(rSingleLine.size() < 2)
			continue;

		// Comment
		if(rSingleLine[0][0] == '#')
			continue;

		//if(rSingleLine[0] == "newmtl")
		if(strcmp(rSingleLine[0], "newmtl") == 0)
		{
			strCurrMatName = rSingleLine[1];
		}
		// else if(rSingleLine[0] == "map_Kd")
		else if(strcmp(rSingleLine[0], "map_Kd") == 0)
		{
			rInfo.myName = strCurrMatName;
			rInfo.myTexture = rSingleLine[1];
			myMaterials[strCurrMatName] = rInfo;
		}
	}
	return true;
}
/********************************************************************************************/
bool CachedFrame3D::loadFromObjFile(const char* pcsName, Window* pWindow,
									 bool bFlipU, bool bFlipV, SVector3D& svDefaultScale, bool bFlipAllTris)
{
#ifdef ALLOW_ENCRYPTION
	bool bEncrypted = true;
#else
	bool bEncrypted = false;
#endif

	FileUtils::loadFromFile(pcsName, theCommonString, FileSourcePackageDir);
	if(theCommonString.length() <= 0)
		return false;
/*
#ifdef ALLOW_ENCRYPTION
	// Decrypt
	theCommonString2 = theCommonString;
	StringCoder::decode(theCommonString2, theCommonString);
#endif
	*/

	string strPath, strFullName(pcsName);
	/*
if(strFullName.find("marine") != string::npos)
{
	int bp = 0;
} */
	int iSlashPos = strFullName.rfind("\\");
	if(iSlashPos != string::npos)
		strPath = strFullName.substr(0, iSlashPos + 1);

	string strMatFileName;
	SObjFaceInfo rFaceInfo;

	TObjFaceInfos vecFaces;
	TPoint3DVector vecCurrVerts, vecCurrNormals, vecCurrTxCoords;

	myMaterials.clear();

	//TStringVector rLines;
	TCharPtrVector rLines;
	TCharPtrVector rSingleLine;
	//TokenizeString(theCommonString, "\r\n", rLines);
	TokenizeUtils::tokenizeStringToCharPtrsInPlace(theCommonString, "\r\n", rLines);
	int iCurrLine, iNumLines = rLines.size();
	bool bDidBeginNewGroup = false;
	string strCurrGroupName, strObjMaterialName;
	SVector3D svPoint;
	for(iCurrLine = 0; iCurrLine < iNumLines; iCurrLine++)
	{
		TokenizeUtils::tokenizeStringToCharPtrsInPlace(rLines[iCurrLine], " \t\r/", rSingleLine);
////		TokenizeString(rLines[iCurrLine], " \t\r/", rSingleLine);

		// Unrecognzied or empty line
		if(rSingleLine.size() < 2)
			continue;

		// Comment
		if(rSingleLine[0][0] == '#')
			continue;

		if(vecFaces.size() > 0 && strcmp(rSingleLine[0], "f") != 0 && strcmp(rSingleLine[0], "s") != 0)
		{
			this->addPartFromFaces(strCurrGroupName.c_str(), vecFaces, findMaterial(strObjMaterialName.c_str()), pWindow, bFlipU, bFlipV, svDefaultScale, bFlipAllTris);
			vecFaces.clear();
		}

		//if(rSingleLine[0] == "mtllib")
		if(strcmp(rSingleLine[0], "mtllib") == 0)
		{
			// Material library
			strMatFileName = rSingleLine[1];
			strMatFileName = strPath + strMatFileName;
			loadMaterials(strMatFileName.c_str());
		}
		//else if(rSingleLine[0] == "g")
		else if(strcmp(rSingleLine[0], "g") == 0)
		{
			strObjMaterialName = "";
			bDidBeginNewGroup = true;
			strCurrGroupName = rSingleLine[1];
			vecFaces.clear();
		}
		//else if(rSingleLine[0] == "v")
		else if(strcmp(rSingleLine[0], "v") == 0)
		{
/*
			if(bDidBeginNewGroup)
			{
				vecCurrTxCoords.clear();
				vecCurrVerts.clear();
				vecCurrNormals.clear();
			}
*/
			// Vertices
			//svPoint.set(atof(rSingleLine[1].c_str()), atof(rSingleLine[2].c_str()), atof(rSingleLine[3].c_str()));
			svPoint.set(atof(rSingleLine[1]), atof(rSingleLine[2]), atof(rSingleLine[3]));
			vecCurrVerts.push_back(svPoint);
		}
		//else if(rSingleLine[0] == "vt")
		else if(strcmp(rSingleLine[0], "vt") == 0)
		{
			/*
			if(bDidBeginNewGroup)
			{
				vecCurrTxCoords.clear();
				vecCurrVerts.clear();
				vecCurrNormals.clear();
			}
			*/
			//svPoint.set(atof(rSingleLine[1].c_str()), atof(rSingleLine[2].c_str()), 0.0);
			svPoint.set(atof(rSingleLine[1]), atof(rSingleLine[2]), 0.0);
			vecCurrTxCoords.push_back(svPoint);
		}
		//else if(rSingleLine[0] == "vn")
		else if(strcmp(rSingleLine[0], "vn") == 0)
		{
			/*
			if(bDidBeginNewGroup)
			{
				vecCurrTxCoords.clear();
				vecCurrVerts.clear();
				vecCurrNormals.clear();
			}
			*/
			//svPoint.set(atof(rSingleLine[1].c_str()), atof(rSingleLine[2].c_str()), atof(rSingleLine[3].c_str()));
			svPoint.set(atof(rSingleLine[1]), atof(rSingleLine[2]), atof(rSingleLine[3]));
			vecCurrNormals.push_back(svPoint);
		}
//		else if(rSingleLine[0] == "usemtl")
		else if(strcmp(rSingleLine[0], "usemtl") == 0)
		{
			// Object material
			strObjMaterialName = rSingleLine[1];
		}
		//else if(rSingleLine[0] == "f")
		else if(strcmp(rSingleLine[0], "f") == 0)
		{
			// We only support models with normals and tex coords,
			// and only triangles.
			_ASSERT(rSingleLine.size() == 10);

			rFaceInfo.myVertIndex[0] = vecCurrVerts[atoi(rSingleLine[1]) - 1];
			rFaceInfo.myTxIndex[0] = vecCurrTxCoords[atoi(rSingleLine[2]) - 1];
			rFaceInfo.myNormIndex[0] = vecCurrNormals[atoi(rSingleLine[3]) - 1];

			rFaceInfo.myVertIndex[1] = vecCurrVerts[atoi(rSingleLine[4]) - 1];
			rFaceInfo.myTxIndex[1] = vecCurrTxCoords[atoi(rSingleLine[5]) - 1];
			rFaceInfo.myNormIndex[1] = vecCurrNormals[atoi(rSingleLine[6]) - 1];

			rFaceInfo.myVertIndex[2] = vecCurrVerts[atoi(rSingleLine[7]) - 1];
			rFaceInfo.myTxIndex[2] = vecCurrTxCoords[atoi(rSingleLine[8]) - 1];
			rFaceInfo.myNormIndex[2] = vecCurrNormals[atoi(rSingleLine[9]) - 1];

/*
			// Geo info
			rFaceInfo.myVertIndex[0] = vecCurrVerts[atoi(rSingleLine[1].c_str()) - 1];
			rFaceInfo.myTxIndex[0] = vecCurrTxCoords[atoi(rSingleLine[2].c_str()) - 1];
			rFaceInfo.myNormIndex[0] = vecCurrNormals[atoi(rSingleLine[3].c_str()) - 1];

			rFaceInfo.myVertIndex[1] = vecCurrVerts[atoi(rSingleLine[4].c_str()) - 1];
			rFaceInfo.myTxIndex[1] = vecCurrTxCoords[atoi(rSingleLine[5].c_str()) - 1];
			rFaceInfo.myNormIndex[1] = vecCurrNormals[atoi(rSingleLine[6].c_str()) - 1];

			rFaceInfo.myVertIndex[2] = vecCurrVerts[atoi(rSingleLine[7].c_str()) - 1];
			rFaceInfo.myTxIndex[2] = vecCurrTxCoords[atoi(rSingleLine[8].c_str()) - 1];
			rFaceInfo.myNormIndex[2] = vecCurrNormals[atoi(rSingleLine[9].c_str()) - 1];
*/
			// Mash it all together.
			vecFaces.push_back(rFaceInfo);
		}

		//if(rSingleLine[0] != "g")
		if(strcmp(rSingleLine[0], "g") != 0)
			bDidBeginNewGroup = false;
	}

	if(vecFaces.size() > 0)
	{
		this->addPartFromFaces(strCurrGroupName.c_str(), vecFaces, findMaterial(strObjMaterialName.c_str()), pWindow, bFlipU, bFlipV, svDefaultScale, bFlipAllTris);
		vecFaces.clear();
	}
/*
	if(svDefaultScale.x < 0 || svDefaultScale.y < 0 || svDefaultScale.z < 0)
	{
		SVector3D svCenter;
		this->computeCentroid(svCenter);
		int iCurr, iNum = myParts.size();
		for(iCurr = 0; iCurr < iNum; iCurr++)
		{
			//myParts[iCurr]->ensureOrientation(svCenter);
		}
	}
*/	

	return true;
}
/********************************************************************************************/
void CachedFrame3D::centerOnOrigin(void)
{
	SBBox3D sbbOwnBox;
	this->getBBox(sbbOwnBox);

	SVector3D svTranslations;
	svTranslations = (sbbOwnBox.myMax + sbbOwnBox.myMin)*0.5;

	// Now, translate all the parts
	int iCurr, iNum = myParts.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		myParts[iCurr]->translate(-svTranslations.x, -svTranslations.y, -svTranslations.z);
	}
}
/********************************************************************************************/
void CachedFrame3D::getBBox(SBBox3D& bbox_out)
{
	SBBox3D bbox_temp;

	bbox_out.reset();

	int iCurr, iNum = myParts.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		myParts[iCurr]->getBBox(bbox_temp);
		bbox_out.addBBox(bbox_temp.myMin, bbox_temp.myMax);
	}
}
/********************************************************************************************/
void CachedFrame3D::translate(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fZ)
{
	int iCurr, iNum = myParts.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		myParts[iCurr]->translate(fX, fY, fZ);
	}
}
/********************************************************************************************/
int CachedFrame3D::getDataMemUsage(void)
{
	int iRes = 0;
	int iCurr, iNum = myParts.size();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		iRes += myParts[iCurr]->getDataMemUsage();
		// For size of that part
		iRes += sizeof(int);
	}

	return iRes;
}
/********************************************************************************************/
bool CachedFrame3D::loadFromClipFile(char* pData, int& iMemPos, CachedFrame3D* pMainFrame)
{
	int iCurrPart, iNumParts;
	memcpy(&iNumParts, pData + iMemPos, sizeof(int));
	iMemPos += sizeof(int);

	this->copyFrom(pMainFrame);

	// If we have more parts in this frame, load them.
	CachedPart3D* pPart;
	char pcsBuff[128];
	while(myParts.size() < iNumParts)
	{
		sprintf(pcsBuff, "extraPart%d", myParts.size());
		pPart = addPart(pcsBuff);
		pPart->copyFrom(myParts[0]);
	}
	
	for(iCurrPart = 0; iCurrPart < iNumParts; iCurrPart++)
	{
		myParts[iCurrPart]->loadFromClipFile(pData, iMemPos);
	}

	return true;
}
/********************************************************************************************/
void CachedFrame3D::copyFrom(CachedFrame3D* pOther)
{
	clear();
	myMaterials = pOther->myMaterials;

	CachedPart3D* pPart;
	int iCurr, iNum = pOther->getNumParts();
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pPart = addPart(pOther->getPart(iCurr)->getName());
		pPart->copyFrom(pOther->getPart(iCurr));
	}
}
/********************************************************************************************/
void CachedFrame3D::computeCentroid(SVector3D& svCenterOut)
{
	SVertex3D* pData;
	CachedPart3D* pPart;
	int iNumVerts, iTotalNumVerts = 0;
	int iCurr, iNum = this->getNumParts();
	svCenterOut.set(0,0,0);
	int iCurrVert;
	for(iCurr = 0; iCurr < iNum; iCurr++)
	{
		pData = myParts[iCurr]->getData();

		iNumVerts = myParts[iCurr]->getNumVerts();
		iTotalNumVerts += iNumVerts;			
		for(iCurrVert = 0; iCurrVert < iNumVerts; iCurrVert++)
		{
			svCenterOut.x += pData[iCurrVert].x;
			svCenterOut.y += pData[iCurrVert].y;
			svCenterOut.z += pData[iCurrVert].z;
		}
	}

	svCenterOut = svCenterOut*(1.0/(FLOAT_TYPE)iTotalNumVerts);
}
/********************************************************************************************/
};