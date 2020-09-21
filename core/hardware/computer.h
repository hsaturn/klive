#pragma once
#include <list>

namespace hw
{

class Cpu;
class Memory;
class Keyboard;
class Screen;
class Peripheral;

class Computer
{
public:
    Computer();

    Cpu* cpu;
    Memory* memory;
    Screen* screen;
    Keyboard* keyboard;
    std::list<Peripheral*> peripherals;
};

}
