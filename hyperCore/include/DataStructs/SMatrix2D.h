#pragma once

/*****************************************************************************/
struct HYPERCORE_API SMatrix2D
{
	SMatrix2D()
	{
		resetToIdentity();
	}

	SMatrix2D(const SMatrix2D& smSourceMatrix)
	{
		*this = smSourceMatrix;
	}

	const SMatrix2D& operator= (const SMatrix2D& src)
	{
		myMatrix[0][0] = src.myMatrix[0][0];
		myMatrix[1][0] = src.myMatrix[1][0];
		myMatrix[2][0] = src.myMatrix[2][0];

		myMatrix[0][1] = src.myMatrix[0][1];
		myMatrix[1][1] = src.myMatrix[1][1];
		myMatrix[2][1] = src.myMatrix[2][1];

		myMatrix[0][2] = src.myMatrix[0][2];
		myMatrix[1][2] = src.myMatrix[1][2];
		myMatrix[2][2] = src.myMatrix[2][2];

		return *this;
	}

	inline bool getIsIdentity()
	{
		if(fabs(myMatrix[0][0] - 1.0) > FLOAT_EPSILON)
			return false;
		if(fabs(myMatrix[1][0]) > FLOAT_EPSILON)
			return false;
		if(fabs(myMatrix[2][0]) > FLOAT_EPSILON)
			return false;

		if(fabs(myMatrix[0][1]) > FLOAT_EPSILON)
			return false;
		if(fabs(myMatrix[1][1] - 1.0) > FLOAT_EPSILON)
			return false;
		if(fabs(myMatrix[2][1]) > FLOAT_EPSILON)
			return false;

		if(fabs(myMatrix[0][2]) > FLOAT_EPSILON)
			return false;
		if(fabs(myMatrix[1][2]) > FLOAT_EPSILON)
			return false;
		if(fabs(myMatrix[2][2] - 1.0) > FLOAT_EPSILON)
			return false;
		return true;
	}

	inline void resetToIdentity()
	{
		myMatrix[0][0] = 1.0;
		myMatrix[1][0] = 0.0;
		myMatrix[2][0] = 0.0;

		myMatrix[0][1] = 0.0;
		myMatrix[1][1] = 1.0;
		myMatrix[2][1] = 0.0;

		myMatrix[0][2] = 0.0;
		myMatrix[1][2] = 0.0;
		myMatrix[2][2] = 1.0;
	}

	inline void makeXyzToLmsMatrix()
	{
		myMatrix[0][0] = 0.8951; myMatrix[1][0] = 0.2664; myMatrix[2][0] = -0.1614;
		myMatrix[0][1] = -0.7502; myMatrix[1][1] = 1.7135; myMatrix[2][1] = 0.0367;
		myMatrix[0][2] = 0.0389; myMatrix[1][2] = -0.0685; myMatrix[2][2] = 1.0296;
	}

	inline void makeRotationMatrix(FLOAT_TYPE dDegreeAngle)
	{
		resetToIdentity();
		myMatrix[0][0] = F_COS_DEG(dDegreeAngle);
		myMatrix[0][1] = F_SIN_DEG(dDegreeAngle);
		myMatrix[1][0] = -myMatrix[0][1];
		myMatrix[1][1] = myMatrix[0][0];
	}

	inline void makeSkewXMatrix(FLOAT_TYPE fDegAngle)	
	{
		resetToIdentity();
		myMatrix[1][0] = tan(fDegAngle/180.0*M_PI);
	}

	inline void makeSkewYMatrix(FLOAT_TYPE fDegAngle)	
	{
		resetToIdentity();
		myMatrix[0][1] = tan(fDegAngle/180.0*M_PI);
	}

	inline void makeReflectionMatrix(FLOAT_TYPE dDegreeAngle)
	{
		resetToIdentity();
		FLOAT_TYPE fCos2 = F_COS_DEG(2.0*dDegreeAngle);
		FLOAT_TYPE fSin2 = F_SIN_DEG(2.0*dDegreeAngle);
		myMatrix[0][0] = fCos2;
		myMatrix[0][1] = fSin2 ;
		myMatrix[1][0] = fSin2 ;
		myMatrix[1][1] = -fCos2;
	}

