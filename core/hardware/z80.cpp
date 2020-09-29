#include "z80.h"
#include <iostream>
#include <bitset>

using namespace std;

namespace hw
{
Z80::Z80(Memory* memory)
    : Cpu(memory, R.pc),
      pc(R.pc), sp(R.sp), ix(R.ix), iy(R.iy),
      af(R.af), bc(R.bc), de(R.de), hl(R.hl),
      af2(R.af2),bc2(R.bc2),de2(R.de2),hl2(R.hl2),
      a(R.a), b(R.b), c(R.c), d(R.d), e(R.e), h(R.h), l(R.l),
      i(R.i), r(R.r)
{
    reset();
}

void Z80::reset()
{
    pc=0;
    irq_mode(0);
    di();
    i=0;
    r=0;

    af.f=0;	// TODO not sure at all but convenient

    clock.setFrequency(3750000);
    clock.restart();

    Message rst;
    notify(rst);
}

void Z80::nop(byte opcode, const char* prefix)
{
    const char c=0;
    if (prefix==nullptr) prefix=&c;
    printf("Z80 %04x: %s%02x Unknown opcode\n", pc-1, prefix, opcode);
    burn(4);	// Dummy
    cout << std::flush;
}

void Z80::step_no_obs()
{
    CpuClock before(clock);
    incr();

    byte opcode = getByte();

    switch(opcode)
    {
        case 0x00: burn(4); break; 						// Nop
        case 0x01: bc.val = getWord(10); break; 		// ld bc, **
        case 0x04: b=inc(b); burn(4); break;
        case 0x09: add_hl(bc.val, 11); break;
        case 0x0E: c=getByte(7); break;
        case 0x0F: rrca(); break;
        case 0x10: burn(1); b--; jr(b); break; 			// djnz
        case 0x11: de.val = getWord(10); break; 		// ld de, nn
        case 0x16: d=getByte(7); break;					// ld d,*
        case 0x18: jr(true); break;
        case 0x19: add_hl(de.val, 11); break;
        case 0x20: jr(!af.z); break;					// jr nz
        case 0x21: hl.val=getWord(10); break;			// ld hl, (nn)
        case 0x22: writeMem16(hl.val, 16); break;		// ld (**), hl
        case 0x23: hl.val++; burn(6); break;
        case 0x28: jr(af.z); break;
        case 0x29: add_hl(hl.val, 11); break;
        case 0x2A: hl.val=readMem16(16); break; 		// ld hl, (nn)
        case 0x2B: hl.val--; burn(6); break; 			// dec hl
        case 0x30: jr(!af.c); break;
        case 0x32: memory->poke(getWord(13), a); break;	// ld (nn), a
        case 0x33: sp++; burn(6); break;
        case 0x34: 										// inc(hl)
            {
                uint8_t m=memory->peek(hl.val);
                m=inc(m);
                memory->poke(hl.val, m);
                burn(11);
            }
            break;
        case 0x35: 										// dec(hl)
            {
                uint8_t m=memory->peek(hl.val);
                m=dec(m);
                memory->poke(hl.val, m);
                burn(11);
            }
            break;
        case 0x36: memory->poke(hl.val, getByte()); burn(10); break; // ld(hl), n
        case 0x38: jr(af.c); break;
        case 0x39: add_hl(sp, 11); break;
        case 0x3e: a = getByte(); burn(7); break; // ld a, n
        case 0x47: b = a; burn(4); break; 	// ld b,a
        case 0x57: d = a; burn(4); break;	// ld d,a
        case 0x5F: e = a; burn(4); break; 	// ld e,a
        case 0x62: h = d; burn(4); break; 	// ld h,d
        case 0x63: h = e; burn(4); break; 	// ld h,e
        case 0x67: h = a; burn(4); break; 	// ld h,a
        case 0x6b: l = e; burn(4); break; 	// ld l,e
        case 0x6f: l = a; burn(4); break; 	// ld l,a
        case 0x77: memory->poke(hl.val, a); burn(7); break; // ld (hl), a)
        case 0x78: a = b; burn(4); break;
        case 0x7a: a = d; burn(4); break;		// ld a,d
        case 0x90: a=compare(b); burn(4); break;		// sub b
        case 0x91: a=compare(c); burn(4); break;	// sub c
        case 0xA0: and_(b, 4); break;					// and b
        case 0xA1: and_(c, 4); break; 					// and c
        case 0xA2: and_(d, 4); break; 					// and d
        case 0xA3: and_(e, 4); break; 					// and e
        case 0xA4: and_(h, 4); break; 					// and h
        case 0xA5: and_(l, 4); break; 					// and l
        case 0xA6: and_(memory->peek(hl.val), 7); break;// and (HL)
        case 0xA7: and_(a, 4); break;	// and a

        case 0xAF: a=0; af.c=0; af.pv=1; af.z=1; af.n=0; af.s=0; burn(4); break; // xor a
        case 0xB8: compare(b); burn(4); break;	// cp b
        case 0xB9: compare(c); burn(4); break;	// cp c
        case 0xBA: compare(d); burn(4); break;	// cp d
        case 0xBB: compare(e); burn(4); break;	// cp e
        case 0xBC: compare(h); burn(4); break;	// cp h
        case 0xc1: pop(bc.val, 11); break;
        case 0xC3: pc = getWord(10); break; // Jump
        case 0xc5: push(bc.val, 11); break; 	// push bc
        case 0xC9: ret(); break;
        case 0xCB: step_cb(); break;
        case 0xCD: call(17); break;
        case 0xD1: pop(de.val, 11); break;
        case 0xD3: out(getByte(), a); burn(11); break;
        case 0xD9: swap(hl,hl2); swap(de,de2); swap(bc,bc2); burn(4); break;
        case 0xD5: push(de.val, 11); break;		// push de
        case 0xDD: step_dd_fd(ix); break;
        case 0xE1: pop(hl.val, 11); break;
        case 0xE5: push(hl.val, 11); break;		// push hl
        case 0xE6: and_(getByte(), 7); break; // and n
        case 0xEB: swap(de, hl); burn(4); break;	// ex de,hl
        case 0xED: step_ed(); break;
        case 0xF1: pop(af.val, 11); break;
        case 0xF3:	// DI
            cerr << "z80: DI nyi" << endl; burn(4); break;
        case 0xF5: push(af.val, 11); break;		// push hl
        case 0xf6: or_(getByte(), 7); break;
        case 0xFB: // EI
            cerr << "z80: EI nyi" << endl; burn(4); break;
        case 0xFD: step_dd_fd(iy); break;
        case 0xF9: sp=hl.val; burn(6); break; // ld sp,hl
        default: nop(opcode); break;
    }
    if (clock == before)
    {
        std::cerr << "WARNING: CPU CLOCK 0 cycle for " << (int)opcode << "! " << endl;
    }
}

void Z80::call(cycle burnt)
{
    reg16 next_pc = getWord();
    push(pc, burnt);
    pc = next_pc;
}

void Z80::ret()
{
    pc = memory->peek(sp)+(memory->peek(sp+1)<<8);
    sp += 2;
    burn(10);
}

void Z80::push(uint16_t value, cycle burnt)
{
    burn(burnt);
    sp -=2;
    memory->poke(sp, value & 0xff);
    memory->poke(sp+1, value >> 8);
}

void Z80::pop(uint16_t& reg, cycle burnt)
{
     reg = memory->peek(sp)+(memory->peek(sp+1)<<8);
     burn(burnt);
}

uint8_t Z80::inc(uint8_t val)
{
    af.pv= val==0x7F ? 1 : 0;
    af.h = (val&0x0F)==0x0F;	// TODO borrow from bit 3
    val++;
    af.s = val & 0x80 ? 1 : 0;
    af.z = val == 0 ? 1 : 0;
    af.n = 0;
    return val;
}

uint8_t Z80::dec(uint8_t val)
{
    af.pv= val==0x80 ? 1 : 0;
    af.h = (val&0x10)==0x10;	// TODO borrow from bit 4
    val--;
    af.s = val & 0x80 ? 1 : 0;
    af.z = val == 0 ? 1 : 0;
    af.n = 1;
    return val;
}

void Z80::add_hl(uint16_t val, cycle burnt)
{
    burn(burnt);
    hl.val += val;
    af.h = hl.val & 0x800;	// TODO 1 if carry from bit 11
    af.n = 0;
}

void Z80::jr(bool condition)
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

void Z80::step_cb()
{
    incr();
    nop(0xcb);   // TODO
}

uint16_t Z80::readMem16(const cycle burnt)
{
    Memory::addr_t dest=getWord(burnt);
    return memory->peek(dest)+(memory->peek(dest+1)<<8);
}

void Z80::writeMem16(uint16_t val, const cycle burnt)
{
    Memory::addr_t dest=getWord(burnt);
    memory->poke(dest, val & 0xff);
    memory->poke(dest+1, val >> 8);
}

void Z80::step_ed()
{
    incr();
    uint8_t opcode=getByte();
    switch(opcode)
    {
        case 0x43: writeMem16(bc.val, 20); break;

        case 0x47: i=a; burn(9); break; // ld i,a
        case 0x52:	// sbc hl, de
            burn(15);
            {
                uint8_t carry = af.c;
                af.c = de.val+af.c > hl.val;
                hl.val -= de.val+carry;

                af.s = h & 0x80 ? 1 : 0;
                af.u5 = h & 0x20 ? 1 : 0;
                af.u3 = h & 0x08 ? 1 : 0;

                af.z = hl.val == 0 ? 1 : 0;
                af.h = 0;	// TODO set if borrow from bit 12
                af.pv = af.c; // TODO set if overflow
                af.n = 1;
            }
            break;
        case 0x53: writeMem16(de.val, 20); break;	// ld(**), de
        case 0x56:
            cout << "z80: IM1 nyi" << endl;
            burn(8);
            break;
        case 0xb0: // ldir
            {
                // TODO it seems that interrupts are taken in account
                af.h=0; af.pv= (bc.val==1); af.n=0;
                do
                {
                    burn(bc.val ? 21 : 16);
                    incr();
                    memory->poke(de.val, memory->peek(hl.val));
                    hl.val++;
                    de.val++;
                    bc.val--;
                } while(bc.val);
            }
            break;
        case 0xb8: // lddr
            {
                af.h=0; af.pv=0; af.n=0;
                do
                {
                    // TODO it seems that interrupts are taken in account
                    burn(bc.val ? 21 : 16);
                    incr();
                    memory->poke(de.val, memory->peek(hl.val));
                    hl.val--;
                    de.val--;
                    bc.val--;
                } while(bc.val);
            }
            break;
        default:
            nop(opcode, "0xed");
            break;
    }
}

void Z80::step_dd_fd(reg16& in)
{
    incr();
    uint8_t opcode=getByte();
    switch(opcode)
    {
        case 0x21: in=getWord(14); break;	// ld i?, nn
        case 0x35: // dec (i?+*)
        {
            Memory::addr_t addr=in+static_cast<int8_t>(getByte());
            uint8_t m=memory->peek(addr);
            memory->poke(addr, dec(m));
            burn(6);
        }
            break;

        case 0x60:	// ld i?h, reg
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x67:
            {
                burn(8);
                uint8_t* reg = calc_dest_reg(opcode);
                if (reg==nullptr) { cerr << "z80: step_dd_fd Should never occur, unable to compute reg" << endl;  return; }
                in = (in & 0x00FF) | (*reg << 8);
            }
            break;
        case 0x70:	// ld i?h, reg
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x77:
            {
                burn(19);
                uint8_t* reg = calc_dest_reg(opcode);
                if (reg==nullptr) { cerr << "z80: step_dd_fd Should never occur, unable to compute reg" << endl;  return; }
                int8_t offset = static_cast<int8_t>(getByte());
                memory->poke(in+offset, *reg);
            }
            break;

        case 0xCB: step_xxcb(in); break;
        default: nop(opcode, "0xdd/fd "); break;
    }
}

inline uint8_t parity(uint8_t r)
{
    return (__builtin_popcount(r) & 1) ^ 1;
}

uint8_t* Z80::calc_dest_reg(uint8_t opcode)
{
    switch(opcode & 0x07)
    {
        case 0x00: return &b;
        case 0x01: return &c;
        case 0x02: return &d;
        case 0x03: return &e;
        case 0x04: return &h;
        case 0x05: return &l;
        case 0x07: return &a;
    }
    return nullptr;
}

void Z80::step_xxcb(reg16 base_addr)
{
    // FDCB**XX ou DDCB**XX
    // ** -> operation offset
    // XX -> sub opcode

    Memory::addr_t addr=base_addr+static_cast<int8_t>(getByte());
    uint8_t opcode=getByte();

    // 2 compute dest reg
    uint8_t* dest_reg = nullptr;

    uint8_t ohi = opcode & 0xF0;

    if (ohi < 0x40 || ohi > 0x70)
    {
        dest_reg = calc_dest_reg(opcode);
    }

    uint8_t val = memory->peek(addr);

    // 3 compute cycles
    if (((opcode & 0xF0) < 0x40) ||
        ((opcode & 0xF0) > 0x70))
    {
        burn(23);
    }
    else
    {
        burn(20);
    }

    // 4 now execute the instruction on val
    switch(opcode & 0xF8)
    {
        case 0x00: val = rlc(val); break;
        case 0x08: val = rrc(val); break;
        case 0x10: val = rl(val); break;
        case 0x18: val = rr(val); break;
        case 0x20: val = sla(val); break;
        case 0x28: val = sra(val); break;
        case 0x30: val = sll(val); break;
        case 0x38: val = srl(val); break;
        default: // All bit functions
        {
            uint8_t bit_nr=(opcode & 0x38) >> 3;
            std::bitset<8> b=val;

            // Decode bit instruction type
            switch((opcode & 0xC0)>>6)
            {
                case 0x1:	// test bit
                    af.z = b.test(bit_nr) ? 0 : 1;
                    af.h = 1;
                    af.n = 0;
                    break;

                case 0x2: // reset bit
                    b.reset(bit_nr);
                    break;

                case 0x3: // set bit_n;
                    b.set(bit_nr);
                    break;

                default:
                    // Should not arrive here
                    nop(opcode, "step_xxcb should not arrive here");
                    break;
            }
        }
        break;
    }

    // 5 store modified value back in memory
    memory->poke(addr, val);

    // 6 store modified value in register if any
    if (dest_reg)
    {
        *dest_reg=val;
    }

}

void Z80::rrca()
{
    af.h=0;
    af.n=0;
    af.c=a&1;
    a=((a&1)<<7)|(a>>1);
    burn(4);
}

uint8_t Z80::rlc(uint8_t val)
{
    val=(val<<1)|(val>>7);
    af.c=val & 1;
    af.s=val & 0x80;
    af.h=0;
    af.pv=parity(val);
    af.n=0;
    return val;
}

uint8_t Z80::rrc(uint8_t) { nop(0, "rrc");  return 0; }
uint8_t Z80::rl(uint8_t) { nop(0, "rl");  return 0; }
uint8_t Z80::rr(uint8_t) { nop(0, "rr");  return 0; }
uint8_t Z80::sla(uint8_t) { nop(0, "sla");  return 0; }
uint8_t Z80::sra(uint8_t) { nop(0, "sra");  return 0; }
uint8_t Z80::sll(uint8_t) { nop(0, "sll");  return 0; }
uint8_t Z80::srl(uint8_t) { nop(0, "srl");  return 0; }

void Z80::out(uint8_t port, uint8_t val)
{
    // TODO
    // Send notification to out listeners
    cout << "Z80: nyi out" << endl;
}

void Z80::and_(reg8 n, cycle burnt)
{
    burn(burnt);
    a = a & n;
    af.s = a & 0x80 ? 1 : 0;
    af.z = a == 0 ? 1 : 0;
    af.h = 1;
    af.pv = parity(a);	// TODO, this is overflow not parity
    af.n = 0;
    af.c = 0;
}

void Z80::or_(reg8 n, cycle burnt)
{
    burn(burnt);
    a = a | n;
    af.s = a & 0x80 ? 1 : 0;
    af.z = a == 0;
    af.h = 0;
    af.pv = parity(a);
    af.n = 0;
    af.c = 0;
}

reg8 Z80::compare(reg8 n)
{
    reg8 r = a-n;
    af.s = a & 0x80 ? 1 : 0;
    af.z = r == 0 ? 1 : 0;
    af.h = (a ^ n ^ r) & 16 ? 1 : 0;
    af.pv = (a^n) & 0x80 ? (r&0x80)==(n&0x80) : 0;
    af.n = 1;
    af.c = a <  n ? 1 : 0;
    af.u5 = n & 0x20 ? 1 : 0;
    af.u3 = n & 0x08 ? 1 : 0;
    return r;
}


}

