#include "stdafx.h"
#include "WinManaged.h"

namespace HyperCore
{

#ifdef WINDOWS
#pragma managed(pop)
#pragma managed(push, on)
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


/*****************************************************************************/
void FileUtils::createFolderIn(const char* pcsPath, const char* pcsFolder)
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else
	string strTemp(pcsPath);
	if(pcsFolder)
	{
		strTemp += FOLDER_SEP;
		strTemp += pcsFolder;
	}
	System::IO::Directory::CreateDirectory(gcnew System::String(strTemp.c_str()));
#endif
}
/*****************************************************************************/
void FileUtils::listSubfoldersOf(const char* pcsPath, TStringVector& strSubfoldersOut, bool bRecursive, bool bAppendToVector)
{
	if(!bAppendToVector)
		strSubfoldersOut.clear();

#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else
	try
	{
		cli::array<System::String^> ^pSubdirs = System::IO::Directory::GetDirectories(gcnew System::String(pcsPath));
		int iElem;
		string strTemp;
		for(iElem = 0; pSubdirs && iElem < pSubdirs->Length; iElem++)
		{
			stringToSTLString(pSubdirs[iElem], strTemp);
			strSubfoldersOut.push_back(strTemp);

			// If recursive, recurse:
			if(bRecursive)
				FileUtils::listSubfoldersOf(strTemp.c_str(), strSubfoldersOut, true, true);
		}
	}
	catch (...) { }
#endif
}
/*****************************************************************************/
void FileUtils::deleteFile(const char*pcsFilename, bool bFullPath)
{
#ifdef DIRECTX_PIPELINE

	HANDLE sigEvent = CreateEventEx(NULL, NULL, 0, EVENT_MODIFY_STATE | SYNCHRONIZE | DELETE);

	wchar_t* wchar_str = StringUtils::asciiToWide(pcsFilename);
	StorageFolder^ documentsFolder = ApplicationData::Current->LocalFolder;
	Platform::String ^filename = ref new Platform::String(wchar_str);
	StringUtils::freeWideText(wchar_str);

	auto op1 = create_async([sigEvent, documentsFolder, filename] 
	{

		auto fileOperation = documentsFolder->GetFileAsync(filename);

		// This is retarded. All I want is to synchronosly delete a file. Is that so much to ask for?
		// This is what is it took in 2.0: System::IO::File::Delete(ref new System::String(pcsFilename));
		// Thank you MSFT for fucking up .NET 2.0 so badly.
		fileOperation->Completed = ref new AsyncOperationCompletedHandler<StorageFile^>([sigEvent](IAsyncOperation<StorageFile^>^ operation, AsyncStatus status) 
		{
			// Get the returned file and displays basic file properties
			//if (operation->Status == AsyncStatus::Completed)
			if (status == AsyncStatus::Completed)
			{
				StorageFile^ storagefileSample = operation->GetResults();
				//Platform::String^ outputText = storagefileSample->FileName + " was deleted.";
				auto deleteOperation = storagefileSample->DeleteAsync();
				deleteOperation->Completed = ref new AsyncActionCompletedHandler([sigEvent](IAsyncAction^ action, AsyncStatus status)
				{
					SetEvent(sigEvent);
				});
			}
			else
			{
				// Error
				SetEvent(sigEvent);
			}
		});

	});


	// 	DWORD dummy;
	// 	CoWaitForMultipleHandles(COWAIT_INPUTAVAILABLE /*| COWAIT_DISPATCH_WINDOW_MESSAGES*/ | COWAIT_DISPATCH_CALLS, INFINITE, 1, &sigEvent, &dummy);
	WaitForMultipleObjectsEx(1, &sigEvent, true, INFINITE, true);
	CloseHandle(sigEvent);
#else
	const char* pcsFinalPtr;
	string strPath;
	if(bFullPath)
		pcsFinalPtr = pcsFilename;
	else
	{
		getCommonAppDataPath(strPath);
		strPath += "\\";
		strPath += pcsFilename;
		pcsFinalPtr = strPath.c_str();
	}

	try 
	{
		System::IO::File::Delete(gcnew System::String(pcsFinalPtr));
	}
	catch (...) { }
#endif
}
/*****************************************************************************/
bool FileUtils::doesFolderExist(const char* pcsFolderPath, FileSourceType eFileSourceType)
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else

	try
	{
		string strPath;
		const char* pcsFinalFilePathPtr = pcsFolderPath;
		if(eFileSourceType == FileSourceUserDir)
		{
			getCommonAppDataPath(strPath);
			strPath += FOLDER_SEP;
			strPath += pcsFolderPath;
			pcsFinalFilePathPtr = strPath.c_str();
		}

		return System::IO::Directory::Exists(gcnew System::String(pcsFinalFilePathPtr));
	}
	catch(...)
	{
		//_ASSERT(0);
	}
