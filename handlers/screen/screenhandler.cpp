#include "screenhandler.h"
#include <iostream>

#include <core/hardware/computer.h>
#include <handlers/gadgets/zx/spectrum/SpectrumScreen.h>
#include <handlers/gadgets/z80/monsview.h>
#include <handlers/gadgets/gdb/minigdb.h>

using namespace hw;
static ScreenHandler screen_handler_instance;

ScreenHandler::ScreenHandler() : Handler("Screen")
{
}

ScreenHandler::~ScreenHandler()
{
    setCpu(nullptr);
}

void ScreenHandler::observableDies(const Z80* dying)
{
    if (cpu==dying) setCpu(nullptr);
}

void ScreenHandler::setCpu(Z80 *new_cpu)
{
    if (cpu) cpu->detach(this);
    cpu=new_cpu;
    if (cpu) cpu->attach(this);
}

void ScreenHandler::initialize(MainWindow *main)
{
    // screen = new Screen();
    // main->createDockWindow(this, screen, "Lines");

    // TODO c'est pour tester
    auto computer = new hw::Computer;

    screen = new SpectrumScreen;
    screen->setMemory(computer->memory);
    main->createDockWindow(this, screen,  "ZX Screen");

    // Disasm test
    monsView = new MonsView;
    monsView->setMemory(computer->memory);
    main->createDockWindow(this, monsView, "Disasm");

    // TODO archi, supprimer ce downcast
    cpu = dynamic_cast<Z80*>(computer->cpu);
    if (cpu)
        cpu->attach(this);

    registers_form = cpu->regs()->createViewForm(nullptr);
    main->createDockWindow(this, registers_form, "Registers");

    gdb = new MiniGdb(computer);
    main->createDockWindow(this, gdb, "Mini gdb");

}

void ScreenHandler::update(Z80* z80, const Z80::Message& msg)
{
    // TODO down cast grrr
    Z80Registers* regs = dynamic_cast<Z80Registers*>(z80->regs());
    if (monsView) monsView->setPointer(regs->pc);
    regs->update();
}
