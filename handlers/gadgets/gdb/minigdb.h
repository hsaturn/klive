#pragma once

#include <core/hardware/computer.h>
#include <core/hardware/cpu.h>
#include <core/hardware/memory.h>
#include <QLineEdit>
#include <QLabel>
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

private:
    Computer* computer;
    QLineEdit* cmdLine;
    QLabel* result;
    MiniGdb();
};
