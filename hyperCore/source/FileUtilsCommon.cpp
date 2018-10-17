#include "stdafx.h"
#include <sys/stat.h>

#ifdef LINUX
#include <unistd.h>
#endif

#ifdef DIRECTX_PIPELINE
using namespace Windows::System::Threading;
#include <ppl.h>
#include <ppltasks.h>

using namespace Windows::Storage;
using namespace Concurrency;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage::AccessCache;
using namespace Windows::Storage::FileProperties;
using namespace Windows::Foundation;

#endif

namespace HyperCore
{
/*****************************************************************************/
bool FileUtils::doesFileExist(const char* pcsFilename, FileSourceType eFileSourceType)
{
#ifdef DIRECTX_PIPELINE
//	if(bInUserDocuments)
	{
		HANDLE sigEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE | DELETE);

		bool bResult = false;
		wchar_t* wchar_str = StringUtils::asciiToWide(pcsFilename);
		StorageFolder^ documentsFolder = nullptr;

		if(eFileSourceType == FileSourceUserDir)
			documentsFolder = ApplicationData::Current->LocalFolder;
		else if(eFileSourceType == FileSourcePackageDir)
			documentsFolder = Windows::ApplicationModel::Package::Current->InstalledLocation;
		else
		{
			_ASSERT(0);
		}

		Platform::String ^filename = ref new Platform::String(wchar_str);
		StringUtils::freeWideText(wchar_str);
		auto op1 = create_async([sigEvent, documentsFolder, filename, &bResult] 
		{ 
			auto fileOperation = documentsFolder->GetFileAsync(filename);
			fileOperation->Completed = ref new AsyncOperationCompletedHandler<StorageFile^>([&bResult, sigEvent](IAsyncOperation<StorageFile^>^ operation, AsyncStatus status) 
			{
/*
				// Get the returned file and displays basic file properties
				char bTest[16];
				for(int i =0; i< 100000000; i++)
				{ memset(bTest, 0, 16);	}
				*/
				if (status == AsyncStatus::Completed)
				{  bResult = true;	}
				else
				{  bResult = false; }
				SetEvent(sigEvent);
			});

/*
			task<StorageFile^>(documentsFolder->GetFileAsync(filename)).then([&bResult, sigEvent] (StorageFile^ file) 
			{ 
				char bTest[16];
				for(int i =0; i< 100000000; i++)
				{ memset(bTest, 0, 16);	}

				bResult = true;
				SetEvent(sigEvent);
				// return FileIO::ReadBufferAsync(file); 
			}, task_continuation_context::use_current());
*/
			//SetEvent(sigEvent);
		});

		

		WaitForMultipleObjectsEx(1, &sigEvent, true, INFINITE, true);
		//CoWaitForMultipleHandles(COWAIT_INPUTAVAILABLE /*| COWAIT_DISPATCH_WINDOW_MESSAGES */ | COWAIT_DISPATCH_CALLS, INFINITE, 1, &sigEvent, &dummy);
		CloseHandle(sigEvent);		

		return bResult;

	}
#else

	string strPath;
	const char* pcsFinalFilePathPtr = pcsFilename;
	if(eFileSourceType == FileSourceUserDir)
	{
		getCommonAppDataPath(strPath);
		strPath += FOLDER_SEP;
		strPath += pcsFilename;
		pcsFinalFilePathPtr = strPath.c_str();
	}

