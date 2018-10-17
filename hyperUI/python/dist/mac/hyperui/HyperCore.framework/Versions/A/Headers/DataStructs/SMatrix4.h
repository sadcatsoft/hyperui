#pragma once

#define MAT4_SIZE    16

/*
Matrix format:
 0  1  2  3
 4  5  6  7
 8  9 10 11
12 13 14 15

Vertical vector on the right:
x
y
z
1

*/

// This matrix is based on OpenGL coordinate system.
struct HYPERCORE_API SMatrix4
{
	SMatrix4()
	{
		resetToIdentity();
	}

	SMatrix4(const SMatrix4& other)
	{
		*this = other;
	}

	SMatrix4& operator=(const SMatrix4& other)
	{
		memcpy(myValues, other.myValues, sizeof(FLOAT_TYPE)*MAT4_SIZE);
		return *this;
	}

	inline FLOAT_TYPE getValue(int iColumn, int iRow) const
	{
		return myValues[iRow*4 + iColumn];
	}

	void resetToIdentity()
	{
		memset(myValues, 0, sizeof(FLOAT_TYPE)*MAT4_SIZE);
		myValues[0] = myValues[5] = myValues[10] = myValues[15] = 1;
	}

	// Note that this assumes w = 1, i.e. it will translate it.
	SVector3D operator*(const SVector3D& other_vec) const
	{
		FLOAT_TYPE fWOut;
		FLOAT_TYPE w = 1.0;
		SVector3D svRes;

		svRes.x = myValues[0]*other_vec.x + myValues[1]*other_vec.y + myValues[2]*other_vec.z + myValues[3]*w;
		svRes.y = myValues[4]*other_vec.x + myValues[5]*other_vec.y + myValues[6]*other_vec.z + myValues[7]*w;
		svRes.z = myValues[8]*other_vec.x + myValues[9]*other_vec.y + myValues[10]*other_vec.z + myValues[11]*w;
		fWOut = myValues[12]*other_vec.x + myValues[13]*other_vec.y + myValues[14]*other_vec.z + myValues[15]*w;

		return svRes;
	}

	SVector4D operator*(const SVector4D& other_vec) const
	{
		SVector4D svRes;

		svRes.x = myValues[0]*other_vec.x + myValues[1]*other_vec.y + myValues[2]*other_vec.z + myValues[3]*other_vec.w;
		svRes.y = myValues[4]*other_vec.x + myValues[5]*other_vec.y + myValues[6]*other_vec.z + myValues[7]*other_vec.w;
		svRes.z = myValues[8]*other_vec.x + myValues[9]*other_vec.y + myValues[10]*other_vec.z + myValues[11]*other_vec.w;
		svRes.w = myValues[12]*other_vec.x + myValues[13]*other_vec.y + myValues[14]*other_vec.z + myValues[15]*other_vec.w;

		return svRes;
	}


