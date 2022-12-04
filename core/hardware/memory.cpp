#include "memory.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <iomanip>
#include <stdio.h>
#include <QTextStream>
#include <QFile>
#include <common/crc.h>

using namespace std;

namespace hw

{

Memory::byte_t Memory::peek(const addr_t& addr) const
{
    if (addr < ramtop)
    {
        return bytes[addr];
    }
    else
    {
        std::cerr << "TODO: manage memory access error" << std::endl;
        return 0;   // TODO err ?
    }
}

void Memory::poke(const addr_t& start, byte_t value, attrib type /* = UNCHANGE */)

{
    // TODO faster implem
    fill(start, value, 1, type);
}

void Memory::fill(addr_t start, byte_t value, size_t size, attrib type)
{
    Message msg;
    msg.event = Message::WRITTEN;
    msg.flags = Message::DATA | (type==UNCHANGE ? 0 : Message::TYPE);
    msg.size = size;

    while(size--)
    {
        msg.start = start;

        if (start>=ramtop)
        {
            msg.event = Message::BAD_WRITE;
            notify(msg);
            return;
        }

        if ((attribs[start] & RD_ONLY) && mem_protection)
        {
            if (detectBadWrites)
            {
                msg.event = Message::BAD_WRITE;
                notify(msg);
                msg.event = Message::WRITTEN;
            }
            start++;
            continue;
        }
        else
            bytes[start] = value;

        if (type != UNCHANGE)
        {
            attribs[start] = type;
        }
        start++;
    }
    notify(msg);
}

uint16_t Memory::crc16() const
{
    Crc16 crc;
    crc.add(bytes.data(), bytes.size());
    return crc;
}

void Memory::loadRomImage(string f, Memory::addr_t start, bool dump)
{
    bool memprotect(mem_protection);
    mem_protection = false;
    long bytesRead=0;
    QFile rom(f.c_str());
    // ifstream rom(f.c_str(), ios::in | ios::binary);
    if (rom.open(QFile::ReadOnly))
    {
        int count=0;
        char c;
        while(!rom.atEnd())
        {
            rom.read(&c, 1);
            poke(start, c, RD_ONLY);
            if (dump)
            {
                if (count==0)
                {
                    count=16;
                    printf("\n%04x: ", start);
                }
                printf("%02x ", (uint8_t)c);
            }
            count--;
            start++;
            bytesRead++;
        }
    }
    else
    {
        cerr << "Unable to open ROM file " << f << endl;
    }
    cout << "Rom image load result: " << f << ", " << bytesRead << " bytes." << endl;
    mem_protection=memprotect;
}

}

