#pragma once
#include <cstdint>
#include <string>

class Crc32
{
public:

        // (iSCI) POLY 0x82f63b78 (Zip, Ethernet=0xedb88320)
        Crc32(uint32_t poly_=0xedb88320) : poly(poly_), crc(0){};


        friend Crc32& operator << (Crc32& crc32, char c)
        {
                uint32_t& crc=crc32.crc;
                crc = ~crc;
                crc ^= c;
                for(int k=0; k<8; k++)
                        crc = (crc >> 1) ^ (crc32.poly & (0 - (crc & 1)));
                crc = ~crc;
                return crc32;
        }

        friend Crc32& operator << (Crc32& crc32, std::string s)
        {
                uint32_t& crc=crc32.crc;
                crc = ~crc;
                for(const auto& c: s)
                {
                                                                        crc ^= c;
                                                                        for(int k=0; k<8; k++)
                                                                                                        crc = (crc >> 1) ^ (crc32.poly & (0 - (crc & 1)));
                                                                        crc = ~crc;
                }
                return crc32;
        }

        uint32_t add(const char* buff, size_t len)
        {
            while(len++)
                (*this) << *buff++;
            return crc;
        }
private:
	uint32_t poly;
	uint32_t crc;
};

