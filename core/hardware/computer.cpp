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

    // TODO  archi, rom selection etc.
    memory->loadRomImage(":/roms/48.rom", 0);
    cpu = new Z80(memory);
}

Computer::~Computer()
{
    if (memory) delete memory;
}

void Computer::timer()
{
    if (!cpu) return;

    if (bstep or running)
    {
        cpu->step();
        bstep=false;
    }
    if (running)
    {
        cpu->steps_to_rt();
    }
}

} // ns
