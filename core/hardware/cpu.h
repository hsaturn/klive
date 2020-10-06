#pragma once
#include "memory.h"
#include <QElapsedTimer>
#include <QWidget>
#include <map>
#include <unordered_map>
#include <string>

namespace hw
{

using reg16 = uint16_t;
using cycle=uint64_t;

class BreakPoints
{
public:

    class BreakPoint
    {
    public:
        enum type_t
        {
            ALWAYS,
            CONDITIONAL,
            CHECKPOINT
        };

        BreakPoint(type_t type_) : enabled(true), typex(type_) {}
        BreakPoint() : BreakPoint(ALWAYS) {}

        bool isEnabled() const { return enabled; }

        std::string getString() const { return use; }
        void setString(const std::string str) { use=str; }

        void type(type_t t) { typex=t; }
        type_t type() const { return typex; }

    private:
        type_t typex;
        std::string use;	// depends of breakpoint type
        uint16_t flags;	// bit 0: enabled bit 1: if bit 2: checkpoint

        bool enabled;
    };

    void remove(Memory::addr_t addr) { breakpoints.erase(addr); }
    void add(Memory::addr_t addr, BreakPoint bp = {})    { breakpoints[addr] = bp; }

    const BreakPoint* get(Memory::addr_t addr) const;
    size_t size() const { return breakpoints.size(); }
    void clearAll() { breakpoints.clear(); }

private:
    std::unordered_map<Memory::addr_t, BreakPoint> breakpoints;
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
    QElapsedTimer timer;
    uint32_t freq_hz;
};

class Cpu: public Observable<Cpu>
{
public:

    struct Message
    {
       enum event_t { STEP, BREAK_POINT, WHILE_REACHED, UNTIL_REACHED, UNKNOWN_OP};
       Message(event_t event_in=STEP): event(event_in) {}
       event_t event;
       union
       {
         unsigned long data;
         const BreakPoints::BreakPoint* brk;
       };
    };

    struct Registers
    {
        Registers() = default;
        virtual ~Registers() = default;

        // TODO Bad design, registers should not share display responsability
        virtual bool set(std::string reg, int32_t value)=0;
        virtual uint16_t get(const std::string reg)=0;
        virtual void update() = 0;
        virtual QWidget* createViewForm(QWidget* parent) = 0;
        virtual std::string serialize() const = 0;
    };

    Cpu(Memory* memory_, reg16& pc_in) : pc_(pc_in), memory(memory_) {}
    virtual ~Cpu();

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

    reg16 getPc() const { return pc_; }

    BreakPoints breaks;

protected:
    virtual void _reset() =0;
    reg16& pc_;
    Memory* memory;
    CpuClock clock;
    long nsteps = 1;

    std::string sWhile;	// debugging purpose
    std::string sUntil;

    bool running = true;
};


}
