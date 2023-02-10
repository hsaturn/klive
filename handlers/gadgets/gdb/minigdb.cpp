#include "minigdb.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <string>
#include <sstream>
#include <fstream>
#include <QPushButton>
#include <common/expr.h>
#include <common/scopevar.h>

using namespace std;

MiniGdb::MiniGdb(Computer* computer_, Console* con)
    : computer(computer_), console(con)
{
    QVBoxLayout* layout=new QVBoxLayout;

    cmdLine = new QLineEdit;
    connect(cmdLine, SIGNAL(returnPressed()), this, SLOT(onCmdLine()));

    QHBoxLayout* h=new QHBoxLayout;
    h->addWidget(cmdLine);
    QPushButton* btn_step=new QPushButton(">");
    btn_step->setMaximumWidth(20);
    connect(btn_step, SIGNAL(clicked()), this, SLOT(cpuStep()));
    h->addWidget(btn_step);
    layout->addLayout(h);

    result = new QTextEdit;
    result->setEnabled(false);
    layout->addWidget(result);
    setLayout(layout);

    if (computer) computer->cpu->attach(this);	// Note this could be multi cpu !
    if (console)
    {
        console->attach(this);
        console->registerHelp("gdb", {
            { "jp {addr}" 	                   , "Jump to addr" },
            { "set {reg}={expr}"               , "Change register"},

            // Write machine state to checkpoints.dat
            { "checkpoint"                     , "Build a checkpoint"},

    // Read checkpoints.dat and create checkpoints
    // that'll be checked while the computer is running
            { "test"                           , "Test on/off" },
            { "break {addr}"                   , "Add breakpoint [if...]" },
            { "delete {addr}"                  , "Delete breakpoint" },
            { "next"                           , "Next statement" },
            { "step"                           , "Step in" },
            { "finish"                         , "Step out" },
            { "continue"                       , "Continue" },
            { "info [b]"                       , "info" },
            { "reset"                          , "Reset cpu" },
            { "run {file}"                     , "Reset and run cpu, or execute a gdb script" },
            { "display {expr}"                 , "display expr" },
            { "until {expr}"                   , "Run until condition" },
            { "while {expr}"                   , "Run while condition" },
            { "stop"                           , "Stop cpu" },
            { "print | ? {expr}"               , "eval expr" },
            { "memload {addr} {file}"          , "Load binary file at address" },
            { "memsave {addr} {file} {expr}"   , "Save memory into file" },
            { "undisplay {#} | *"              , "Remove a display" },
        });
    }
}

static bool readAddr(Computer* comp, string& s, uint32_t& addr)
{
    exprtype result;
    if (parseExpression(s, result, comp->cpu))
    {
        addr=static_cast<uint32_t>(result);
        trim(s);
        // TODO if addr > max ram
        return true;
    }
    return false;
}

void MiniGdb::cpuStep()
{
    computer->cpu->step();
}

void MiniGdb::update(Console*, const Console::Message& msg)
{
    if (evaluate(msg.data))
    {
        msg.processed++;
    }
}

void MiniGdb::update(Cpu* , const Cpu::Message& msg)
{
    if (!isVisible()) return;

    stringstream d;
    switch(msg.event)
    {
        case Cpu::Message::BREAK_POINT:
            if (msg.brk->isEnabled())
            {
                switch(msg.brk->type())
                {
                    case hw::BreakPoints::BreakPoint::ALWAYS:
                        computer->cpu->stop();
                        d << "Break point reached" << endl;
                        break;
                    case hw::BreakPoints::BreakPoint::CHECKPOINT:
                        if (checkpoints.check(computer))
                        {
                            d << "Checkpoint " << computer->cpu->getPc() << ", ok" << endl;
                        }
                        else
                        {
                            d << "ERROR checkpoint (cycle/addr)=(" <<
                                 computer->cpu->getClock().cycles() << ", " << computer->cpu->getPc() <<
                                 ')' << endl;
                            computer->cpu->stop();
                        }
                        break;

                    default:
                        d << "Unknown breakpoint type" << endl;
                        break;
                }
            }
            break;
        case Cpu::Message::UNTIL_REACHED:
            d << "Until condition reached" << endl;
            break;
        case Cpu::Message::WHILE_REACHED:
            d << "While condition reached" << endl;
            break;
        case Cpu::Message::UNKNOWN_OP:
            {
                Memory::addr_t addr=(msg.data & 0xFFFF0000) >> 16;
                result->setStyleSheet("color: red;");
                d << uppercase << hex << showbase;
                d << "Unknown opcode [" << (msg.data & 0xFF) << "] at " << addr << endl;
                result->setStyleSheet("color: black;");
            }
            break;
        case Cpu::Message::MACROSTEP:
            break;

        default:
            return;
    }
    update(d);
    result->setText(d.str().c_str());
}

