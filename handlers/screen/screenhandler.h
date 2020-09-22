#ifndef SCREENHANDLER_H
#define SCREENHANDLER_H

#include <core/handler.h>

#include "screen.h"

using namespace hw;

class ScreenHandler : public Handler
{
public:
    ScreenHandler();
    virtual ~ScreenHandler()=default;

    virtual void initialize(MainWindow*) override;

private:
    Screen* screen;
};

#endif // SCREENHANDLER_H
