
#pragma once

#include <string>
#include <random>
#include <sstream>
#include <sstream>
#include "VoyaUtil/VoyaAssert.h"

namespace voya
{
	/***
	幻数(加密数字，只支持32Bit长度数字，包括浮点数和整形)
	*/
	template<typename _T>
	class CMagicNum
	{
		using Byte = unsigned char;
		using UShort = unsigned short;
		using UInt = unsigned int;
		using ULongLong = unsigned long long;
		using Bool = bool;

		//静态变量
		const static Byte MAGIC_SIGN = 2;		//签名
		const static Byte MAGIC_DEFAULT = 1;	//默认算法类型

		typedef _T NumberType;
	private:
		//随机数顺序结构
		union sRandomData
		{
			struct
			{
				UShort random1 : 3;
				UShort random2 : 2;
				UShort random4 : 7;
				UShort random3 : 4;
			}Part;
			UShort data;
		};

		//真实数字顺序结构
		union sNumberData
		{
			struct
			{
				UInt number1 : 2;
				UInt number2 : 5;
				UInt number4 : 9;
				UInt number3 : 5;
				UInt number5 : 11;
			}Part;
			UInt data;
		};

		//幻数存储结构
		union sMagicData
		{
			struct
			{
				ULongLong encryttype : 2;		//加密算法，此值为0非法 1.默认算法
				ULongLong random1 : 3;
				ULongLong number1 : 2;
				ULongLong random2 : 2;
				ULongLong sign : 3;		//签名 此处的值必定和MAGIC_SIGN相等，否则为非法值
				ULongLong number2 : 5;
				ULongLong nothing : 3;		//随机数，内存原来是多少就是多少，必须保证不为0，等于0为非法值
				ULongLong random4 : 7;
				ULongLong number4 : 9;
				ULongLong check : 8;		//校验位，此值为加密后数字和随机值Byte异或
				ULongLong number3 : 5;
				ULongLong random3 : 4;
				ULongLong number5 : 11;
			}DataMask;
			ULongLong data;
		};

	private:
		//获得内存拼接的原始随机数
		inline UShort getInterRandom() const
		{
			UShort ret;
			sRandomData* pRandom = reinterpret_cast<sRandomData*>(&ret);
			pRandom->Part.random1 = m_data.DataMask.random1;
			pRandom->Part.random2 = m_data.DataMask.random2;
			pRandom->Part.random3 = m_data.DataMask.random3;
			pRandom->Part.random4 = m_data.DataMask.random4;
			return ret;
		}

		//设置内存拼接的原始随机数
		inline void setInterRandom(UShort _random)
		{
			sRandomData* pRandom = reinterpret_cast<sRandomData*>(&_random);
			m_data.DataMask.random1 = pRandom->Part.random1;
			m_data.DataMask.random2 = pRandom->Part.random2;
			m_data.DataMask.random3 = pRandom->Part.random3;
			m_data.DataMask.random4 = pRandom->Part.random4;
		}

		//获得内存拼接的原始数字
		inline UInt getInterNumber() const
		{
			UInt ret;
			sNumberData* pNumber = reinterpret_cast<sNumberData*>(&ret);
			pNumber->Part.number1 = m_data.DataMask.number1;
			pNumber->Part.number2 = m_data.DataMask.number2;
			pNumber->Part.number3 = m_data.DataMask.number3;
			pNumber->Part.number4 = m_data.DataMask.number4;
			pNumber->Part.number5 = m_data.DataMask.number5;
			return ret;
		}

		//设置内存拼接的原始数字
		inline void setInterNumber(UInt _number)
		{
			sNumberData* pNumber = reinterpret_cast<sNumberData*>(&_number);
			m_data.DataMask.number1 = pNumber->Part.number1;
			m_data.DataMask.number2 = pNumber->Part.number2;
			m_data.DataMask.number3 = pNumber->Part.number3;
			m_data.DataMask.number4 = pNumber->Part.number4;
			m_data.DataMask.number5 = pNumber->Part.number5;
		}

