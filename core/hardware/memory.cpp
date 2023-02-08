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
        Message msg;
        msg.event = Message::BAD_READ;
        msg.start = addr;
        msg.size = 1;
        // TODO, find a better way to call observers !
        // Adding a const notify is bad because both notify functions should then be called
        // at each notification
        Memory* wtf = const_cast<Memory*>(this);
        wtf->notify(msg);
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

bool Memory::load(string f, Memory::addr_t start_addr, bool dump)
{
    bool result = true;
    Memory::addr_t memptr = start_addr;
    bool memprotect(mem_protection);
    mem_protection = false;
    long bytesRead=0;
    QFile memfile(f.c_str());
    // ifstream rom(f.c_str(), ios::in | ios::binary);
    if (memfile.open(QFile::ReadOnly))
    {
        int count=0;
        char c;
        while(!memfile.atEnd())
        {
            memfile.read(&c, 1);
            poke(memptr, c, RD_ONLY);
            if (dump)
            {
                if (count==0)
                {
                    count=16;
                    printf("\n%04x: ", memptr);
                }
                printf("%02x ", (uint8_t)c);
            }
            count--;
            memptr++;
            bytesRead++;
        }
    }
    else
    {
        cerr << "Unable to open ROM file " << f << endl;
        result = false;
    }
    cout << "Memory loaded from file: " << f << ", " << bytesRead << " bytes at " << start_addr << "." << endl;
    mem_protection=memprotect;
    return result;
}

}

