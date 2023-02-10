#pragma once

#include <core/hardware/computer.h>
#include <core/hardware/checkpoints.h>
#include <core/hardware/cpu.h>
#include <core/hardware/memory.h>

#include <handlers/gadgets/console/console.h>
#include <QLineEdit>
#include <QTextEdit>
#include <map>
#include <set>

using hw::Memory;
using hw::Computer;
using hw::Cpu;

// TODO miniGdb should be responsible of all cpu related operations
// This should be the z80 observer that may forward to monsview register view etc etc
class MiniGdb : public QWidget, Observer<Cpu>, Observer<Console>
{
    Q_OBJECT

public:
    MiniGdb(Computer*, Console*);

    void update(Cpu* sender, const Cpu::Message& msg) override;
    void update(Console* sender, const Console::Message& msg) override;
    void observableDies(const Console* console) override;
    void observableDies(const Cpu* sender) override;

public slots:
    void onCmdLine();
    void cpuStep();

private:
    bool evaluate(std::string command);
    void update(std::ostream& out);
    bool outexpr(std::ostream& out, std::string& expr);
    Computer* computer;
    Console* console = nullptr;
    QLineEdit* cmdLine;
    QTextEdit* result;
    MiniGdb();

    std::set<std::string> displays;
    CheckPoints checkpoints;
};
