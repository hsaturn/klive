#pragma once

#include <core/hardware/cpu.h>
#include <common/observer.h>

namespace hw
{

class Keyboard : public Observer<Cpu>, public QObject
{
public:
    Keyboard(Cpu*);
    ~Keyboard() override;

    virtual bool eventFilter(QObject* object, QEvent* event) override;

    virtual void update(Cpu* sender, const typename Cpu::Message& msg) override;
    virtual void observableDies(const Cpu* sender) override;

private:
    Cpu* cpu = nullptr;
};

}