	inline void makeScalingMatrix(FLOAT_TYPE dScalingX, FLOAT_TYPE dScalingY)
	{		
		resetToIdentity();
		myMatrix[0][0] = dScalingX;
		myMatrix[1][1] = dScalingY;
	}

	inline void makeTranslationMatrix(FLOAT_TYPE dX, FLOAT_TYPE dY)
	{
		resetToIdentity();
		myMatrix[2][0] = dX;
		myMatrix[2][1] = dY;
	}

	inline void setTranslation(FLOAT_TYPE dX, FLOAT_TYPE dY)
	{
		myMatrix[2][0] = dX;
		myMatrix[2][1] = dY;
	}

	FLOAT_TYPE calculateAdjoint(SMatrix2D& svMatrixOut) const 
	{
		svMatrixOut.myMatrix[0][0]= myMatrix[1][1]*myMatrix[2][2] - myMatrix[1][2]*myMatrix[2][1];
		svMatrixOut.myMatrix[1][0]= -(myMatrix[1][0]*myMatrix[2][2] - myMatrix[1][2]*myMatrix[2][0]);
		svMatrixOut.myMatrix[2][0]= myMatrix[1][0]*myMatrix[2][1] - myMatrix[1][1]*myMatrix[2][0];

		svMatrixOut.myMatrix[0][1]= -(myMatrix[0][1]*myMatrix[2][2] - myMatrix[0][2]*myMatrix[2][1]);
		svMatrixOut.myMatrix[1][1]= myMatrix[0][0]*myMatrix[2][2] - myMatrix[0][2]*myMatrix[2][0];
		svMatrixOut.myMatrix[2][1]= -(myMatrix[0][0]*myMatrix[2][1] - myMatrix[0][1]*myMatrix[2][0]);

		svMatrixOut.myMatrix[0][2]= myMatrix[0][1]*myMatrix[1][2] - myMatrix[0][2]*myMatrix[1][1];
		svMatrixOut.myMatrix[1][2]= -(myMatrix[0][0]*myMatrix[1][2] - myMatrix[0][2]*myMatrix[1][0]);
		svMatrixOut.myMatrix[2][2]= myMatrix[0][0]*myMatrix[1][1] - myMatrix[0][1]*myMatrix[1][0];

		return ((FLOAT_TYPE) (myMatrix[0][0]*svMatrixOut.myMatrix[0][0] + myMatrix[0][1]*svMatrixOut.myMatrix[1][0] + myMatrix[0][2]*svMatrixOut.myMatrix[2][0]));
	}

	bool invert(void)
	{
		FLOAT_TYPE dDet, dInvDet;
		SMatrix2D svAdjoint;
		dDet = calculateAdjoint(svAdjoint);

		if (dDet < FLOAT_EPSILON && dDet > -FLOAT_EPSILON)
			return false;

		dInvDet = 1.0 / dDet;
		myMatrix[0][0] = svAdjoint.myMatrix[0][0] * dInvDet;
		myMatrix[0][1] = svAdjoint.myMatrix[0][1] * dInvDet;
		myMatrix[0][2] = svAdjoint.myMatrix[0][2] * dInvDet;
		myMatrix[1][0] = svAdjoint.myMatrix[1][0] * dInvDet;
		myMatrix[1][1] = svAdjoint.myMatrix[1][1] * dInvDet;
		myMatrix[1][2] = svAdjoint.myMatrix[1][2] * dInvDet;
		myMatrix[2][0] = svAdjoint.myMatrix[2][0] * dInvDet;
		myMatrix[2][1] = svAdjoint.myMatrix[2][1] * dInvDet;
		myMatrix[2][2] = svAdjoint.myMatrix[2][2] * dInvDet;

		return true;

	}

	inline void leftAppendTranslationMatrix(FLOAT_TYPE fX, FLOAT_TYPE fY)
	{
		SMatrix2D smTemp;
		smTemp.makeTranslationMatrix(fX, fY);
		*this = this->multiplyLeft(smTemp);
	}

	inline void leftAppendScaleMatrix(FLOAT_TYPE fX, FLOAT_TYPE fY)
	{
		SMatrix2D smTemp;
		smTemp.makeScalingMatrix(fX, fY);
		*this = this->multiplyLeft(smTemp);
	}