bool MiniGdb::outexpr(ostream& out, string& expr)
{
    string copy(expr);
    exprtype eval;
    if (parseExpression(expr, eval, computer->cpu))
    {
        size_t len=copy.length()-expr.length();
        if (len)	// Should be....
        {
            out << '(' << copy.substr(0,len) << ") = ";
        }
        out << dec << eval << ", " << showbase << hex << eval << ' ';
        if (eval <= ' ' or eval >=128)
            out << '-';
        else
            out << '\'' << (char)eval << '\'';
        out << endl;
        return true;
    }
    else
    {
        out << "(error " << expr << ")";
        return false;
    }
}

void MiniGdb::update(ostream& out)
{
    char c='a';

    for(const auto& display: displays)
    {
        string expr(display);
        outexpr(out, expr);
        c++;
    }
    if (computer)
    {
        out << "cycles " << computer->cpu->getClock().cycles() << endl;
    }
}

void MiniGdb::observableDies(const Cpu *)
{
    // TODO what to do ?
}

void MiniGdb::observableDies(const Console*)
{
}

void MiniGdb::onCmdLine()
{
    static string lastCmd;
    result->setText("");

    string s=cmdLine->text().toStdString();
    if (s=="")
    {
        s=lastCmd;
    }
    else
    {
        lastCmd=s;
    }
    evaluate(s);
    cmdLine->setText("");

}

