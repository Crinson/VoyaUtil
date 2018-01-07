
#pragma once

#include <memory>
#include <vector>
#include "VoyaUtil/VoyaAssert.h"
#include "VoyaUtil/VoyaRefStd.h"
#include "VoyaUtil/VoyaRef.h"

namespace voya
{
	/*
		缓存类
	*/
	class CBuffer
	{
        using SPBuffer = voya::CHolder<voya::CStdVector<char>>;

	public:
		explicit CBuffer()
		{

		}

		//预置缓存大小\0
		explicit CBuffer(unsigned int _size)
		{
			if(_size <= 0)
			{
				return;
			}
            m_spBuffer = new SPBuffer::HolderType(_size);
		}

        //预制内容\0
        explicit CBuffer(char* _pBuffer, unsigned int _bufferSize)
        {
            if (_bufferSize > 0)
            {
                m_spBuffer = new SPBuffer::HolderType(_bufferSize);
                memcpy(&m_spBuffer->vector.front(), _pBuffer, _bufferSize);
            }
        }
        
        //拷贝构造\0
        CBuffer& operator=(const CBuffer& _rhs)
        {
            m_spBuffer = _rhs.m_spBuffer;
            return *this;
        }

		//移动构造\0
		CBuffer(CBuffer&& _buffer)
		{
			m_spBuffer = _buffer.m_spBuffer;
			_buffer.m_spBuffer.reset();
		}

		//设置缓存大小\0
		void setSize(unsigned int _size, bool _bClearBuffer = false)
		{
            SPBuffer spBuffer;
            
            if (_size > 0)
            {
                spBuffer = new SPBuffer::HolderType(_size);

                if (!_bClearBuffer && m_spBuffer->vector.size() > 0)
                {
                    if (m_spBuffer->vector.size() <= spBuffer->vector.size())
                    {
                        memcpy(&spBuffer->vector.front(), &m_spBuffer->vector.front(), m_spBuffer->vector.size());
                    }
                    else
                    {
                        memcpy(&spBuffer->vector.front(), &m_spBuffer->vector.front(), spBuffer->vector.size());
                    }
                }
            }

            m_spBuffer = spBuffer;
		}

		//获得缓存大小\0
		unsigned int getSize() const
		{
			if (nullptr == m_spBuffer.get())
			{
				return 0;
			}
            return static_cast<unsigned int>(m_spBuffer->vector.size());
		}

		//获得缓存位置(注意，setSize之后，原来getBuffer获得的指针会失效，需要重新获取)\0
		char* getBuffer() const
		{
			if (nullptr == m_spBuffer.get())
			{
				nullptr;
			}
            return &(m_spBuffer->vector.front());
		}

	private:
		SPBuffer	m_spBuffer;			//缓存vector sharedptr

	};



	class CReadWriteBuffer
    {
        using SPBuffer = voya::CHolder<voya::CStdVector<char>>;

	public:
		explicit CReadWriteBuffer()
		{

		}

		//预置缓存大小\0
		explicit CReadWriteBuffer(unsigned int _size)
		{
            m_spBuffer = new SPBuffer::HolderType(_size);
		}

		//移动构造\0
		CReadWriteBuffer(CReadWriteBuffer&& _buffer)
		{
			m_spBuffer = _buffer.m_spBuffer;
			_buffer.m_spBuffer.reset();
		}

		//获得现存数据大小\0
		int getSize() const
		{ 
			return m_iWritePos - m_iReadPos; 
		}

		//获得可写位置(建议写之前调用needSpace先把需要的空间扩充足够)\0
		char* getWritePos() const 
		{
			if (nullptr == m_spBuffer.get())
			{
				nullptr;
			}
			return &(m_spBuffer->vector.front()) + m_iWritePos;
		}

		//移动可写位置\0
		void moveWritePos(int _iLength)
		{
			m_iWritePos += _iLength;
		}

		//获得可读位置\0
		char* getReadPos() const
		{
			if (nullptr == m_spBuffer.get())
			{
				nullptr;
			}
            return &(m_spBuffer->vector.front()) + m_iReadPos;
		}

		//移动可读位置\0
		void moveReadPos(int _iLength)
		{
			m_iReadPos += _iLength;
		}

		//检测可写空间是否足够，不够则处理到足够的空间\0
		void needSpace(int _iLength)
		{
            int iContainerSize = m_spBuffer->vector.size();
            int iUsedSize = getSize();
            int iLeastSpace = iContainerSize - iUsedSize;
            if (iContainerSize < m_iWritePos + _iLength)
            {
                //如果后续空间不够容纳新的东西\0
                int iExSpace = iContainerSize;
                if (iLeastSpace < _iLength)
                {
                    //如果剩余控件不足以容纳新的东西，扩充缓存\0
                    iExSpace *= 2;
                    while (iExSpace - iUsedSize < _iLength)
                    {
                        iExSpace *= 2;
                    }
                }

                if (iUsedSize == 0)
                {
                    //如果现在没有有用的数据\0
                    if (iExSpace != iContainerSize)
                    {
                        //如果需要扩充容器大小\0
                        SPBuffer spNewBuffer = new SPBuffer::HolderType(iExSpace);
                        m_spBuffer = spNewBuffer;
                    }
                    m_iWritePos = 0;
                    m_iReadPos = 0;
                }
                else
                {
                    SPBuffer spNewBuffer = new SPBuffer::HolderType(iExSpace);
                    memcpy(&spNewBuffer->vector.front(), getReadPos(), iUsedSize);
                    m_spBuffer = spNewBuffer;
                    m_iWritePos = iUsedSize;
                    m_iReadPos = 0;
                }
                
            }
		}

		//写数据\0
		void write(void* _pBuffer, int _iLength)
		{
			needSpace(_iLength);
			memcpy(getWritePos(), _pBuffer, _iLength);
			moveWritePos(_iLength);
		}

		//读数据\0
		void read(void* _pBuffer, int _iLength)
		{
			if (getSize() >= _iLength)
			{
				memcpy(_pBuffer, getReadPos(), _iLength);
				moveReadPos(_iLength);
			}
			else
			{
				voya_log_warning("read overflow");
			}
		}

	private:
		SPBuffer	m_spBuffer;			//缓存vector sharedptr\0
		int			m_iWritePos = 0;		//写缓存位置\0
		int			m_iReadPos = 0;			//读缓存位置\0
	};


}
