#pragma once
#include <core/hardware/cpu.h>

#include <cstdint>

namespace hw
{
using reg8 = uint8_t;
using reg16 = uint16_t;
using byte = Memory::Byte::value_type;
union reg16u
{
    uint16_t val;
    struct
    {
        uint8_t lo;
        uint8_t hi;
    };
};

union regaf
{
    uint16_t val;
    struct
    {
        uint8_t a;
        union
        {
            uint8_t f;
            struct
            {
                unsigned s: 1;
                unsigned z: 1;
                unsigned u1: 1;
                unsigned h: 1;
                unsigned u2: 1;
                unsigned pv: 1;
                unsigned n: 1;
                unsigned c: 1;
            };
        };
    };
};

class z80: public Cpu
{
public:
    z80(Memory* memory);

    virtual void step() override;
    virtual void reset() override;
    void irq_mode(int mode){}
    void di() {};
    void burn(cycle cycles) { clock.burn(cycles); };

protected:
    void step_ed();
    void step_cb();
    void step_dd();
    void step_fd();
    void nop(byte opcode, const char* prefix=nullptr);
    void out(uint8_t port, uint8_t val);

    inline uint8_t getByte() { return memory->peek(pc++); }

    inline uint16_t getWord()
    { 	uint16_t hi =memory->peek(pc++);
        uint16_t low=memory->peek(pc++)<<8;
        return hi+low;
    }

    void add_hl(uint16_t, cycle burnt);
    uint8_t dec(uint8_t value);
    void and_(reg8, cycle burnt);
    reg8 compare(reg8);
    void jr(bool condition);

private:
    reg16   pc;
    reg16   sp;
    reg16   ix;
    reg16   iy;
    regaf   af;
    reg16u  bc;
    reg16u  de;
    reg16u  hl;
    regaf   af2;
    reg16u  bc2;
    reg16u  de2;
    reg16u  hl2;
    reg8    i;
    reg8    r;

    reg8&	a;
    reg8&	b;
    reg8&	c;
    reg8&	d;
    reg8&	e;
    reg8&	h;
    reg8&	l;

};

}
