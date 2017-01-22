/**
 * @file    FileIoHelperClass
 * @brief   
 *
 * This file contains test code for `FileIoHelper` class.
 *
 * @author  Yonhgwhan, Roh (fixbrain@gmail.com)
 * @date    2011.10.13 created.
 * @copyright All rights reserved by Yonghwan, Noh.
**/
#include "stdafx.h"

#include "Win32Utils.h"
#include "FileIoHelperClass.h"

FileIoHelper::FileIoHelper()
:	mReadOnly(TRUE), 
	mFileHandle(INVALID_HANDLE_VALUE), 
	mFileSize(0),
	mFileMap(NULL), 
	mFileView(NULL)
{
}

FileIoHelper::~FileIoHelper()
{
	this->close();
}

/// @brief	I/O �� ����ȭ�� ������� �����Ѵ�. 
///			��Ȯ�� ��ġ�� �𸣰����� SYSTEM_INFO.dwAllocationGranularity(64k) * 8 ������ ���� 
///			������ ������ �����°� ����. (win7, win10 ���� �׽�Ʈ)
//static
uint32_t FileIoHelper::GetOptimizedBlockSize()
{
	static DWORD AllocationGranularity = 0;
	if (0 == AllocationGranularity)
	{
		SYSTEM_INFO si = { 0 };
		GetSystemInfo(&si);
		AllocationGranularity = si.dwAllocationGranularity;
	}

	if (0 == AllocationGranularity)
	{
		return (64 * 1024) * 8;
	}
	else
	{
		return AllocationGranularity * 8;
	}
}

/// @brief	������ �б���� �����Ѵ�. 
bool FileIoHelper::OpenForRead(_In_ const wchar_t* file_path)
{
	if (TRUE == Initialized()) { close(); }

	mReadOnly = TRUE;	
	if (TRUE != is_file_existsW(file_path))
	{
		log_err "no file exists. file=%ws", file_path log_end
		return false;
	}

#pragma warning(disable: 4127)
	bool ret = false;
    do 
    {
        mFileHandle = CreateFileW(file_path,
								  GENERIC_READ,
								  NULL,
								  NULL, 
								  OPEN_EXISTING,
								  FILE_ATTRIBUTE_NORMAL, 
								  NULL);
        if (INVALID_HANDLE_VALUE == mFileHandle)
        {
            log_err
                "CreateFile() failed, file=%ws, gle=0x%08x", 
                file_path,
                GetLastError()
            log_end
            break;
        }

        // check file size 
        // 
		if (TRUE != GetFileSizeEx(mFileHandle, (PLARGE_INTEGER)&mFileSize))
        {
            log_err
                "GetFileSizeEx() failed. file=%ws, gle=0x%08x", 
                file_path,
                GetLastError() 
            log_end
            break;
        }
        
        mFileMap = CreateFileMapping(mFileHandle, 
									 NULL, 
									 PAGE_READONLY,
									 0, 
									 0, 
									 NULL);
        if (NULL == mFileMap)
        {
            log_err
                "CreateFileMapping() failed, file=%ws, gle=0x%08x", 
				file_path,
                GetLastError() 
            log_end
            break;
        }
				
		ret = true;
    } while (FALSE);
#pragma warning(default: 4127)

    if (true != ret)
    {
        if (INVALID_HANDLE_VALUE!=mFileHandle) 
		{
			CloseHandle(mFileHandle);
			mFileHandle = INVALID_HANDLE_VALUE;
		}

		if (NULL != mFileMap)
		{
			CloseHandle(mFileMap);
		}
    }

	return ret;
}


