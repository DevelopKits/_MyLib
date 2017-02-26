/**
 * @file    FileIoHelperClass
 * @brief   
 *
 * This file contains test code for `FileIoHelper` class.
 *
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2011.10.13 created.
 * @copyright All rights reserved by Yonghwan, Roh.
**/
#pragma once

/// @brief	MMIO �� ��ƿ��Ƽ Ŭ����.
///			mFileView �����ʹ� ������ �������� �������� �����Ƿ�, 
///			��Ƽ������ ȯ�濡�� ����ϸ� �ȵ�
typedef class FileIoHelper
{
private:
	BOOL			mReadOnly;
	HANDLE			mFileHandle;
	uint64_t		mFileSize;
	HANDLE			mFileMap;
	PUCHAR			mFileView;
public:
	FileIoHelper();
	~FileIoHelper();

	static uint32_t GetOptimizedBlockSize();

	BOOL Initialized()	{ return (INVALID_HANDLE_VALUE != mFileHandle) ? TRUE : FALSE;}
	BOOL IsReadOnly()	{ return (TRUE == mReadOnly) ? TRUE : FALSE;}	

	bool OpenForRead(_In_ const wchar_t* file_path);
	bool OpenForRead(_In_ const HANDLE file_handle);
	bool OpenForWrite(_In_ const wchar_t* file_path, _In_ uint64_t file_size);
	void close();

	bool GetMappedFileName(_In_ bool convet_to_dosname, _Out_ std::wstring& file_path);

	uint8_t* GetFilePointer(_In_ bool read_only, _In_ uint64_t Offset, _In_ uint32_t Size);
	void ReleaseFilePointer();

	bool ReadFromFile(_In_ uint64_t Offset, _In_ DWORD Size, _Inout_updates_bytes_(Size) PUCHAR Buffer);
	bool WriteToFile(_In_ uint64_t Offset, _In_ DWORD Size, _In_reads_bytes_(Size) PUCHAR Buffer);

	uint64_t  FileSize(){ return mFileSize; }

	

}*PFileIoHelper;