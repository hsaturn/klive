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
        UNCHANGE = 255, // Do not change the type (parameter of poke, fill ...)
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
            BAD_WRITE,
            BAD_READ
        };

        Event event;
        size_t size;
        addr_t start;   // Start of memory range modified
        uint32_t flags;   // Flag (ored)
    };


    Memory(size_t size)
    {
        ramtop=size;
        bytes.resize(size);
        attribs.resize(size);
    }

    uint32_t size() const { return ramtop; }
    uint16_t crc16() const;

    byte_t peek(const addr_t& addr) const;

    void fill(addr_t start, byte_t value, size_t size=1, attrib type=UNCHANGE);

    void loadRomImage(std::string f, addr_t start, bool dump=false);

    void poke(const addr_t& start, byte_t value, attrib type=UNCHANGE);

    void zero() { fill(0, 0, ramtop); }

private:
    std::vector<uint8_t> bytes;
    std::vector<uint8_t> attribs;
    addr_t ramtop=0;				// size of memory, or first non writable byte
    bool detectBadWrites=false;     // True will send a BAD_WRITE event when occurs
    bool mem_protection=true;       // True if write is void on unwritable areas
};

}
