#include "stdafx.h"

// 32 Mb buffer.
#define INITIAL_STORAGE_CAPACITY		((1024*1024)*32)
#define GROWTH_STORAGE_CAPACITY			((1024*1024)*32)
#define ENDIANNESS_TEST					6
#define TYPE_STRING						"OPF"
#define CURRENT_VERSION					100

namespace HyperCore
{
/*****************************************************************************/
PackedFile::PackedFile(char* pcsData, CHUNK_SIZE_TYPE lBufferCapacity, CHUNK_SIZE_TYPE lDataSize)
{
	setDataInternal(pcsData, lBufferCapacity, lDataSize);
}
/*****************************************************************************/
PackedFile::PackedFile()
{
	myData = NULL;
	clear();
}
/*****************************************************************************/
PackedFile::~PackedFile()
{
	clear();
}
/*****************************************************************************/
void PackedFile::setDataInternal(char* pcsData, CHUNK_SIZE_TYPE lBufferCapacity, CHUNK_SIZE_TYPE lDataSize)
{
	myData = pcsData;
	myDataSize = lDataSize;
	myBufferCapacity = lBufferCapacity;
}
/*****************************************************************************/
PackedFile* PackedFile::createFromFilesInFolder(const char* pcsPath, IBinaryDataCompressor* pOptCompressor)
{
	TStringVector vecFiles;
	FileUtils::listFilesOf(pcsPath, vecFiles);

	int iFile, iNumFiles = vecFiles.size();
	if(iNumFiles <= 0)
		return NULL;

	TCharLossyDynamicArray rStorage;
	char* pStorage = rStorage.getArray(INITIAL_STORAGE_CAPACITY);

	// Write out the header
	BinaryData rHeaderData;
	rHeaderData.setData(pStorage, rStorage.getCurrentCapacity(), false);

	rHeaderData.write<short>((short)ENDIANNESS_TEST);
	rHeaderData.writeChars(TYPE_STRING, strlen(TYPE_STRING) + 1);
	rHeaderData.write<int>((int)CURRENT_VERSION);
	rHeaderData.write<int>((int)iNumFiles);

	int iOffsetFromHeader = iNumFiles*sizeof(CHUNK_SIZE_TYPE);
	BinaryData rContentData;
	rContentData.setData(pStorage, rStorage.getCurrentCapacity(), false);
	rContentData.setWritePositionFromOrigin(rHeaderData.getWritePosition() + iOffsetFromHeader);

	bool bNewResult;
	string strShortFileName;
	CHUNK_SIZE_TYPE lCurrLength;
	for(iFile = 0; iFile < iNumFiles; iFile++)
	{
		// Now, load each file as binary.
		BinaryData rLoadedData;
		FileUtils::loadFromFileBinary(vecFiles[iFile].c_str(), FileSourceFullPath, rLoadedData);

		// Write the chunk size
		PathUtils::extractFileNameFromPath(vecFiles[iFile].c_str(), strShortFileName);
		lCurrLength = strShortFileName.length() + 1 + sizeof(int) + rLoadedData.getLength();
		rHeaderData.write<CHUNK_SIZE_TYPE>(lCurrLength);

		// Write the actual chunk data
		if(!rContentData.doHaveEnoughSpaceFor(lCurrLength))
		{
			// If we failed, it's because we're out of bounds of our array.
			// Resize the array and try again
			pStorage = rStorage.getArray(rStorage.getCurrentCapacity() + GROWTH_STORAGE_CAPACITY);

			// Now, we must replace our pointers in our binary data structures!
			rHeaderData.replaceDataPointer(pStorage);
			rContentData.replaceDataPointer(pStorage);
		}

		// And try again
		bNewResult = rContentData.writeString(strShortFileName);
		_ASSERT(bNewResult);
		bNewResult = rContentData.writeChars<char>(rLoadedData.getData(), rLoadedData.getLength());
		_ASSERT(bNewResult);
	}

	PackedFile *pResult = NULL;
	if(pOptCompressor)
	{
		int iCompressedSize;
		char* pcsCompressedData = pOptCompressor->compressBinaryData(rStorage.getArray(), rContentData.getWritePosition(), iCompressedSize);

		// Note how we don't disown the memory here - we want it to be freed, since we're already allocated the new chunk 
		// for compressed result.
		if(pcsCompressedData)
			pResult = new PackedFile(pcsCompressedData, iCompressedSize, iCompressedSize);
	}
	else
	{
		pResult = new PackedFile(rStorage.getArray(), rStorage.getCurrentCapacity(), rContentData.getWritePosition());
		rStorage.disownMemory();
	}

	return pResult;
}
/*****************************************************************************/
void PackedFile::clear()
{
	delete myData;
	myData = NULL;
	myDataSize = 0;
	myBufferCapacity = 0;
	myChunksBeginPosition = 0;
	myChunkInfos.clear();
}
/*****************************************************************************/
void PackedFile::save(const char* pcsPath)
{
	FileUtils::saveToFileBinary(pcsPath, myData, myDataSize, true);
}
/*****************************************************************************/
PackedFile* PackedFile::load(const char* pcsPath, IBinaryDataCompressor* pOptDecompressor)
{
	BinaryData rSourceData;
	if(pOptDecompressor)
	{
		BinaryData rCompressedSource;
		FileUtils::loadFromFileBinary(pcsPath, FileSourceFullPath, rCompressedSource);
		int iUncompressedSizeOut;

		char* pcsResult = pOptDecompressor->decompressBinaryData(rCompressedSource.getData(), rCompressedSource.getLength(), iUncompressedSizeOut);
		rSourceData.setData(pcsResult, iUncompressedSizeOut, true);
	}
	else
		FileUtils::loadFromFileBinary(pcsPath, FileSourceFullPath, rSourceData);

	if(!rSourceData.isValid())
		return NULL;

	// Now, start loading data
	rSourceData.testAndSetEndianness<short>(ENDIANNESS_TEST, false);
	char pcsBuff[4];
	rSourceData.readChars(strlen(TYPE_STRING) + 1, pcsBuff);
	if(!IS_STRING_EQUAL(pcsBuff, TYPE_STRING))
		ASSERT_RETURN_NULL;

	int iVersion = 0;
	rSourceData.read<int>(iVersion);
	int iNumSections = 0;
	rSourceData.read<int>(iNumSections);

	if(iNumSections <= 0)
		return NULL;

	PackedFile* pRes = new PackedFile;

	int iOffsetFromHeader = iNumSections*sizeof(CHUNK_SIZE_TYPE);

	CHUNK_SIZE_TYPE lCumulLengthSoFar = rSourceData.getReadPosition();
	lCumulLengthSoFar += iOffsetFromHeader;
	pRes->myChunksBeginPosition = lCumulLengthSoFar;

	// Now, loop through the sections
	BinaryData rContentData(rSourceData.getData(), rSourceData.getLength(), false);
	CHUNK_SIZE_TYPE lCurrSectionLength;
	int iSection;
	for(iSection = 0; iSection < iNumSections; iSection++)
	{
		rSourceData.read<CHUNK_SIZE_TYPE>(lCurrSectionLength);

		// Read the name
		SPackedFileSectionInfo rChunkInfo;
		rContentData.setReadPositionFromOrigin(lCumulLengthSoFar);
		rChunkInfo.myName = rContentData.readString();
		rChunkInfo.myCumulOffset = rContentData.getReadPosition();
		// Subtract our name.
		rChunkInfo.mySize = lCurrSectionLength - (rChunkInfo.myCumulOffset - lCumulLengthSoFar);

		lCumulLengthSoFar += lCurrSectionLength;

		pRes->myChunkInfos[rChunkInfo.myName] = rChunkInfo;
	}

	// Now, what we do is save the data, disown it, and return the pointer.
	pRes->setDataInternal(rSourceData.getData(), rSourceData.getLength(), rSourceData.getLength());
	rSourceData.disownMemory();

	return pRes;
}
/*****************************************************************************/
const char* PackedFile::getSectionDataPointer(const char* pcsName, CHUNK_SIZE_TYPE& lSizeOut) const
{
	lSizeOut = 0;
	TPackedFileSectionInfos::const_iterator mi = myChunkInfos.find(pcsName);
	if(mi == myChunkInfos.end())
		return NULL;
	else
	{
		lSizeOut = mi->second.mySize;
		return myData + mi->second.myCumulOffset;
	}
}
/*****************************************************************************/
void PackedFile::extractAsFilesIntoFolder(const char* pcsPath)
{
	const char* pcsBuff;
	string strFullName;
	CHUNK_SIZE_TYPE lSecSize;
	Iterator si;
	for(si = sectionsBegin(); !si.isEnd(); si++)
	{
		strFullName = pcsPath;
		if(strFullName[strFullName.length() - 1] != FOLDER_SEP[0])
			strFullName += FOLDER_SEP;

		strFullName += si.getSectionInfo()->myName;
		pcsBuff = si.getSectionDataPointer(lSecSize);
		FileUtils::saveToFileBinary(strFullName.c_str(), pcsBuff, lSecSize, true);
	}
}
/*****************************************************************************/
};