	struct stat buffer;   
	return (stat (pcsFinalFilePathPtr, &buffer) == 0);  
#endif
}
/*****************************************************************************/
bool FileUtils::loadFromFile(const char* pcsFilename, string &str, FileSourceType eFileSource)
{
#ifdef DIRECTX_PIPELINE
	str = "";

	if(!FileUtils::doesFileExist(pcsFilename, eFileSource))
		return false;

	//if(eFileSource == FileSourceUserDir)
	{
		HANDLE sigEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE | DELETE);
		StorageFolder^ documentsFolder = nullptr;
		if(eFileSource == FileSourceUserDir)
			documentsFolder = ApplicationData::Current->LocalFolder;
		else if(eFileSource == FileSourcePackageDir)
			documentsFolder = Windows::ApplicationModel::Package::Current->InstalledLocation;
		else
		{
			// Unknown or not allowed.
			_ASSERT(0);
		}

		wchar_t* wchar_str = StringUtils::asciiToWide(pcsFilename);
		Platform::String ^filename = ref new Platform::String(wchar_str);
		StringUtils::freeWideText(wchar_str);

		auto op1 = create_async([sigEvent, documentsFolder, filename, &str] 
		{

			task<StorageFile^> getFileTask(documentsFolder->GetFileAsync(filename));		
			auto readBufferTask = getFileTask.then([] (StorageFile^ f) 
			{
				return FileIO::ReadBufferAsync(f);
			});

			// NEW TEMP
			auto byteArrayTask = readBufferTask.then([&str, sigEvent] (Streams::IBuffer^ b) // -> ByteArray 
				//auto byteArrayTask = readBufferTask.then([&str] (Streams::IBuffer^ b) // -> ByteArray 
			{
				auto a = ref new Platform::Array<byte>(b->Length);
				Streams::DataReader::FromBuffer(b)->ReadBytes(a);

				char* pcsTempChar = new char[a->Length + 1];
				int i;
				for(i = 0; i < a->Length; ++i )
					pcsTempChar[i] = a[i];
				pcsTempChar[i] = 0;

				str = pcsTempChar;

				delete[] pcsTempChar;

				// NEW TEMP
				SetEvent(sigEvent);

				delete a;

				//ByteArray ba = { a };
				//return ba;
			});

		});

		// NEW TEMP
		// 		DWORD dummy;
		// 		CoWaitForMultipleHandles(COWAIT_INPUTAVAILABLE /*| COWAIT_DISPATCH_WINDOW_MESSAGES */ | COWAIT_DISPATCH_CALLS, INFINITE, 1, &sigEvent, &dummy);
		WaitForMultipleObjectsEx(1, &sigEvent, true, INFINITE, true);
		CloseHandle(sigEvent);
	}
	//else
	return true;
#else
	string strPath;
	const char* pcsFinalFilePathPtr = pcsFilename;
	if(eFileSource == FileSourceUserDir)
	{
		getCommonAppDataPath(strPath);
		strPath += FOLDER_SEP;
		strPath += pcsFilename;
		pcsFinalFilePathPtr = strPath.c_str();
	}


	str = "";

	FILE *out;
	out = fopen(pcsFinalFilePathPtr, "r");
	if(!out)
		return false;

	// Read num bytes
	fseek(out, 0, SEEK_END);
	int iReadSize = ftell(out);
#ifdef LINUX
	if(iReadSize == 0)
	{
        // This may be a sym link
        fclose(out);
        out = NULL;
        char buf[2048];
        int len;
        if ((len = readlink(pcsFinalFilePathPtr, buf, sizeof(buf)-1)) != -1)
            buf[len] = '\0';

        if(len > 0)
        {
            string strTemp;
            if(buf[0] != '/')
            {
                PathUtils::extractPathFromFullFilename(pcsFinalFilePathPtr, strTemp);
            }
            strTemp += buf;
            out = fopen(strTemp.c_str(), "r");
        }
        if(out)
            iReadSize = ftell(out);
	}
	if(!out)
        return false;
#endif

	fseek(out, 0, SEEK_SET);
	char *pcsTemp = new char[iReadSize + 1];
	memset(pcsTemp, 0, sizeof(char)*(iReadSize + 1));
	int iActuallyRead = fread(pcsTemp, sizeof(char), iReadSize, out);
	fclose(out);

	pcsTemp[iActuallyRead] = 0;
	str = pcsTemp;

	delete[] pcsTemp;

	return true;
#endif
}
/*****************************************************************************/
char* FileUtils::loadFromFileBinary(const char* pcsFilename, FileSourceType eFileSource, int &iBytesOut)
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE1
		return NULL;
#else
	string strPath;
	const char* pcsFinalFilePathPtr = pcsFilename;
	if(eFileSource == FileSourceUserDir)
	{
		getCommonAppDataPath(strPath);
		strPath += FOLDER_SEP;
		strPath += pcsFilename;
		pcsFinalFilePathPtr = strPath.c_str();
	}

	iBytesOut = 0;
	char *pcsTemp;
	FILE *out;
	out = fopen(pcsFinalFilePathPtr, "rb");
	if(!out)
		return NULL;

	// Read num bytes
	fseek(out, 0, SEEK_END);
	int iReadSize = ftell(out);
	fseek(out, 0, SEEK_SET);
	pcsTemp = new char[iReadSize + 1];
	memset(pcsTemp, 0, sizeof(char)*(iReadSize + 1));
	int iActuallyRead = fread(pcsTemp, sizeof(char), iReadSize, out);
	fclose(out);

	iBytesOut = iActuallyRead;
	return pcsTemp;
