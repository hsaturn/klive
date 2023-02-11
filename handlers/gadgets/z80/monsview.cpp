#include "monsview.h"
#include <sstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>

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
    if (memory == new_memory) return;
    if (memory) memory->detach(this);
    memory = new_memory;
    if (not memory) return;

    decoded.clear();
    decoded = vector<int>(memory->size()+10);

    memory->attach(this);

    model=new QStandardItemModel();
    QStringList list = QString("ADDR,LABEL,MNEMO").simplified().split(',');
    model->setHorizontalHeaderLabels(list);
    setModel(model);

    setPointer(curr_pc);
    return;
}

void MonsView::update(Memory* , const Memory::Message& msg)
{
    return;

    int addr = static_cast<int>(msg.start);
    for(int offset=0; offset<msg.size; offset++)
    {
        decoded[addr+offset] = 0;
    }
}

void MonsView::setPointer(Memory::addr_t pc)
{
    static int j=0;
    if (j++<100) return;
    j = 0;

    if (memory==nullptr) return;
    if (!isVisible()) return;
    curr_pc = pc;

    QStandardItem* newItem;

    // Decode up to 30 mnemonics
    std::vector<int> to_hide;
    for(int i=0; i<30; i++)
    {
        if (decoded[pc])
        {
            pc+=decoded[pc];
            int index = static_cast<int>(pc);
            continue;
        }

        int index = static_cast<int>(pc);
        if (pc > decoded.size())
            std::cout << "error " << pc << std::endl;
        showRow(index);
        Mons::Row row = mons.decode(memory, pc);
        decoded[index] = static_cast<int>(pc);
        stringstream h;
        h << hex << showbase << uppercase << setw(4) << pc;

        newItem = new QStandardItem(h.str().c_str());
        model->setItem(index, 0, newItem);
        newItem = new QStandardItem(row.label.c_str());
        model->setItem(index, 1, newItem);
        newItem = new QStandardItem(row.mnemo.c_str());
        model->setItem(index, 2, newItem);
        // cout << hex << setw(4) << pc << (dec) << mons.decode(memory, pc) << endl;

        while(++index < pc)
        {
            decoded[index] = 0;
            to_hide.push_back(index);
        }
    }
    for(int hide: to_hide)
        hideRow(hide);

    resizeColumnsToContents();
    setCurrentIndex(model->index(curr_pc, 0));
}
