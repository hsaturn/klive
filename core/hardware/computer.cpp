#include "computer.h"
#include <QTimer>

#include <core/hardware/memory.h>
#include <iostream>

namespace hw
{

Computer::Computer() : QWidget(nullptr)
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&Computer::timer));
    timer->start(1);  // Update interval ms

    memory = new Memory(64*1024);
}

Computer::~Computer()
{
    if (memory)
    {
        delete memory;
    }
}

void Computer::timer()
{
    static Memory::addr_t hl=16384+6144;
    static uint16_t byte=1;
    static unsigned char attr;
    static int c=0;

/*    std::cout << "ATTR IN " << (int)attr << " / ";
    for(hl=16384+6144; hl<23296; hl++)
    {
        memory->poke(hl, attr);
    }
    attr++;
    return;
*/

    if (memory)
    {
        byte=qrand();
        attr=qrand()&255;
        if (hl>=16384+6144)
        {
            memory->poke(hl++, attr);
        }
        else
        {
            memory->poke(hl++, byte>>8);
        }
        if (hl >= 23296)
        {
            hl=16384;
            // hl = 16384 + 6144;
            attr++;
        }
        if (c++==257)
        {
c=0;
        byte *=2;
        if (byte==0) byte=1;
        }
    }
}
}