bool MiniGdb::evaluate(std::string s)
{
    static int recurse = 0;
    ScopeVar<int> rec(recurse);

    stringstream error;
    stringstream output;

    if (rec++ >= 10)
    {
        error << "Too many recursive calls" << endl;
        return false;
    }

    while (s.length())
    {
        std::string cmd = getlex(s);
        if (cmd=="checkpoint")
        {
            output << checkpoints.takeSnapshot(computer) << endl;
            output << "Checkpoint written" << endl;
        }
        else if (cmd=="test")
        {
            error << "NOT FINISHED" << endl;
            string state=getlex(s);
            if (state=="off")
            {
                error << "TODO" << endl;
            }
            else if (state=="on")
            {
                if (checkpoints.start(computer))
                {
                    output << "Started auto check" << endl;
                }
                else
                {
                    error << "Unknown error" << endl;
                }
            }
            else
            {
                error << "on or off expected " << endl;
            }
        }
        else if (cmd=="delete")
        {
            Memory::addr_t addr;
            if (readAddr(computer, s, addr))
            {
                if (not computer->cpu->breaks.remove(addr))
                    output << cmd << ": ok" << endl;
                else
                    error << "Not found (" << hex << addr << dec << ')';
            }
        }
        else if (cmd=="info")
        {
            while(s.length())
            {
                string what=getlex(s);
                if (what=="b")
                {
                    for(const auto& it : computer->cpu->breaks)
                    {
                        const hw::BreakPoints::BreakPoint &b = it.second;
                        output << hex << it.first << dec << " : "
                               << b.getString()
                               << endl;
                    }
                    output << cmd << ' ' << s << ": ok" << endl;
                }
                else
                    error << "Unknown info (" << what << ')' << endl;
            }
        }
        else if (cmd=="memload")
        {
            Memory::addr_t addr;
            if (readAddr(computer, s, addr))
            {
                if (not computer->memory->load(s, addr))
                    error << "loadmem Error" << endl;
                s.clear();
            }
            else
                error << "Bad address" << endl;
        }
        else if (cmd=="memsave")
        {
            s.clear();
        /*    Memory::addr_t addr;
            if (readAddr(computer, s, addr))
            {
                if (not computer->memory->save(s, addr, size))
                    error << "loadmem Error" << endl;
                s.clear();
            }
            else
                error << "Bad address" << endl;
                */
        }
        else if (cmd=="break")
        {
            Memory::addr_t addr;
            if (readAddr(computer, s, addr))
            {
                hw::BreakPoints::BreakPoint bp;
                if (s[0]=='i' && s[1]=='f' && s[2]==' ')
                {
                    getlex(s);	// remove if
                    bp.type(hw::BreakPoints::BreakPoint::CONDITIONAL);
                    bp.setString(s);
                    s="";
                }

                computer->cpu->breaks.add(addr, bp);
                output << "ok, breaks: " << computer->cpu->breaks.size() << endl;
            }
            else
            {
                error << "Missing argument after " << cmd << endl;
            }
        }
        else if (cmd=="jp")
        {
            Memory::addr_t addr;
            if (readAddr(computer, s, addr))
            {
                computer->cpu->jp(addr);
                output << cmd << ": ok" << endl;
            }
            else {
                error << "Expected valid address" << endl;
            }
        }
        else if (cmd=="while" or cmd=="until")
        {
            string expr=s;
            exprtype eval;
            if (parseExpression(s, eval, computer->cpu))
            {
                output << "running " << cmd << expr << endl;
                if (cmd=="while")
                    computer->cpu->setWhile(expr);
                else
                    computer->cpu->setUntil(expr);
                output << cmd << ": ok" << endl;
            }
            else {
                error << "Error in expression " << expr << endl;
            }
        }
        else if (cmd=="stop")
        {
            computer->cpu->stop();
            output << cmd << ": ok" << endl;
        }
        else if (cmd=="display")
        {
            string expr(s);
            exprtype eval;
            if (parseExpression(s, eval, computer->cpu))
            {
                output << "Inserting display(" << expr << ")" << endl;
                displays.insert(expr);
            }
        }
        else if (cmd=="undisplay")
        {
            if (s.length())
            {
                char i=static_cast<char>(toupper(s[0]));
                s.erase(0,1);
                for(set<string>::iterator it=displays.begin(); it!=displays.end(); it++)
                {
                    if (i=='A' || i=='*')
                    {
                        displays.erase(it);
                        output << cmd << ": ok" << endl;
                        if (i != '*')
                            break;
                    }
                    else {
                        i--;
                    }
                }
            }
            else
            {
                error << "Missing argument" << endl;
            }
        }
        else if (cmd=="print")
        {
            outexpr(output, s);
        }
        else if (cmd=="set" or (s[0]=='='))
        {
            string reg;
            if (s[0]=='=')
                reg=cmd;
            else
                reg=getlex(s);
            cmd="set";

            if (s[0] != '=')
            {
                error << "Syntax error" << endl;
                break;
            }
            s.erase(0,1);
            int32_t value;
            if (!parseExpression(s, value, computer->cpu))
            {
                error << "Bad expression" << endl;
                break;
            }
            if (computer->cpu->regs()->set(reg, value))
            {
                output << "SET " << reg  << '=' << value << endl;
            }
            else
            {
                error << "Bad register:" << reg << endl;
            }
        }
        else if (cmd=="reset")
        {
            computer->reset();
            output << cmd << ": ok" << endl;
        }
        else if (cmd=="step")
        {
            int32_t count(1);
            if (s.length())
            {
                if (!parseExpression(s, count, computer->cpu))
                {
                    error << "Invalid expression" << endl;
                    count=0;
                }
            }
            if (count)
            {
                computer->cpu->run_steps(count);
                output << cmd << ' ' << count << ": ok" << endl;
            }
        }
        else if (cmd=="next")
        {
            computer->cpu->step_over();
            output << cmd << ": ok" << endl;
        }
        else if (cmd=="finish")
            computer->cpu->step_out();
        else if (cmd=="continue")
            computer->cpu->start();
        else if (cmd=="run")
        {
            if (s.length())
            {
                // TODO move at a more generic place (resolveName() ?)
                if (s[0]=='~')
                    s=getenv("HOME")+s.substr(1);
                std::ifstream script(s);
                if (script.good())
                {
                    while (script.good())
                    {
                        std::string line;
                        std::getline(script, line);
                        if (not this->evaluate(line))
                            break;
                    }
                }
                else
                    error << "Unable to open (" << s << ')' << endl;
                s.clear();
            }
            else
            {
                computer->cpu->reset();
                computer->cpu->start();
            }
        }
        else if (cmd=="")
        {
            error << "Command not found: " << cmd << endl;
            break;
        }
        else
        {
            error << "Shouldn't arrive here found(" << cmd << ")";
            break;
        }
    }
    bool ok = true;
    result->setStyleSheet("color: black;");
    if (error.str().length())
    {
        ok = false;
        result->setStyleSheet("color: red;");
        output << "ERROR: " << error.str();
    }
    result->setStyleSheet("color: black;");
    if (displays.size())
    {
        output << "---------------------" << endl;
        update(output);
    }
    if (output.str().length())
    {
        result->setText(result->toPlainText()+output.str().c_str());
    }
    return ok;
}
