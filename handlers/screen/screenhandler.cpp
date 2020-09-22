#include "screenhandler.h"
#include <iostream>

static ScreenHandler screen_handler_instance;

ScreenHandler::ScreenHandler() : Handler("Screen")
{
    // TODO Comment s'enregister et être certain que la mémoire est prête ....
}

void ScreenHandler::initialize(MainWindow *main)
{
    screen = new Screen();
    main->createDockWindow(this, screen);
}
