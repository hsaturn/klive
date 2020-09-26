#include "mons.h"
#include <common/mapreader.h>

#include <string>
#include <iostream>

#include <QFile>
#include <QTextSTream>

using namespace std;

static string formatHexa(const string& key)
{
    return key;
}

static string formatValue(const string& value)
{
    return value;
}

Mons::Mons()
{
    cout << "Opcode : " <<
        MapReader::ReadTabFile(":/z80.txt", opcodes
    )
        << endl;
}