#endif
	return false;
}
/*****************************************************************************/
UTC_TYPE FileUtils::getFileModifiedDate(const char* pcsPath)
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else

	try
	{
		System::IO::FileInfo^ pFileInfo = gcnew  System::IO::FileInfo(gcnew System::String(pcsPath));
		return (pFileInfo->LastWriteTimeUtc - System::DateTime(1970, 1, 1)).TotalSeconds;
	}
	catch (...)
	{
		//_ASSERT(0);
	}
#endif
	return 0;
}
/*****************************************************************************/
HUGE_SIZE_TYPE FileUtils::getFileSize(const char* pcsPath)
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else

	try
	{
		return (gcnew System::IO::FileInfo(gcnew System::String(pcsPath)))->Length;
	}
	catch(...)
	{
	}
#endif
	return 0;
}
/*****************************************************************************/
void FileUtils::killLoadFileCache()
{
	// Nothing on Windows.
}
/*****************************************************************************/
void FileUtils::listFilesOf(const char* pcsPath, TStringVector& strFilesOut, bool bAppendToVector)
{
	if(!bAppendToVector)
		strFilesOut.clear();

#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else

	try
	{
		cli::array<System::String^> ^pSubdirs = System::IO::Directory::GetFiles(gcnew System::String(pcsPath));
		int iElem;
		string strTemp;
		for(iElem = 0; pSubdirs && iElem < pSubdirs->Length; iElem++)
		{
			stringToSTLString(pSubdirs[iElem], strTemp);
			strFilesOut.push_back(strTemp);
		}
	}
	catch (...) { }
#endif
}
/*****************************************************************************/
void FileUtils::deleteFolder(const char*pcsFullFolderPath)
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else
	System::IO::Directory::Delete(gcnew System::String(pcsFullFolderPath), true);
#endif
}
/*****************************************************************************/
void FileUtils::makeFileHidden(const char* pcsPath)
{
#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else
	System::String ^pPath = gcnew System::String(pcsPath);
	System::IO::File::SetAttributes(pPath, System::IO::File::GetAttributes(pPath) | System::IO::FileAttributes::Hidden);
#endif
}
/*****************************************************************************/
bool FileUtils::getIsPathAbsolute(const char* pcsPath)
{
	if(strlen(pcsPath) < 3)
		return false;

	return (isalpha(pcsPath[0]) && pcsPath[1] == ':');
}
/*****************************************************************************/
void FileUtils::listAllDrives(TDriveInfos& vecDrivesOut)
{
	vecDrivesOut.clear();

#ifdef DIRECTX_PIPELINE
	DXSTAGE1
#else
	try
	{
		cli::array<System::IO::DriveInfo^>^ pDrives = System::IO::DriveInfo::GetDrives();
		int iElem;
		string strTemp;
		SDriveInfo rInfo;
		for(iElem = 0; pDrives && iElem < pDrives->Length; iElem++)
		{
			rInfo.myType = DriveUnknown;
			rInfo.myLabel = "";
			stringToSTLString(pDrives[iElem]->Name, rInfo.myName);
			try
			{
				stringToSTLString(pDrives[iElem]->VolumeLabel, rInfo.myLabel);
			}
			catch (...)	{ }
			vecDrivesOut.push_back(rInfo);
		}
	}
	catch(...) { }
#endif
}
/*****************************************************************************/
void gStartOpeningPath(const char* pcsPath)
{

}
/*****************************************************************************/
void gEndOpeningPath(const char* pcsPath)
{

}
/*****************************************************************************/
};