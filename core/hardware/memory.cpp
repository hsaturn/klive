#include "memory.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <iomanip>
#include <stdio.h>

using namespace std;

namespace hw
{
void Memory::fill(addr_t start, Byte::value_type value, size_t size, type_t type)
{
    Message msg;
    msg.event = Message::WRITTEN;
    msg.flags = Message::DATA | (type==UNCHANGE ? 0 : Message::TYPE);
    msg.size = 1;

    if (start+size>=ramtop)
    {
        msg.event = Message::BAD_WRITE;
        notify(msg);
        return;
    }

    while(size--)
    {
        msg.start = start;
        Byte& byte = bytes[start];

        if (byte.type()&RD_ONLY && mem_protection)
        {
            if (detectBadWrites)
            {
                Message msg;
                msg.event = Message::BAD_WRITE;
                notify(msg);
                msg.event = Message::WRITTEN;
            }
            continue;
        }

        byte = value;
        if (type != UNCHANGE)
        {
            byte.setType(type);
        }
        notify(msg);
        start++;
    }
}

void Memory::loadRomImage(string f, Memory::addr_t start)
{
    ifstream rom(f.c_str(), ios::in | ios::binary);
    if (rom)
    {
        int count=0;
        char c;
        while(rom)
        {
            rom.read(&c, 1);
            poke(start, c);	// TODO rom & rdonly attributes !
            if (count==0)
            {
                count=16;
                printf("\n%04x: ", start);
            }
            printf("%02x ", (uint8_t)c);
            count--;
            start++;
        }
        cout << endl;
        std::cout << dec;
    }
    else
    {
        cerr << "Unable to open ROM file " << f << endl;
    }
}

}

