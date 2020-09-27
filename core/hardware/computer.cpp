#include "computer.h"
#include <QTimer>

#include <core/hardware/memory.h>
#include <core/hardware/z80.h>
#include <core/hardware/cpu.h>
#include <iostream>

namespace hw
{

Computer::Computer() : QWidget(nullptr)
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&Computer::timer));
    timer->start(1);  // Update interval ms

    memory = new Memory(64*1024);

    // TODO  archi
    memory->loadRomImage("C:\\Users\\hsaturn\\Google Drive\\Spectrum\\Klive\\48.ROM", 0);
    cpu = new Z80(memory);
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
    if (cpu)
    {
        cpu->step();
        cpu->steps_to_rt();
    }
    return;
    static Memory::addr_t hl=16384+6144;
    static uint16_t byte=3;
    static unsigned char attr=1;
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
        // byte=qrand();
        // attr=qrand()&255;
        if (hl>=16384+6144)
        {
            memory->poke(hl++, attr);
        }
        else
        {
            memory->poke(hl++, byte);
        }
        if (hl >= 23296)
        {
            hl=16384;
            // hl = 16384 + 6144;
            attr++;
            for(Memory::addr_t xl=16384; xl<16384+6144; xl++)
                memory->poke(xl, 0);
        }
        if (c++==256)
        {
c=0;
/*        byte *=2;
        if (byte==0) byte=1;
        */
        }
    }
}
}
