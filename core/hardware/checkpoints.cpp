#include "checkpoints.h"

#include <fstream>

// TODO better design to choose the file
static const char* cp_file="checkpoints.dat";

const std::string CheckPoints::takeSnapshot(hw::Computer *computer)
{
        std::string cp = computer->checkPoint();
        std::ofstream checkpoints;
        checkpoints.open(cp_file, std::ios_base::app);
        checkpoints << cp << std::endl;
        return cp;
}


bool CheckPoints::start(hw::Computer* computer)
{
    std::fstream file;
    tests=0;

    file.open(cp_file, std::ios_base::in);
    if (file.good())
    {
        snapshots.clear();
        file.clear();
        computer->reset();

        auto& breaks = computer->cpu->breaks;
        breaks.clearAll();

        int row=0;

        while(file.good())
        {
            row++;
            hw::cycle cycle;
            file >> cycle;

            hw::Memory::addr_t addr;
            file >> addr;

            std::string s;
            getline(file, s);

            SnapShots& snaps = snapshots[addr];
            if (s[0]==':')
            {
                s.erase(0,1);
                if (snaps.size()==0)
                {
                    hw::BreakPoints::BreakPoint bp;
                    bp.type(hw::BreakPoints::BreakPoint::CHECKPOINT);
                    breaks.add(addr, bp);
                }
                snaps.insert({cycle, {s, row}});
            }
            else
            {
                std::cerr << "CheckPoints::row " << row << ", ignored" << std::endl;
            }
        }
        return true;
    }
    else
    {
        std::cerr << "Unable to open (" << cp_file << ")" << std::endl;
        return false;
    }
}

bool CheckPoints::check(const hw::Computer* computer)
{
    // TODO should find ?
    SnapShots& snaps = snapshots[computer->cpu->getPc()];
    if (snaps.count(computer->cpu->getClock().cycles()))
    {
        SnapShot snap = snaps[computer->cpu->getClock().cycles()];
        std::string state=computer->checkPoint(false);
        if (state!=snap.snap)
        {
            std::cerr << "CheckPoint ERROR (" << cp_file << "):" << std::endl;
            std::cerr << "        row: " << snap.row << std::endl;
            std::cerr << "        addr: " << computer->cpu->getPc() << std::endl;
            std::cerr << "        expected: " << snap.snap << std::endl;
            std::cerr << "        got     : " << state << std::endl;

            return false;
        }
        else
        {
            std::cout << "TEST OK " << state << std::endl;
            tests++;
            return true;
        }
    }
    return true;
}
