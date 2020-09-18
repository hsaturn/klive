#ifndef SCREENHANDLER_H
#define SCREENHANDLER_H

#include <core/handler.h>
#include "screen.h"

class ScreenHandler : public Handler
{
public:
    ScreenHandler();

    virtual void initialize(MainWindow*) override;

private:
    Screen* screen;
};

#endif // SCREENHANDLER_H
