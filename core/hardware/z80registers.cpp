#include "z80registers.h"
#include <string>
#include <sstream>
#include <iomanip>

#include <QTableView>
#include <QHeaderView>
#include <QFontDatabase>
#include <QStandardItemModel>
#include <QStandardItem>

using namespace std;

namespace hw
{

void addCells(QTableView* table, int col, std::string list)
{
    string::size_type pos;
    int row=2;

    auto model=dynamic_cast<QStandardItemModel*>(table->model());
    while((pos=list.find(','))!=string::npos)
    {
        string reg=list.substr(0,pos)+':';
        list.erase(pos+1);

        QStandardItem *newItem;
        newItem = new QStandardItem(reg.c_str());
        model->setItem(row++, col, newItem);
    }
}

QWidget* Z80Registers::createViewForm(QWidget* parent)
{
    QTableView* table=new QTableView;

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

    // TODO flags, IM, cycles (?)
    return table;
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

void Z80Registers::fillViewForm(QWidget *form)
{
    // TODO find table instead of casting
    QTableView* table=dynamic_cast<QTableView*>(form);

    setCells(table, 0, pc, af, bc, de, hl, ix, i);
    setCells(table, 2, sp, af2, bc2, de2, hl2, ix, r);
}

} // ns
