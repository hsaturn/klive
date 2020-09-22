#pragma once

#include <cstdint>
#include <vector>

#include "common/observer.h"

namespace hw
{

/*
         \
        / \
       /   \
      /  |  \
     /   |   \
    /    |    \
   /     |     \
  /             \
 /       *       \
-------------------

This class will probably change a lot !
TODO
*/

class Memory : public Observable<Memory>
{
public:
    enum type_t
    {
        RAM      = 0,
        TEXT     = 1,    // code section
        ASM      = 2,    // start of z80 instruction (detect slide)
        RD_ONLY  = 128,
        UNCHANGE = 255, // Special value for fill set etc.. functions
    };

    class Byte
    {
    public:
        using value_type=char;

        explicit Byte(value_type byte, type_t type)
            : byte(byte) { flags = type; }

        explicit Byte(value_type byte)
            : Byte(byte, RAM){}

        Byte() : Byte(0, RAM){};


        bool isWritable() const
        {
            return (flags & (RD_ONLY) )==0;
        }

        void setType(type_t type) { flags=type; }

        operator value_type() const { return byte; }
        value_type operator=(value_type value) { byte=value; return byte; }

    private:
        value_type byte;
        type_t     flags;
    };


    using size_t = uint16_t;
    using addr_t = uint16_t;  // Enough for ZX Spectrum yet (could change in future)

    Memory();
    virtual ~Memory(){}

    void poke(const addr_t& start, Byte::value_type byte);


    Byte::value_type peek(const addr_t& addr) const
    {
        if (addr < bytes.size())
        {
            return bytes[addr];
        }
        else
        {
            return 0;   // TODO err ?
        }
    }

    void fill(addr_t& start, Byte::value_type byte, size_t size, type_t type=UNCHANGE)
    {
        if (bytes.size() < start+size) bytes.resize(bytes.size());

        // TODO memset ?
        while(size--)
        {
            bytes[start] = byte;
            if (type != UNCHANGE)
            {
                bytes[start] .setType(type);
            }
            start++;
        }
    }

    void setType(addr_t start, size_t size, type_t type)
    {
        if (bytes.size() < start+size) bytes.resize(bytes.size());

        while(size--)
        {
            bytes[start++].setType(type);
        }
    }


private:
    std::vector<Byte> bytes;
};

}
