// TODO z80.h does not belong to core


#pragma once
#include <core/hardware/cpu.h>
#include <core/hardware/z80registers.h>
#include <common/observer.h>

#include <cstdint>

namespace hw
{

// TODO observable z80 is a bad idea for polymorphism reasons
class Z80: public Cpu
{

public:
    Z80(Memory* memory);

    void step() override;
    void step_no_obs() override;
    void reset() override;

    // Run steps until the 'real-time' is reached
    void steps_to_rt(uint32_t max_steps=1e6);
    void irq_mode(int mode){}
    void di() {};
    Registers* regs() override { return &R; }

protected:
    inline void burn(cycle cycles) { clock.burn(cycles); };
    void step_ed();
    void step_cb();
    void step_dd_fd(reg16&);
    void step_xxcb(reg16);
    void nop(byte opcode, const char* prefix=nullptr);
    void out(uint8_t port, uint8_t val);

    inline uint8_t getByte(const cycle t=0) { burn(t); return memory->peek(pc++); }

    inline uint16_t getWord(const cycle t=0)
    {
        uint16_t lo =memory->peek(pc++);
        uint16_t hi=memory->peek(pc++);
        burn(t);
        return (hi<<8)+lo;
    }

    // read write 16 bits value from addr stored at pc & pc+1
    uint16_t readMem16(const cycle burnt);
    void writeMem16(const uint16_t, const cycle burnt);

    void add_hl(uint16_t, cycle burnt);
    uint8_t dec(uint8_t value);
    uint8_t inc(uint8_t value);
    void and_(reg8, cycle burnt);
    void or_(reg8, cycle burnt);
    reg8 compare(reg8);
    void jr(bool condition);
    void call(cycle burnt);
    void ret();

    void push(uint16_t, cycle burnt);
    void pop(uint16_t&, cycle burnt);

    void rrca();
    uint8_t rlc(uint8_t);
    uint8_t rrc(uint8_t);
    uint8_t rl(uint8_t);
    uint8_t rr(uint8_t);
    uint8_t sla(uint8_t);
    uint8_t sra(uint8_t);
    uint8_t sll(uint8_t);
    uint8_t srl(uint8_t);

    // return adress of srce/dest register
    // from opcode (see DDxx, FDxx, FDCB**XX instructions etc.)
    uint8_t* calc_dest_reg(uint8_t opcode);

private:
    Z80Registers R;

    reg16&   pc;
    reg16&   sp;
    reg16&   ix;
    reg16&   iy;
    regaf&   af;
    reg16u&  bc;
    reg16u&  de;
    reg16u&  hl;
    regaf&   af2;
    reg16u&  bc2;
    reg16u&  de2;
    reg16u&  hl2;

    reg8&	a;
    reg8&	b;
    reg8&	c;
    reg8&	d;
    reg8&	e;
    reg8&	h;
    reg8&	l;
    reg8&    i;
    reg8&    r;

};

}
