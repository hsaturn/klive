#pragma once
#include "memory.h"
#include <QElapsedTimer>
#include <QWidget>
#include <map>

using namespace std;

namespace hw
{

using reg16 = uint16_t;
using cycle=uint64_t;

class BreakPoints
{
public:

    enum Status
    {
        ENABLED
    };

    bool enabled(Memory::addr_t pc) const
    {
        const auto& it=breakpoints.find(pc);
        return (it!=breakpoints.end() && it->second==ENABLED);
    }
    void remove(Memory::addr_t addr) { breakpoints.erase(addr); }
    void add(Memory::addr_t addr)    { breakpoints[addr] = ENABLED; }

    bool has(Memory::addr_t addr) { return breakpoints.count(addr); }
    size_t size() const { return breakpoints.size(); }

private:
    map<Memory::addr_t, Status> breakpoints;
};

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
    void restart() { timer.restart(); curr=0; }

    cycle cycles() const { return curr; }

private:
    cycle curr;
    uint32_t freq_hz;
    QElapsedTimer timer;
};

class Cpu: public Observable<Cpu>
{
public:
    struct Message {
       enum event_t { STEP, BREAK_POINT, WHILE_REACHED, UNTIL_REACHED};
       Message(event_t event=STEP): event(event) {}
       event_t event;
    };
    struct Registers
    {
        Registers() = default;
        virtual ~Registers() = default;

        // TODO Bad design, registers should not share display responsability
        virtual bool set(string reg, int32_t value){ return false; };
        virtual uint16_t get(const string reg)=0;
        virtual void update() = 0;
        virtual QWidget* createViewForm(QWidget* parent) = 0;
    };

    Cpu(Memory* memory, reg16& pc) : pc(pc), memory(memory) {}
    virtual ~Cpu() = default;

    void update();
    void step();
    void jp(Memory::addr_t);
    void reset();
    virtual void step_no_obs()=0;
    bool steps_to_rt(uint32_t max_steps=20000);

    void stop() { running=false; nsteps=0; }
    void start() { running=true; nsteps=0;}
    void run_steps(long steps=1) { nsteps += steps; }

    void burn(cycle n) { clock.burn(n); }

    void setWhile(std::string s) { sWhile = s; }
    void setUntil(std::string s) { sUntil = s; }

    const CpuClock& getClock() const { return clock; }

    virtual Registers* regs() = 0;
    const Memory* getMemory() const { return memory; }

    BreakPoints breaks;

protected:
    virtual void _reset() =0;
    reg16& pc;
    Memory* memory;
    CpuClock clock;

    bool running = false;
    unsigned long nsteps = 1;

    string sWhile;	// debugging purpose
    string sUntil;
};


}
