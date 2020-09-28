#pragma once
#include <list>

#include <QWidget>

#include "memory.h"

namespace hw
{

class Cpu;
class Keyboard;
class Screen;
class Peripheral;

class Computer : public QWidget
{
    Q_OBJECT

public slots:
    void stop() { running=false; }
    void start() { running=true; }
    void step() { bstep=true; }

public:
    Computer();
    virtual ~Computer();
    void timer();

    Cpu* cpu;
    Memory* memory;
    Screen* screen;
    Keyboard* keyboard;
    std::list<Peripheral*> peripherals;

    bool running = true;
    bool bstep = true;
};

}
