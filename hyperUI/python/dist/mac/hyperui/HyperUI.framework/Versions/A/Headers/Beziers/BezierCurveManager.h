#pragma once

/*****************************************************************************/
struct HYPERUI_API SCurveInfo
{
	TPointVector myLinearForm;
	TPointVector myTangents;
	TFloatVector myDistances;
	FLOAT_TYPE myTotalLength;
};

typedef map < string, SCurveInfo > TStringCurveMap;
/*****************************************************************************/
class HYPERUI_API BezierCurveManager
{
public:
	~BezierCurveManager();
	static BezierCurveManager* getInstance();

	TPointVector* getLinearForm(const char* pcsCurve);
	TFloatVector* getDistances(const char* pcsCurve);
	TPointVector* getTangents(const char* pcsCurve);
	FLOAT_TYPE getCurveLength(const char* pcsCurve);

	void reloadAll();

private:
	BezierCurveManager();

	SCurveInfo* cacheCurveInfo(const char*pcsCurve);

private:

	static BezierCurveManager* theInstance;

	TStringCurveMap myCurves;
};
/*****************************************************************************/