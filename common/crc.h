#pragma once
#include <cstdint>
#include <string>

class Crc16
{
    public:

        template<typename T, typename _ = typename std::enable_if<sizeof(T)==1>::type>
        friend Crc16& operator<< (Crc16& crc16, T c)
        {
            uint16_t& crc=crc16.crc;
            uint16_t x=((crc>>8)^c)&0xff;
            crc = crc<<8 ^ (x<<12) ^ (x<<5) ^ x;
            return crc16;
        }
        friend Crc16& operator << (Crc16& crc16, const std::string& s)
        {
            for(const auto& c: s) crc16 << c;
            return crc16;
        }

        template<typename T, typename _ = typename std::enable_if<sizeof(T)==1>::type>
        uint16_t add(const T* buff, uint32_t len)
        {
            while(len--)
                (*this)<<*buff++;
            return crc;
        }
        operator uint16_t() const { return crc; }

   private:
        uint16_t crc=0;
};
