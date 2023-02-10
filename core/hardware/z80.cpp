#include "z80.h"
#include <iostream>
#include <iomanip>
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
      i(R.i), r(R.r), ixl(R.ixl), ixh(R.ixh)
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
    af.val=0;
    bc.val=0;
    de.val=0;
    hl.val=0;
    af2.val=0;
    bc2.val=0;
    de2.val=0;
    hl2.val=0;
    sp=0;
    ix.val=0;
    iy.val=0;

    clock.setFrequency(3750000);
    clock.restart();
    clock.irq(70800);

    // TODO should be called by rst(0x0)
    Message rst(Message::RESET);
    rst.data = 0x0;
    notify(rst);

    break_on_ret=0;
}

void Z80::nop(byte opcode, const char* prefix)
{
    Message unknown(Message::UNKNOWN_OP);
    const char c=0;
    if (prefix==nullptr) prefix=&c;
    printf("Z80 %04x: %s %02x Unknown opcode\n", pc-1, prefix, opcode);
    burn(4);	// Avoid cycle error
    running=false;
    unknown.data = opcode | (static_cast<uint32_t>(last) << 16);
    notify(unknown);
    cout << std::flush;
}

void Z80::halt()
{
    static Message hlt(Message::HALTED);
    burn(4);	// TODO, what is the real value ?
    notify(hlt);
    running=false;
}

