#include "cpu.h"
#include <iostream>

namespace hw
{

using namespace std;
static Cpu::Message stepMsg;

bool Cpu::steps_to_rt(uint32_t max_steps)
{
    float ns_per_cycle =1e9 / clock.getFrequency();

    cycle to_reach = clock.getTimer().nsecsElapsed()/ns_per_cycle;

    while(max_steps && (clock.cycles() < to_reach))
    {
        step_no_obs();
        if (max_steps-- == 0)
            return false;
        if (breaks.has(pc))
        {
            notify(stepMsg);
            running = false;
            return false;
        }
    }
    return true;
}

void Cpu::jp(Memory::addr_t addr)
{
    pc=addr;
    notify(stepMsg);
}

void Cpu::step()
{
    step_no_obs();
    notify(stepMsg);
}

void Cpu::update()
{
    if (bstep or running)
    {
        step();
        bstep=false;
    }
    if (running)
    {
        steps_to_rt();
    }
}

}
