#pragma once
#include <core/handler.h>
#include <common/observer.h>
#include <core/hardware/z80.h>

using hw::Cpu;

class MonsView;
class SpectrumScreen;
class MiniGdb;

class ScreenHandler : public Handler,
        public Observer<Cpu>
{
public:
    ScreenHandler();
    virtual ~ScreenHandler();

    virtual void update(Cpu*, const Cpu::Message& msg) override;
    virtual void observableDies(const Cpu* cpu) override;
    void setCpu(Cpu*);

    virtual void initialize(MainWindow*) override;

private:
    Cpu* cpu = nullptr;
    MonsView* monsView = nullptr;
    SpectrumScreen* screen = nullptr;
    QWidget* registers_form = nullptr;
    MiniGdb* gdb = nullptr;
};
