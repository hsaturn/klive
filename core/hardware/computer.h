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

public:
    Computer();
    virtual ~Computer();
    void timer();
    void reset();

    std::string buildCheckPoint();

    Cpu* cpu;
    Memory* memory;
    Screen* screen;
    Keyboard* keyboard;
    std::list<Peripheral*> peripherals;
};

}
