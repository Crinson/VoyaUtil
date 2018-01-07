
#pragma once

namespace voya
{
    template<unsigned int _iNumber>
    class CNumberClass
    {
    public:
        enum 
        {
            value = 1 << _iNumber,
            index = _iNumber,
            indexMax = sizeof(unsigned int) * 8,
        };

        static bool isType(unsigned int _data)
        {
            return _data == value;
        }

        static bool hasType(unsigned int _data)
        {
            return _data & value;
        }

        static unsigned int addType(unsigned int _data)
        {
            return _data | value;
        }

        static unsigned int removeType(unsigned int _data)
        {
            return _data & (~value);
        }
    };

    template<typename ... NumberClasss> 
    class CNumberMask;

    template<typename NumberClass, typename ... NumberClasss>
    class CNumberMask<NumberClass, NumberClasss...> : public CNumberMask<NumberClasss...>
    {
    public:
        enum 
        {
            value = CNumberMask<NumberClass>::value | CNumberMask<NumberClasss...>::value,
            rvalue = ~value,
        };

        static unsigned int clearBits(unsigned int _value)
        {
            return _value & rvalue;
        }

        static unsigned int onlyBits(unsigned int _value)
        {
            return _value & value;
        }

    };

    template<typename NumberClass>
    class CNumberMask<NumberClass>
    {
    public:
        enum 
        {
            value = NumberClass::value,
        };
    };

}
