#include "mons.h"
#include <common/mapreader.h>

#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>

#include <QFile>
#include <QTextSTream>

using namespace std;

Mons::Labels Mons::rom_labels;
Mons::Opcodes Mons::opcodes;

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
    if (opcodes.size()==0)
    {
        cout << "Opcode : " <<
            MapReader::ReadTabFile(":/z80.txt", opcodes, formatKeyValue)
            << endl;
        loadStaticLabels();
    }
}

void Mons::loadStaticLabels()
{
    rom_labels[0x11CB]="START_NEW";
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

string Mons::getLabel(const Memory::addr_t addr) const
{
    const auto& it = rom_labels.find(addr);
    if (it != rom_labels.end())
        return it->second;

    const auto& it2 = dyn_labels.find(addr);
    if (it2 != dyn_labels.end())
        return it2->second;

    return "";
}

string Mons::getOrCreateLabel(const Memory::addr_t addr)
{
    string label = getLabel(addr);
    if (label.length())
        return label;

    stringstream lbl;
    lbl << "L_" << setw(4) << hex << uppercase << addr;
    dyn_labels[addr]=lbl.str();
    return lbl.str();
}

Mons::Row Mons::decode(const Memory* memory, Memory::addr_t& addr)
{
    Row row;
    row.label = getLabel(addr);
    string hexa = peek(memory, addr);
    const auto& it = opcodes.find(hexa);
    if (it != opcodes.end())
    {
        string sasm=it->second;
        auto post=sasm.find('\t');
        if (post == string::npos)
        {
            row.mnemo=sasm;
            return row;
        }
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
                    value=static_cast<int8_t>(memory->peek(addr++));
                    value += addr;
                    getOrCreateLabel(value);
                    break;
                case 'A':
                    value=memory->peek(addr++)+(memory->peek(addr++)<<8);
                    getOrCreateLabel(value);
                    break;
                case 'W':
                    value=memory->peek(addr++)+(memory->peek(addr++)<<8);
                    break;
                default:
                    cerr << "Mons error: " << hexa << "Unable to decode " << it->second << " ($" << decode[1] << ")" << endl;
                    row.mnemo="??";
                    break;
            }
            decode.erase(0,2);
            auto poss=sasm.find('*');
            if (poss==string::npos)
            {
                cerr << "Mons error: cannot bind value " << decode[1] << " in (" << sasm << ")" << endl;
                row.mnemo = "?? "+sasm;
                return row;
            }
            while(sasm[poss+1]=='*') sasm.erase(poss,1);
            sasm=sasm.substr(0,poss)+std::to_string(value)+sasm.substr(poss+1);
        }
        row.mnemo = sasm;
        return row;
    }
    row.mnemo = "??";
    return row;
}
