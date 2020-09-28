#pragma once
#include "memory.h"
#include <QElapsedTimer>
#include <QWidget>

namespace hw
{

using cycle=uint64_t;


// TODO move elsewhere
class CpuClock
{
public:
    CpuClock() : curr(0) {}

    inline void burn(const cycle n) { curr+=n; }

    friend bool operator==(const CpuClock& c1, const CpuClock& c2)
    { return c1.curr == c2.curr; }

    void setFrequency(uint32_t hertz) { freq_hz = hertz; }
    auto getEllapsedNs() const { return timer.nsecsElapsed(); }
    auto getFrequency() const { return freq_hz; }
    auto getTimer() const { return timer; }
    void restart() { timer.restart(); }

    cycle cycles() const { return curr; }

private:
    cycle curr;
    uint32_t freq_hz;
    QElapsedTimer timer;
};

class Cpu
{
public:
    struct Registers
    {
        Registers() = default;
        virtual ~Registers() = default;

        // TODO Bad design, registers should not share display responsability
        virtual void update() = 0;
        virtual QWidget* createViewForm(QWidget* parent) = 0;
    };

    Cpu(Memory* memory) : memory(memory)
    {}

    virtual void step()=0;
    virtual void step_no_obs()=0;
    virtual void reset()=0;
    bool steps_to_rt(uint32_t max_steps=20000);

    void burn(cycle n) { clock.burn(n); }

    virtual Registers* regs() = 0;
    const Memory* getMemory() const { return memory; }

protected:
    Memory* memory;
    CpuClock clock;
};


}