		//在数值和随机数都设定完后，做最后的合法性设置
		inline void setInterLegal()
		{
			//设置sign
			m_data.DataMask.sign = MAGIC_SIGN;
			//内存随机值
			m_data.DataMask.nothing == 0 ? m_data.DataMask.nothing = 1 : 0;
			//数值异或
			UShort ran = getInterRandom();
			UInt num = getInterNumber();
			m_data.DataMask.check = ( (ran & 0x0F) ^ ((ran >> 8) & 0x0F) ^
				(num & 0x0F) ^ ((num >> 8) & 0x0F) ^ ((num >> 16) & 0x0F) ^ ((num >> 24) & 0x0F));
			//加密算法
			m_data.DataMask.encryttype = MAGIC_DEFAULT;
		}

		//校验数据是否合法(防止内存修改器篡改)
		inline Bool isInterLegal()
		{
			do
			{
				if (0 == m_data.DataMask.encryttype) break;
				if (MAGIC_SIGN != m_data.DataMask.sign) break;
				if (0 == m_data.DataMask.nothing) break;
				UShort ran = getInterRandom();
				UInt num = getInterNumber();
				if (m_data.DataMask.check != ((ran & 0x0F) ^ ((ran >> 8) & 0x0F) ^
					(num & 0x0F) ^ ((num >> 8) & 0x0F) ^ ((num >> 16) & 0x0F) ^ ((num >> 24) & 0x0F))) break;

				return true;
			} while (false);

			voya_assert(false, "CMagicNum check error, is data be cheat?");
			return false;
		}

		//生成系统随机数
		UShort createRandom() const
		{
			static std::random_device rd;
			return static_cast<UShort>(rd());
		}

		//生成加密数字
		inline UInt createNumber(NumberType _data) const
		{
			UShort* pEncryptNum = reinterpret_cast<UShort*>(&_data);
			pEncryptNum[0] ^= getInterRandom();
			pEncryptNum[1] ^= getInterRandom();
			UInt* pRet = reinterpret_cast<UInt*>(pEncryptNum);
			return *pRet;
		}

		//设置数据
		void setData(const NumberType _data)
		{
			setInterRandom(createRandom());
			setInterNumber(createNumber(_data));
			setInterLegal();
		}

		//获得数据
		NumberType getData()
		{
			if (isInterLegal())
			{
				UShort ran = getInterRandom();
				UInt num = getInterNumber();
				UShort* pDecryptNum = reinterpret_cast<UShort*>(&num);
				pDecryptNum[0] ^= ran;
				pDecryptNum[1] ^= ran;
				NumberType* pRet = reinterpret_cast<NumberType*>(&num);
				return *pRet;
			}
			return NumberType();
		}

	public:
		//幻数只能存放4字节大小的数字
		explicit CMagicNum(NumberType _number,
			typename std::enable_if<sizeof(UInt) == sizeof(NumberType), void*>::type = nullptr,
			typename std::enable_if<std::is_arithmetic<NumberType>::value, void*>::type = nullptr)
		{
			setData(_number);
		}

		//获得值
		NumberType getValue()
		{
			return getData();
		}

		//序列化
		//注意，序列化保存到文件前，最好先用变量名异或字符串等方法融入到字符串中，
		//防止玩家从保存文件中使用拷贝数值替换别的变量的方式来欺骗程序
		Bool serialize(std::string& _str)
		{
            std::stringstream strSm;
            strSm << m_data.data;
            _str = strSm.str();

			return true;
		}

		//反序列化
		Bool deserialize(const std::string& _str)
		{

			return true;
		}

		//同类型四则运算
		CMagicNum<NumberType> operator + (CMagicNum<NumberType>& _rhs)
		{
			return CMagicNum(getData() + _rhs.getData());
		}
		CMagicNum<NumberType> operator - (CMagicNum<NumberType>& _rhs)
		{
			return CMagicNum(getData() - _rhs.getData());
		}
		CMagicNum<NumberType> operator * (CMagicNum<NumberType>& _rhs)
		{
			return CMagicNum(getData() * _rhs.getData());
		}
		CMagicNum<NumberType> operator / (CMagicNum<NumberType>& _rhs)
		{
			return CMagicNum(getData() / _rhs.getData());
		}
		CMagicNum<NumberType> operator % (CMagicNum<NumberType>& _rhs)
		{
			return CMagicNum(getData() % _rhs.getData());
		}

