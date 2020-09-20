#include "screenhandler.h"

static ScreenHandler screen;

ScreenHandler::ScreenHandler() : Handler("LHANDLER")
{

}

void ScreenHandler::initialize(MainWindow *main)
{
    screen = new Screen();
    main->createDockWindow(this, screen);
}