	inline void leftAppendRotationMatrix(FLOAT_TYPE fDegAngle)
	{
		SMatrix2D smTemp;
		smTemp.makeRotationMatrix(fDegAngle);
		*this = this->multiplyLeft(smTemp);
	}

	inline void rightAppendTranslationMatrix(FLOAT_TYPE fX, FLOAT_TYPE fY)
	{
		SMatrix2D smTemp;
		smTemp.makeTranslationMatrix(fX, fY);
		*this = this->multiplyRight(smTemp);
	}

	inline void rightAppendScaleMatrix(FLOAT_TYPE fX, FLOAT_TYPE fY)
	{
		SMatrix2D smTemp;
		smTemp.makeScalingMatrix(fX, fY);
		*this = this->multiplyRight(smTemp);
	}

	inline void rightAppendRotationMatrix(FLOAT_TYPE fDegAngle)
	{
		SMatrix2D smTemp;
		smTemp.makeRotationMatrix(fDegAngle);
		*this = this->multiplyRight(smTemp);
	}

	inline void rightAppendReflectionMatrix(FLOAT_TYPE fDegAngle)
	{
		SMatrix2D smTemp;
		smTemp.makeReflectionMatrix(fDegAngle);
		*this = this->multiplyRight(smTemp);
	}

	// Multiplies rOther on the LEFT of this matrix.
	SMatrix2D multiplyLeft(const SMatrix2D& rOther) const
	{
		SMatrix2D smResult;

		smResult.myMatrix[0][0] = this->myMatrix[0][0]*rOther.myMatrix[0][0] + this->myMatrix[0][1]*rOther.myMatrix[1][0] + this->myMatrix[0][2]*rOther.myMatrix[2][0];
		smResult.myMatrix[0][1] = this->myMatrix[0][0]*rOther.myMatrix[0][1] + this->myMatrix[0][1]*rOther.myMatrix[1][1] + this->myMatrix[0][2]*rOther.myMatrix[2][1];
		smResult.myMatrix[0][2] = this->myMatrix[0][0]*rOther.myMatrix[0][2] + this->myMatrix[0][1]*rOther.myMatrix[1][2] + this->myMatrix[0][2]*rOther.myMatrix[2][2];

		smResult.myMatrix[1][0] = this->myMatrix[1][0]*rOther.myMatrix[0][0] + this->myMatrix[1][1]*rOther.myMatrix[1][0] + this->myMatrix[1][2]*rOther.myMatrix[2][0];
		smResult.myMatrix[1][1] = this->myMatrix[1][0]*rOther.myMatrix[0][1] + this->myMatrix[1][1]*rOther.myMatrix[1][1] + this->myMatrix[1][2]*rOther.myMatrix[2][1];
		smResult.myMatrix[1][2] = this->myMatrix[1][0]*rOther.myMatrix[0][2] + this->myMatrix[1][1]*rOther.myMatrix[1][2] + this->myMatrix[1][2]*rOther.myMatrix[2][2];

		smResult.myMatrix[2][0] = this->myMatrix[2][0]*rOther.myMatrix[0][0] + this->myMatrix[2][1]*rOther.myMatrix[1][0] + this->myMatrix[2][2]*rOther.myMatrix[2][0];
		smResult.myMatrix[2][1] = this->myMatrix[2][0]*rOther.myMatrix[0][1] + this->myMatrix[2][1]*rOther.myMatrix[1][1] + this->myMatrix[2][2]*rOther.myMatrix[2][1];
		smResult.myMatrix[2][2] = this->myMatrix[2][0]*rOther.myMatrix[0][2] + this->myMatrix[2][1]*rOther.myMatrix[1][2] + this->myMatrix[2][2]*rOther.myMatrix[2][2];

		return smResult;
	}

