#include "mons.h"
#include <common/mapreader.h>

#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <queue>
#include <set>

#include <QFile>

using namespace std;

Mons::Labels Mons::rom_labels;
Mons::Opcodes Mons::opcodes;

void Mons::formatKeyValue(long line, string& key, string& value)
{
    if (value=="")
    {
        key="";
        return;
    }
    for_each(key.begin(), key.end(), [](char&c){ c = toupper(c); });
    for_each(value.begin(), value.end(), [](char&c){ c = toupper(c); });
    string hexa(key);
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
            // cout << "Convert (" << key << ") ";
            key[pos]='.';
            key[pos+1]='.';
            opcodes[hexa.substr(0,pos)] = hexa.substr(pos)+'\t'+value;
            // cout << " fix (" << hexa.substr(0,pos) << '=' << opcodes[hexa.substr(0,pos)]  << ") ";
            // cout << '[' << key << "=" << value << ']' << endl;
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
            MapReader::ReadTabFile(":/mons/z80.txt", opcodes, formatKeyValue)
            << ", count=" << opcodes.size() << endl;

        loadStaticLabels();
    }
}

void Mons::loadStaticLabels()
{
    rom_labels= {
        {0x0008, "ERROR_1"},
        {0x0018, "GET_CHAR"},
        {0x0020, "NEXT_CHAR"},
        {0x0028, "FP_CALC"},
        {0x0038, "MASK_INT"},
        {0x0048, "KEY_INT"},
        {0x0053, "ERROR-2"},
        {0x0055, "ERROR-3"},
        {0x0066, "RESET"},
        {0x0080, "NO-RESET"},
        {0x0074, "CH-ADD+1"},
        {0x0077, "TEMP-PTR1"},
        {0x0078, "TEMP-PTR2"},
        {0x007D, "SKIP-OVER"},
        {0x0090, "SKIPS"},
        {0x0095, "TOKENS_T"},
        {0x0205, "KEY_TBL"},
        {0x022C, "EKEY_TBL"},
        {0x028E, "KEY_SCAN"},
        {0x0296, "KEY_LINE"},
        {0x029F, "KEY_3KEYS"},
        {0x02A1, "KEY_BITS"},
        {0x02AB, "KEY_DONE"},
        {0x02BF, "KEYBOARD"},
        {0x02F1, "K_NEW"},
        {0x0310, "K_REPEAT"},
        {0x0D4D, "TEMPS"},
        {0x0E44, "CL_LINE"},
        {0x0E9B, "CL_ADDR"},
        {0x10A8, "KEY_INPUT"},
        {0x11CB, "START_NEW"},
        {0x15D4, "WAIT_KEY"},
        {0x15DE, "WAIT_KEY1"},
        {0x15E6, "INPUT_AD"},
        {0x15EF, "OUT_CODE"},
        {0x15F2, "PRINT_A_2"},
        {0x15F7, "CALL_SUB"},
        {0x1601, "CHAN_OPEN"},
        {0x162D, "CHANCODE"},
        {0x1642, "CHAN_S"},
        {0x16DB, "INDEXER_1"},
        {0x16DC, "INDEXER"},

        // RAM labels... TODO.. in {]
        {0x5C00, "KSTATE0"},
        {0x5C3D, "ERR_SP"},
        {0x5C78, "FRAMES"},
        {0x5C51, "CURCHL"}};

}

static string peek(const Memory* memory, Memory::addr_t& addr, bool increment=true)
{
    char buffer[3];
    auto b = memory->peek(addr);
    if (increment) addr++;
    sprintf(buffer, "%02x", b);
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
    lbl << "L_" << setw(4) << hex << uppercase << setfill('0') << addr;
    dyn_labels[addr]=lbl.str();
    return lbl.str();
}

uint16_t peekWord(const Memory* memory, Memory::addr_t& addr)
{
    uint16_t lo=memory->peek(addr++);
    uint16_t hi=memory->peek(addr++);

    return (hi<<8)+lo;
}

Mons::Row Mons::decode(const Memory* memory, Memory::addr_t& addr)
{
    if (addr==0x1287)
    {
        cout << "stop here" << endl;	// Easy bp set for gdb
    }
    Row row;
    row.label = getLabel(addr);
    string hexa;
    Memory::addr_t scan = addr;
    while(hexa.length()<8)
    {
        hexa += peek(memory, scan);
        if (opcodes.count(hexa))
        {
            addr=scan;
            break;
        }
    }
    const auto& it = opcodes.find(hexa);
    row.hexa=hexa;
    if (it == opcodes.end())
    {
        addr++;
        row.mnemo = "??";
    }
    else
    {
        string sasm=it->second;
        auto postab=sasm.find('\t');
        if (postab == string::npos)
        {
            row.mnemo=sasm;
            return row;
        }
        else if (sasm[0] !='$')
        {
            cerr << "Mons: Error in z80.txt (" << hexa << ',' << sasm << ')' << endl;
        }
        queue<int32_t> values;
        string decode=sasm.substr(0, postab);
        // sasm=sasm.substr(postab+1);
        while(decode.length() && decode.length()>=2)
        {
            row.hexa += peek(memory, addr, false);
            if (decode[0]=='$')
            {
                hexa += "..";
                int32_t value;
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
                        value=peekWord(memory, addr);
                        getOrCreateLabel(value);
                        break;
                    case 'W':
                        value=peekWord(memory, addr);
                        break;
                    default:
                        cerr << "Mons error: " << hexa << "Unable to decode " << it->second << " ($" << decode[1] << ")" << endl;
                        row.mnemo="??";
                        addr++;
                        break;
                }
                values.push(value);
            }
            else
            {
                hexa += peek(memory, addr, false);
                addr++;
            }
            decode.erase(0,2);	// erase $x or hexa
        }
        auto it=opcodes.find(hexa);
        if (it == opcodes.end())
        {
            static std::set<decltype(hexa)> mem;
            if (mem.count(hexa)==0)
            {
                mem.insert(hexa);
                std::cerr << "Mons, cannot find complex hexa (" << hexa << ')' << std::endl;
            }
            row.mnemo="?? "+hexa;
            return row;
        }
        sasm=it->second;
        while(values.size())
        {
            auto poss=sasm.find('*');
            auto value=values.front();
            values.pop();
            if (poss==string::npos)
            {
                cerr << "Mons error: cannot bind value " << decode[1] << " in (" << sasm << ")" << endl;
                row.mnemo = "?? "+sasm;
                return row;
            }
            while(sasm[poss+1]=='*') sasm.erase(poss,1);
            string label=getLabel(value);
            if (label.length())
                sasm=sasm.substr(0,poss)+label+sasm.substr(poss+1);
            else
                sasm=sasm.substr(0,poss)+std::to_string(value)+sasm.substr(poss+1);
        }
        row.mnemo = sasm;
    }
    return row;
}
