
#pragma once


#include "VoyaUtil/VoyaSTL.h"
#include "VoyaUtil/VoyaRef.h"

namespace voya
{
    //////////////////////////////////////////////////////////////////////////
    //Std辅助类\0
    template<class _KEY, class _VALUE>
    class CStdMap : public CCounter
    {
        using MAP_VALUE = voya::map<_KEY, _VALUE>;

    public:
        MAP_VALUE  map;
    };

    template<class _KEY, class _VALUE>
    class CStdUnorderMap : public CCounter
    {
        using MAP_VALUE = voya::unordered_map<_KEY, _VALUE>;

    public:
        MAP_VALUE  map;
    };

    template<class _VALUE>
    class CStdList : public CCounter
    {
        using LIST_VALUE = voya::list<_VALUE>;

    public:
        CStdList(int _iCapacity, _VALUE _initValue)
            : list(_iCapacity, _initValue)
        {

        }

        CStdList(int _iCapacity)
            : list(_iCapacity)
        {

        }

        CStdList()
        {

        }

        LIST_VALUE  list;
    };

    template<class _VALUE>
    class CStdVector : public CCounter
    {
        using VECTOR_VALUE = voya::vector<_VALUE>;
    public:
        CStdVector(int _iCapacity, _VALUE _initValue)
            : vector(_iCapacity, _initValue)
        {

        }

        CStdVector(int _iCapacity)
            : vector(_iCapacity)
        {

        }

        CStdVector()
        {

        }

        VECTOR_VALUE vector;
    };
    //////////////////////////////////////////////////////////////////////////

}
