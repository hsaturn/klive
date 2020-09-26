#include "monsview.h"
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

    QStringList list = QString("ADDR,MNEMO").simplified().split(',');
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
        stringstream h;
        h << hex << showbase << uppercase << setw(4) << pc;

        newItem = new QStandardItem(h.str().c_str());
        model->setItem(i, 0, newItem);
        newItem = new QStandardItem(mons.decode(memory, pc).c_str());
        model->setItem(i, 1, newItem);
        // cout << hex << setw(4) << pc << (dec) << mons.decode(memory, pc) << endl;
    }
}

// void SpectrumScreen::resizeEvent(QResizeEvent *event)