	// Multiplication
	SMatrix4 operator*(const SMatrix4& other) const
	{
		SMatrix4 result;

		// Row 1
		result.myValues[0] = this->myValues[0]*other.myValues[0] + this->myValues[1]*other.myValues[4]
		+ this->myValues[2]*other.myValues[8] + this->myValues[3]*other.myValues[12];
		result.myValues[1] = this->myValues[0]*other.myValues[1] + this->myValues[1]*other.myValues[5]
		+ this->myValues[2]*other.myValues[9] + this->myValues[3]*other.myValues[13];
		result.myValues[2] = this->myValues[0]*other.myValues[2] + this->myValues[1]*other.myValues[6]
		+ this->myValues[2]*other.myValues[10] + this->myValues[3]*other.myValues[14];
		result.myValues[3] = this->myValues[0]*other.myValues[3] + this->myValues[1]*other.myValues[7]
		+ this->myValues[2]*other.myValues[11] + this->myValues[3]*other.myValues[15];

		// Row 2
		result.myValues[4] = this->myValues[4]*other.myValues[0] + this->myValues[5]*other.myValues[4]
		+ this->myValues[6]*other.myValues[8] + this->myValues[7]*other.myValues[12];
		result.myValues[5] = this->myValues[4]*other.myValues[1] + this->myValues[5]*other.myValues[5]
		+ this->myValues[6]*other.myValues[9] + this->myValues[7]*other.myValues[13];
		result.myValues[6] = this->myValues[4]*other.myValues[2] + this->myValues[5]*other.myValues[6]
		+ this->myValues[6]*other.myValues[10] + this->myValues[7]*other.myValues[14];
		result.myValues[7] = this->myValues[4]*other.myValues[3] + this->myValues[5]*other.myValues[7]
		+ this->myValues[6]*other.myValues[11] + this->myValues[7]*other.myValues[15];

		// Row 3
		result.myValues[8] = this->myValues[8]*other.myValues[0] + this->myValues[9]*other.myValues[4]
		+ this->myValues[10]*other.myValues[8] + this->myValues[11]*other.myValues[12];
		result.myValues[9] = this->myValues[8]*other.myValues[1] + this->myValues[9]*other.myValues[5]
		+ this->myValues[10]*other.myValues[9] + this->myValues[11]*other.myValues[13];
		result.myValues[10] = this->myValues[8]*other.myValues[2] + this->myValues[9]*other.myValues[6]
		+ this->myValues[10]*other.myValues[10] + this->myValues[11]*other.myValues[14];
		result.myValues[11] = this->myValues[8]*other.myValues[3] + this->myValues[9]*other.myValues[7]
		+ this->myValues[10]*other.myValues[11] + this->myValues[11]*other.myValues[15];

		// Row 4
		result.myValues[12] = this->myValues[12]*other.myValues[0] + this->myValues[13]*other.myValues[4]
		+ this->myValues[14]*other.myValues[8] + this->myValues[15]*other.myValues[12];
		result.myValues[13] = this->myValues[12]*other.myValues[1] + this->myValues[13]*other.myValues[5]
		+ this->myValues[14]*other.myValues[9] + this->myValues[15]*other.myValues[13];
		result.myValues[14] = this->myValues[12]*other.myValues[2] + this->myValues[13]*other.myValues[6]
		+ this->myValues[14]*other.myValues[10] + this->myValues[15]*other.myValues[14];
		result.myValues[15] = this->myValues[12]*other.myValues[3] + this->myValues[13]*other.myValues[7]
		+ this->myValues[14]*other.myValues[11] + this->myValues[15]*other.myValues[15];

		return result;
	}

	// Generation routines
	static void generateTranslationMatrix(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fZ, SMatrix4& smOut)
	{
		smOut.resetToIdentity();
		smOut.myValues[3] = fX;
		smOut.myValues[7] = fY;
		smOut.myValues[11] = fZ;
	}

	static void generateScaleMatrix(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fZ, SMatrix4& smOut)
	{
		smOut.resetToIdentity();
		smOut.myValues[0] = fX;
		smOut.myValues[5] = fY;
		smOut.myValues[10] = fZ;
	}

	static void generateXRotationMatrix(FLOAT_TYPE fDegAngle, SMatrix4& smOut)
	{
		smOut.resetToIdentity();
		FLOAT_TYPE fCos = F_COS_DEG(fDegAngle);
		FLOAT_TYPE fSin = F_SIN_DEG(fDegAngle);
		smOut.myValues[5] = fCos;
		smOut.myValues[6] = -fSin;
		smOut.myValues[9] = fSin;
		smOut.myValues[10] = fCos;
	}

	static void generateYRotationMatrix(FLOAT_TYPE fDegAngle, SMatrix4& smOut)
	{
		smOut.resetToIdentity();
		FLOAT_TYPE fCos = F_COS_DEG(fDegAngle);
		FLOAT_TYPE fSin = F_SIN_DEG(fDegAngle);
		smOut.myValues[0] = fCos;
		smOut.myValues[2] = fSin;
		smOut.myValues[8] = -fSin;
		smOut.myValues[10] = fCos;
	}

