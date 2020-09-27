#include "monsview.h"
#include <sstream>
#include <sstream>
#include <iostream>
#include <iomanip>

#include <QFile>
#include <QTextStream>
#include <QString>
#include <QStringList>

using namespace std;

MonsView::MonsView(QWidget *parent) :
    QTableView(parent),
    memory(nullptr)
{
    model=new QStandardItemModel();

    QStringList list = QString("ADDR,LABEL,MNEMO").simplified().split(',');
    model->setHorizontalHeaderLabels(list);

    setModel(model);
    setWindowTitle(QObject::tr("Frozen Column Example"));
    show();
}

MonsView::~MonsView()
{
    setMemory(nullptr);
}

void MonsView::observableDies(const Memory *sender)
{
    memory = nullptr;
}

void MonsView::setMemory(Memory *new_memory)
{
    if (memory) memory->detach(this);

    memory = new_memory;

    if (memory) memory->attach(this);
}

void MonsView::update(Memory* memory, const Memory::Message& msg)
{
}

void MonsView::setPointer(Memory::addr_t pc)
{
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
