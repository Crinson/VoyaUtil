
#pragma once

#include <string>
#include <memory>
#include "VoyaUtil/VoyaAssert.h"
#include "VoyaUtil/VoyaBuffer.h"

namespace voya
{

#ifdef WIN32
	#include <Windows.h>
	#include <direct.h>
	#include <io.h>
#else
	#include <stdarg.h>
	#include <sys/stat.h>
	#include <unistd.h>
#endif

#ifdef WIN32
	#define ACCESS _access
	#define MKDIR(_pszPath) _mkdir(_pszPath)
#else
	#define ACCESS access
	#define MKDIR(_pszPath) mkdir((_pszPath), 0755)
#endif

	/************************************************************************/
	/* 文件相关操作                                                                     */
	/************************************************************************/
	class CFileUtil
	{
	private:
		const static int FILE_NOT_EXIST = -1;
		const static int FILE_BUFFER_SIZE = 1024 * 128;		//128K

	public:
		//路径所指是否存在
		static bool isExist(const char* _pszPath)
		{
			voya_assert(_pszPath, "Path is nullptr!");
			return (FILE_NOT_EXIST != ACCESS(_pszPath, 0));
		}

		//路径是否文件夹
		static bool isDirectory(const char* _pszPath)
		{
			voya_assert(_pszPath, "Path is nullptr!");
			if (!isExist(_pszPath))
			{
				return false;
			}

#ifdef WIN32
			unsigned long lAttr = GetFileAttributesA(_pszPath);
			if (INVALID_FILE_ATTRIBUTES != lAttr && (FILE_ATTRIBUTE_DIRECTORY & lAttr))
			{
				return true;
			}
			return false;
#else
			struct stat stats;
			if ((0 == stat(_pszPath, &stats)) && S_ISDIR(stats.st_mode))
			{
				return true;
			}
			return false;
#endif

		}

		//路径标准化(把'\\'变成'/')
		static void pathNormalize(std::string& _strPath)
		{
			for (char& c : _strPath)
			{
				if ('\\' == c)
				{
					c = '/';
				}
			}
		}

		//创建多层目录(_bStrict为true，最后一层也需要检测'/'才会创建目录)
		static bool mkdirs(const char* _pszPath, bool _bStrict = false)
		{
			voya_assert(_pszPath, "Path is nullptr!");

			if (isExist(_pszPath))
			{
				return true;
			}

			std::string strPath(_pszPath);
			pathNormalize(strPath);

			for (char& c : strPath)
			{
				if ('/' == c)
				{
					c = '\0';
					if (!isExist(strPath.c_str()))
					{
						MKDIR(strPath.c_str());
					}
					c = '/';
				}
			}

			//检测最后一层
			if (!_bStrict && !isExist(strPath.c_str()))
			{
				MKDIR(strPath.c_str());
			}

			return true;
		}

		//文件删除
		static bool deleteFile(const char* _pszPath)
		{
			voya_assert(_pszPath, "Path is null!");
			return (0 == remove(_pszPath));
		}

		//文件拷贝(_bCover为true, 则即使文件存在，也覆盖文件)
		static bool copyFile(const char* _pszSrc, const char* _pszDst, bool _bCover = true)
		{
			voya_assert(_pszDst, "Path is null!");
			voya_assert(_pszSrc, "Path is null!");

			if (!_bCover && isExist(_pszDst))
			{
				return true;
			}

			std::string strSrc = _pszSrc;
			std::string strDst = _pszDst;
			CBuffer buffer;
			return (wirteFile(strDst, readFile(strSrc)));
		}
		
		//读取文件到缓存
		static CBuffer readFile(const std::string& _strFilePath, bool _bBinaryFormat = true)
		{
			CBuffer returnBuffer;

			if (!CFileUtil::isExist(_strFilePath.c_str()))
			{
				return returnBuffer;
			}

			FILE* pFile = nullptr;
			if (_bBinaryFormat)
			{
				pFile = fopen(_strFilePath.c_str(), "rb");
			}
			else
			{
				pFile = fopen(_strFilePath.c_str(), "r");
			}

			if (nullptr == pFile)
			{
				return returnBuffer;
			}

			fseek(pFile, 0, SEEK_END);
			long lSize = ftell(pFile);
			fseek(pFile, 0, SEEK_SET);

            if (0 != lSize)
            {
                returnBuffer.setSize(lSize, true);
                fread(returnBuffer.getBuffer(), sizeof(unsigned char), lSize, pFile);
            }
			fclose(pFile);
			pFile = nullptr;

			return returnBuffer;
		}

		//把缓存内容写入文件
		static bool wirteFile(const std::string& _strFilePath, const CBuffer& _buffer, bool _bBinaryFormat = true)
		{
			if (0 == _buffer.getSize())
			{
				return false;
			}

			if (CFileUtil::isExist(_strFilePath.c_str()))
			{
				CFileUtil::deleteFile(_strFilePath.c_str());
			}

			CFileUtil::mkdirs(_strFilePath.c_str(), true);

			FILE* pFile = nullptr;
			if (_bBinaryFormat)
			{
				pFile = fopen(_strFilePath.c_str(), "ab+");
			}
			else
			{
				pFile = fopen(_strFilePath.c_str(), "a+");
			}

			if (nullptr == pFile)
			{
				return false;
			}

			fwrite(_buffer.getBuffer(), sizeof(unsigned char), _buffer.getSize(), pFile);
			fclose(pFile);
			pFile = nullptr;

			return true;
		}

	};

}