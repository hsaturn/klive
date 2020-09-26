#include "monsview.h"
#include <QTableView>
#include <iostream>

using namespace std;

MonsView::MonsView(QWidget *parent) :
    QWidget(parent),
    memory(nullptr)
{
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
    cout << "MONS " << pc << endl;
}

// void SpectrumScreen::resizeEvent(QResizeEvent *event)
