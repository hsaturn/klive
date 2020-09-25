#pragma once
#include "memory.h"

namespace hw
{

using cycle=uint32_t;

// TODO move elsewhere
class CpuClock
{
public:
    CpuClock() : curr(0) {}

    inline void burn(const cycle n) { curr+=n; }

    friend bool operator==(const CpuClock& c1, const CpuClock& c2)
    { return c1.curr == c2.curr; }
private:
    cycle curr;
};

class Cpu
{
public:
    Cpu(Memory* memory) : memory(memory)
    {}

    virtual void step()=0;
    virtual void reset()=0;

    void burn(cycle n) { clock.burn(n); }
    const Memory* getMemory() const { return memory; }

protected:
    Memory* memory;
    CpuClock clock;
};
}
