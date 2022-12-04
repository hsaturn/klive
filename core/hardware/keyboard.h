#pragma once

#include <core/hardware/cpu.h>
#include <common/observer.h>
#include <vector>
#include <unordered_map>

using std::string;

namespace hw
{

struct Port
{
    Memory::addr_t addr;
    uint8_t value;	// bits (one bit value=0 depending of the key)
};
using Ports = std::vector<Port>;

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
    std::unordered_map<int, Ports> ascii;				// Keyboard mapping
    std::unordered_map<Memory::addr_t, uint8_t> ports;	// Physical port values
};

}