/// @brief	file_size ����Ʈ ¥�� ������ �����Ѵ�.
bool 
FileIoHelper::OpenForWrite(
	_In_ const wchar_t* file_path, 
	_In_ uint64_t file_size
	)
{
	if (TRUE == Initialized()) { close(); }
	if (file_size == 0) return false;

	mReadOnly = FALSE;	
	
#pragma warning(disable: 4127)
	bool ret = false;
    do 
    {
		mFileSize = file_size;

        mFileHandle = CreateFileW(file_path,
								  GENERIC_READ | GENERIC_WRITE, 
								  FILE_SHARE_READ,		// write ���� �ٸ� ���μ������� �бⰡ ����
								  NULL, 
								  CREATE_ALWAYS,
								  FILE_ATTRIBUTE_NORMAL, 
								  NULL);
        if (INVALID_HANDLE_VALUE == mFileHandle)
        {
            log_err
                "CreateFile() failed, file=%ws, gle=0x%08x", 
                file_path,
                GetLastError()
            log_end
            break;
        }

		//
		//	��û�� ũ�⸸ŭ ���ϻ���� �ø���.
		// 
		if (TRUE != SetFilePointerEx(mFileHandle, 
									 *(PLARGE_INTEGER)&mFileSize, 
									 NULL, 
									 FILE_BEGIN))
		{
			log_err
				"SetFilePointerEx() failed, file=%ws, size=%I64d, gle=0x%08x", 
				file_path, 
				file_size,
				GetLastError()				
			log_end
			break;
		}
		
		if (TRUE != SetEndOfFile(mFileHandle))
		{
			log_err "SetEndOfFile() failed, file=%ws, gle=0x%08x", 
				file_path,
				GetLastError() 
				log_end
			break;
		}
        
        mFileMap = CreateFileMapping(mFileHandle, 
									 NULL, 
									 PAGE_READWRITE,
									 0, 
									 0, 
									 NULL);
        if (NULL == mFileMap)
        {
            log_err
                "CreateFileMapping() failed, file=%ws, gle=0x%08x", 
                file_path,
                GetLastError() 
            log_end
            break;
        }
				
		ret = true;
    } while (FALSE);
#pragma warning(default: 4127)

    if (true != ret)
    {
        if (INVALID_HANDLE_VALUE!=mFileHandle) 
		{
			CloseHandle(mFileHandle);
			mFileHandle = INVALID_HANDLE_VALUE;
		}
        if (NULL!= mFileMap) CloseHandle(mFileMap);
    }	

	return ret;
}

/// @brief	��� ���ҽ��� �����Ѵ�.
void FileIoHelper::close()
{
    if (TRUE != Initialized()) return;

    ReleaseFilePointer();
	CloseHandle(mFileMap); mFileMap=NULL;
	CloseHandle(mFileHandle); mFileHandle = INVALID_HANDLE_VALUE;		
}

