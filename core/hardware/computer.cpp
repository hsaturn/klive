#include "computer.h"
#include <QTimer>

#include <core/hardware/memory.h>
#include <core/hardware/z80.h>
#include <core/hardware/cpu.h>
#include <iostream>
#include <iomanip>
#include <sstream>

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

    cpu->start();
}

Computer::~Computer()
{
    if (memory) delete memory;
}

void Computer::timer()
{
    if (!cpu) return;
    cpu->update();

}

void Computer::reset()
{
    if (cpu) cpu->reset();
    if (memory) memory->zero();
}

std::string Computer::checkPoint(bool addCycleAddr) const
{
    std::stringstream cp;

    if (addCycleAddr)
    {
            // Machine state is 'cycle' dependant
            // Store cycle addr for performance reasons
            cp << std::dec << cpu->getClock().cycles();
            cp << ' ' << std::showbase << cpu->getPc();

            cp << ':';
    }

    cp << cpu->regs()->serialize();

    cp << " mem:" << cpu->getMemory()->crc16();

    return cp.str();
}


} // ns
