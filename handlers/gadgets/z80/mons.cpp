#include "mons.h"
#include <common/mapreader.h>

#include <stdio.h>
#include <string>
#include <iostream>
#include <algorithm>

#include <QFile>
#include <QTextSTream>

using namespace std;

static void formatKeyValue(long line, string& key, string& value)
{
    if (value=="")
    {
        key="";
        return;
    }
    string hexa(key);
    for_each(hexa.begin(), hexa.end(), [](char&c){ c = toupper(c); });
    if (hexa.length()%2)
    {
        cerr << "Error at line " << line << ": Bad hexaopcode format(" << key << ")" << endl;
        key="";
    }
    else
    {
        auto pos=hexa.find('$');
        while(value.find('\t') != string::npos)
            value[value.find('\t')]=' ';

        if (pos != string::npos)
        {
            cout << "Convert [" << key << " - " << value << "] to [";
            key=hexa.substr(0, pos);
            cout << key << " - ";
            value=hexa.substr(pos)+'\t'+value;
            cout << value << ']' << endl;
        }
        else
            key=hexa;
    }
}

Mons::Mons()
{
    cout << "Opcode : " <<
        MapReader::ReadTabFile(":/z80.txt", opcodes, formatKeyValue)
        << endl;
}

static string peek(const Memory* memory, Memory::addr_t& addr)
{
    char buffer[3];
    auto b = memory->peek(addr++);
    sprintf(buffer, "%2x", b);
    buffer[2]=0;
    string hexa=string(buffer);
    for_each(hexa.begin(), hexa.end(), [](char&c) { c=toupper(c);});
    return hexa;
}

string Mons::decode(const Memory* memory, Memory::addr_t& addr)
{
    string hexa = peek(memory, addr);
    const auto& it = opcodes.find(hexa);
    if (it != opcodes.end())
    {
        string sasm=it->second;
        auto post=sasm.find('\t');
        if (post == string::npos)
            return sasm;
        else if (sasm[0] !='$')
        {
            cerr << "Mons: Error in z80.txt (" << hexa << ',' << sasm << ')' << endl;
        }
        string decode=sasm.substr(0, post);
        sasm=sasm.substr(post+1);
        while(decode.length() && decode.length()>=2)
        {
            uint16_t value;
            switch(decode[1])
            {
                case 'V':
                    value=memory->peek(addr++);
                   break;
                case 'O':
                    value=static_cast<int8_t>(memory->peek(addr++));
                    break;
                case 'J':
                    value=pc+static_cast<int8_t>(memory->peek(addr++));
                case 'A':
                case 'W':
                    value=memory->peek(addr++)+(memory->peek(addr++)<<8);
                    break;
                default:
                    cerr << "Mons error: " << hexa << "Unable to decode " << it->second << " ($" << decode[1] << ")" << endl;
                    return "??";
                    break;
            }
            decode.erase(0,2);
            auto poss=sasm.find('*');
            if (poss==string::npos)
            {
                cerr << "Mons error: cannot bind value " << decode[1] << " in (" << sasm << ")" << endl;
                return "?? "+sasm;
            }
            while(sasm[poss+1]=='*') sasm.erase(poss,1);
            sasm=sasm.substr(0,poss)+std::to_string(value)+sasm.substr(poss+1);
        }
        return sasm;
    }
    return "??";
}
