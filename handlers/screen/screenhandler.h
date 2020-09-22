#ifndef SCREENHANDLER_H
#define SCREENHANDLER_H

#include <common/observer.h>

#include <core/handler.h>
#include <core/hardware/memory.h>

#include "screen.h"

using namespace hw;

class ScreenHandler : public Handler, public Observer<Memory>
{
public:
    ScreenHandler();

    virtual void initialize(MainWindow*) override;

    virtual void update(Memory* sender, typename Memory::Message* msg) override {};

private:
    Screen* screen;
};

#endif // SCREENHANDLER_H
