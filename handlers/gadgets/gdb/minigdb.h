#pragma once

#include <core/hardware/computer.h>
#include <core/hardware/checkpoints.h>
#include <core/hardware/cpu.h>
#include <core/hardware/memory.h>
#include <QLineEdit>
#include <QTextEdit>
#include <map>
#include <set>

using hw::Memory;
using hw::Computer;
using hw::Cpu;

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
    bool evaluate(std::string command);
    void update(std::ostream& out);
    bool outexpr(std::ostream& out, std::string& expr);
    Computer* computer;
    QLineEdit* cmdLine;
    QTextEdit* result;
    MiniGdb();

    std::set<std::string> displays;
    CheckPoints checkpoints;
};
