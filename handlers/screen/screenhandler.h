#pragma once
#include <core/handler.h>

#include "screen.h"

class ScreenHandler : public Handler
{
public:
    ScreenHandler();
    virtual ~ScreenHandler()=default;

    virtual void initialize(MainWindow*) override;

private:
    Screen* screen;
};
