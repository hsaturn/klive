#include "z80.h"
#include <iostream>
using namespace std;

namespace hw
{
z80::z80(Memory* memory)
    : Cpu(memory),
    a(af.a),
    b(bc.hi),
    c(bc.lo),
    d(de.hi),
    e(de.lo),
    h(hl.hi),
    l(hl.lo)
{
    reset();
}

void z80::reset()
{
    pc=0;
    irq_mode(0);
    di();
    i=0;
    r=0;
}

void z80::nop(byte opcode, const char* prefix)
{
    const char c=0;
    if (prefix==nullptr) prefix=&c;
    printf("Z80 %04x: %s%02x Unknown opcode\n", pc-1, prefix, opcode);
    cout << std::flush;
}

void z80::step()
{
    CpuClock before(clock);

    byte opcode = getByte();

    switch(opcode)
    {
        case 0x00:  // Nop
            burn(4);
            break;
        case 0x11:  // ld de, nn
            de.val = getWord();
            burn(10);
            break;
        case 0x18: jr(true); break;
        case 0x20: jr(!af.z); break;	// jr nz
        case 0x28: jr(af.z); break;
        case 0x2B:	// dec hl
            hl.val--;
            burn(6);
            break;
        case 0x30: jr(!af.c); break;
        case 0x36: // ld(hl), n
            memory->poke(hl.val, getByte());
            burn(10);
            break;
        case 0x38: jr(af.c); break;
        case 0x3e:	// ld a, n
            a = getByte();
            burn(7);
            break;
        case 0x47:	// ld b, a
            b = a;
            burn(4);
            break;
        case 0x62:	// ld h,d
            h = d;
            burn(4);
            break;
        case 0x63:	// ld h,e
            h = e;
            burn(4);
            break;
        case 0x6b:  // ld l,e
            l = e;
            burn(4);
            break;

        case 0xAF:  // xor a
            a=0;
            af.c=0;
            af.pv=1;
            af.z=1;
            af.n=0;
            af.s=0;
            burn(4);
            break;
        case 0xB8: compare(b); burn(4); break;	// cp b
        case 0xB9: compare(c); burn(4); break;	// cp c
        case 0xBA: compare(d); burn(4); break;	// cp d
        case 0xBB: compare(e); burn(4); break;	// cp e
        case 0xBC: compare(h); burn(4); break;	// cp h

        case 0xC3: // Jump
            pc = getWord();
            burn(10);
            break;
        case 0xCB:
            step_cb();
            break;
        case 0xD3:
            out(getByte(), a);
            burn(11);
            break;
        case 0xDD:
            step_dd();
            break;
        case 0xED:
            step_ed();
            break;
        case 0xF3:	// DI
            // TODO
            burn(4);
            break;
        case 0xFD:
            step_fd();
            break;
        default:
            nop(opcode);
            break;
    }
    if (clock == before)
    {
        std::cerr << "WARNING: CPU CLOCK 0 cycle for " << (int)opcode << "! " << endl;
    }
}

void z80::jr(bool condition)
{
    uint8_t depl=getByte();

    if (condition)
    {
        pc += static_cast<int8_t>(depl);
        burn(12);
    }
    else
        burn(7);
}

void z80::step_cb()
{
    nop(0xcb);   // TODO
}

void z80::step_dd()
{
    nop(0xdd);
}

void z80::step_ed()
{
    uint8_t opcode=getByte();
    switch(opcode)
    {
        case 0x47:	// ld i,a
            i=a;
            burn(9);
            break;
        default:
            nop(opcode, "0xed ");
            break;
    }
}

void z80::step_fd()
{
    nop(0xfd);
}

void z80::out(uint8_t port, uint8_t val)
{
    // TODO
    // Send notification to out listeners
    cout << "Z80: nyi out" << endl;
}

inline uint8_t parity(uint8_t r)
{
    r ^= r>>4;
    r ^= r>>2;
    r ^= r>>1;
    return r&1;
}

reg8 z80::compare(reg8& n)
{
    reg8 r = (a-n) & 0xFF;
    af.s = a & 0x80 ? 1 : 0;
    af.z = r == 0 ? 1 : 0;
    af.h = (a & 0xf) >= (n & 0Xf) ? 1 : 0;
    af.pv = parity(r);
    af.n = 1;
    af.c = a <  n ? 1 : 0;
    return r;
}

}
