#pragma once

#include <cstdint>
#include <vector>
#include <iostream>

#include "common/observer.h"

namespace hw
{

class Memory : public Observable<Memory>
{
public:

    enum type_t
    {
        RAM      = 0,    // Byte belongs to RAM
        ROM      = 1,    // Byte belongs to ROM
        TEXT     = 2,    // code section
        ASM      = 4,    // start of z80 instruction (detect slide)
        RD_ONLY  = 128,
        UNCHANGE = 255, // Special value for fill set etc.. functions, UNCHANGE means do not change the type
    };

    class Byte
    {
    public:
        using value_type=unsigned char;

        explicit Byte(value_type byte, type_t type)
            : byte(byte) { flags = type; }

        explicit Byte(value_type byte)
            : Byte(byte, RAM){}

        Byte() : Byte(0, RAM){};


        bool isWritable() const
        {
            return (flags & (RD_ONLY) )==0;
        }

        type_t type() const { return flags; }
        void setType(type_t type) { flags=type; }

        operator value_type() const { return byte; }
        value_type operator=(value_type value) { byte=value; return byte; }

    private:
        value_type byte;
        type_t     flags;
    };

    using size_t = uint32_t;
    using addr_t = uint32_t;

    struct Message
    {
        enum Flag   // flags in attr
        {
            DATA=1,
            TYPE=2
        };

        enum Event
        {
            WRITTEN,
            BAD_WRITE
        };

        Event event;
        uint8_t flags;   // Flag (ored)
        addr_t start;   // Start of memory range modified
        size_t size;
    };


    Memory(size_t size)
    {
        ramtop=size-1;
        bytes.resize(size);
    };

    virtual ~Memory(){}

    uint32_t size() const { return ramtop+1; }

    Byte::value_type peek(const addr_t& addr) const
    {
        if (addr < bytes.size())
        {
            return bytes[addr];
        }
        else
        {
            std::cerr << "TODO: manage memory access error" << std::endl;
            return 0;   // TODO err ?
        }
    }

    void fill(addr_t start, Byte::value_type value, size_t size=1, type_t type=UNCHANGE);

    void loadRomImage(std::string f, addr_t start, bool dump=false);

    void poke(const addr_t& start, Byte::value_type value, type_t type=UNCHANGE)
    {
        // TODO faster implem (exh
        fill(start, value, 1, type);
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
    bool detectBadWrites=false;     // True will send a BAD_WRITE event when occurs
    bool mem_protection=true;       // True if write is void on unwritable areas
    addr_t ramtop=0;
};

}
