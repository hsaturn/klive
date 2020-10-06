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

    enum attrib
    {
        RAM      = 0,    // Byte belongs to RAM
        ROM      = 1,    // Byte belongs to ROM
        TEXT     = 2,    // code section
        ASM      = 4,    // start of z80 instruction (detect slide)
        RD_ONLY  = 128,
        UNCHANGE = 255, // Special value for fill set etc.. functions, UNCHANGE means do not change the type
    };

    using byte_t = unsigned char;
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
        size_t size;
        addr_t start;   // Start of memory range modified
        uint32_t flags;   // Flag (ored)
    };


    Memory(size_t size)
    {
        ramtop=size-1;
        bytes.resize(size);
        attribs.resize(size);
    }

    uint32_t size() const { return ramtop+1; }
    uint16_t crc16() const;

    byte_t peek(const addr_t& addr) const
    {
        if (addr <= ramtop)
        {
            return bytes[addr];
        }
        else
        {
            std::cerr << "TODO: manage memory access error" << std::endl;
            return 0;   // TODO err ?
        }
    }

    void fill(addr_t start, byte_t value, size_t size=1, attrib type=UNCHANGE);

    void loadRomImage(std::string f, addr_t start, bool dump=false);

    void poke(const addr_t& start, byte_t value, attrib type=UNCHANGE)
    {
        // TODO faster implem (exh
        fill(start, value, 1, type);
    }

private:
    std::vector<uint8_t> bytes;
    std::vector<uint8_t> attribs;
    addr_t ramtop=0;
    bool detectBadWrites=false;     // True will send a BAD_WRITE event when occurs
    bool mem_protection=true;       // True if write is void on unwritable areas
};

}
