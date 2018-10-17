#pragma once

/*

Packed file format v1:

Size			Type		Value			Desc
[2 bytes]		Short		6				Tester for big/little endianness. Always equals 6.
[3 bytes]		char		OPF				Format id. OPF = Ormr Packed File
[4 bytes]		int			100				Version number, as a multiple of 100 = 1.00
[4 bytes]		int			[varies]		Total number of chunks in file
[8 bytes]		long long	[varies]		A list of chunk sizes.
...
[Section data chunks]

*/

#define CHUNK_SIZE_TYPE					long long
/*****************************************************************************/
struct SPackedFileSectionInfo
{
	CHUNK_SIZE_TYPE mySize;
	CHUNK_SIZE_TYPE myCumulOffset;
	string myName;
};
typedef map < string, SPackedFileSectionInfo > TPackedFileSectionInfos;
/*****************************************************************************/
class IBinaryDataCompressor
{
public:
	virtual ~IBinaryDataCompressor() { }

	virtual char* compressBinaryData(const char* input, long long input_length, int& iCompressedDataSizeOut) = 0;
	virtual char* decompressBinaryData(const char* input, size_t input_length, int& iUncompressedSizeOut) = 0;
};
/*****************************************************************************/
class PackedFile
{
public:

	HYPERCORE_API PackedFile();
	HYPERCORE_API ~PackedFile();
	HYPERCORE_API void clear();

	HYPERCORE_API static PackedFile* createFromFilesInFolder(const char* pcsPath, IBinaryDataCompressor* pOptCompressor = NULL);
	
	HYPERCORE_API static PackedFile* load(const char* pcsPath, IBinaryDataCompressor* pOptDecompressor = NULL);
	HYPERCORE_API void save(const char* pcsPath);

	HYPERCORE_API const char* getSectionDataPointer(const char* pcsName, CHUNK_SIZE_TYPE& lSizeOut) const;

	HYPERCORE_API void extractAsFilesIntoFolder(const char* pcsPath);

	// Iterator
	class Iterator
	{
	public:
		Iterator(const PackedFile& rCollection)
		{
			myPackedFile = &rCollection;
			myIterator = myPackedFile->myChunkInfos.begin();
		}
		Iterator() { myPackedFile = NULL; }

		inline void operator++(int) { myIterator++; }
		inline void operator--(int) { myIterator--; }
		inline void toEnd() { myIterator = myPackedFile->myChunkInfos.end(); }
		inline bool isEnd() const { return myIterator == myPackedFile->myChunkInfos.end(); }
		inline bool operator==(const Iterator& other) const { return myIterator == other.myIterator && myPackedFile == other.myPackedFile; }
		inline bool operator!=(const Iterator& other) const { return ! (*this == other); }
		inline const SPackedFileSectionInfo* getSectionInfo() { return &myIterator->second; }
		inline const char* getSectionDataPointer(CHUNK_SIZE_TYPE& lSizeOut) { return myPackedFile->getSectionDataPointer(myIterator->second.myName.c_str(), lSizeOut); }
	private:
		const PackedFile *myPackedFile;
		TPackedFileSectionInfos::const_iterator myIterator;
	};
	Iterator sectionsBegin() const { Iterator rTempIter(*this); return rTempIter; }

private:
	PackedFile(char* pcsData, CHUNK_SIZE_TYPE lBufferCapacity, CHUNK_SIZE_TYPE lDataSize);
	void setDataInternal(char* pcsData, CHUNK_SIZE_TYPE lBufferCapacity, CHUNK_SIZE_TYPE lDataSize);


private:
	char* myData;
	// Actual size of meaningful data.
	CHUNK_SIZE_TYPE myDataSize;
	// Buffer capacity, which may be larger.
	CHUNK_SIZE_TYPE myBufferCapacity;

	CHUNK_SIZE_TYPE myChunksBeginPosition;
	TPackedFileSectionInfos myChunkInfos;
};
/*****************************************************************************/