#endif
}
/*****************************************************************************/
void FileUtils::loadFromFileBinary(const char* pcsFilename, FileSourceType eFileSource, BinaryData& rDataOut)
{
	int iLength;
	char* pcsData = loadFromFileBinary(pcsFilename, eFileSource, iLength);
	rDataOut.setData(pcsData, iLength, true);
	//rDataOut.myData = loadFromFileBinary(pcsFilename, eFileSource, rDataOut.myLength);
}
/*****************************************************************************/
bool FileUtils::saveToFile(const char* pcsFilename, const char* pcsString, bool bFullPath)
{
#ifdef DIRECTX_PIPELINE
	_ASSERT(!bFullPath);
	return FileUtils::saveToFileBinary(pcsFilename, pcsString, STRLEN(pcsString)*sizeof(CHAR_TYPE) + 1, bFullPath);
#else

	const char* pcsFinalPtr = pcsFilename;
	string strPath;
	if(!bFullPath)
	{
		getCommonAppDataPath(strPath);
		strPath += FOLDER_SEP;
		strPath += pcsFilename;
		pcsFilename = strPath.c_str();
	}

	FILE* out = fopen(pcsFilename, "w");
	if(!out)
		return false;
	fprintf(out, "%s", pcsString);
	fclose(out);

	return true;
#endif
}
/*****************************************************************************/
bool FileUtils::saveToFileBinary(const char* pcsFilename, const char* pcsBuff, int iLen, bool bFullPath /*= false*/)
{
#ifdef DIRECTX_PIPELINE

	HANDLE sigEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE | DELETE);

	StorageFolder^ documentsFolder = ApplicationData::Current->LocalFolder;
	wchar_t* wchar_str = StringUtils::asciiToWide(pcsFilename);
	Platform::String^ filename = ref new Platform::String(wchar_str);
	StringUtils::freeWideText(wchar_str);

	Platform::Array < unsigned char> ^ pTempArray =  ref new Platform::Array < unsigned char>(iLen); // +1
	for(int i = 0; i < iLen; i++)
		pTempArray[i] = pcsBuff[i];

	_ASSERT(!bFullPath);

	bool bResult = false;
	auto op1 = create_async([sigEvent, documentsFolder, filename, iLen, pTempArray, bResult]
	{
		InMemoryRandomAccessStream^ memoryStream = ref new InMemoryRandomAccessStream();
		DataWriter^ dataWriter = ref new DataWriter(memoryStream);

		dataWriter->WriteBytes(pTempArray);
		IBuffer^ buffer = dataWriter->DetachBuffer();


		auto fileOperation = documentsFolder->CreateFileAsync(filename, CreationCollisionOption::ReplaceExisting);

		fileOperation->Completed = ref new AsyncOperationCompletedHandler<StorageFile^>([buffer, sigEvent, pTempArray, bResult](IAsyncOperation<StorageFile^>^ operation, AsyncStatus status)  mutable
		{
			if (status == AsyncStatus::Completed)
			{			
				StorageFile^ storagefileSample = operation->GetResults();
				task<void>(FileIO::WriteBufferAsync(storagefileSample, buffer)).then([buffer, sigEvent, pTempArray, bResult]() mutable
				{
					bResult = true;
					delete pTempArray;
					SetEvent(sigEvent);
				});
			}
			else
			{ 
				_ASSERT(0); 
				SetEvent(sigEvent);
			}

		});

	});

	// 	DWORD dummy;
	// 	CoWaitForMultipleHandles(COWAIT_INPUTAVAILABLE | COWAIT_DISPATCH_CALLS, INFINITE, 1, &sigEvent, &dummy);
	WaitForMultipleObjectsEx(1, &sigEvent, true, INFINITE, true);
	CloseHandle(sigEvent);

	return bResult;

#else
	const char* pcsFinalPtr = pcsFilename;
	string strPath;
	if(!bFullPath)
	{
		getCommonAppDataPath(strPath);
		strPath += FOLDER_SEP;
		strPath += pcsFilename;
		pcsFilename = strPath.c_str();
	}


	FILE* out = fopen(pcsFinalPtr, "wb");
	if(!out)
		return false;

	int iWritten = fwrite(pcsBuff, sizeof(char), iLen, out);
	_ASSERT(iWritten == iLen);
	fclose(out);

	return true;
#endif
}
/*****************************************************************************/
bool FileUtils::canAccessFileForWriting(const char* pcsPath)
{
	FILE *out = fopen(pcsPath, "w");
	if(out)
	{
		fclose(out);
		return true;
	}
	else
		return false;
}
/*****************************************************************************/
bool FileUtils::canAccessFileForReading(const char* pcsPath)
{
	FILE *out = fopen(pcsPath, "rb");
	if(out)
	{
		fclose(out);
		return true;
	}
	else
		return false;
}
/*****************************************************************************/
};