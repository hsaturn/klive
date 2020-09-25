#include "screenhandler.h"
#include <iostream>

#include <core/hardware/computer.h>
#include <handlers/gadgets/zx/spectrum/SpectrumScreen.h>

static ScreenHandler screen_handler_instance;

ScreenHandler::ScreenHandler() : Handler("Screen")
{
}

void ScreenHandler::initialize(MainWindow *main)
{
    // screen = new Screen();
    // main->createDockWindow(this, screen, "Lines");

    // TODO c'est pour tester
    auto computer = new hw::Computer;
    auto zx = new SpectrumScreen;
    zx->setMemory(computer->memory);
    main->createDockWindow(this, zx,"ZX Screen");

}
