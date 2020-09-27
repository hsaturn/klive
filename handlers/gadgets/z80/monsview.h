#pragma once

#include "mons.h"

#include <QTableView>
#include <QStandardItemModel>
#include <core/hardware/memory.h>

using hw::Memory;

class MonsView : public QTableView,
        public Observer<Memory>
{
    Q_OBJECT

public:
    explicit MonsView(QWidget *parent = nullptr);
    virtual ~MonsView();

    virtual void update(Memory* sender, const Memory::Message& msg) override;
    virtual void observableDies(const Memory* sender) override;

    // void resizeEvent(QResizeEvent *event) override {};
    void setMemory(Memory*);
    void setPointer(Memory::addr_t);

private:
    Mons mons;
    Memory* memory;
    Memory::addr_t visible_mem;
    QStandardItemModel *model;
    map<Memory::addr_t, long> addr_to_table_line;
};

