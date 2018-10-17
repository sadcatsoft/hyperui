#pragma once

/*****************************************************************************/
enum FormatIOType
{
	FormatWrite =	0x0001,
	FormatRead =	0x0020,
};

#define FormatReadWrite (FormatRead | FormatWrite)
/*****************************************************************************/
class HYPERUI_API IImageFormatDef
{
public:
	IImageFormatDef()
	{
		myFlags = -1;
	}

	IImageFormatDef(const char* pcsExtension, int iFlags)
	{
		myExtension = pcsExtension;
		myFlags = iFlags;
	}

	virtual ~IImageFormatDef() { }

	inline const char* getExtension() const { return myExtension.c_str(); }
	inline int getFlags() const { return myFlags; }

	inline const char* getDescription() const { return getMainItem()->getStringProp(PropertyDescription); }
	inline const char* getSaveDialogName() const { if(!getMainItem()->doesPropertyExist(PropertySaveDialogName)) return NULL; else return getMainItem()->getStringProp(PropertySaveDialogName); }

	virtual ResourceItem* getMainItem() = 0;
	virtual const ResourceItem* getMainItem() const = 0;

protected:
	string myExtension;
	int myFlags;
};
typedef vector < IImageFormatDef* > TFormatsVector;
/*****************************************************************************/
class HYPERUI_API FormatsManager
{
public:
	virtual ~FormatsManager() { }

	void getExtensionsList(bool bIncludeAllCaps, int iIOType, TStringVector& vecOut, bool bDontListNative = false);
	void getDescriptionsList(bool bIncludeAllCaps, int iIOType, TStringVector& vecOut, bool bDontListNative = false);
	int getFilterIndexFromPath(const char* pcsPath, bool bForWriting);

	bool canOpenAnyFileFrom(const TStringVector& vecFileNames, bool bAllowFolders);
	bool canOpenFile(const char* pcsPath);

	void reduceToSupportedOpenTypes(TStringVector& vecFiles);

	bool getIsNativeFormat(const char* pcsExtension);

	virtual void getAllFormats(TFormatsVector& vecOut) = 0;
	virtual const char* getNativeExtension() = 0;

private:

	IImageFormatDef** getAllFormats();

private:

	// Note that this is currently only set once and not refreshed ever.
	TFormatsVector myCachedFormats;
};
/*****************************************************************************/