/// @brief	������ ������ Offset ��ġ�� Size ��ŭ �����ϰ�, �ش� �޸� ������ �����Ѵ�.
///			Offset �� SYSTEM_INFO.dwAllocationGranularity �� ����� �����ؾ� �Ѵ�. 
///			�׷��� ���� ��� �ڵ����� SYSTEM_INFO.dwAllocationGranularity ������ �����ؼ�
///			������ �����ϰ�, pointer �� ������ �����ؼ� �����Ѵ�.
uint8_t* 
FileIoHelper::GetFilePointer(
	_In_ bool read_only, 
	_In_ uint64_t Offset, 
	_In_ uint32_t Size
	)
{
	_ASSERTE(NULL == mFileView);
	if (NULL != mFileView)
	{
		log_err "ReleaseFilePointer() first!" log_end;
		return NULL;
	}

	if (TRUE != Initialized()) return false;
	if (IsReadOnly() && !read_only)
	{
		log_err "file mapped read only." log_end;
		return NULL;
	}

	if (Offset + Size > mFileSize)
	{
		log_err
			"invalid offset. file size=%I64d, req offset=%I64d",
			mFileSize, Offset
			log_end;
		return NULL;
	}

	//
	//	MapViewOfFile() �Լ��� dwFileOffsetLow �Ķ���ʹ� 
	//	SYSTEM_INFO::dwAllocationGranularity ���� ����̾�� �Ѵ�.
	//	Ȥ�ö� ������ ���� 64k �� �����Ѵ�. 
	// 
	static DWORD AllocationGranularity = 0;
	if (0 == AllocationGranularity)
	{
		SYSTEM_INFO si = { 0 };
		GetSystemInfo(&si);
		AllocationGranularity = si.dwAllocationGranularity;
	}
	
	_ASSERTE(0 != AllocationGranularity);
	if (0 == AllocationGranularity)
	{
		AllocationGranularity = (64 * 1024);		
	}

	//
	//	AllocationGranularity ������ ���� ������. 
	//	�ᱹ �����ؾ� �� ������� ������ ������ ��ŭ Ŀ���� �Ѵ�.
	// 	
	uint64_t AdjustMask = (uint64_t)(AllocationGranularity - 1);
	uint64_t adjusted_offset = Offset & ~AdjustMask;
	DWORD adjusted_size = (DWORD)(Offset & AdjustMask) + Size;

	mFileView = (PUCHAR)MapViewOfFile(mFileMap,
								      (TRUE == read_only) ? FILE_MAP_READ : FILE_MAP_READ | FILE_MAP_WRITE,
									  ((PLARGE_INTEGER)&adjusted_offset)->HighPart,
									  ((PLARGE_INTEGER)&adjusted_offset)->LowPart,
									  adjusted_size);
	if (NULL == mFileView)
	{
		log_err
			"MapViewOfFile(high=0x%08x, low=0x%08x, bytes to map=%u) failed, gle=0x%08x",
			((PLARGE_INTEGER)&adjusted_offset)->HighPart,
			((PLARGE_INTEGER)&adjusted_offset)->LowPart,
			adjusted_size,
			GetLastError()
			log_end;
		return NULL;
	}

	//
	//	������ adjust offset ���� ������ �����ϴ� �޸� �����ʹ� 
	//	��û�� ��� �������־�� �Ѵ�.
	// 
	return &mFileView[Offset & AdjustMask];
}


/// @brief	���ε� ���������͸� �������Ѵ�. 
void FileIoHelper::ReleaseFilePointer()
{
	if (NULL != mFileView)
	{
		UnmapViewOfFile(mFileView);
		mFileView = NULL;
	}
}

/// @brief	������ Offset ���� Size ��ŭ �о Buffer �� �����Ѵ�.
bool 
FileIoHelper::ReadFromFile(
	_In_ uint64_t Offset, 
	_In_ DWORD Size, 
	_Inout_updates_bytes_(Size) PUCHAR Buffer
	)
{
	_ASSERTE(NULL != Buffer);
	if (NULL == Buffer) return false;

	uint8_t* src_ptr = GetFilePointer(true, Offset, Size);
	if (NULL == src_ptr)
	{
		log_err "GetFilePointer() failed. offset=0x%I64d, size=%u",
			Offset,
			Size
			log_end;
		return false;
	}

	bool ret = false;
	__try
	{
		RtlCopyMemory(Buffer, src_ptr, Size);
		ret = true;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		log_err
			"exception. offset=0x%I64d, size=%u, code=0x%08x",
			Offset,
			Size,
			GetExceptionCode()
		log_end		
	}

	ReleaseFilePointer();
	return ret;
	
}

/// @brief	Buffer �� ������ Offset �� Size ��ŭ ����.
bool 
FileIoHelper::WriteToFile(
	_In_ uint64_t Offset, 
	_In_ DWORD Size, 
	_In_reads_bytes_(Size) PUCHAR Buffer
	)
{
	_ASSERTE(NULL != Buffer);
	_ASSERTE(0 != Size);
	_ASSERTE(NULL != Buffer);
	if (NULL == Buffer || 0 == Size || NULL == Buffer) return false;

	uint8_t* dst_ptr = GetFilePointer(false, Offset, Size);
	if (NULL == dst_ptr)
	{
		log_err "GetFilePointer() failed. offset=0x%I64d, size=%u",
			Offset,
			Size
			log_end;
		return false;
	}

	bool ret = false;
	__try
	{
		RtlCopyMemory(dst_ptr, Buffer, Size);
		ret = true;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		log_err
			"exception. offset=0x%I64d, size=%u, code=0x%08x",
			Offset,
			Size,
			GetExceptionCode()
			log_end
	}

	ReleaseFilePointer();
	return ret;
}
