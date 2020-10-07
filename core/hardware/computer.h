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

    // TODO, not sure that computer should be responsible of
    // handle that...
    std::string checkPoint(bool addCycleAddr=true) const;

    Cpu* cpu;
    Memory* memory;
    Screen* screen;
    Keyboard* keyboard;
    std::list<Peripheral*> peripherals;	// TODO aren't screen and keyboard some kind of peripherals ???
};

}
