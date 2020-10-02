#pragma once

#include <core/hardware/computer.h>
#include <core/hardware/cpu.h>
#include <core/hardware/memory.h>
#include <QLineEdit>
#include <QTextEdit>
#include <map>
#include <set>

using namespace hw;
using namespace std;


class te_expr;
// TODO miniGdb should be responsible of all cpu related operations
// This should be the z80 observer that may forward to monsview register view etc etc
class MiniGdb : public QWidget, Observer<Cpu>
{
    Q_OBJECT

public:
    MiniGdb(Computer*);

    void update(Cpu* sender, const Cpu::Message& msg) override;
    void observableDies(const Cpu* sender) override;

public slots:
    void onCmdLine();
    void cpuStep();

private:
    void update(ostream& out);
    bool outexpr(ostream& out, string& expr);
    Computer* computer;
    QLineEdit* cmdLine;
    QTextEdit* result;
    MiniGdb();

    std::set<string> displays;
};
