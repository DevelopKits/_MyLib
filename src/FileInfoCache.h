/**
 * @file    FileInfoCache.h
 * @brief   
 *
 * @author  Yonhgwhan, Roh (somma@somma.kr)
 * @date    2017/11/14 11:54 created.
 * @copyright (C)Somma, Inc. All rights reserved.
**/
#pragma once

#include "CppSQLite\CppSQLite3.h"


/// @brief	FileInformationn class
typedef class FileInformation
{
public:
	FileInformation()
		:
		size(0xffffffffffffffff),
		create_time(0),
		write_time(0),
		md5("none"),
		sha2("none")
	{
	}

	uint64_t    size;
	uint64_t    create_time;
	uint64_t    write_time;
	std::string md5;            // hex string
	std::string sha2;           // hex string	

} *PFileInformation;



#pragma todo("�� �����ؾ� �� ���� ����")
//	- ĳ���� ����� ������ Ŀ�� �� ����
//		hitcount �ʵ带 �߰��ϰ�
//		�ֱ�������? ĳ�� ����� Ư�� ������ �����ϸ�?
//		hitcount �� ���� ������ ���� �� VACUUM �� ���� (��� �׳� �� ����)
//
//	- file path �� utf8 �� �����ؾ� ��
//	- file path �� ���� ��ҹ��ڰ� ���еǴ°� ������, ��ҹ��� ���� ����� ��
//

typedef class FileInfoCache
{
public:
	FileInfoCache();
	virtual ~FileInfoCache();

	bool initialize(_In_ const wchar_t* db_file_path, _In_ bool delete_if_exist = false);
	void finalize();

	bool get_file_information(_In_ const wchar_t* file_path, _Out_ FileInformation& file_information);

	uint32_t size() {return _size; }
	uint32_t hit_count() {return _hit_count;}

private:
	bool insert_file_info(_In_ const wchar_t* path,
						  _In_ uint64_t create_time,
						  _In_ uint64_t write_time,
						  _In_ uint64_t size,
						  _In_ const char* md5_str,
						  _In_ const char* sha2_str);

	bool get_flie_info(_In_ const wchar_t* path,
					   _In_ uint64_t create_time,
					   _In_ uint64_t write_time,
					   _In_ uint64_t size,
					   _Out_ std::string& md5_str,
					   _Out_ std::string& sha2_str);

	bool file_util_get_hash(_In_ const wchar_t* file_path,
							_Out_ std::string& md5,
							_Out_ std::string& sha2);

private:
	bool            _initialized;
	CppSQLite3DB	_db;
	uint32_t        _size;
	uint32_t        _hit_count;

} *PFileInfoCache;



//
//	Singleton ��ü�� �̿��� C api 
// 
bool fi_initialize();
void fi_finalize();

bool fi_get_file_information(_In_ const wchar_t* file_path, _Out_ FileInformation& file_information);