void Z80::set_irq_mode(int mode)
{
    if (irq_mode != mode)
    {
        irq_mode = mode;
        std::cout << "IRQ MODE " << mode << std::endl;
    }
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
    if (break_on_ret)
    {
        // running=true;	// TODO sure ?
        // break_on_ret++;

// NOTE: we've lost one or more irq (except in debug mode) TODO
    }
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

void Z80::step_over()
{
    uint8_t opcode=memory->peek(pc);
    uint8_t opcode2=memory->peek(pc+1);
    uint8_t hi = (opcode & 0xf0) >> 4;
    uint8_t lo = opcode & 0x0f;
    bool gosub = false;

    if ((hi==0xc || hi==0xd || hi==0xe || hi==0xf) and (lo==0x7 || lo==0x0f)) gosub=true;
    if (opcode==0xcd) gosub=true;

    if (gosub)
    {
        running = true;
        break_on_ret++;
    }
    else  if (opcode==0xed and (opcode2==0xb0 or opcode2==0xb8))	// lddr ldir
    {
        auto current_pc = pc;
        do
        {
            step_no_obs();
        } while (pc == current_pc);
        notify(stepMsg);
    }
    else
        run_steps(1);
}

void Z80::step_out()
{
    break_on_ret++;
    running=true;
}

void Z80::step_no_obs()
{
    last = pc;
    CpuClock before(clock);	// TODO used to check if the clock advances, could be removed later for speed
    inc_r();

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
    byte opcode2 = memory->peek(pc); // TODO for debugging purpose

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
        case 0x17: rla(); break;
        case 0x18: jr(true); break;
        case 0x19: add_hl(de.val, 11); break;
        case 0x1A: a=memory->peek(de.val); burn(7); break;	// ld a, (de)
        case 0x1B: de.val--; burn(6); break;			// dec de
        case 0x1C: e=inc(e, 4); break;
        case 0x1D: e=dec(e, 4); break;
        case 0x1E: e=memory->peek(getByte(7)); break;	// le e,*
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
        case 0x2E: l=getByte(7); break;
        case 0x2F: a=~a; f.h=1; f.n=1; burn(4); break;
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
        case 0x3c: a=inc(a, 4); break;
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
        case 0x52: burn(4); break;	// ld d,d
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
        case 0x76: halt(); break;
        case 0x77: memory->poke(hl.val, a); burn(7); break; // ld (hl), a)
        case 0x78: a = b; burn(4); break;
        case 0x79: a = c; burn(4); break;
        case 0x7a: a = d; burn(4); break;		// ld a,d
        case 0x7b: a = e; burn(4); break;		// ld a,e
        case 0x7c: a = h; burn(4); break;		// ld a,h
        case 0x7d: a = l; burn(4); break;		// ld a,l
        case 0x7e: a = memory->peek(hl.val); burn(7); break; // ld a, (hl)
        case 0x7f: burn(4); break;							// ld a,a
        case 0x80: a=add_(a,b,4); break;	// add a,b
        case 0x81: a=add_(a,c,4); break;	// add a,c
        case 0x82: a=add_(a,d,4); break;	// add a,d
        case 0x83: a=add_(a,e,4); break;	// add a,e
        case 0x84: a=add_(a,h,4); break;	// add a,h
        case 0x85: a=add_(a,l,4); break;	// add a,l
        case 0x86: a=add_(a,memory->peek(hl.val),7); break;	// add a,(hl)
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
        case 0xBD: compare(l); burn(4); break;  // cp l
        case 0xBE: compare(memory->peek(hl.val)); burn(7); break;	// cp (hl)
        case 0xC0: ret(not f.z, 11,5); break;
        case 0xc1: pop(bc.val, 11); break;
        case 0xC2: jp_if(not f.z); break;
        case 0xC3: pc = getWord(10); break; // Jump
        case 0xC4: call_if(not f.z); break;
        case 0xc5: push(bc.val, 11); break; 	// push bc
        case 0xc6: a=add_(a, getByte(), 7); break;	// add a, *
        case 0xc8: ret(f.z, 11, 5); break; // ret z
        case 0xC7: rst(0x00); break;
        case 0xC9: ret(true, 10); break;
        case 0xCA: jp_if(f.z); break;
        case 0xCB: step_cb(); break;
        case 0xCC: call_if(f.z); break;
        case 0xCD: call(); break;
        // case 0xCE: adc_(getByte(), 7); break; // adc a,*
        case 0xCF: rst(0x08); break;
        case 0xD0: ret(not f.c, 11,5); break;
        case 0xD1: pop(de.val, 11); break;
        case 0xD3: out((a<<8) + getByte(), a, 11); break;
        case 0xD4: call_if(not f.c); break;
        case 0xD9: swap(hl,hl2); swap(de,de2); swap(bc,bc2); burn(4); break;
        case 0xD2: jp_if(not f.c); break;
        case 0xD5: push(de.val, 11); break;		// push de
        case 0xD6: a=compare(getByte(7)); break;		// sub *
        case 0xD7: rst(0x10); break;
        case 0xD8: ret(f.c, 11, 5); break;
        case 0xDA: jp_if(f.c); break;
        case 0xDB: a = in((static_cast<uint16_t>(a)<<8) + getByte(11),0); break;
        case 0xDC: call_if(f.c); break;
        case 0xDD: step_dd_fd(ix.val); break;
        case 0xDF: rst(0x18); break;
        case 0xE1: pop(hl.val, 11); break;
        case 0xE2: jp_if(!f.pv); break;				// jp po
        case 0xE3: ex_word_at_sp(hl.val); break;	// ex (sp); hl
        case 0xE5: push(hl.val, 11); break;			// push hl
        case 0xE6: and_(getByte(), 7); break;		// and n
        case 0xE7: rst(0x20); break;
        case 0xE9: pc=hl.val; burn(4); break;		// jp (hl)
        case 0xEA: jp_if(f.pv); break;				// jp pe
        case 0xEB: swap(de, hl); burn(4); break;	// ex de,hl
        case 0xED: step_ed(); break;
        case 0xEE: xor_(getByte(), 7); break;
        case 0xEF: rst(0x28); break;
        case 0xF1: pop(af.val, 11); break;
        case 0xF2: jp_if(!f.s); break;				// jp p
        case 0xF3: di(); burn(4); break;
        case 0xF5: push(af.val, 11); break;			// push hl
        case 0xF6: or_(getByte(), 7); break;
        case 0xF7: rst(0x30); break;
        case 0xF9: sp=hl.val; burn(6); break; // ld sp,hl
        case 0xFA: jp_if(f.s); break;
        case 0xFB: ei(); burn(4); break;

        case 0xFD: step_dd_fd(iy.val); break;
        case 0xFE: compare(getByte(7)); break;	// cp *
        case 0xFF: rst(0x38); break;
        default: nop(opcode); break;
    }
    if (clock == before)
    {
        std::cerr << std::hex << "WARNING: CPU CLOCK 0 cycle for " << (int)opcode << "." << (int)opcode2 << std::dec << endl;
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
    if (flag)
        call();
    else
        burn(10);
}

void Z80::call()
{
    if (break_on_ret) break_on_ret++;
    reg16 next_pc = getWord();
    push(pc, 17);
    pc = next_pc;
}

void Z80::rst(uint8_t addr)
{
    if (break_on_ret) break_on_ret++;
    push(pc, 11);
    pc = addr;

    Message rst(Message::RESET);
    rst.data = addr;
    notify(rst);
}

void Z80::ret(bool flag, cycle burn_ret, cycle burn_noret)
{
    if (flag)
    {
        pc = memory->peek(sp)+(memory->peek(sp+1)<<8);
        sp += 2;
        burn(burn_ret);
        if (break_on_ret)
        {
            break_on_ret--;
            if (break_on_ret == 1)
            {
                break_on_ret = 0;
                if (running) notify(stepMsg);
                running=false;
            }
        }
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
    f.h = ((val&0x0f)-1) & 0x10 ? 1 : 0;
    val--;
    f.s = val & 0x80 ? 1 : 0;
    f.z = val == 0 ? 1 : 0;
    f.n = 1;
    f.u3 = val & 0x08;
    f.u5 = val & 0x20;
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

void Z80::sbc_hl(uint16_t val)
{
    uint16_t hl0 = hl.val;
    burn(15);
    uint8_t carry = f.c;
    hl.val -= val+carry;

    f.s = h & 0x80 ? 1 : 0;
    f.u3 = h & 0x08 ? 1 : 0;
    f.u5 = h & 0x20 ? 1 : 0;

    f.z = hl.val == 0 ? 1 : 0;
    f.h = (((hl0 & 0x0fff) - (val & 0x0fff) - f.c) & 0x1000) != 0;
    f.c = ((hl0-val) & 0x10000) != 0 ? 1 : 0;
    f.pv = ((hl0 ^ val) & (hl0 ^ hl.val) & 0x8000) != 0 ? 1 : 0;
    f.n = 1;
}

void Z80::step_ed()
{
    inc_r();
    uint8_t opcode=getByte();
    switch(opcode)
    {
        case 0x40: b=in(bc.val, 12); break;
        case 0x42: sbc_hl(bc.val); break;
        case 0x43: writeMem16(bc.val, 20); break;
        case 0x44: neg(); break;
        case 0x45: retn(); break;
        case 0x46: set_irq_mode(0); burn(8); break;
        case 0x47: i=a; burn(9); break; // ld i,a
        case 0x48: c=in(bc.val, 12); break;
        case 0x4B: // ld bc, (**)
            {
                Memory::addr_t addr=getWord(20);
                c=memory->peek(addr);
                b=memory->peek(addr+1);
                break;
            }
        case 0x4C: neg(); break;
        case 0x4D:
            reti(); break;
        case 0x4E: set_irq_mode(0); burn(8); break;
        case 0x50: d=in(bc.val, 12); break;
        case 0x52: sbc_hl(de.val); break;	// sbc hl, de
        case 0x53: writeMem16(de.val, 20); break;	// ld(**), de
        case 0x54: neg(); break;
        case 0x55: retn(); break;
        case 0x56: set_irq_mode(1); burn(8); break;
        case 0x57: // ld a,i
            a = i;
            f.n = 0;
            f.h = 0;
            f.z = a==0 ? 1 : 0;
            f.s = a&0x80 ? 1 : 0;
            f.pv = iff2;
            break;
        case 0x58: e=in(bc.val, 12); break;
        case 0x5B:	// ld de, (**)
            {
                Memory::addr_t addr=getWord(20);
                e=memory->peek(addr);
                d=memory->peek(addr+1);
                break;
            }
        case 0x5C: neg(); break;
        case 0x5D: retn(); break;
        case 0x5E: set_irq_mode(2); burn(8); break;
        case 0x5F:	// ld a,r
            a = r;
            f.n = 0;
            f.h = 0;
            f.z = a==0 ? 1 : 0;
            f.s = a&0x80 ? 1 : 0;
            f.pv = iff2;
            break;
        case 0x60: h=in(bc.val, 12); break;
        case 0x62: sbc_hl(hl.val); break;
        case 0x64: neg(); break;
        case 0x65: retn(); break;
        case 0x66: set_irq_mode(0); burn(8); break;
        case 0x68: l=in(bc.val, 12); break;
        case 0x6C: neg(); break;
        case 0x6D: retn(); break;
        case 0x6E: set_irq_mode(0); burn(8); break;
        case 0x72: sbc_hl(sp); break;
        case 0x73:	// ld (**), sp
            {
                Memory::addr_t addr=getWord(20);
                memory->poke(addr, sp & 0xff);
                memory->poke(addr+1, sp >> 8);
                break;
            }
        case 0x74: neg(); break;
        case 0x75: retn(); break;
        case 0x76: set_irq_mode(1); burn(8); break;
        case 0x78: a = in(bc.val,12); break; // in a,(c)
        case 0x79: out(bc.val, a, 11); break;
        case 0x7B:
            {
                Memory::addr_t addr=getWord(20);
                sp = memory->peek(addr)+(memory->peek(addr+1)<<8);
                break;
            }
        case 0x7C: neg(); break;
        case 0x7D: retn(); break;
        case 0x7E: set_irq_mode(2); burn(8); break;
        case 0xA0: ldi(); break;
        case 0xA8: ldd(); break;
        case 0xB0: // ldir
            ldi();
            if (bc.val) pc -=2;
            break;
        case 0xb8: // lddr
            ldd();
            if (bc.val) pc -=2;
            break;
        default:
            nop(opcode, "0xed");
            break;
    }
}

void Z80::ldd()
{
    r-=2;

    burn(bc.val ? 21 : 16);
    inc_r();
    inc_r();
    memory->poke(de.val, memory->peek(hl.val));
    hl.val--;
    de.val--;
    bc.val--;
    if (bc.val==0)
    {
        f.h=0; f.pv=0; f.n=0;
    }
}

void Z80::ldi()
{
    r-=2;

    burn(bc.val ? 21 : 16);
    inc_r();
    inc_r();
    memory->poke(de.val, memory->peek(hl.val));
    hl.val++;
    de.val++;
    bc.val--;
    if (bc.val==0)
    {
        f.h=0; f.pv= (bc.val==1); f.n=0;
    }
}

void Z80::step_dd_fd(reg16& in)
{
    inc_r();
    uint8_t opcode=getByte();
    switch(opcode)
    {
        case 0x09: add_in_pp(in, bc.val); break;
        case 0x19: add_in_pp(in, de.val); break;
        case 0x21: in=getWord(14); break;	// ld i?, nn
        case 0x22:
            {
                Memory::addr_t addr = getWord(20);
                memory->poke(addr,   in &  0xff);
                memory->poke(addr+1, in >> 8);
                break;
            }
        case 0x23: in++; burn(10); break;
        case 0x24: // inc inh
            {
                int8_t v=inc(in >> 8, 10);
                in = (v << 8) | (in & 0xFF);
                break;
            }
        case 0x29: add_in_pp(in, in); break;
        case 0x2A: in=readMem16(20); break; // ld in, (**)
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
        case 0x39: add_in_pp(in, sp); break;
        case 0x46:	// ld b, (ii+*)
            {
                Memory::addr_t addr=in+static_cast<int8_t>(getByte(19));
                b = memory->peek(addr);
            }
            break;
        case 0x4E:
            {
                Memory::addr_t addr=in+static_cast<int8_t>(getByte(19));
                c = memory->peek(addr);
            }
            break;
        case 0x54: d=in>>8; burn(8); break;
        case 0x55: d=in & 0xff; burn(8); break;
        case 0x56:
            {
                Memory::addr_t addr=in+static_cast<int8_t>(getByte(19));
                d = memory->peek(addr);
            }
            break;
        case 0x5C: e=in>>8; burn(8); break;
        case 0x5D: e=in & 0xff; burn(8); break;
        case 0x5E:
            {
                Memory::addr_t addr=in+static_cast<int8_t>(getByte(19));
                e = memory->peek(addr);
            }
            break;

        case 0x60:	// ld i?h, reg
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x67:
            {
                burn(8);
                uint8_t* reg = calc_dest_reg(opcode & 0x7);
                if (reg) in = (in & 0x00FF) | (*reg << 8);
            }
            break;
        case 0x64: burn(8); break;  // ld inh, inh
        case 0x65: in = ((in & 0x00FF) << 8) | (in & 0x00FF); burn(8); break; // ld inh, inl
        case 0x66:
            {
                burn(19);
                Memory::addr_t addr = in + static_cast<int8_t>(getByte());
                h=memory->peek(addr);
                burn(3);
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
                uint8_t* reg = calc_dest_reg(opcode & 0x7);
                if (reg==nullptr) { cerr << "z80: step_dd_fd Should never occur, unable to compute reg" << endl;  return; }
                int8_t offset = static_cast<int8_t>(getByte());
                memory->poke(in+offset, *reg);
            }
            break;
        case 0x7C: a=in>>8; burn(8); break;
        case 0x7D: a=in & 0xff; burn(8); break;

        case 0x7E:
            {
                Memory::addr_t addr=in+static_cast<int8_t>(getByte(19));
                a = memory->peek(addr);
            }
            break;
        case 0x86:	// add a, (ii+*)
            {
                burn(19);
                int8_t offset = static_cast<int8_t>(getByte());
                a=add_(a, memory->peek(in+offset), 19);
            }
            break;
        case 0x96:
            {
                Memory::addr_t addr=in+static_cast<int8_t>(getByte(19));
                a=compare(memory->peek(addr)); break;		// sub a, (in+*)
            }
        case 0xCB: step_xxcb(in); break;
        case 0xE1: pop(in, 14); break;
        case 0xE5: push(in, 15); break;
        case 0xE9: pc=in; burn(8); break; // jp (in)

        default: nop(opcode, "0xdd or 0xfd "); break;
    }
}

inline uint8_t parity(uint8_t r)
{
    return (__builtin_popcount(r) & 1) ^ 1;
}

uint8_t* Z80::calc_dest_reg(uint8_t reg)
{
    switch(reg) // was opcode & 0x07)
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
    inc_r();
    bool ismem=false;
    uint8_t mem;
    uint8_t opcode=getByte();

    uint8_t* reg = calc_dest_reg(opcode & 0x07);
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
        dest_reg = calc_dest_reg(opcode & 0x07);
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
uint8_t Z80::rl(uint8_t v)
{
   bool new_c = v & 0x80;
    v = (f.c ? 0x01 : 0x00) | (v<<1);
    f.s = (v & 0x80 ? 1 : 0);
    f.z = (v==0) ? 1 : 0;
    f.h = 0;
    f.pv = parity(v);
    f.n = 0;
    f.c= new_c;
    return v;
}

uint8_t Z80::rr(uint8_t v)
{
    bool new_c = v&1;
    v = (f.c ? 0x80 : 0x00) | (v>>1);
    f.s = (v & 0x80 ? 1 : 0);
    f.z = (v==0) ? 1 : 0;
    f.h = 0;
    f.pv = parity(v);
    f.n = 0;
    f.c= new_c;
    return v;
}

uint8_t Z80::sla(uint8_t v)
{
    f.c = v&0x80 ? 1:0;
    v <<= 1;
    f.s = (v & 0x80 ? 1 : 0);
    f.z = (v==0) ? 1 : 0;
    f.h = 0;
    f.pv = parity(v);
    f.n = 0;
    return v;
}
uint8_t Z80::sra(uint8_t) { nop(0, "sra");  return 0; }
uint8_t Z80::sll(uint8_t) { nop(0, "sll");  return 0; }

uint8_t Z80::srl(uint8_t reg)
{
    f.s = 0;
    f.c = reg & 0x01 ? 1 : 0;
    reg >>= 1;
    f.z = reg == 0 ? 1 : 0;
    f.h = 0;
    f.pv = parity(reg);
    f.n = 0;

    return reg;
}

void Z80::out(Memory::addr_t port, uint8_t val, cycle burnt)
{
    //!\ : See Z80 specs.... It seems that upper byte of port is set but unused...
    //!
    static Message outport(Message::OUTPORT);
    if (outport.port == nullptr)
    {
        outport.port = new CpuPort;
    }
    outport.port->port = port;

    // TODO using notification for in/out is a poor design ?
    notify(outport);
    clock.burn(burnt);
}

uint8_t Z80::in(Memory::addr_t port, cycle burnt)
{
    static int recurse=0;

    int l=0;

    if (l++>1000000)
    {
        l=0;
        std::cout << "IN " << recurse++ << std::endl;
    }
    recurse++;

    static Message inport(Message::INPORT);
    if (inport.port == nullptr)
    {
        inport.port = new CpuPort;
    }
    inport.port->port = port;
    inport.port->filled = false;

    // TODO using notification for in/out is a poor design
    // performances problems and freezes...
    notify(inport);
    clock.burn(burnt);
    if (!inport.port->filled)
    {
        std::cerr << "in(" << std::hex << port << std::dec << ") not filled." << endl;
    }
    // TODO inport.port->filled ?
    recurse--;
    return inport.port->value;
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

void Z80::neg()
{
    burn(8);
    a=compare(0,a);
}

reg8 Z80::compare(reg8 n)
{
    return compare(a,n);
}

reg8 Z80::compare(reg8 n1, reg8 n2)
{
    reg8 r = n1-n2;
    f.s = r & 0x80 ? 1 : 0;
    f.z = r == 0 ? 1 : 0;
    f.h = (n1 ^ n2 ^ r) & 16 ? 1 : 0;
    // f.pv = (n1^n2) & 0x80 ? (r&0x80)==(n2&0x80) : 0;
    int16_t r16 = to_int16(n1)-to_int16(n2);
    f.pv = (r16< -128 || r > 127) ? 1 : 0;
    f.n = 1;
    f.c = n1 <  n2 ? 1 : 0;
    f.u5 = n2 & 0x20 ? 1 : 0;	// TODO set for SUB/SBC but not for CP
    f.u3 = n2 & 0x08 ? 1 : 0;
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

void Z80::rla()
{
    bool new_c = a & 0x80;
    a = (a<<1) | (c ? 1 : 0);
    f.h=0;
    f.n=0;
    f.c = new_c;
    burn(4);
}

void Z80::add_in_pp(reg16& in, uint16_t value)
{
    f.c = (static_cast<uint32_t>(in)+static_cast<uint32_t>(value)) > 65535;
    f.h = ((in & 0xFFF)+(value & 0xFFF)) >= 0x1000;	// TODO correct ?
    in += value;
    f.n = 0;
    burn(15);
}


}

