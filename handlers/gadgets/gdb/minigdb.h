#pragma once

#include <core/hardware/computer.h>
#include <core/hardware/cpu.h>
#include <core/hardware/memory.h>
#include <QLineEdit>
#include <QTextEdit>
#include <map>

using namespace hw;
using namespace std;


// TODO miniGdb should be responsible of all cpu related operations
// This should be the z80 observer that may forward to monsview register view etc etc
class MiniGdb : public QWidget
{
    Q_OBJECT

public:
    MiniGdb(Computer*);

public slots:
    void onCmdLine();
    void cpuStep();

private:
    Computer* computer;
    QLineEdit* cmdLine;
    QTextEdit* result;
    MiniGdb();
};
