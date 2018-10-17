#include "stdafx.h"

namespace HyperUI
{
RenderStateManager* RenderStateManager::theInstance = NULL;
/********************************************************************************************/
RenderStateManager::RenderStateManager()
{
	myLastLightingUpdateTime = 0;
	myNumTotalLights = 0;
	//myCurrShaderSet = ShaderSetFlatTexture;
	// This ensures we call glUseProgram() first...
	myCurrShaderSet = ShaderSetLastPlaceholder;
	dirtyMatricesStatus();
	resetMaterials();
	disableLighting();
}
/********************************************************************************************/
RenderStateManager::~RenderStateManager()
{

}
/********************************************************************************************/
RenderStateManager* RenderStateManager::getInstance()
{
	if(!theInstance)
		theInstance = new RenderStateManager;
	return theInstance;
}
/********************************************************************************************/
void RenderStateManager::recomputeCachedMatrices()
{
	_ASSERT(myIsTotalMatrixDirty);

	// MVP matrix.
	myCachedModelViewMatrix = myModelMatrix*myViewMatrix;
	myCachedTotalMatrix = myCachedModelViewMatrix*myProjectionMatrix;

	// Normal matrix
	SMatrix4 smatTemp;
	smatTemp = myModelMatrix*myViewMatrix;
	
	myCachedNormalMatrix.from4D(smatTemp);
	myCachedNormalMatrix.invert();
	myCachedNormalMatrix.transpose();

	myIsTotalMatrixDirty = false;
}
/********************************************************************************************
const SMatrix3& RenderStateManager::getNormalMatrix()
{
	if(myIsTotalMatrixDirty)
		recomputeCachedMatrices();
	return myCachedNormalMatrix;
}
/********************************************************************************************
const SMatrix4& RenderStateManager::getViewMatrix()
{
	if(myIsTotalMatrixDirty)
		recomputeCachedMatrices();
	return myViewMatrix;
}
/********************************************************************************************
const SMatrix4& RenderStateManager::getModelViewMatrix()
{
	if(myIsTotalMatrixDirty)
		recomputeCachedMatrices();
	return myCachedModelViewMatrix;
}
/********************************************************************************************
const SMatrix4& RenderStateManager::getWorldMatrix()
{
	if(myIsTotalMatrixDirty)
		recomputeCachedMatrices();
	return myCachedTotalMatrix;
}
/********************************************************************************************/
void RenderStateManager::setProjectionMatrix(SMatrix4& rMatrixIn)
{
	myProjectionMatrix = rMatrixIn;
	myProjectionMatrix.transpose();
	dirtyMatricesStatus();
}
/********************************************************************************************/
void RenderStateManager::setViewMatrix(SMatrix4& rMatrixIn)
{
	myViewMatrix = rMatrixIn;
	myViewMatrix.transpose();
	dirtyMatricesStatus();
}
/********************************************************************************************/
void RenderStateManager::setModelMatrix(SMatrix4& rMatrixIn)
{
	myModelMatrix = rMatrixIn;
	myModelMatrix.transpose();
	dirtyMatricesStatus();
}
/********************************************************************************************/
void RenderStateManager::resetModelMatrix()
{
	myModelMatrix.resetToIdentity();
	dirtyMatricesStatus();
}
/********************************************************************************************/
void RenderStateManager::resetForCachedText(int iScreenW, int iScreenH)
{
#if !defined(MAC_BUILD) && !defined(WIN32)
	myProjectionMatrix.resetToIdentity();

    //myProjectionMatrix.appendZRotation(sanitizeDegAngle(90));
	myProjectionMatrix.appendTranslation(-1.0, -1.0, 0);
	myProjectionMatrix.appendScale(1.0/(FLOAT_TYPE)iScreenH*2.0, 1.0/(FLOAT_TYPE)iScreenW*2.0, 1.0);
    
   	myProjectionMatrix.transpose();
    
	myViewMatrix.resetToIdentity();
	myModelMatrix.resetToIdentity();
	dirtyMatricesStatus();
 
#endif
}
/********************************************************************************************/
void RenderStateManager::resetForOrtho(int iScreenW, int iScreenH, FLOAT_TYPE fOrthoDepthLimit)
{
	myProjectionMatrix.resetToIdentity();

	
#if defined(ORIENTATION_LANDSCAPE) && !defined(MAC_BUILD) && !defined(WIN32)
    glViewport(0, 0, iScreenH, iScreenW);
/*
    myProjectionMatrix.appendZRotation(sanitizeDegAngle(90));
	myProjectionMatrix.appendTranslation(1.0, -1.0, 0);
	myProjectionMatrix.appendScale(-1.0/(FLOAT_TYPE)iScreenW*2.0, 1.0/(FLOAT_TYPE)iScreenH*2.0, 1.0);
*/
#else

#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else
    glViewport(0, 0, iScreenW, iScreenH);
#endif

//	myProjectionMatrix.appendTranslation(-1.0, 1.0, 0);
//	myProjectionMatrix.appendScale(1.0/(FLOAT_TYPE)iScreenW*2.0, -1.0/(FLOAT_TYPE)iScreenH*2.0, 1.0);
    
#endif
	//checkGlError();	

#if defined(WIN32) || defined(MAC_BUILD)
	int iX = iScreenW;
	int iY = iScreenH;
#else
	int iX = iScreenW;
	int iY = iScreenH;
#endif

#if !defined(MAC_BUILD) && !defined(WIN32)
	myProjectionMatrix.appendZRotation(sanitizeDegAngle(-90));
#endif
	myProjectionMatrix.appendOrthoProjectionMatrix(0.0, iX, iY, 0.0, -fOrthoDepthLimit, fOrthoDepthLimit);


	myProjectionMatrix.transpose();

	myViewMatrix.resetToIdentity();
	myModelMatrix.resetToIdentity();
	dirtyMatricesStatus();
}
/********************************************************************************************/
void RenderStateManager::disableLighting()
{
	myIsLightingEnabled = false;
}
/********************************************************************************************/
void RenderStateManager::enableLighting()
{
	myIsLightingEnabled = true;
}
/********************************************************************************************/
void RenderStateManager::disableAllLights()
{
	int iLight;
	for(iLight = 0; iLight < MAX_CUSTOM_LIGHTS + NUM_FIXED_LIGHTS; iLight++)
		disableLight(iLight);
}
/********************************************************************************************/
void RenderStateManager::enableLight(int iLightIndex)
{
	myLights[iLightIndex].myIsEnabled = true;
}
/********************************************************************************************/
void RenderStateManager::setLightSpecular(int iLightIndex, SColor& scolLight, FLOAT_TYPE fSpecPower)
{
	myLights[iLightIndex].mySpecularColor = scolLight;
	myLights[iLightIndex].mySpecularPower = fSpecPower;
}
/********************************************************************************************/
void RenderStateManager::setLightColor(int iLightIndex, SColor& scolLight)
{
	myLights[iLightIndex].myColor = scolLight;
}
/********************************************************************************************/
void RenderStateManager::setLightPosition(int iLightIndex, SVector4D& svPos)
{
	myLights[iLightIndex].myPosition = svPos;
}
/********************************************************************************************/
void RenderStateManager::setLightAttenuation(int iLightIndex, FLOAT_TYPE fAttenConst, FLOAT_TYPE fAttenLinear, FLOAT_TYPE fAttenQuad)
{
	myLights[iLightIndex].myAttenuation.set(fAttenConst, fAttenLinear, fAttenQuad);
}
/********************************************************************************************/
void RenderStateManager::disableLight(int iLightIndex)
{
	myLights[iLightIndex].myIsEnabled = false;
}
/********************************************************************************************/
void RenderStateManager::updateLightBuffers(GTIME lTime)
{
	if(lTime == myLastLightingUpdateTime)
		return;

	if(!myIsLightingEnabled)
		return;

	// Here, we update the lights for all programs that will need them, since now they're separate.
	int iConfig;
	for(iConfig = ShaderSetLighted; iConfig < ShaderSetLastPlaceholder; iConfig++)
		updateLightBufferForShaderSet((ShaderSetType)iConfig);

#ifdef USE_OPENGL2 
	// Since the light calls used a different program, reset it
	glUseProgram(getCurrProgramGlobalInfo().myProgram);
#endif
	myLastLightingUpdateTime = lTime;
}
/********************************************************************************************/
void RenderStateManager::updateLightBufferForShaderSet(ShaderSetType eSet)
{
	if(!myIsLightingEnabled)
		return;
#ifdef USE_OPENGL2
	SGlobalShaderProgramInfo& rCurrProgramInfo = getProgramGlobalInfo(eSet);
	glUseProgram(rCurrProgramInfo.myProgram);

	glUniform1i(rCurrProgramInfo.myNumLightsLocation, myNumTotalLights);

	int iLight;
	//for(iLight = 0; iLight < NUM_TOTAL_LIGHTS; iLight++)
	for(iLight = 0; iLight < myNumTotalLights; iLight++)
	{
		//glUniform1i(rCurrProgramInfo.myLightEnabled[iLight], myLights[iLight].myIsEnabled);

		_ASSERT(myLights[iLight].myIsEnabled);
// 		if(!myLights[iLight].myIsEnabled)
// 			continue;

		glUniform4f(rCurrProgramInfo.myLightPosition[iLight], myLights[iLight].myPosition.x, myLights[iLight].myPosition.y, myLights[iLight].myPosition.z, myLights[iLight].myPosition.w);
		glUniform3f(rCurrProgramInfo.myLightDiffuse[iLight], myLights[iLight].myColor.r, myLights[iLight].myColor.g, myLights[iLight].myColor.b);
		// For now, use diffuse for specular
		glUniform3f(rCurrProgramInfo.myLightSpecular[iLight], myLights[iLight].mySpecularColor.r, myLights[iLight].mySpecularColor.g, myLights[iLight].myColor.b);

		glUniform1f(rCurrProgramInfo.myLightConstAtten[iLight], myLights[iLight].myAttenuation.x);
		glUniform1f(rCurrProgramInfo.myLightLinearAtten[iLight], myLights[iLight].myAttenuation.y);
		glUniform1f(rCurrProgramInfo.myLightQuadAtten[iLight], myLights[iLight].myAttenuation.z);

		glUniform1f(rCurrProgramInfo.myLightSpecularPower[iLight], myLights[iLight].mySpecularPower);		
	}
#endif
}
/********************************************************************************************/
void RenderStateManager::dirtyMaterialsStatus()
{
	memset(myIsMaterialDirty, 1, sizeof(bool)*ShaderSetLastPlaceholder);
}
/********************************************************************************************/
void RenderStateManager::dirtyMatricesStatus()
{
	memset(myIsMatrixDirty, 1, sizeof(bool)*ShaderSetLastPlaceholder);
	myIsTotalMatrixDirty = true;
}
/********************************************************************************************/
void RenderStateManager::updateGLMatrices()
{
	if(!myIsMatrixDirty[myCurrShaderSet])
		return;

	SGlobalShaderProgramInfo& rInfo = this->getCurrProgramGlobalInfo();

	if(myIsTotalMatrixDirty)
		this->recomputeCachedMatrices();
	_ASSERT(myIsTotalMatrixDirty == false);

#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else
#ifdef USE_OPENGL2
	// Normal and MVP matrices.
	glUniformMatrix4fv(rInfo.myMVPLocation, 1, GL_FALSE, &myCachedTotalMatrix.myValues[0]);
	checkGlError();
	glUniformMatrix4fv(rInfo.myMVLocation, 1, GL_FALSE, &myCachedModelViewMatrix.myValues[0]);
	checkGlError();
	glUniformMatrix3fv(rInfo.myNormalMatrixLocation, 1, GL_FALSE, &myCachedNormalMatrix.myValues[0]);	
	checkGlError();
	glUniformMatrix4fv(rInfo.myViewMatrixLocation, 1, GL_FALSE, &myViewMatrix.myValues[0]);	
	checkGlError();
#endif
#endif
	myIsMatrixDirty[myCurrShaderSet] = false;
}
/********************************************************************************************/
void RenderStateManager::updateGLMaterials()
{
	if(!myIsMaterialDirty[myCurrShaderSet])
		return;

	SGlobalShaderProgramInfo& rInfo = this->getCurrProgramGlobalInfo();

#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else
#ifdef USE_OPENGL2
	glUniform4fv(rInfo.myDiffuseLocation, 1, &myDiffuse.r);
	checkGlError();
	glUniform4fv(rInfo.myAmbientLocation, 1, &myAmbient.r);
	checkGlError();
	glUniform4fv(rInfo.mySpecularLocation, 1, &mySpecular.r);
	checkGlError();
	glUniform1f(rInfo.mySpecPowerLocation, mySpecularPower);
	checkGlError();
#endif
#endif
	myIsMaterialDirty[myCurrShaderSet] = false;
}
/********************************************************************************************/
};