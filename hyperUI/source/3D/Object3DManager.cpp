#include "stdafx.h"

namespace HyperUI
{
/********************************************************************************************/
Object3DManager::Object3DManager()
{

}
/********************************************************************************************/
Object3DManager::~Object3DManager()
{
	clear();
}
/********************************************************************************************/
void Object3DManager::initFromCollection(ResourceCollection* pColl, Window* pEngine)
{
#ifndef ALLOW_3D
	return;
#endif

	if(!pColl)
		return;

	CachedObject3D* pObject;
	string strFileName;
	const char* pcsTypeName;

	//bool bFlipU, bFlipV;
	ResourceItem* pItem = NULL;
	ResourceCollection::Iterator ci;
	//ResourceItem* pItem = pColl->traverseBegin();
	SVector3D svDefScale;
	//while(pItem)
	for(ci = pColl->itemsBegin(); !ci.isEnd(); ci++)
	{
		pItem = ci.getItem();
		pObject = new CachedObject3D;

		pcsTypeName = pItem->getStringProp(PropertyId);
/*
		strFileName = pItem->getStringProp(PropertyAnimFilename);		


		bFlipU = pItem->getBoolProp(PropertyObj3dFlipU);
		bFlipV = pItem->getBoolProp(PropertyObj3dFlipV);

		if(pItem->doesPropertyExist(PropertyObj3dDefaultScale))
			pItem->getAsVector3(PropertyObj3dDefaultScale, svDefScale);
		else
			svDefScale.set(1,1,1);

		if(pObject->loadFromObjFile(strFileName.c_str(), pEngine, false, bFlipU, bFlipV, svDefScale) == false)
		*/
		if(pObject->loadFromItem(pItem, pEngine) == false)
		{
			// Kill it and forget about it.
			_ASSERT(0);
			delete pObject;
		}
		else
		{
			myObjects[pcsTypeName] = pObject;
		}

		//pItem = pColl->traverseNext();
	}
}
/********************************************************************************************/
void Object3DManager::clear(void)
{
	TStringObject3DMap::iterator mi;
	for(mi = myObjects.begin(); mi != myObjects.end(); mi++)
	{
		delete mi->second;
	}

	myObjects.clear();
}
/********************************************************************************************/
CachedObject3D* Object3DManager::findObject(const char* pcsType)
{
	TStringObject3DMap::iterator mi;
	mi = myObjects.find(pcsType);
	if(mi == myObjects.end())
		return NULL;
	else
		return mi->second;
}
/********************************************************************************************/
void Object3DManager::onTimerTick(void)
{
	TStringObject3DMap::iterator mi;
	for(mi = myObjects.begin(); mi != myObjects.end(); mi++)
	{
		mi->second->onTimerTick();
	}	
}
/********************************************************************************************/
};