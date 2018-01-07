
#pragma once

#include <list>
#include <memory>
#include "VoyaUtil/VoyaAssert.h"
#include "VoyaUtil/VoyaSingleton.h"
#include "VoyaUtil/VoyaNoCopyAble.h"
#include "VoyaUtil/VoyaAllocator.h"
#include "VoyaUtil/VoyaSharePtr.h"

namespace voya
{
	/************************************************************************/
	/* 类型缓存池\0                                                                     */
	/************************************************************************/
	template<class _T, unsigned int _Unit_Size = 64>
	class CObjectPool : public CNoCopyAble
	{
		using ObjectType = _T;

	public:
		explicit CObjectPool()
            : m_allocator(sizeof(ObjectType), _Unit_Size * sizeof(ObjectType))
		{

		}

		virtual ~CObjectPool()
		{

		}

		//借出实例\0
		template<typename ... Args>
		ObjectType* borrowObject(Args ... _args)
        {
            return new(m_allocator.allocate()) _T(std::forward<Args>(_args)...);
		}

		//归还实例\0
		void returnObject(ObjectType* _pObject)
		{
            if (nullptr != _pObject)
            {
                _pObject->~ObjectType();
                m_allocator.deallocate(_pObject);
            }
		}

        template<typename ... Args>
        shared_ptr<ObjectType> borrowSharedObject(Args ... _args)
        {
            return shared_ptr<ObjectType>(borrowObject(std::forward<Args>(_args)...), [this](ObjectType* _pObject){this->returnObject(_pObject); });
        }

		//获得使用中的实例数目\0
        int getUseCount() const
        {
            return m_allocator.usedBytes() / sizeof(ObjectType);
        }

		//获得缓存池分配的实例数目\0
        int getTotalCount() const
        {
            return m_allocator.totalBytes() / sizeof(ObjectType);
        }

		//释放闲置实例(整段释放，非单独释放)\0
		void releaseFreeObject() 
        {

        }

	private:
        CAllocator  m_allocator;

	};

	/************************************************************************/
	/* 单例对象池\0                                                                     */
	/************************************************************************/
	template<class _T, unsigned int _Unit_Size = 64>
	class CObjectPoolSingleton : 
		public CSingleton<CObjectPool<_T, _Unit_Size> >
	{
		//如果_T的构造不是public, 需要在该类添加friend class voyautil::CObjectPool<_T>;
		//而不是friend class voyautil::CObjectPoolSingleton<_T>;
	};

}