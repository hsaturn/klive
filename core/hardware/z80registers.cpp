#include "z80registers.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

#include <QTableView>
#include <QHeaderView>
#include <QFontDatabase>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>

using namespace std;

namespace hw
{

static int firstRow=0;

FlagCheckBox::FlagCheckBox(reg8& flags_, uint8_t mask_, QWidget* label_)
    :
      label(label_),
      flags(flags_),
      mask(mask_)
{

}

void addCells(QTableView* table, int col, std::string list)
{
    int row=firstRow;

    auto model=dynamic_cast<QStandardItemModel*>(table->model());
    while(list.length())
    {
        auto pos=list.find(',');
        if (pos==string::npos)
            pos=list.length();
        string reg=list.substr(0,pos)+':';
        list.erase(0, pos+1);

        QStandardItem *newItem;
        newItem = new QStandardItem(reg.c_str());
        model->setItem(row, col, newItem);
        row++;
    }
}

bool Z80Registers::set(string reg,int32_t value)
{
    std::for_each(reg.begin(), reg.end(), [](char&c){ c=tolower(c); });

    if (reg=="af") af.val=static_cast<uint16_t>(value);
    else if (reg=="sp") sp=static_cast<uint16_t>(value);
    else if (reg=="pc") pc=static_cast<uint16_t>(value);
    else if (reg=="bc") bc.val=static_cast<uint16_t>(value);
    else if (reg=="de") de.val=static_cast<uint16_t>(value);
    else if (reg=="hl") hl.val=static_cast<uint16_t>(value);
    else if (reg=="af'") af2.val=static_cast<uint16_t>(value);
    else if (reg=="bc'") bc2.val=static_cast<uint16_t>(value);
    else if (reg=="de'") de2.val=static_cast<uint16_t>(value);
    else if (reg=="hl'") hl2.val=static_cast<uint16_t>(value);
    else if (reg=="ix") ix=static_cast<uint16_t>(value);
    else if (reg=="iy") iy=static_cast<uint16_t>(value);
    else if (reg=="a") a=static_cast<reg8>(value);
    else if (reg=="f") af.f=static_cast<reg8>(value);
    else if (reg=="b") b=static_cast<reg8>(value);
    else if (reg=="c") c=static_cast<reg8>(value);
    else if (reg=="d") d=static_cast<reg8>(value);
    else if (reg=="e") e=static_cast<reg8>(value);
    else if (reg=="h") h=static_cast<reg8>(value);
    else if (reg=="l") l=static_cast<reg8>(value);
    else if (reg=="i") i=static_cast<reg8>(value);
    else if (reg=="r") r=static_cast<reg8>(value);
    else return false;

    update();
    return true;
}

std::string Z80Registers::serialize() const
{
    std::stringstream out;

    out << hex << showbase << '{';
    out << " sp:" << sp;
    out << " ix:" << ix;
    out << " iy:" << iy;
    out << " af:" << af.val;
    out << " bc:" << bc.val;
    out << " de:" << de.val;
    out << " hl:" << hl.val;
    out << " af':" << af2.val;
    out << " bc':" << bc2.val;
    out << " de':" << de2.val;
    out << " hl':" << hl2.val;
    out << " i:" << (uint16_t)i;
    out << " r:" << (uint16_t)i;

    out << '}';

    return out.str();
}

uint16_t Z80Registers::get(string regi)
{
    string reg(regi);	// TODO ...
    std::for_each(reg.begin(), reg.end(), [](char& chr){ chr=tolower(chr); });

    if (reg=="af") return af.val;
    else if (reg=="sp") return sp;
    else if (reg=="pc") return pc;
    else if (reg=="bc") return bc.val;
    else if (reg=="de") return de.val;
    else if (reg=="hl") return hl.val;
    else if (reg=="af'") return af2.val;
    else if (reg=="bc'") return bc2.val;
    else if (reg=="de'") return de2.val;
    else if (reg=="hl'") return hl2.val;
    else if (reg=="ix") return ix;
    else if (reg=="iy") return iy;
    else if (reg=="a") return a;
    else if (reg=="f") return af.f;
    else if (reg=="b") return b;
    else if (reg=="c") return c;
    else if (reg=="d") return d;
    else if (reg=="e") return e;
    else if (reg=="h") return h;
    else if (reg=="l") return l;
    else if (reg=="i") return i;
    else if (reg=="r") return r;
    else if (reg=="carry") return af.c;
    else if (reg=="zero") return af.z;
    else if (reg=="pv") return af.pv;
    else if (reg=="sign") return af.s;
    else throw "Unknown register";
}

QWidget* Z80Registers::createViewForm(QWidget* parent)
{
    QVBoxLayout* layout=new QVBoxLayout;
    QWidget* form=new QWidget;
    form->setLayout(layout);

    table=new QTableView;

    auto* model=new QStandardItemModel();
    table->horizontalHeader()->hide();
    table->verticalHeader()->hide();
    table->setModel(model);

    // TODO part of this should be user configuration
    QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    // fixedFont.setPointSize(9);
    table->setShowGrid(false);
    table->setFont(fixedFont);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    table->verticalHeader()->setDefaultSectionSize(12);
    // table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    addCells(table, 0, "PC,AF,BC,DE,HL,IX,I");
    addCells(table, 2, "SP,AF',BC',DE',HL',IY,R");

    layout->addWidget(table);

    { // flags
        QHBoxLayout* flags = new QHBoxLayout;

        QLabel* label;
        label=new QLabel;
        label->setText("Flags:");
        flags->addWidget(label);

        //  ----

        QVBoxLayout* flag = new QVBoxLayout;
        label=new QLabel;
        label->setText("S");
        flag->addWidget(label);
        flag_s = new FlagCheckBox(af.f, 0x80, label);
        flag->addWidget(flag_s);
        flags->addLayout(flag);

        flag = new QVBoxLayout;
        label = new QLabel("Z");
        flag->addWidget(label);
        flag_z = new FlagCheckBox(af.f, 0x40, label);
        flag->addWidget(flag_z);
        flags->addLayout(flag);

        flag = new QVBoxLayout;
        label = new QLabel("5");
        flag->addWidget(label);
        flag_5 = new FlagCheckBox(af.f, 0x20, label);
        flag->addWidget(flag_5);
        flags->addLayout(flag);

        flag = new QVBoxLayout;
        label = new QLabel("H");
        flag->addWidget(label);
        flag_h = new FlagCheckBox(af.f, 0x10, label);
        flag->addWidget(flag_h);
        flags->addLayout(flag);

        flag = new QVBoxLayout;
        label = new QLabel("3");
        flag->addWidget(label);
        flag_3 = new FlagCheckBox(af.f, 0x8, label);
        flag->addWidget(flag_3);
        flags->addLayout(flag);

        flag = new QVBoxLayout;
        label = new QLabel("PV");
        flag->addWidget(label);
        flag_pv = new FlagCheckBox(af.f, 0x4, label);
        flag->addWidget(flag_pv);
        flags->addLayout(flag);

        flag = new QVBoxLayout;
        label = new QLabel("N");
        flag->addWidget(label);
        flag_n = new FlagCheckBox(af.f, 0x2, label);
        flag->addWidget(flag_n);
        flags->addLayout(flag);

        flag = new QVBoxLayout;
        label = new QLabel("C");
        flag->addWidget(label);
        flag_c = new FlagCheckBox(af.f, 0x1, label);
        flag->addWidget(flag_c);
        flags->addLayout(flag);

        layout->addLayout(flags);
    }


    // TODO flags, IM, cycles (?)
    return form;
}

static ostream& operator<<(ostream& out, const regaf& af)
{
    out << af.val;
    return out;
}

static ostream& operator<<(ostream& out, const reg16u& u)
{
    out << u.val;
    return out;
}

static ostream& operator<<(ostream& out, const reg8& u)
{
    out << (int16_t)u;
    return out;
}

template<class T>
static void setCell(QStandardItemModel* model, T& reg, int& row, int& col)
{
static    int counter=0;
    stringstream r;
    r << hex << setw(4) << std::setfill('0') << showbase << reg;
    QString str=QString::fromStdString(r.str());

    QStandardItem *newItem;
    newItem = new QStandardItem(str);
    if (model->index(row, col).data().toString()!=str)
    {
        newItem->setData(QColor(Qt::red), Qt::ForegroundRole);
        counter++;
        // old value  : model->index(row, col).data().toString()
    }

    model->setItem(row, col, newItem);
    row++;
}

static void setCells(QTableView* table, int col, reg16& top, regaf& af, reg16u& bc, reg16u& de, reg16u& hl, reg16& ii, reg8& bottom)
{
    auto model=dynamic_cast<QStandardItemModel*>(table->model());
    int row=firstRow;
    setCell(model, top, row, col);
    setCell(model, af, row, col);
    setCell(model, bc, row, col);
    setCell(model, de, row, col);
    setCell(model, hl, row, col);
    setCell(model, ii, row, col);
    setCell(model, bottom, row, col);
}

void Z80Registers::update()
{
    if (table == nullptr) return;

    setCells(table, 1, pc, af,  bc,  de,  hl,  ix, i);
    setCells(table, 3, sp, af2, bc2, de2, hl2, iy, r);

    flag_s->update();
    flag_z->update();
    flag_5->update();
    flag_h->update();
    flag_3->update();
    flag_n->update();
    flag_c->update();
    flag_pv->update();
}

} // ns
