#pragma once

typedef map < string, CachedObject3D* > TStringObject3DMap;
/********************************************************************************************/
class HYPERUI_API Object3DManager
{
public:

	Object3DManager();
	~Object3DManager();

	void initFromCollection(ResourceCollection* pColl, Window* pEngine);
	void clear(void);

	CachedObject3D* findObject(const char* pcsType);

	void onTimerTick(void);


private:

	TStringObject3DMap myObjects;
};
/********************************************************************************************/