#pragma once

#include "computer.h"
#include "cpu.h"
#include "memory.h"
#include <unordered_map>


// Checkpoints are used for unit tests.
// A checkpoint is a snapshot of the computer at a give time
// Checkpoints are stored in a file in order to be unit tested later

class CheckPoints
{
    struct SnapShot
    {
        std::string snap;
        int row;
    };

    using SnapShots=std::unordered_map<hw::cycle, SnapShot>;

public:

    const std::string takeSnapshot(hw::Computer* computer);
    bool start(hw::Computer*);
    bool check(const hw::Computer*);

private:
    // for a given address, can exist many snapshots, one each
    // time the cpu arrives at the addr
    std::unordered_map<hw::Memory::addr_t, SnapShots> snapshots;
    int tests;
};
