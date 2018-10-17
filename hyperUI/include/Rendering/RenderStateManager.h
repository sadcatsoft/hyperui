#pragma once

/********************************************************************************************/
enum NormalMapType
{
	NormalMapNone = 0,
	NormalMapNormalMapping = 1,
};
/********************************************************************************************/
struct SGlobalShaderProgramInfo
{
	GLint myProgram;

	GLint myVertexLocation;
	GLint myNormalLocation;

	GLint myTangentLocation;
	GLint myBitangentLocation;

	GLint myMVPLocation;
	GLint myMVLocation;
	GLint myViewMatrixLocation;
	GLint myNormalMatrixLocation;
	GLint myNumLightsLocation;

	GLint myDiffuseLocation;
	GLint myAmbientLocation;
	GLint mySpecularLocation;
	GLint mySpecPowerLocation;
	GLint myDummyVertexColorLocation;

	//GLint myUseSpecularMap;

	GLint myTexCoord0Location;
	GLint myTexUnit0Location;
	GLint myTexUnit1Location;
	GLint mySpecularTextureUnitLocation;

	// Lights
	GLint myLightPosition[NUM_TOTAL_LIGHTS];
	GLint myLightSpecular[NUM_TOTAL_LIGHTS];
	GLint myLightSpecularPower[NUM_TOTAL_LIGHTS];
	GLint myLightDiffuse[NUM_TOTAL_LIGHTS];
	GLint myLightConstAtten[NUM_TOTAL_LIGHTS];
	GLint myLightLinearAtten[NUM_TOTAL_LIGHTS];
	GLint myLightQuadAtten[NUM_TOTAL_LIGHTS];
	GLint myLightEnabled[NUM_TOTAL_LIGHTS];
};
/********************************************************************************************/
struct SLightInfo
{
	SLightInfo()
	{
		mySpecularPower = 0.0;
		myIsEnabled = false;
	}

	SColor myColor;
	SVector4D myPosition;
	SVector3D myAttenuation;
	float mySpecularPower;
	SColor mySpecularColor;
	bool myIsEnabled;
};
/********************************************************************************************/
void checkGlError();
/********************************************************************************************/
class RenderStateManager
{

public:
	~RenderStateManager();
	static RenderStateManager* getInstance();

	inline SGlobalShaderProgramInfo& getProgramGlobalInfo(ShaderSetType eSet) { return myGlobalInfo[eSet]; }
	inline SGlobalShaderProgramInfo& getCurrProgramGlobalInfo() { return myGlobalInfo[myCurrShaderSet]; }

/*
	const SMatrix4& getWorldMatrix();
	const SMatrix4& getModelViewMatrix();
	const SMatrix3& getNormalMatrix();
	const SMatrix4& getViewMatrix();
	*/
	void setProjectionMatrix(SMatrix4& rMatrixIn);
	void setViewMatrix(SMatrix4& rMatrixIn);
	void setModelMatrix(SMatrix4& rMatrixIn);
	
	inline ShaderSetType setShaderType(ShaderSetType eSet) 
	{ 
		if(eSet == myCurrShaderSet)
			return myCurrShaderSet;
		ShaderSetType eOld = myCurrShaderSet; 
        myCurrShaderSet = eSet; 
#ifdef USE_OPENGL2
        glUseProgram(getCurrProgramGlobalInfo().myProgram);
		//checkGlError();
#endif
		return eOld; 
	}
	
	void resetForOrtho(int iScreenW, int iScreenH, FLOAT_TYPE fOrthoDepthLimit);
    void resetForCachedText(int iScreenW, int iScreenH);
	void resetModelMatrix();

	// Materials
	void resetMaterials() { myDiffuse.r = -1; dirtyMaterialsStatus(); }
	void setAmbientMaterial(SColor& scolMaterial) { myAmbient = scolMaterial; dirtyMaterialsStatus(); }
	void setDiffuseMaterial(SColor& scolMaterial) { myDiffuse = scolMaterial; dirtyMaterialsStatus(); }
	void setSpecularMaterial(SColor&scolSpecular, FLOAT_TYPE fSpecPower) { mySpecular = scolSpecular; mySpecularPower = fSpecPower; dirtyMaterialsStatus(); }

	void updateGLMaterials();
	void updateGLMatrices();
	/*
	const SColor& getAmbientMaterial() { return myAmbient; }
	const SColor& getDiffuseMaterial() { return myDiffuse; }
	const SColor& getSpecularMaterial(FLOAT_TYPE& fPowerOut) { fPowerOut = mySpecularPower; return mySpecular; }
	*/

	// Lights
	void disableLighting();
	void enableLighting();
	void disableAllLights();
	void enableLight(int iLightIndex);
	void disableLight(int iLightIndex);
	void setLightColor(int iLightIndex, SColor& scolLight);
	void setLightSpecular(int iLightIndex, SColor& scolLight, FLOAT_TYPE fSpecPower);
	void setLightPosition(int iLightIndex, SVector4D& svPos);
	void setLightAttenuation(int iLightIndex, FLOAT_TYPE fAttenConst, FLOAT_TYPE fAttenLinear, FLOAT_TYPE fAttenQuad);

	bool getIsLightingEnabled() { return myIsLightingEnabled; }
	void updateLightBuffers(GTIME lTime);

	void setNumTotalLights(int iVal) { myNumTotalLights = iVal; }

private:
	RenderStateManager();

	void recomputeCachedMatrices();
	void updateLightBufferForShaderSet(ShaderSetType eSet);

	void dirtyMaterialsStatus();
	void dirtyMatricesStatus();

private:

	ShaderSetType myCurrShaderSet;
	SGlobalShaderProgramInfo myGlobalInfo[ShaderSetLastPlaceholder];
	static RenderStateManager* theInstance;

	SMatrix4 myProjectionMatrix, myViewMatrix, myModelMatrix;

	SMatrix4 myCachedTotalMatrix, myCachedModelViewMatrix;
	SMatrix3 myCachedNormalMatrix;
	bool myIsTotalMatrixDirty;

	SColor myDiffuse, myAmbient, mySpecular;
	FLOAT_TYPE mySpecularPower;

	bool myIsLightingEnabled;
	SLightInfo myLights[MAX_CUSTOM_LIGHTS + NUM_FIXED_LIGHTS];

	int myNumTotalLights;

	GTIME myLastLightingUpdateTime;

	bool myIsMaterialDirty[ShaderSetLastPlaceholder];
	bool myIsMatrixDirty[ShaderSetLastPlaceholder];
};
/********************************************************************************************/
