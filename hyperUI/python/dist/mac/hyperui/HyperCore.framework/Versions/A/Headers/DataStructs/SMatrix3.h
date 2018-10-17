#pragma once

/*****************************************************************************/
struct HYPERCORE_API SMatrix3
{
	SMatrix3()
	{
		identity();
	}

	void from4D(SMatrix4& rIn)
	{
		myValues[0] = rIn.myValues[0];
		myValues[1] = rIn.myValues[1];
		myValues[2] = rIn.myValues[2];

		myValues[3] = rIn.myValues[4];
		myValues[4] = rIn.myValues[5];
		myValues[5] = rIn.myValues[6];

		myValues[6] = rIn.myValues[8];
		myValues[7] = rIn.myValues[9];
		myValues[8] = rIn.myValues[10];
	}

	void transpose()
	{
		FLOAT_TYPE tempValues[9];
		memcpy(tempValues, myValues, sizeof(FLOAT_TYPE)*9);

		//myValues[0] = tempValues[];
		myValues[1] = tempValues[3];
		myValues[2] = tempValues[6];

		myValues[3] = tempValues[1];
		//myValues[4] = tempValues[];
		myValues[5] = tempValues[7];

		myValues[6] = tempValues[2];
		myValues[7] = tempValues[5];
		//myValues[8] = tempValues[];
	}

	void identity()
	{
		memset(myValues, 0, sizeof(FLOAT_TYPE)*9);
		myValues[0] = myValues[4] = myValues[8] = 1.0;
	}

	FLOAT_TYPE calculateAdjoint(SMatrix3& svMatrixOut)
	{
		svMatrixOut.myValues[0] = myValues[4]*myValues[8] - myValues[7]*myValues[5];
		svMatrixOut.myValues[1] = -(myValues[1]*myValues[8] - myValues[7]*myValues[2]);
		svMatrixOut.myValues[2] = myValues[1]*myValues[5] - myValues[4]*myValues[2];

		svMatrixOut.myValues[3] = -(myValues[3]*myValues[8] - myValues[6]*myValues[5]);
		svMatrixOut.myValues[4] = myValues[0]*myValues[8] - myValues[6]*myValues[2];
		svMatrixOut.myValues[5] = -(myValues[0]*myValues[5] - myValues[3]*myValues[2]);

		svMatrixOut.myValues[6] = myValues[3]*myValues[7] - myValues[6]*myValues[4];
		svMatrixOut.myValues[7] = -(myValues[0]*myValues[7] - myValues[6]*myValues[1]);
		svMatrixOut.myValues[8] = myValues[0]*myValues[4] - myValues[3]*myValues[1];

		return ((FLOAT_TYPE)(myValues[0]*svMatrixOut.myValues[0] + myValues[3]*svMatrixOut.myValues[1] + myValues[6]*svMatrixOut.myValues[2]));
	}

	bool invert(void)
	{
		FLOAT_TYPE dDet, dInvDet;
		SMatrix3 svAdjoint;
		dDet = calculateAdjoint(svAdjoint);

		if(dDet < FLOAT_EPSILON && dDet > -FLOAT_EPSILON)
			return false;

		dInvDet = 1.0 / dDet;
		myValues[0] = svAdjoint.myValues[0] * dInvDet;
		myValues[3] = svAdjoint.myValues[3] * dInvDet;
		myValues[6] = svAdjoint.myValues[6] * dInvDet;
		myValues[1] = svAdjoint.myValues[1] * dInvDet;
		myValues[4] = svAdjoint.myValues[4] * dInvDet;
		myValues[7] = svAdjoint.myValues[7] * dInvDet;
		myValues[2] = svAdjoint.myValues[2] * dInvDet;
		myValues[5] = svAdjoint.myValues[5] * dInvDet;
		myValues[8] = svAdjoint.myValues[8] * dInvDet;

		return true;

	}

	FLOAT_TYPE myValues[9];
};
/*****************************************************************************/