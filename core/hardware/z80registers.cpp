#include "z80registers.h"
#include <string>
#include <sstream>
#include <iomanip>

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

FlagCheckBox::FlagCheckBox(reg8& flags, uint8_t mask)
    :
      flags(flags),
      mask(mask)
{

}

void addCells(QTableView* table, int col, std::string list)
{
    string::size_type pos;
    int row=2;

    auto model=dynamic_cast<QStandardItemModel*>(table->model());
    while((pos=list.find(','))!=string::npos)
    {
        string reg=list.substr(0,pos)+':';
        list.erase(0, pos+1);

        QStandardItem *newItem;
        newItem = new QStandardItem(reg.c_str());
        model->setItem(row++, col, newItem);
    }
}

QWidget* Z80Registers::createViewForm(QWidget* parent)
{
    QVBoxLayout* layout=new QVBoxLayout;
    QWidget* form=new QWidget;
    form->setLayout(layout);

    table=new QTableView;

    auto* model=new QStandardItemModel();
    QStringList list = QString("REG,VAL,REG,VAL").simplified().split(',');
    model->setHorizontalHeaderLabels(list);
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
        flag_s = new FlagCheckBox(af.f, 0x80);
        flag->addWidget(flag_s);
        flags->addLayout(flag);

        flag = new QVBoxLayout;
        label = new QLabel("Z");
        flag->addWidget(label);
        flag_z = new FlagCheckBox(af.f, 0x40);
        flag->addWidget(flag_z);
        flags->addLayout(flag);

        flag = new QVBoxLayout;
        label = new QLabel("5");
        flag->addWidget(label);
        flag_5 = new FlagCheckBox(af.f, 0x20);
        flag->addWidget(flag_5);
        flags->addLayout(flag);

        flag = new QVBoxLayout;
        label = new QLabel("H");
        flag->addWidget(label);
        flag_h = new FlagCheckBox(af.f, 0x10);
        flag->addWidget(flag_h);
        flags->addLayout(flag);

        flag = new QVBoxLayout;
        label = new QLabel("3");
        flag->addWidget(label);
        flag_3 = new FlagCheckBox(af.f, 0x8);
        flag->addWidget(flag_3);
        flags->addLayout(flag);

        flag = new QVBoxLayout;
        label = new QLabel("PV");
        flag->addWidget(label);
        flag_pv = new FlagCheckBox(af.f, 0x4);
        flag->addWidget(flag_pv);
        flags->addLayout(flag);

        flag = new QVBoxLayout;
        label = new QLabel("N");
        flag->addWidget(label);
        flag_n = new FlagCheckBox(af.f, 0x2);
        flag->addWidget(flag_n);
        flags->addLayout(flag);

        flag = new QVBoxLayout;
        label = new QLabel("C");
        flag->addWidget(label);
        flag_c = new FlagCheckBox(af.f, 0x2);
        flag->addWidget(flag_c);
        flags->addLayout(flag);

        layout->addLayout(flags);
    }


    // TODO flags, IM, cycles (?)
    return form;
}

ostream& operator<<(ostream& out, const regaf& af)
{
    out << af.val;
    return out;
}

ostream& operator<<(ostream& out, const reg16u& u)
{
    out << u.val;
    return out;
}

template<class T>
void setCell(QStandardItemModel* model, T& reg, int& row, int& col)
{
    stringstream r;
    r << hex << setw(4) << showbase << reg;

    QStandardItem *newItem;
    newItem = new QStandardItem(r.str().c_str());
    model->setItem(row++, col, newItem);
}

void setCells(QTableView* table, int col, reg16& top, regaf& af, reg16u& bc, reg16u& de, reg16u& hl, reg16& ii, reg8& bottom)
{
    auto model=dynamic_cast<QStandardItemModel*>(table->model());
    int row=2;
    setCell(model, top, row, col);
    setCell(model, af, row, col);
    setCell(model, bc, row, col);
    setCell(model, de, row, col);
    setCell(model, hl, row, col);
    setCell(model, bottom, row, col);
}

void Z80Registers::update()
{
    if (table == nullptr) return;

    setCells(table, 1, pc, af, bc, de, hl, ix, i);
    setCells(table, 3, sp, af2, bc2, de2, hl2, ix, r);

    flag_s->update();
    flag_z->update();
    flag_5->update();
    flag_h->update();
    flag_3->update();
    flag_n->update();
    flag_c->update();
}

} // ns
