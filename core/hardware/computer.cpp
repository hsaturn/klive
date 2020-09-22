#include "computer.h"
#include <QTimer>

#include <core/hardware/memory.h>

namespace hw
{

Computer::Computer()
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&Computer::timer));
    timer->start(10);  // Update interval ms

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
    static Memory::addr_t hl=0;
    static uint8_t byte;
    if (memory && byte++%10==0)
    {
        memory->poke(hl++, byte);
    }
}
}
