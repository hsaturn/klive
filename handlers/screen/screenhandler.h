#pragma once
#include <core/handler.h>
#include <common/observer.h>
#include <core/hardware/z80.h>

using hw::Z80;

class MonsView;
class SpectrumScreen;
class MiniGdb;

class ScreenHandler : public Handler,
        public Observer<Z80>
{
public:
    ScreenHandler();
    virtual ~ScreenHandler();

    virtual void update(Z80*, const Z80::Message& msg) override;
    virtual void observableDies(const Z80* z80) override;
    void setCpu(Z80*);

    virtual void initialize(MainWindow*) override;

private:
    Z80* cpu = nullptr;
    MonsView* monsView = nullptr;
    SpectrumScreen* screen = nullptr;
    QWidget* registers_form = nullptr;
    MiniGdb* gdb = nullptr;
};
