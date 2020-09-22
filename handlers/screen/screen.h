#pragma once

#include <common/observer.h>
#include <core/hardware/memory.h>

#include <core/hardware/memory.h>
#include <QWidget>
#include <QPoint>
#include <QLine>

#include "linebouncer.h"
#include <list>

using hw::Memory;

class Screen : public QWidget, public Observer<Memory>
{
    Q_OBJECT

public:
    Screen(QWidget* parent = nullptr);
    ~Screen();

    virtual void update(Memory* sender, const Memory::Message& msg) override;
    virtual void observableDies(const Memory* sender) override;

    void setMemory(hw::Memory* new_memory);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    LineBouncer bouncer;
    std::list<LineBouncer*> lines;
    hw::Memory* memory=nullptr;   // TODO should not be a ptr (crashes expected later)
};
