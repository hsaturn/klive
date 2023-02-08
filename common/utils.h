#pragma once

#include <string>

struct Utils
{

    // convert ~ to $HOME
    static std::string resolveFileName(std::string s);
};
