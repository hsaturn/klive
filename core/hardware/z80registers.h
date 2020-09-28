#pragma once

#include <core/hardware/cpu.h>

#include <QTableView>
#include <QCheckBox>

namespace hw
{
using reg8 = uint8_t;
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

class FlagCheckBox : public QCheckBox
{
public:
    FlagCheckBox(reg8& flags, uint8_t mask, QWidget* label);
    virtual ~FlagCheckBox()=default;

    void update()
    {
        QPalette pal = label->palette();

        bool old = isChecked();
        if (old != (flags & mask))
            pal.setColor(QPalette::WindowText, Qt::red);
        else
            pal.setColor(QPalette::WindowText, Qt::black);
        label->setPalette(pal);
        setChecked(flags & mask);
    }

private:
    reg8& flags;
    uint8_t mask;
    QWidget* label;
};


struct Z80Registers : public Cpu::Registers
{
    Z80Registers() :
          a(af.a),
          b(bc.hi),
          c(bc.lo),
          d(de.hi),
          e(de.lo),
          h(hl.hi),
          l(hl.lo) {};

    virtual ~Z80Registers() = default;

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

    // TODO Design : should really not merge registers with view of registers
    void update() override;
    QWidget* createViewForm(QWidget* parent) override;
    QTableView* table = nullptr;
    FlagCheckBox* flag_s;
    FlagCheckBox* flag_z;
    FlagCheckBox* flag_5;
    FlagCheckBox* flag_h;
    FlagCheckBox* flag_3;
    FlagCheckBox* flag_pv;
    FlagCheckBox* flag_n;
    FlagCheckBox* flag_c;
};

} // ns hw