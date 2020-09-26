#include "monsview.h"
#include <iostream>

#include <QStandardItemModel>
#include <QFile>
#include <QTextStream>

using namespace std;

MonsView::MonsView(QWidget *parent) :
    QTableView(parent),
    memory(nullptr)
{

    QStandardItemModel *model=new QStandardItemModel();

    // model->setHorizontalHeaderLabels(list);
    // model->setItem(row, col, newItem);

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
    if (memory==nullptr) return;



}

// void SpectrumScreen::resizeEvent(QResizeEvent *event)
