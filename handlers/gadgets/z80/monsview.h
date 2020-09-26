#pragma once
#include <QWidget>
#include <core/hardware/memory.h>

using hw::Memory;

class MonsView : public QWidget,
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
    Memory* memory;
    Memory::addr_t visible_mem;
};

