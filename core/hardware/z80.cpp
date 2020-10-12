#include "z80.h"
#include <iostream>
#include <bitset>

using namespace std;

namespace hw
{
Z80::Z80(Memory* mem)
    : Cpu(mem, R.pc),
      pc(R.pc), sp(R.sp), ix(R.ix), iy(R.iy),
      af(R.af), bc(R.bc), de(R.de), hl(R.hl),
      af2(R.af2),bc2(R.bc2),de2(R.de2),hl2(R.hl2),
      a(R.a), b(R.b), c(R.c), d(R.d), e(R.e), f(R.f), h(R.h), l(R.l),
      i(R.i), r(R.r)
{
    reset();
}

void Z80::_reset()
{
    pc=0;
    set_irq_mode(0);
    di();
    i=0;
    r=0;

    f.f=0;	// TODO not sure at all but convenient
    bc.val=0;
    de.val=0;
    hl.val=0;
    af2.val=0;
    bc2.val=0;
    de2.val=0;
    hl2.val=0;
    sp=0;
    ix=0;
    iy=0;

    clock.setFrequency(3750000);
    clock.restart();
    clock.irq(70800);

    Message rst;
    notify(rst);
}

void Z80::nop(byte opcode, const char* prefix)
{
    Message unknown(Message::UNKNOWN_OP);
    const char c=0;
    if (prefix==nullptr) prefix=&c;
    printf("Z80 %04x: %s%02x Unknown opcode\n", pc-1, prefix, opcode);
    burn(4);	// Avoid cycle error
    running=false;
    unknown.data = opcode | (static_cast<uint32_t>(last) << 16);
    notify(unknown);
    cout << std::flush;
}

void Z80::nmi()
{
    iff2 = iff1;
    std::cout << "TODO : NMI" << std::endl;
}

void Z80::retn()
{
    iff1 = iff2;
    ret(true, 14);
}

void Z80::reti()
{
    ret(true, 14);
}

void Z80::irq(Memory::addr_t next_pc)
{
    push(pc, 17);
    pc = next_pc;
}

void Z80::di()
{
    iff1 = 0;
    iff2 = 0;
}

void Z80::ei()
{
    iff1 = 1;
    iff2 = 1;
    irq_enabler = 1;
}

void Z80::step_no_obs()
{
    last = pc;
    CpuClock before(clock);
    incr();

    if (irq_enabler>0)
    {
        irq_enabler--;
    }
    else if (iff1 && (irq_enabler==0) && clock.irq())
    {
        irq(0x38);	// TODO, the addr has to be computed
        return;
    }
    byte opcode = getByte();

    switch(opcode)
    {
        case 0x00: burn(4); break; 						// Nop
        case 0x01: bc.val = getWord(10); break; 		// ld bc, **
        case 0x02: memory->poke(bc.val, a); burn(7); break;
        case 0x03: bc.val++; burn(6); break;					// inc bc
        case 0x04: b=inc(b, 4); break;
        case 0x05: b=dec(b, 4); break;
        case 0x06: b=getByte(); burn(7); break;			// ld b, *
        case 0x07: rlca(); break;
        case 0x08: std::swap(af.val, af2.val); burn(4); break;
        case 0x09: add_hl(bc.val, 11); break;
        case 0x0a: a=memory->peek(bc.val); burn(7); break;	// ld a, (bc)
        case 0x0B: bc.val--; burn(6); break;
        case 0x0C: c=inc(c, 4); break;
        case 0x0D: c=dec(c, 4); break;					// dec c
        case 0x0E: c=getByte(7); break;
        case 0x0F: rrca(); break;
        case 0x10: burn(1); b--; jr(b); break; 			// djnz
        case 0x11: de.val = getWord(10); break; 		// ld de, nn
        case 0x12: memory->poke(de.val, a); burn(7); break;
        case 0x13: de.val++; burn(6); break;					// inc de
        case 0x14: d=inc(d, 4); break;
        case 0x15: d=dec(d, 4); break;
        case 0x16: d=getByte(7); break;					// ld d,*
        case 0x18: jr(true); break;
        case 0x19: add_hl(de.val, 11); break;
        case 0x1A: a=memory->peek(de.val); burn(7); break;	// ld a, (de)
        case 0x1B: de.val--; burn(6); break;			// dec de
        case 0x1C: e=inc(e, 4); break;
        case 0x1D: e=dec(e, 4); break;
        case 0x1F: rra(); break;
        case 0x20: jr(!f.z); break;					// jr nz
        case 0x21: hl.val=getWord(10); break;			// ld hl, (nn)
        case 0x22: writeMem16(hl.val, 16); break;		// ld (**), hl
        case 0x23: hl.val++; burn(6); break;
        case 0x24: h=inc(h, 4); break;
        case 0x25: h=dec(h, 4); break;
        case 0x26: h=getByte(); burn(7); break;			// ld h, *
        case 0x28: jr(f.z); break;
        case 0x29: add_hl(hl.val, 11); break;
        case 0x2A: hl.val=readMem16(16); break; 		// ld hl, (nn)
        case 0x2B: hl.val--; burn(6); break; 			// dec hl
        case 0x2C: l=inc(l, 4); break;
        case 0x2D: l=dec(l, 4); break;
        case 0x30:
                    jr(!f.c); break;
        case 0x31: sp=readMem16(10); break;				// ld sp, **
        case 0x32: memory->poke(getWord(13), a); break;	// ld (nn), a
        case 0x33: sp++; burn(6); break;				// inc sp
        case 0x34: 										// inc(hl)
            {
                uint8_t m=memory->peek(hl.val);
                m=inc(m, 11);
                memory->poke(hl.val, m);
            }
            break;
        case 0x35: 										// dec(hl)
            {
                uint8_t m=memory->peek(hl.val);
                m=dec(m,11);
                memory->poke(hl.val, m);
            }
            break;
        case 0x36: memory->poke(hl.val, getByte()); burn(10); break; // ld(hl), n
        case 0x37: f.c=1; f.h=0; f.n=0; burn(4); break;		// scf
        case 0x38: jr(f.c); break;
        case 0x39: add_hl(sp, 11); break;
        case 0x3a: a=memory->peek(getWord(13)); break;
        case 0x3b: sp--; burn(6); break;
        case 0X3c: a=inc(a, 4); break;
        case 0x3d: a=dec(a, 4); break;
        case 0x3e: a = getByte(); burn(7); break; // ld a, n
        case 0x3f: f.c = f.c ? 0 : 1; f.n=0; burn(4); break;	// ccf	TODO f.u5=a.5 f.u3=a.3
        case 0x40: burn(4); break;	//ld b,b
        case 0x41: b = c; burn(4); break;
        case 0x42: b = d; burn(4); break;
        case 0x43: b = e; burn(4); break;
        case 0x44: b = h; burn(4); break;
        case 0x45: b = l; burn(4); break;
        case 0x46: b = memory->peek(hl.val); burn(7); break;
        case 0x47: b = a; burn(4); break; 	// ld b,a
        case 0x48: c = b; burn(4); break;
        case 0x49: burn(4); break;	// ld c,c
        case 0x4a: c = d; burn(4); break;
        case 0x4b: c = e; burn(4); break;
        case 0x4c: c = h; burn(4); break;
        case 0x4d: c = l; burn(4); break;
        case 0x4e: c = memory->peek(hl.val); burn(7); break;  // ld c,(hl)
        case 0x4f: c = a; burn(4); break;
        case 0x50: d = b; burn(4); break;
        case 0x51: d = c; burn(4); break;
        case 0x52: burn(4); break;	// ld d=d
        case 0x53: d = e; burn(4); break;
        case 0x54: d = h; burn(4); break;
        case 0x55: d = l; burn(4); break;
        case 0x56: d = memory->peek(hl.val); burn(7); break;  // ld d,(hl)
        case 0x57: d = a; burn(4); break;	// ld d,a
        case 0x58: e = b; burn(4); break;
        case 0x59: e = c; burn(4); break;
        case 0x5a: e = d; burn(4); break;
        case 0x5b: burn(4); break;	// ld e,e
        case 0x5c: e = h; burn(4); break;
        case 0x5d: e = l; burn(4); break;
        case 0x5e: e = memory->peek(hl.val); burn(7); break;  // ld e,(hl)
        case 0x5F: e = a; burn(4); break; 	// ld e,a
        case 0x60: h = b; burn(4); break; 	// ld h,b
        case 0x61: h = c; burn(4); break; 	// ld h,c
        case 0x62: h = d; burn(4); break; 	// ld h,d
        case 0x63: h = e; burn(4); break; 	// ld h,e
        case 0x64: burn(4); break; 			// ld h,h
        case 0x65: h = l; burn(4); break; 	// ld h,l
        case 0x66: h = memory->peek(hl.val); burn(7); break;	// ld h, (hl)
        case 0x67: h = a; burn(4); break; 	// ld h,a
        case 0x68: l = b; burn(4); break; 	// ld l,b
        case 0x69: l = c; burn(4); break; 	// ld l,c
        case 0x6a: l = d; burn(4); break; 	// ld l,d
        case 0x6b: l = e; burn(4); break; 	// ld l,e
        case 0x6c: l = h; burn(4); break; 	// ld l,h
        case 0x6d: burn(4); break; 	// ld l,l
        case 0x6e: l = memory->peek(hl.val); burn(7); break;	// ld l, (hl)
        case 0x6f: l = a; burn(4); break; 	// ld l,a
        case 0x70: memory->poke(hl.val, b); burn(7); break;	// ld (hl),b
        case 0x71: memory->poke(hl.val, c); burn(7); break;	// ld (hl),c
        case 0x72: memory->poke(hl.val, d); burn(7); break;	// ld (hl),d
        case 0x73: memory->poke(hl.val, e); burn(7); break;	// ld (hl),e
        case 0x74: memory->poke(hl.val, h); burn(7); break;	// ld (hl),h
        case 0x75: memory->poke(hl.val, l); burn(7); break;	// ld (hl),l

        case 0x77: memory->poke(hl.val, a); burn(7); break; // ld (hl), a)
        case 0x78: a = b; burn(4); break;
        case 0x79: a = c; burn(4); break;
        case 0x7a: a = d; burn(4); break;		// ld a,d
        case 0x7b: a = e; burn(4); break;		// ld a,e
        case 0x7c: a = h; burn(4); break;		// ld a,h
        case 0x7d: a = l; burn(4); break;		// ld a,l
        case 0x7e: a = memory->peek(hl.val); burn(7); break; // ld a, (hl)
        case 0x7f: burn(4); break;							// ld a,a
        case 0x87: a=add_(a,a,4); break;	// add a,a
        case 0x88: adc_(b); break;
        case 0x89: adc_(c); break;
        case 0x8a: adc_(d); break;
        case 0x8b: adc_(e); break;
        case 0x8c: adc_(h); break;
        case 0x8d: adc_(l); break;
        case 0x8e: adc_(memory->peek(hl.val),7); break;
        case 0x8f: adc_(a); break;
        case 0x90: a=compare(b); burn(4); break;		// sub r
        case 0x91: a=compare(c); burn(4); break;
        case 0x92: a=compare(d); burn(4); break;
        case 0x93: a=compare(e); burn(4); break;
        case 0x94: a=compare(h); burn(4); break;
        case 0x95: a=compare(l); burn(4); break;
        case 0x96: a=compare(memory->peek(hl.val)); burn(7); break;
        case 0x97: a=compare(a); burn(4); break;
        case 0x98: sbc(b, 4); break;
        case 0x99: sbc(c, 4); break;
        case 0x9A: sbc(d, 4); break;
        case 0x9B: sbc(e, 4); break;
        case 0x9C: sbc(h, 4); break;
        case 0x9D: sbc(l, 4); break;
        case 0x9E: sbc(memory->peek(hl.val), 7); break;
        case 0x9F: sbc(a, 4); break;
        case 0xA0: and_(b, 4); break;					// and b
        case 0xA1: and_(c, 4); break; 					// and c
        case 0xA2: and_(d, 4); break; 					// and d
        case 0xA3: and_(e, 4); break; 					// and e
        case 0xA4: and_(h, 4); break; 					// and h
        case 0xA5: and_(l, 4); break; 					// and l
        case 0xA6: and_(memory->peek(hl.val), 7); break;// and (HL)
        case 0xA7: and_(a, 4); break;	// and a
        case 0xA8: xor_(b, 4); break;
        case 0xA9: xor_(c, 4); break;
        case 0xAA: xor_(d, 4); break;
        case 0xAB: xor_(e, 4); break;
        case 0xAC: xor_(h, 4); break;
        case 0xAD: xor_(l, 4); break;
        case 0xAE: xor_(memory->peek(hl.val), 7); break;	// xor (hl)
        case 0xAF: a=0; f.c=0; f.pv=1; f.z=1; f.n=0; f.s=0; burn(4); break; // xor a
        case 0xb0: or_(b, 4); break; // or b
        case 0xb1: or_(c, 4); break; // or c
        case 0xb2: or_(d, 4); break; // or d
        case 0xb3: or_(e, 4); break; // or e
        case 0xb4: or_(h, 4); break; // or h
        case 0xb5: or_(l, 4); break; // or l
        case 0xb6: or_(memory->peek(hl.val), 7); break; // or (hl)
        case 0xb7: or_(a, 4); break; // or a
        case 0xB8: compare(b); burn(4); break;	// cp b
        case 0xB9: compare(c); burn(4); break;	// cp c
        case 0xBA: compare(d); burn(4); break;	// cp d
        case 0xBB: compare(e); burn(4); break;	// cp e
        case 0xBC: compare(h); burn(4); break;	// cp h
        case 0xC0: ret(not f.z, 11,5); break;
        case 0xc1: pop(bc.val, 11); break;
        case 0xC2: jp_if(not f.z); break;
        case 0xC3: pc = getWord(10); break; // Jump
        case 0xC4: call_if(not f.z); break;
        case 0xc5: push(bc.val, 11); break; 	// push bc
        case 0xc6: a=add_(a, getByte(), 7); break;	// add a, *
        case 0xc8: ret(f.z, 11, 5); break; // ret z
        case 0xC9: ret(true, 10); break;
        case 0xCA: jp_if(f.z); break;
        case 0xCB: step_cb(); break;
        case 0xCC: call_if(f.z); break;
        case 0xCD: call(); break;
        case 0xD0: ret(not f.c, 11,5); break;
        case 0xD1: pop(de.val, 11); break;
        case 0xD3: out(getByte(), a); burn(11); break;
        case 0xD4: call_if(not f.c); break;
        case 0xD9: swap(hl,hl2); swap(de,de2); swap(bc,bc2); burn(4); break;
        case 0xD2: jp_if(not f.c); break;
        case 0xD5: push(de.val, 11); break;		// push de
        case 0xD6: a=compare(getByte(7)); break;		// sub *
        case 0xD7: rst(0x10); break;
        case 0xD8: ret(f.c, 11, 5); break;
        case 0xDA: jp_if(f.c); break;
        case 0xDC: call_if(f.c); break;
        case 0xDD: step_dd_fd(ix); break;
        case 0xE1: pop(hl.val, 11); break;
        case 0xE3: ex_word_at_sp(hl.val); break;	// ex (sp); hl
        case 0xE5: push(hl.val, 11); break;		// push hl
        case 0xE6: and_(getByte(), 7); break; // and n
        case 0xE9: pc=hl.val; burn(4); break;	// jp (hl)
        case 0xEB: swap(de, hl); burn(4); break;	// ex de,hl
        case 0xED: step_ed(); break;
        case 0xF1: pop(af.val, 11); break;
        case 0xF3: di(); burn(4); break;
        case 0xF5: push(af.val, 11); break;		// push hl
        case 0xf6: or_(getByte(), 7); break;
        case 0xF9: sp=hl.val; burn(6); break; // ld sp,hl
        case 0xFB: ei(); burn(4); break;
        case 0xFD: step_dd_fd(iy); break;
        case 0xFE: compare(getByte(7)); break;	// cp *
        default: nop(opcode); break;
    }
    if (clock == before)
    {
        std::cerr << "WARNING: CPU CLOCK 0 cycle for " << (int)opcode << "! " << endl;
    }
}

void Z80::ex_word_at_sp(uint16_t &val)
{
    uint8_t lo = memory->peek(sp);
    uint8_t hi = memory->peek(sp+1);
    memory->poke(sp  , val & 0xFF);
    memory->poke(sp+1, val >> 8);
    val = (hi<<8)|lo;
    burn(19);
}

void Z80::jp_if(bool flag)
{
    burn(10);
    Memory::addr_t addr=getWord();
    if (flag) pc=addr;
}

void Z80::call_if(bool flag)
{
    reg16 next_pc = getWord();
    if (flag)
    {
        push(pc, 17);
        pc = next_pc;
    }
    else
        burn(10);
}

void Z80::call()
{
    reg16 next_pc = getWord();
    push(pc, 17);
    pc = next_pc;
}

void Z80::rst(uint8_t addr)
{
    push(pc, 11);
    pc = addr;
}

void Z80::ret(bool flag, cycle burn_ret, cycle burn_noret)
{
    if (flag)
    {
        pc = memory->peek(sp)+(memory->peek(sp+1)<<8);
        sp += 2;
        burn(burn_ret);
    }
    else {
        burn(burn_noret);
    }
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
     sp+=2;
     burn(burnt);
}

uint8_t Z80::inc(uint8_t val, cycle burnt)
{
    f.pv= val==0x7F ? 1 : 0;
    f.h = (val&0x0F)==0x0F;	// TODO borrow from bit 3
    val++;
    f.s = val & 0x80 ? 1 : 0;
    f.z = val == 0 ? 1 : 0;
    f.n = 0;
    burn(burnt);
    return val;
}

uint8_t Z80::dec(uint8_t val, cycle burnt)
{
    f.pv= val==0x80 ? 1 : 0;
    f.h = (val&0x10)==0x10;	// TODO borrow from bit 4
    val--;
    f.s = val & 0x80 ? 1 : 0;
    f.z = val == 0 ? 1 : 0;
    f.n = 1;
    burn(burnt);
    return val;
}

void Z80::add_hl(uint16_t val, cycle burnt)
{
    burn(burnt);
    hl.val += val;
    f.h = hl.val & 0x800;	// TODO 1 if carry from bit 11
    f.n = 0;
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

        case 0x45: retn(); break;
        case 0x46: set_irq_mode(0); burn(8); break;
        case 0x47: i=a; burn(9); break; // ld i,a
        case 0x4B: // ld bc, (**)
            {
                Memory::addr_t addr=getWord(20);
                c=memory->peek(addr);
                b=memory->peek(addr+1);
                break;
            }
        case 0x4D:
            reti(); break;
        case 0x4E: set_irq_mode(0); burn(8); break;
        case 0x52:	// sbc hl, de
            burn(15);
            {
                uint8_t carry = f.c;
                f.c = de.val+f.c > hl.val;
                hl.val -= de.val+carry;

                f.s = h & 0x80 ? 1 : 0;
                f.u5 = h & 0x20 ? 1 : 0;
                f.u3 = h & 0x08 ? 1 : 0;

                f.z = hl.val == 0 ? 1 : 0;
                f.h = 0;	// TODO set if borrow from bit 12
                f.pv = f.c; // TODO set if overflow
                f.n = 1;
            }
            break;
        case 0x53: writeMem16(de.val, 20); break;	// ld(**), de
        case 0x55: retn(); break;
        case 0x56: set_irq_mode(1); burn(8); break;
            break;
        case 0x5B:	// ld de, (**)
            {
                Memory::addr_t addr=getWord(20);
                e=memory->peek(addr);
                d=memory->peek(addr+1);
                break;
            }
        case 0x5D: retn(); break;
        case 0x5E: set_irq_mode(2); burn(8); break;
        case 0x65: retn(); break;
        case 0x66: set_irq_mode(0); burn(8); break;
        case 0x6D: retn(); break;
        case 0x6E: set_irq_mode(0); burn(8); break;
        case 0x75: retn(); break;
        case 0x76: set_irq_mode(1); burn(8); break;
        case 0x7D: retn(); break;
        case 0x7E: set_irq_mode(2); burn(8); break;
        case 0xB0: // ldir
            {
                r-=2;
                // TODO it seems that interrupts are taken in account
                f.h=0; f.pv= (bc.val==1); f.n=0;
                do
                {
                    burn(bc.val ? 21 : 16);
                    incr();
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
                r-=2;
                f.h=0; f.pv=0; f.n=0;
                do
                {
                    // TODO it seems that interrupts are taken in account
                    burn(bc.val ? 21 : 16);
                    incr();
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
                memory->poke(addr, dec(m, 6));
            }
            break;
        case 0x36:	// ld (ii+*), *
            {
                Memory::addr_t addr=in+static_cast<int8_t>(getByte());
                memory->poke(addr, getByte());
                burn(19);
            }
            break;
        case 0x46:	// ld b, (ii+*)
            {
                Memory::addr_t addr=in+static_cast<int8_t>(getByte(19));
                b = memory->peek(addr);
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
        case 0x6e:
            {
                burn(19);
                Memory::addr_t addr = in + static_cast<int8_t>(getByte());
                l=memory->peek(addr);
                burn(3);
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
        case 0x86:	// add a, (ii+*)
            {
                burn(19);
                int8_t offset = static_cast<int8_t>(getByte());
                a=add_(a, memory->peek(in+offset), 19);
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

void Z80::step_cb()
{
    incr();
    bool ismem=false;
    uint8_t mem;
    uint8_t opcode=getByte();

    uint8_t* reg = calc_dest_reg(opcode);
    if (reg==nullptr)
    {
        burn(15);
        ismem=true;
        reg=&mem;
        mem=memory->peek(hl.val);
    }
    else {
        burn(8);
    }

    switch(opcode>>3)
    {
    case 0: *reg=rlc(*reg); break;
    case 1: *reg=rrc(*reg); break;
    case 2: *reg=rl(*reg); break;
    case 3: *reg=rr(*reg); break;
    case 4: *reg=sla(*reg); break;
    case 5: *reg=sra(*reg); break;
    case 6: *reg=sll(*reg); break;
    case 7: *reg=srl(*reg); break;
    default:
        uint8_t bit_nr=(opcode & 0x38) >> 3;
        uint8_t mask=1 << bit_nr;

        if (opcode<0x80)		// bit x, reg
        {
            f.z = (*reg & mask) ? 0 : 1;
        }
        else if (opcode<0xC0) 	// res x, reg
        {
            *reg &= ~mask;
        } 						// set x, reg
        else
        {
            *reg |= mask;
        }
        break;
    }

    if (ismem)
    {
        memory->poke(hl.val, *reg);
    }
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
            uint8_t mask=1 << bit_nr;

            // Decode bit instruction type
            switch((opcode & 0xC0)>>6)
            {
                case 0x1:	// test bit
                    f.z = val & mask ? 0 : 1;
                    f.h = 1;
                    f.n = 0;
                    break;

                case 0x2: // reset bit
                    val &= ~mask;
                    memory->poke(addr, val);
                    break;

                case 0x3: // set bit_n;
                    val |= mask;
                    memory->poke(addr, val);
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

    // 6 store modified value in register if any
    if (dest_reg)
    {
        *dest_reg=val;
    }

}

void Z80::rrca()
{
    f.h=0;
    f.n=0;
    f.c=a&1;
    a=((a&1)<<7)|(a>>1);
    burn(4);
}

void Z80::rra()
{
    auto a0 = a;
    f.h=0;
    f.n=0;
    a=a>>1;
    if (f.c) a=a|0x80;
    f.c=a0&1;
    burn(4);
}

uint8_t Z80::rlc(uint8_t val)
{
    val=(val<<1)|(val>>7);
    f.c=val & 1;
    f.s=val & 0x80;
    f.h=0;
    f.pv=parity(val);
    f.n=0;
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
    f.s = a & 0x80 ? 1 : 0;
    f.z = a == 0 ? 1 : 0;
    f.h = 1;
    f.pv = parity(a);	// TODO, this is overflow not parity
    f.n = 0;
    f.c = 0;
    f.u5 = a & 0x20 ? 1 : 0;	// TODO set for SUB/SBC but not for CP
    f.u3 = a & 0x08 ? 1 : 0;
}

void Z80::xor_(reg8 n, cycle burnt)
{
    burn(burnt);
    a = a^n;
    f.f=0;
    f.s = a & 0x80 ? 1 : 0;
    f.z = a == 0;
    f.pv = parity(a);
    f.u3 = a & 0x08 ? 1 : 0;
}

void Z80::or_(reg8 n, cycle burnt)
{
    burn(burnt);
    a = a | n;
    f.s = a & 0x80 ? 1 : 0;
    f.z = a == 0;
    f.h = 0;
    f.pv = parity(a);
    f.n = 0;
    f.c = 0;
    f.u3 = a & 0x08 ? 1 : 0;
}

static int16_t to_int16(uint8_t v)
{
    return static_cast<int16_t>(static_cast<int8_t>(v));
}
static int16_t to_uint16(uint8_t v)
{
    return static_cast<uint16_t>(v);
}

reg8 Z80::add_(reg8 a, reg8 n, cycle burnt)
{
    reg8 r = a+n;
    f.s = r & 0x80 ? 1 : 0;
    f.z = r == 0 ? 1 : 0;
    f.h = (a & n & 0x10) ? 1 : 0;
    int16_t r16 = to_int16(a)-to_int16(n);
    f.pv = (r16< -128 || r > 127) ? 1 : 0;
    f.n = 0;
    f.c = (to_uint16(a)+to_uint16(n) > 255) ? 1 : 0;	// Simpler ?
    f.u5 = r & 0x20 ? 1 : 0;
    f.u3 = r & 0x08 ? 1 : 0;
    burn(burnt);
    return r;
}

reg8 Z80::adc_(reg8 n, cycle burnt)
{
    reg8 r = a+n+ (f.c ? 1: 0);
    f.s = r & 0x80 ? 1 : 0;
    f.z = r == 0 ? 1 : 0;
    f.h = (a & n & 0x10) ? 1 : 0;
    int16_t r16 = to_int16(a)-to_int16(n);
    f.pv = (r16< -128 || r > 127) ? 1 : 0;
    f.n = 0;
    f.c = (to_uint16(a)+to_uint16(n) > 255) ? 1 : 0;	// Simpler ?
    f.u5 = r & 0x20 ? 1 : 0;
    f.u3 = r & 0x08 ? 1 : 0;
    burn(burnt);
    return r;
}

reg8 Z80::compare(reg8 n)
{
    reg8 r = a-n;
    f.s = r & 0x80 ? 1 : 0;
    f.z = r == 0 ? 1 : 0;
    f.h = (a ^ n ^ r) & 16 ? 1 : 0;
    // f.pv = (a^n) & 0x80 ? (r&0x80)==(n&0x80) : 0;
    int16_t r16 = to_int16(a)-to_int16(n);
    f.pv = (r16< -128 || r > 127) ? 1 : 0;
    f.n = 1;
    f.c = a <  n ? 1 : 0;
    f.u5 = n & 0x20 ? 1 : 0;	// TODO set for SUB/SBC but not for CP
    f.u3 = n & 0x08 ? 1 : 0;
    return r;
}

void Z80::sbc(uint8_t val, cycle burnt)
{
    a=compare(val + (f.c ? 1 : 0));
    burn(burnt);
}

void Z80::rlca()
{
    a = (a<<1) | (a&80 ? 1:0);
    f.h=0;
    f.n=0;
    f.c= (a & 1) ? 1 : 0;
    burn(4);
}


}

