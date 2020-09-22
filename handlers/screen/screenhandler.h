#ifndef SCREENHANDLER_H
#define SCREENHANDLER_H

#include <common/observer.h>

#include <core/handler.h>
#include <core/hardware/memory.h>

#include "screen.h"

class ScreenHandler : public Handler, public Observer<hw::Memory>
{
public:
    ScreenHandler();

    virtual void initialize(MainWindow*) override;

private:
    Screen* screen;
};

#endif // SCREENHANDLER_H
