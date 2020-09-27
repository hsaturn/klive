#include "cpu.h"

bool hw::Cpu::steps_to_rt(uint32_t max_steps)
{
    float ns_per_cycle =1e9 / clock.getFrequency();

    cycle to_reach = clock.getTimer().nsecsElapsed()/ns_per_cycle;

    while(max_steps && (clock.cycles() < to_reach))
    {
        step_no_obs();
        if (max_steps-- == 0)
            return false;
    }
    return true;
}
