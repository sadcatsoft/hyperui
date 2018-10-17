#pragma once

/*****************************************************************************/
class IBaseImage
{
public:
	virtual ~IBaseImage() { }

	virtual bool isValid() = 0;
	virtual void setFrom(const IBaseImage* pSource) = 0;
	virtual void reset() = 0;
	virtual int getWidth() const = 0;
	virtual int getHeight() const = 0;		
};
/*****************************************************************************/