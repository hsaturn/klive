#include "cpu.h"
#include <iostream>
#include <common/expr.h>

namespace hw
{

using namespace std;
Cpu::Message stepMsg(Cpu::Message::STEP);

const BreakPoints::BreakPoint* BreakPoints::get(Memory::addr_t addr) const
{
    const auto& it=breakpoints.find(addr);

    if (it != breakpoints.end())
    {
        return &it->second;
    }
    return nullptr;
}

CpuClock::CpuClock()
{
    restart();
}

void CpuClock::restart()
{
    timer.restart();
    curr=0;
    irq_interval=0;
    next_irq = 0;
}

Cpu::~Cpu(){}

bool Cpu::steps_to_rt(uint32_t max_steps)
{
    float ns_per_cycle =1e9 / clock.getFrequency();

    cycle to_reach = clock.getTimer().nsecsElapsed()/ns_per_cycle;

    while(running && max_steps && (clock.cycles() < to_reach))
    {
        static Message breakMsg(Message::BREAK_POINT);
        step_no_obs();
        if (max_steps-- == 0)
            return false;

        breakMsg.brk = breaks.get(pc_);
        if (breakMsg.brk)
        {
            // Observers should take decision: stop or continue
            notify(breakMsg);
            if (!running)
            {
                return false;
            }
        }
        else if (sUntil.length())
        {
            exprtype result;
            string tmp = sUntil;
            // TODO use an AST for speed
            if (parseExpression(tmp, result, this))
            {
                if (result)
                {
                    static Message untilReached(Message::UNTIL_REACHED);
                    cout << "Until " << sUntil << " reached." << endl;
                    sUntil.clear();
                    running = false;
                    notify(untilReached);
                }
            }
            else
            {
                cerr << "While " << sUntil << " error " << endl; // TODO check sUntil validity (setUntil)
            }
        }
        else if (sWhile.length())
        {
            exprtype result;
            string tmp = sWhile;
            // TODO use an AST for speed
            if (parseExpression(tmp, result, this))
            {
                if (result==0)
                {
                    static Message whileReached(Message::WHILE_REACHED);
                    cout << "While " << sWhile << " reached." << endl;
                    running = false;
                    sWhile.clear();
                    notify(whileReached);
                }
            }
            else
            {
                cerr << "While " << sWhile << " error " << endl; // TODO check sWhile validity (setWhile)
            }
        }
    }
    return true;
}

void Cpu::reset()
{
    Message rst(Message::RESET);
    notify(rst);
    clock.restart();
    running = false;
    _reset();
}

void Cpu::jp(Memory::addr_t addr)
{
    pc_=addr;
    notify(stepMsg);	// TODO really ?
}

void Cpu::step()
{
    step_no_obs();
    notify(stepMsg);
}

void Cpu::update()
{
    if (nsteps or running)
    {
        step();
        if (nsteps) nsteps--;
        while(nsteps)
        {
            step_no_obs();
            nsteps--;
        }
    }
    if (running)
    {
        steps_to_rt();
    }
}

}
