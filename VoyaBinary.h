
#pragma once

#include <string>

namespace voya
{
	/*
	二进制内存读写\0
	*/
	template<typename StringLengthType = unsigned short>
	class CBinHelper
	{
		using Int = int;
		using UInt = unsigned int;
		using Short = short;
		using UShort = unsigned short;
		using Char = char;
		using UChar = unsigned char;
		using Long = long;
		using ULong = unsigned long;
		using LongLong = long long;
		using ULongLong = unsigned long long;
		using Float = float;
		using Double = double;

	private:
		template<typename _T>
		inline UInt readMemory(void* _pMemory, _T& _typeValue) const
		{
			_T* pTarget = reinterpret_cast<_T*>(_pMemory);
			_typeValue = *pTarget;
			return sizeof(_T);
		}

		template<typename _T>
		inline UInt writeMemory(void* _pMemory,const _T& _typeValue) const
		{
			_T* pTarget = reinterpret_cast<_T*>(_pMemory);
			*pTarget = _typeValue;
			return sizeof(_T);
		}

		inline UInt readMemory(void* _pMemory, char* _pTarget, UInt _size) const
		{
			char* pDst = _pTarget;
			char* pNow = (char*)_pMemory;
			char* pEnd = pNow + _size;
			while (pNow <= pEnd)
			{
				*pDst = *pNow;
				++pDst;
				++pNow;
			}
			return _size;
		}

	public:
		CBinHelper(Char* _pBuffer, Int _iBufferSize)
			: m_pBuffer(_pBuffer)
			, m_pBufferEnd(_pBuffer + _iBufferSize)
		{

		}

	public:
		//返回值，读取内存长度
		inline void readInt(Int& _typeValue){ m_pBuffer += readMemory(m_pBuffer, _typeValue); }
		inline void readUInt(UInt& _typeValue) { m_pBuffer += readMemory(m_pBuffer, _typeValue); }
		inline void readShort(Short& _typeValue) { m_pBuffer += readMemory(m_pBuffer, _typeValue); }
		inline void readUShort(UShort& _typeValue) { m_pBuffer += readMemory(m_pBuffer, _typeValue); }
		inline void readChar(Char& _typeValue) { m_pBuffer += readMemory(m_pBuffer, _typeValue); }
		inline void readUChar(UChar& _typeValue) { m_pBuffer += readMemory(m_pBuffer, _typeValue); }
		inline void readFloat(Float& _typeValue) { m_pBuffer += readMemory(m_pBuffer, _typeValue); }
		inline void readLong(Long& _typeValue) { m_pBuffer += readMemory(m_pBuffer, _typeValue); }
		inline void readULong(ULong& _typeValue) { m_pBuffer += readMemory(m_pBuffer, _typeValue); }
		inline void readDouble(Double& _typeValue) { m_pBuffer += readMemory(m_pBuffer, _typeValue); }
		inline void readString(std::string& _str)
		{
			StringLengthType length;
			m_pBuffer += readMemory(m_pBuffer, length);
			if (length > 0)
			{
				_str.resize(length);
				m_pBuffer += readMemory(m_pBuffer, (char*)_str.c_str(), length);
			}
			else
			{
				_str.clear();
			}
		}

		//读取任意类型数据到内存(成员排布与编译器相关，跨平台慎用)
		template<typename _T>
		inline void readType(_T& _typeValue) { m_pBuffer += readMemory(m_pBuffer, _typeValue); }

		//返回值，写入内存长度
		inline void writeInt(Int& _typeValue) { m_pBuffer += writeMemory(m_pBuffer, _typeValue); }
		inline void writeUInt(UInt& _typeValue) { m_pBuffer += writeMemory(m_pBuffer, _typeValue); }
		inline void writeShort(Short& _typeValue) { m_pBuffer += writeMemory(m_pBuffer, _typeValue); }
		inline void writeUShort(UShort& _typeValue) { m_pBuffer += writeMemory(m_pBuffer, _typeValue); }
		inline void writeChar(Char& _typeValue) { m_pBuffer += writeMemory(m_pBuffer, _typeValue); }
		inline void writeUChar(UChar& _typeValue) { m_pBuffer += writeMemory(m_pBuffer, _typeValue); }
		inline void writeFloat(Float& _typeValue) { m_pBuffer += writeMemory(m_pBuffer, _typeValue); }
		inline void writeLong(Long& _typeValue) { m_pBuffer += writeMemory(m_pBuffer, _typeValue); }
		inline void writeULong(ULong& _typeValue) { m_pBuffer += writeMemory(m_pBuffer, _typeValue); }
		inline void writeDouble(Double& _typeValue) { m_pBuffer += writeMemory(m_pBuffer, _typeValue); }
		//写入任意类型数据到内存(成员排布与编译器相关，跨平台慎用)
		template<typename _T>
		inline void writeType(_T& _typeValue) { m_pBuffer += writeMemory(m_pBuffer, _typeValue); }


	private:
		Char*	m_pBuffer;
		Char*	m_pBufferEnd;
	};


}
