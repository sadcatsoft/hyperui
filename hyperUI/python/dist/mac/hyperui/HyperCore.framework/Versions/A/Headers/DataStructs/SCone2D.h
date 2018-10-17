#pragma once

/*****************************************************************************/
// A cone defined by direction and angle.
// The angle, in degrees, specifies the total cone angle, not half angle.
struct HYPERCORE_API SCone2D
{
	bool getIsVisible(const SVector2D& svOwnPos, const SVector2D& svOtherPos) const
	{
		if(myDegAngle <= 0)
			return true;

		SVector2D svDirTo;
		svDirTo = svOtherPos - svOwnPos;
		svDirTo.normalize();

		FLOAT_TYPE fAngle = myDir.getAngleFrom(svDirTo);
		if(fAngle <= myDegAngle/2.0 && fAngle >= -myDegAngle/2.0)
			return true;
		else
			return false;
	}

	SVector2D myDir;
	FLOAT_TYPE myDegAngle;
};
/*****************************************************************************/
