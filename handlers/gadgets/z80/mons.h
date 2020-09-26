#pragma once
#include <core/hardware/memory.h>

#include <map>
#include <string>

using namespace std;
using hw::Memory;

class Mons
{
public:
    Mons();

    string decode(const Memory* memory, Memory::addr_t& addr);

private:
   std::map<std::string, std::string> opcodes;
};