	static void generateZRotationMatrix(FLOAT_TYPE fDegAngle, SMatrix4& smOut)
	{
		smOut.resetToIdentity();
		FLOAT_TYPE fCos = F_COS_DEG(fDegAngle);
		FLOAT_TYPE fSin = F_SIN_DEG(fDegAngle);
		smOut.myValues[0] = fCos;
		smOut.myValues[1] = -fSin;
		smOut.myValues[4] = fSin;
		smOut.myValues[5] = fCos;
	}

	static void generateRotationMatrix(FLOAT_TYPE fDegAngle, FLOAT_TYPE fAxisX, FLOAT_TYPE fAxisY, FLOAT_TYPE fAxisZ, SMatrix4& smOut)
	{
		SVector3D svAxis(fAxisX, fAxisY, fAxisZ);
		svAxis.normalize();

		smOut.resetToIdentity();
		FLOAT_TYPE fCos = F_COS_DEG(fDegAngle);
		FLOAT_TYPE fSin = F_SIN_DEG(fDegAngle);

		smOut.myValues[0] = svAxis.x*svAxis.x*(1.0 - fCos) + fCos;
		smOut.myValues[1] = svAxis.x*svAxis.y*(1.0 - fCos) - svAxis.z*fSin;
		smOut.myValues[2] = svAxis.x*svAxis.z*(1.0 - fCos) + svAxis.y*fSin;

		smOut.myValues[4] = svAxis.x*svAxis.y*(1.0 - fCos) + svAxis.z*fSin;
		smOut.myValues[5] = svAxis.y*svAxis.y*(1.0 - fCos) + fCos;
		smOut.myValues[6] = svAxis.y*svAxis.z*(1.0 - fCos) - svAxis.x*fSin;

		smOut.myValues[8] = svAxis.x*svAxis.z*(1.0 - fCos) - svAxis.y*fSin;
		smOut.myValues[9] = svAxis.y*svAxis.z*(1.0 - fCos) + svAxis.x*fSin;
		smOut.myValues[10] = svAxis.z*svAxis.z*(1.0 - fCos) + fCos;
	}

	static void generateOrthoProjectionMatrix(FLOAT_TYPE fLeft, FLOAT_TYPE fRight, FLOAT_TYPE fBottom, FLOAT_TYPE fTop, FLOAT_TYPE fNear, FLOAT_TYPE fFar, SMatrix4& rOut)
	{
		rOut.resetToIdentity();

		rOut.myValues[0] = 2.0/(fRight - fLeft);
		rOut.myValues[5] = 2.0/(fTop - fBottom);
		rOut.myValues[10] = -2.0/(fFar - fNear);
		rOut.myValues[3] = -(fRight + fLeft)/(fRight - fLeft);
		rOut.myValues[7] = -(fTop + fBottom)/(fTop - fBottom);
		rOut.myValues[11] = -(fFar + fNear)/(fFar - fNear);
	}

