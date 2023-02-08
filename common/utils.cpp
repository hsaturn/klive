#include "utils.h"

std::string Utils::resolveFileName(std::string s)
{
    if (s[0] == '~')
    {
        const char* home=getenv("HOME");
        if (home) return home+s.substr(1);
    }
    return s;
}
