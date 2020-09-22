#include "screen.h"
#include <stdlib.h>

#include <QPainter>
#include <QTime>
#include <QTimer>
#include <QPalette>

#include <iostream>

Screen::Screen(QWidget *parent)
    : QWidget(parent), bouncer(this)
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
    timer->start(10);  // Update interval ms

    // TODO should be setTitle, and setTitle should call all what is
    // needed to change the title everywhere
    // setWindowTitle(tr("ZX Screen"));

    memory = new Memory(64*1024);
    memory->attach(this);
    resize(200, 200);
}

Screen::~Screen()
{
    if (memory)
    {
        memory->detach(this);
        delete memory;
    }
}

void Screen::paintEvent(QPaintEvent *e)
{
    bouncer.paintEvent(e);

    static Memory::addr_t hl=0;
    static uint8_t byte;
    if (memory && byte++%10==0)
    {
        memory->poke(hl++, byte);
    }

    if (lines.size()<5)
    {
        lines.push_back(new LineBouncer(this));
    }
    for(auto line: lines)
    {
        line->paintEvent(e);
    }
}

void Screen::update(Memory* sender, const Memory::Message& msg)
{
    std::cout << "Modification of memory : " << msg.start << ":" << msg.size << std::endl;
}

void Screen::observableDies(const Memory *sender)
{
    memory=nullptr;
}