	static void generateFrustumProjectMatrix(FLOAT_TYPE fFovY, FLOAT_TYPE fAspect, FLOAT_TYPE fNear, FLOAT_TYPE fFar, SMatrix4& smOut, bool bFlipForIOS)
	{
		smOut.resetToIdentity();

//		const double pi = 3.1415926535897932384626433832795;
		double fW, fH;
#if defined(WIN32) || defined(MAC_BUILD)
		fH = tan( fFovY / 360.0 * M_PI ) * fNear;
		fW = fH * fAspect;
		//glFrustum( -fW, fW, -fH, fH, fNear, fFar );
#else
        if(bFlipForIOS)
        {
            fW = tan( fFovY / 360.0 * M_PI ) * fNear;
            fH = fW * fAspect;
        }
        else 
        {
            fH = tan( fFovY / 360.0 * M_PI ) * fNear;
            fW = fH * fAspect;
        }
// 		fH = tan( fFovY / 360.0 * pi ) * fNear;
// 		fW = fH * fAspect;
// 		glFrustumf( -fH, fH, -fW, fW, fNear, fFar );
#endif

		FLOAT_TYPE fLeft = -fW;
		FLOAT_TYPE fRight = fW;
		FLOAT_TYPE fBottom = -fH;
		FLOAT_TYPE fTop =  fH;

		smOut.myValues[0] = 2.0*fNear/(fRight - fLeft);
		smOut.myValues[5] = 2.0*fNear/(fTop - fBottom);
		smOut.myValues[2] = (fRight + fLeft)/(fRight - fLeft);
		smOut.myValues[6] = (fTop + fBottom)/(fTop - fBottom);
		smOut.myValues[10] = -(fFar + fNear)/(fFar - fNear);
		smOut.myValues[11] = -2.0*(fFar * fNear)/(fFar - fNear);
		smOut.myValues[14] = -1;
		smOut.myValues[15] = 0.0;
	}

/*
	// The projection matrix doesn't properly work on an iPhone...
	static void generatePerspectiveProjectionMatrix(FLOAT_TYPE fFov, FLOAT_TYPE fAspect, FLOAT_TYPE fNear, FLOAT_TYPE fFar, SMatrix4& smOut)
	{
		smOut.resetToIdentity();
		FLOAT_TYPE rad = (fFov/2.0)/180.0*M_PI;
		float fCot = cos(rad) / sin(rad);
		//FLOAT_TYPE fCot = 1.0/tan((fFov/2.0)/180.0*M_PI);

		smOut.myValues[0] = fCot/fAspect;
		smOut.myValues[5] = fCot;
		smOut.myValues[10] = (fFar + fNear)/(fNear - fFar);
		smOut.myValues[11] = (2.0*fFar*fNear)/(fNear - fFar);
		smOut.myValues[14] = -1.0;
		smOut.myValues[15] = 0.0;
	}
	*/

	static void generateLookAtMatrix(SVector3D& svEyePos, SVector3D& svTargetPos, SVector3D& svUpVec, SMatrix4& smOut)
	{
		smOut.resetToIdentity();

		SVector3D svF, svS, svU;
		svF = svTargetPos - svEyePos;
		svF.normalize();

		svS = svF.cross(svUpVec);
		svU = svS.cross(svF);

		// Here we actually generate the transpose
		smOut.myValues[0] = svS.x;
		smOut.myValues[1] = svU.x;
		smOut.myValues[2] = -svF.x;
		smOut.myValues[3] = 0.0f;

		smOut.myValues[4] = svS.y;
		smOut.myValues[5] = svU.y;
		smOut.myValues[6] = -svF.y;
		smOut.myValues[7] = 0.0f;

		smOut.myValues[8] = svS.z;
		smOut.myValues[9] = svU.z;
		smOut.myValues[10] = -svF.z;
		smOut.myValues[11] = 0.0f;

		smOut.myValues[12] = 0.0f;
		smOut.myValues[13] = 0.0f;
		smOut.myValues[14] = 0.0f;
		smOut.myValues[15] = 1.0f;

		SVector4D svT, svTemp(-svEyePos.x, -svEyePos.y, -svEyePos.z, 1.0f);
		svT = smOut*svTemp;

		smOut.myValues[12] = svT.x;
		smOut.myValues[13] = svT.y;
		smOut.myValues[14] = svT.z;

		smOut.transpose();
	}

	// Append routines
	void appendTranslation(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fZ)
	{
		SMatrix4 smTemp;
		generateTranslationMatrix(fX, fY, fZ, smTemp);
		*this = (*this)*smTemp;
	}

	void appendScale(FLOAT_TYPE fX, FLOAT_TYPE fY, FLOAT_TYPE fZ)
	{
		SMatrix4 smTemp;
		generateScaleMatrix(fX, fY, fZ, smTemp);
		*this = (*this)*smTemp;
	}

	void appendXRotation(FLOAT_TYPE fDegAngle)
	{
		SMatrix4 smTemp;
		generateXRotationMatrix(fDegAngle, smTemp);
		*this = (*this)*smTemp;
	}

