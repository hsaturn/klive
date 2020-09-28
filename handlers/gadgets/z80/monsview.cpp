#include "monsview.h"
#include <sstream>
#include <sstream>
#include <iostream>
#include <iomanip>

#include <QFontDatabase>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QStringList>
#include <QHeaderView>
#include <QFontDialog>

using namespace std;

MonsView::MonsView(QWidget *parent) :
    QTableView(parent),
    memory(nullptr)
{
    bool ok;

    // TODO part of this should be user configuration
    QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    // fixedFont = QFontDialog::getFont(&ok, fixedFont, this);
    fixedFont.setPointSize(9);
    setShowGrid(false);
    setFont(fixedFont);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setDefaultSectionSize(12);
    horizontalHeader()->setStretchLastSection(true);
    // horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    show();
    setSelectionBehavior(QAbstractItemView::SelectRows);
    verticalHeader()->hide();
}

MonsView::~MonsView()
{
    setMemory(nullptr);
}

void MonsView::observableDies(const Memory *)
{
    memory = nullptr;
}

void MonsView::setMemory(Memory *new_memory)
{
    if (memory) memory->detach(this);
    memory = new_memory;
    if (memory==nullptr) return;

    memory->attach(this);

    addr_to_table_line.clear();

    model=new QStandardItemModel();
    QStringList list = QString("ADDR,LABEL,MNEMO").simplified().split(',');
    model->setHorizontalHeaderLabels(list);
    setModel(model);
    QStandardItem *newItem;

    Memory::addr_t pc=0;

    // Pass 1 : compute labels
// pc=0x55;	// TODO for debug
    while(pc<16384)
    {
        mons.decode(memory, pc);
    }
    pc=0;

    // Pass 2 : populate list
    long line=1;
    while(pc<16384)
    {
        stringstream h;
        h << hex << showbase << uppercase << setw(4) << pc;

        addr_to_table_line[pc] = line;
        Mons::Row row = mons.decode(memory, pc);

        newItem = new QStandardItem(h.str().c_str());
        model->setItem(line, 0, newItem);
        newItem = new QStandardItem(row.label.c_str());
        model->setItem(line, 1, newItem);
        newItem = new QStandardItem(row.mnemo.c_str());
        model->setItem(line, 2, newItem);
        // cout << hex << setw(4) << pc << (dec) << mons.decode(memory, pc) << endl;
        line++;
    }
    // Ne fonctionne pas (ou mal)
    // resizeColumnsToContents();
    resizeRowsToContents();
}

void MonsView::update(Memory* memory, const Memory::Message& msg)
{
}

void MonsView::setPointer(Memory::addr_t pc)
{
    const auto& line = addr_to_table_line.find(pc);
    if (line == addr_to_table_line.end())
        return;
    else
    {
        const QModelIndex& index=model->index(line->second, 0);
        setCurrentIndex(index);
       // scrollTo(index);
    }
    return;
    QStandardItem *newItem;
    if (memory==nullptr) return;

    for(int i=0; i<10; i++)
    {
        Mons::Row row = mons.decode(memory, pc);
        stringstream h;
        h << hex << showbase << uppercase << setw(4) << pc;

        newItem = new QStandardItem(h.str().c_str());
        model->setItem(i, 0, newItem);
        newItem = new QStandardItem(row.label.c_str());
        model->setItem(i, 1, newItem);
        newItem = new QStandardItem(row.mnemo.c_str());
        model->setItem(i, 2, newItem);
        // cout << hex << setw(4) << pc << (dec) << mons.decode(memory, pc) << endl;
    }
}