	// Multiplies rOther on the RIGHT of this matrix, i.e. the vector will be multiplied by rOther before it is
	// multiplied by this matrix.
	SMatrix2D multiplyRight(const SMatrix2D& rOther) const
	{
		SMatrix2D smResult;

		smResult.myMatrix[0][0] = this->myMatrix[0][0]*rOther.myMatrix[0][0] + this->myMatrix[1][0]*rOther.myMatrix[0][1] + this->myMatrix[2][0]*rOther.myMatrix[0][2];
		smResult.myMatrix[0][1] = this->myMatrix[0][1]*rOther.myMatrix[0][0] + this->myMatrix[1][1]*rOther.myMatrix[0][1] + this->myMatrix[2][1]*rOther.myMatrix[0][2];
		smResult.myMatrix[0][2] = this->myMatrix[0][2]*rOther.myMatrix[0][0] + this->myMatrix[1][2]*rOther.myMatrix[0][1] + this->myMatrix[2][2]*rOther.myMatrix[0][2];

		smResult.myMatrix[1][0] = this->myMatrix[0][0]*rOther.myMatrix[1][0] + this->myMatrix[1][0]*rOther.myMatrix[1][1] + this->myMatrix[2][0]*rOther.myMatrix[1][2];
		smResult.myMatrix[1][1] = this->myMatrix[0][1]*rOther.myMatrix[1][0] + this->myMatrix[1][1]*rOther.myMatrix[1][1] + this->myMatrix[2][1]*rOther.myMatrix[1][2];
		smResult.myMatrix[1][2] = this->myMatrix[0][2]*rOther.myMatrix[1][0] + this->myMatrix[1][2]*rOther.myMatrix[1][1] + this->myMatrix[2][2]*rOther.myMatrix[1][2];

		smResult.myMatrix[2][0] = this->myMatrix[0][0]*rOther.myMatrix[2][0] + this->myMatrix[1][0]*rOther.myMatrix[2][1] + this->myMatrix[2][0]*rOther.myMatrix[2][2];
		smResult.myMatrix[2][1] = this->myMatrix[0][1]*rOther.myMatrix[2][0] + this->myMatrix[1][1]*rOther.myMatrix[2][1] + this->myMatrix[2][1]*rOther.myMatrix[2][2];
		smResult.myMatrix[2][2] = this->myMatrix[0][2]*rOther.myMatrix[2][0] + this->myMatrix[1][2]*rOther.myMatrix[2][1] + this->myMatrix[2][2]*rOther.myMatrix[2][2];

		return smResult;
	}

	// The angle is out in degrees.
	void crack(SVector2D& svScaleOut, SVector2D& svTranslationsOut, FLOAT_TYPE& fRotationOut) const
	{
		svTranslationsOut.set(myMatrix[2][0], myMatrix[2][1]);
		svScaleOut.x = F_SQRT(myMatrix[0][0]*myMatrix[0][0] + myMatrix[1][0]*myMatrix[1][0]);
		if(myMatrix[0][0] < 0)
			svScaleOut.x *= -1.0;
		svScaleOut.y = F_SQRT(myMatrix[0][1]*myMatrix[0][1] + myMatrix[1][1]*myMatrix[1][1]);
		if(myMatrix[1][1] < 0)
			svScaleOut.y *= -1.0;

		fRotationOut = F_ATAN2(myMatrix[0][1], myMatrix[1][1])/M_PI*180.0;
	}

	// Pure equality. No tolerance.
	inline bool operator==(const SMatrix2D& smOther) const
	{
		int iX, iY;
		for(iX = 0; iX < 3; iX++)
		{
			for(iY = 0; iY < 3; iY++)
			{
				if(myMatrix[iX][iY] != smOther.myMatrix[iX][iY])
					return false;
			}
		}

		return true;
	}

	inline bool operator!=(const SMatrix2D& smOther) const 
	{ 
		return !(*this == smOther); 
	}

	inline void getTranslations(SVector2D& svTranslationsOut) const 
	{ 
		svTranslationsOut.set(myMatrix[2][0], myMatrix[2][1]); 
	}

	FLOAT_TYPE myMatrix[3][3];
};
/*****************************************************************************/
// Note that we column mulitply vectors on the right.
HYPERCORE_API SVector2D operator *(const SMatrix2D& smMatrix, const SVector2D& srcVec);
HYPERCORE_API SVector3D operator *(const SMatrix2D& smMatrix, const SVector3D& srcVec);
/*****************************************************************************/