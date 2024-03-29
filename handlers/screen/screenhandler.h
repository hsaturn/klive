#pragma once
#include <core/handler.h>
#include <common/observer.h>
#include <core/hardware/z80.h>

using hw::Cpu;

class Console;
class MonsView;
class SpectrumScreen;
class MiniGdb;
class MemoryViewer;

class ScreenHandler : public Handler, public Observer<Cpu>
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
    Console* console = nullptr;
    MonsView* monsView = nullptr;
    SpectrumScreen* screen = nullptr;
    MemoryViewer* mem_viewer = nullptr;
    QWidget* registers_form = nullptr;
    MiniGdb* gdb = nullptr;
};
