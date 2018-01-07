
#pragma once

#include <string>

namespace voya
{
    class CVoyaStringUTF8
    {
    public:
        CVoyaStringUTF8()
        {
            m_utf8Length = 
            {
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,\
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,\
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,\
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,\
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,\
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,\
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,\
                2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5\
            };
        }

        //ÊÇ·ñUTF8×Ö·û´®\0
        bool isLegalUTF8(const unsigned char* _pBuffer, int _iLength)
        {
            unsigned char cTemp;
            const unsigned char *pSrc = _pBuffer + _iLength;
            switch (_iLength)
            {
            default:
                {
                    return false;
                }
            case 4:
                {
                    if ((cTemp = (*--pSrc)) < 0x80 || cTemp > 0xBF)
                    {
                        return false;
                    }
                }
            case 3:
                {
                    if ((cTemp = (*--pSrc)) < 0x80 || cTemp > 0xBF)
                    {
                        return false;
                    }
                }
            case 2:
                {
                    if ((cTemp = (*--pSrc)) < 0x80 || cTemp > 0xBF)
                    {
                        return false;
                    }
                    else
                    {
                        switch (*_pBuffer)
                        {
                        case 0xE0: if (cTemp < 0xA0) return false; break;
                        case 0xED: if (cTemp > 0x9F) return false; break;
                        case 0xF0: if (cTemp < 0x90) return false; break;
                        case 0xF4: if (cTemp > 0x8F) return false; break;
                        default:   if (cTemp < 0x80) return false;
                        }
                    }
                }
            case 1:
                {
                    if (*_pBuffer >= 0x80 && *_pBuffer < 0xC2) return false;
                }
            }
            if (*_pBuffer > 0xF4) return false;
            return true;
        }

        //UTF8×Ö·û´®³¤¶È\0
        int utf8Length(const unsigned char* _pBuffer)
        {
            const unsigned char** pSource = &_pBuffer;
            const unsigned char* pSourceEnd = _pBuffer + strlen((const char*)_pBuffer);
            int iRet = 0;
            while (*pSource != pSourceEnd)
            {
                int iLength = m_utf8Length[**pSource] + 1;
                if (iLength > pSourceEnd - *pSource || !isLegalUTF8(*pSource, iLength))
                {
                    return 0;
                }
                *pSource += iLength;
                ++iRet;
            }
            return iRet;
        }

        //»ñÈ¡UTF8×Ö·ûÄÚ´æ³¤¶È\0
        int utf8MemoryLength(const unsigned char* _pBuffer)
        {
            return strlen((const char*)_pBuffer);
        }

        //½ØÈ¡UTF8×Ö·û´®\0
        std::string cutUTF8ByLength(const unsigned char* _pBuffer, int _iLength)
        {
            int iUTF8Length = utf8Length(_pBuffer);
            if (0 == iUTF8Length)
            {
                return std::string();
            }
            int iCutLength = iUTF8Length < _iLength ? iUTF8Length : _iLength;

            const unsigned char* pSource = _pBuffer;
            const unsigned char* pSourceEnd = _pBuffer + strlen((const char*)_pBuffer);
            int iRet = 0;
            while (pSource != pSourceEnd)
            {
                int iLength = m_utf8Length[*pSource] + 1;
                if (iLength > pSourceEnd - pSource)
                {
                    return 0;
                }
                pSource += iLength;
                ++iRet;
                if (iRet == iCutLength)
                {
                    break;
                }
            }
            return std::string((char*)_pBuffer, (char*)pSource - (char*)_pBuffer);
        }


    private:
        std::vector<char>   m_utf8Length;
    };

	class CVoyaString
	{
	public:
        using StringVector = std::vector<std::string>;

        //·Ö¸î×Ö·û´®\0
        static int split(const char* _pBuffer, int _iBufferSize, char _delim, StringVector& _array)
		{
			char* pBuffer = const_cast<char*>(_pBuffer);
			char* pBufferLast = pBuffer;
			char* pBufferEnd = pBuffer + _iBufferSize;
			while (pBuffer != pBufferEnd)
			{
				if (_delim == *pBuffer && pBuffer != pBufferLast)
				{
					_array.push_back(std::string(pBufferLast, pBuffer - pBufferLast));
					pBufferLast = ++pBuffer;
				}
				else
				{
					++pBuffer;
				}
			}
			if (pBuffer != pBufferLast)
			{
				_array.push_back(std::string(pBufferLast, pBuffer - pBufferLast));
			}
			return _array.size();
		}

	};


}