	void appendYRotation(FLOAT_TYPE fDegAngle)
	{
		SMatrix4 smTemp;
		generateYRotationMatrix(fDegAngle, smTemp);
		*this = (*this)*smTemp;
	}

	void appendZRotation(FLOAT_TYPE fDegAngle)
	{
		SMatrix4 smTemp;
		generateZRotationMatrix(fDegAngle, smTemp);
		*this = (*this)*smTemp;
	}

	void appendRotation(FLOAT_TYPE fDegAngle, FLOAT_TYPE fAxisX, FLOAT_TYPE fAxisY, FLOAT_TYPE fAxisZ)
	{
		SMatrix4 smTemp;
		generateRotationMatrix(fDegAngle,fAxisX, fAxisY, fAxisZ, smTemp);
		*this = (*this)*smTemp;
	}

/*
	// The projection matrix doesn't properly work on an iPhone...
	void appendPerspectiveProjection(FLOAT_TYPE fFov, FLOAT_TYPE fAspect, FLOAT_TYPE fNear, FLOAT_TYPE fFar)
	{
		SMatrix4 smTemp;
		generatePerspectiveProjectionMatrix(fFov,fAspect, fNear, fFar, smTemp);
		*this = (*this)*smTemp;
	}
	*/

	void appendOrthoProjectionMatrix(FLOAT_TYPE fLeft, FLOAT_TYPE fRight, FLOAT_TYPE fBottom, FLOAT_TYPE fTop, FLOAT_TYPE fNear, FLOAT_TYPE fFar)
	{
		SMatrix4 smTemp;
		generateOrthoProjectionMatrix(fLeft, fRight, fBottom, fTop, fNear, fFar, smTemp);
		*this = (*this)*smTemp;
	}

	void appendFrustumPerspectiveProjection(FLOAT_TYPE fFov, FLOAT_TYPE fAspect, FLOAT_TYPE fNear, FLOAT_TYPE fFar, bool bFlipForIOS)
	{
		SMatrix4 smTemp;
		generateFrustumProjectMatrix(fFov,fAspect, fNear, fFar, smTemp, bFlipForIOS);
		*this = (*this)*smTemp;
	}

	void appendLookAt(SVector3D& svEyePos, SVector3D& svTargetPos, SVector3D& svUpVec)
	{
		SMatrix4 smTemp;
		generateLookAtMatrix(svEyePos, svTargetPos, svUpVec, smTemp);
		*this = (*this)*smTemp;
	}

	void transpose()
	{
		FLOAT_TYPE tempValues[MAT4_SIZE];
		memcpy(tempValues, myValues, sizeof(FLOAT_TYPE)*MAT4_SIZE);

		// Now, copy them back:
		//myValues[0] = tempValues[0];
		myValues[1] = tempValues[4];
		myValues[2] = tempValues[8];
		myValues[3] = tempValues[12];

		myValues[4] = tempValues[1];
		//myValues[5] = tempValues[5];
		myValues[6] = tempValues[9];
		myValues[7] = tempValues[13];

		myValues[8] = tempValues[2];
		myValues[9] = tempValues[6];
		//myValues[10] = tempValues[10];
		myValues[11] = tempValues[14];

		myValues[12] = tempValues[3];
		myValues[13] = tempValues[7];
		myValues[14] = tempValues[11];
		//myValues[15] = tempValues[15];
	}
	 
	void toFloatArray(float rArrayOut[16], bool bTranspose)
	{
		_ASSERT(sizeof(float) == sizeof(FLOAT_TYPE));
		if(!bTranspose)
			memcpy(rArrayOut, myValues, sizeof(FLOAT_TYPE)*MAT4_SIZE);
		else
		{
			SMatrix4 rTemp(*this);
			rTemp.transpose();
			memcpy(rArrayOut, rTemp.myValues, sizeof(FLOAT_TYPE)*MAT4_SIZE);
		}
	}

	FLOAT_TYPE myValues[MAT4_SIZE];
}; 