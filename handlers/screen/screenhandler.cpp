#include "screenhandler.h"
#include <iostream>

#include <core/hardware/computer.h>
#include <handlers/gadgets/zx/spectrum/SpectrumScreen.h>
#include <handlers/gadgets/z80/monsview.h>
#include <handlers/gadgets/gdb/minigdb.h>
#include <handlers/memory/memoryviewer.h>

using namespace hw;
static ScreenHandler screen_handler_instance;

ScreenHandler::ScreenHandler() : Handler("Screen")
{
}

ScreenHandler::~ScreenHandler()
{
    setCpu(nullptr);
}

void ScreenHandler::observableDies(const Cpu* dying)
{
    if (cpu==dying) setCpu(nullptr);
}

void ScreenHandler::setCpu(Cpu *new_cpu)
{
    if (cpu) cpu->detach(this);
    cpu=new_cpu;
    if (cpu) cpu->attach(this);
}

void ScreenHandler::initialize(MainWindow *main)
{
    std::cout << "Initializing screen handler" << std::endl;
    // screen = new Screen();
    // main->createDockWindow(this, screen, "Lines");

    // TODO c'est pour tester
    auto computer = new hw::Computer;

    std::cout << "Creating Zx Screen" << std::endl;
    screen = new SpectrumScreen;
    screen->setMemory(computer->memory);
    main->createDockWindow(this, screen,  "ZX Screen");

    // Disasm test
    std::cout << "Creating disasm view" << std::endl;
    monsView = new MonsView;
    monsView->setMemory(computer->memory);
    main->createDockWindow(this, monsView, "Disasm");

    setCpu(computer->cpu);

    registers_form = cpu->regs()->createViewForm(nullptr);
    main->createDockWindow(this, registers_form, "Registers");

    std::cout << "Creating gdb view" << std::endl;
    gdb = new MiniGdb(computer);
    main->createDockWindow(this, gdb, "Mini gdb");

    std::cout << "Creating memory viewer" << std::endl;
    mem_viewer = new MemoryViewer;
    main->createDockWindow(this, mem_viewer, "Memory");
    mem_viewer->setMemory(computer->memory);
}

void ScreenHandler::update(Cpu* z80, const Z80::Message& msg)
{
    switch(msg.event)
    {
    case Cpu::Message::MACROSTEP:
    case Cpu::Message::BREAK_POINT:
    case Cpu::Message::HALTED:
    case Cpu::Message::RESET:
    {
        // TODO down cast grrr
        Z80Registers* regs = dynamic_cast<Z80Registers*>(z80->regs());
        if (monsView) monsView->setPointer(regs->pc);
        regs->update();
    }
        break;
    default:
        return;
    }
}
