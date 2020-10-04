#include "cpu.h"
#include <iostream>
#include <common/expr.h>

namespace hw
{

using namespace std;
static Cpu::Message stepMsg;

bool Cpu::steps_to_rt(uint32_t max_steps)
{
    float ns_per_cycle =1e9 / clock.getFrequency();

    cycle to_reach = clock.getTimer().nsecsElapsed()/ns_per_cycle;

    while(running && max_steps && (clock.cycles() < to_reach))
    {
        step_no_obs();
        if (max_steps-- == 0)
            return false;
        if (breaks.has(pc))	// TODO is it really working (z80 has a pc member conflicting with cpu::pc)
        {
            static Message breakMsg(Message::BREAK_POINT);
            nsteps=0;
            notify(breakMsg);
            running = false;
            return false;
        }
        else if (sUntil.length())
        {
            exprtype result;
            string tmp = sUntil;
            // TODO use an AST for speed
            if (parseExpression(this, tmp, result))
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
            if (parseExpression(this, tmp, result))
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
    clock.restart();
    running = false;
    _reset();
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
