#pragma once
#include <core/hardware/memory.h>

#include <map>
#include <string>

using namespace std;
using hw::Memory;

class Mons
{
public:
    using Labels = std::map<Memory::addr_t, std::string>;
    using Opcodes = std::map<string,string>;
    struct Row
    {
        string label;
        string mnemo;
    };

    Mons();

    string getLabel(const Memory::addr_t) const;
    string getOrCreateLabel(const Memory::addr_t);
    Row decode(const Memory* memory, Memory::addr_t& addr);

private:
    void loadStaticLabels();
   static Opcodes opcodes;
   static Labels rom_labels;
   Labels dyn_labels;
};