		CMagicNum<NumberType>& operator += (CMagicNum<NumberType>& _rhs)
		{
			setData(getData() + _rhs.getData());
			return *this;
		}
		CMagicNum<NumberType>& operator -= (CMagicNum<NumberType>& _rhs)
		{
			setData(getData() - _rhs.getData());
			return *this;
		}
		CMagicNum<NumberType>& operator *= (CMagicNum<NumberType>& _rhs)
		{
			setData(getData() * _rhs.getData());
			return *this;
		}
		CMagicNum<NumberType>& operator /= (CMagicNum<NumberType>& _rhs)
		{
			setData(getData() / _rhs.getData());
			return *this;
		}
		CMagicNum<NumberType>& operator %= (CMagicNum<NumberType>& _rhs)
		{
			setData(getData() % _rhs.getData());
			return *this;
		}

		//前置++
		CMagicNum<NumberType>& operator++()
		{
			NumberType tmp = getData();
			setData(++tmp);
			return (*this);
		}

		//后置++
		CMagicNum<NumberType> operator++(int)
		{
			CMagicNum<NumberType> tmp = *this;
			++(*this);
			return (tmp);
		}

		//前置--
		CMagicNum<NumberType>& operator--()
		{
			NumberType tmp = getData();
			setData(--tmp);
			return (*this);
		}

		//后置--
		CMagicNum<NumberType> operator--(int)
		{
			CMagicNum<NumberType> tmp = *this;
			--(*this);
			return (tmp);
		}

		//不同类型后置四则运算
		template<typename _UType>
		CMagicNum<NumberType> operator + (_UType _value)
		{
			return CMagicNum<NumberType>(getData() + _value);
		}

		template<typename _UType>
		CMagicNum<NumberType>& operator += (_UType _value)
		{
			setData(getData() + _value);
			return *this;
		}

		template<typename _UType>
		CMagicNum<NumberType> operator - (_UType _value)
		{
			return CMagicNum<NumberType>(getData() - _value);
		}

		template<typename _UType>
		CMagicNum<NumberType>& operator -= (_UType _value)
		{
			setData(getData() - _value);
			return *this;
		}

		template<typename _UType>
		CMagicNum<NumberType> operator * (_UType _value)
		{
			return CMagicNum<NumberType>(getData() * _value);
		}

		template<typename _UType>
		CMagicNum<NumberType>& operator *= (_UType _value)
		{
			setData(getData() * _value);
			return *this;
		}

		template<typename _UType>
		CMagicNum<NumberType> operator / (_UType _value)
		{
			return CMagicNum<NumberType>(getData() / _value);
		}

		template<typename _UType>
		CMagicNum<NumberType>& operator /= (_UType _value)
		{
			setData(getData() / _value);
			return *this;
		}

		template<typename _UType>
		CMagicNum<NumberType> operator % (_UType _value)
		{
			return CMagicNum<NumberType>(getData() % _value);
		}

		template<typename _UType>
		CMagicNum<NumberType>& operator %= (_UType _value)
		{
			setData(getData() % _value);
			return *this;
		}
		
	private:
		explicit CMagicNum();		//不提供默认构造函数

	private:

		sMagicData	m_data;
	};


	//不同类型前置四则运算
	template<typename _UType, typename _V>
	CMagicNum<_V> operator + (_UType _number, CMagicNum<_V>& _magicNumber)
	{
		CMagicNum<_V> tmp(_number + _magicNumber.getValue());
		return tmp;
	}

	template<typename _UType, typename _V>
	CMagicNum<_V> operator - (_UType _number, CMagicNum<_V>& _magicNumber)
	{
		CMagicNum<_V> tmp(_number - _magicNumber.getValue());
		return tmp;
	}

	template<typename _UType, typename _V>
	CMagicNum<_V> operator * (_UType _number, CMagicNum<_V>& _magicNumber)
	{
		CMagicNum<_V> tmp(_number * _magicNumber.getValue());
		return tmp;
	}

	template<typename _UType, typename _V>
	CMagicNum<_V> operator / (_UType _number, CMagicNum<_V>& _magicNumber)
	{
		CMagicNum<_V> tmp(_number / _magicNumber.getValue());
		return tmp;
	}

	template<typename _UType, typename _V>
	CMagicNum<_V> operator % (_UType _number, CMagicNum<_V>& _magicNumber)
	{
		CMagicNum<_V> tmp(_number % _magicNumber.getValue());
		return tmp;
	}

}
