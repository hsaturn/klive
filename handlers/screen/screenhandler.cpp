#include "screenhandler.h"

static ScreenHandler screen;

ScreenHandler::ScreenHandler() : Handler("Screen")
{

}

void ScreenHandler::initialize(MainWindow *main)
{
    screen = new Screen();
    main->createDockWindow(this, screen);
}

