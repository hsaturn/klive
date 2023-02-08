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


MiniGdb::MiniGdb(Computer* computer_)
    : computer(computer_)
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

    computer->cpu->attach(this);	// Note this could be multi cpu !
}

static bool readAddr(Computer* comp, string& s, uint32_t& addr)
{
    exprtype result;
    if (parseExpression(s, result, comp->cpu))
    {
        addr=static_cast<uint32_t>(result);
        // TODO if addr > max ram
        return true;
    }
    return false;
}

void MiniGdb::cpuStep()
{
    computer->cpu->step();
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
    stringstream error;
    stringstream output;

    // TODO better help system completion etc
    static map<string, string> cmds;
    cmds["jp addr"] = "Jump to addr";
    cmds["set reg=expr"] = "Change register";

    // Write machine state to checkpoints.dat
    cmds["checkpoint"] = "Build a checkpoint";

    // Read checkpoints.dat and create checkpoints
    // that'll be checked while the computer is running
    cmds["test"] = "Test on/off";

    cmds["break addr"] = "Add breakpoint [if...]";
    cmds["delete addr"] = "Delete breakpoint";
    cmds["next"] = "Next statement";
    cmds["step"] = "Step in";
    cmds["finish"] = "Step out";
    cmds["continue"] = "Continue";
    cmds["info [b]"] = "info";
    cmds["loadmem addr file"] = "Load binary file at address.";
    cmds["reset"] = "Reset cpu";
    cmds["print or ? expr"] = "eval expr";
    cmds["display expr"] = "display expr";
    cmds["until expr"] = "Run until condition";
    cmds["undisplay # or '*'"] = "Remove a display";
    cmds["while cond"] = "Run while condition";
    cmds["stop"] = "Stop cpu";
    cmds["run (file)"] = "Reset and run cpu, or execute a gdb script";
    cmds["help"] = "Display this help";

    ScopeVar<int> rec(recurse);
    if (rec++ >= 10)
    {
        error << "Too many recursive calls" << endl;
        s.clear();
    }

    while (s.length())
    {
        string cmd=getlex(s);
        string ambiguous;
        string found;

        if (cmd.length())
        {
            for(const auto& it: cmds)
            {
                string command=it.first;
                command=getword(command);
                if (command.substr(0, cmd.length())==cmd)
                {
                    if (ambiguous.length() || found != "")
                    {
                        if (ambiguous=="") ambiguous=found;
                        ambiguous=command+" "+ambiguous;
                    }
                    else if (found=="")
                        found=command;
                }
            }
        }

        if (ambiguous.length())
        {
            s=cmd+' '+s;
            error << "Ambiguous command : " << cmd << endl;
            error << "Candidates: " << ambiguous << endl;
            break;
        }
        else if (cmd=="")
        {
            s=cmd+' '+s;
            error << "Syntax error (" << s << ")" << endl;
            break;
        }
        else if (found=="checkpoint")
        {
            output << checkpoints.takeSnapshot(computer) << endl;
            output << "Checkpoint written" << endl;
        }
        else if (found=="test")
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
        else if (found=="delete")
        {
            Memory::addr_t addr;
            if (readAddr(computer, s, addr))
            {
                if (not computer->cpu->breaks.remove(addr))
                    output << found << ": ok" << endl;
                else
                    error << "Not found (" << hex << addr << dec << ')';
            }
        }
        else if (found=="info")
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
                    output << found << ' ' << s << ": ok" << endl;
                }
                else
                    error << "Unknown info (" << what << ')' << endl;
            }
        }
        else if (found=="loadmem")
        {
            Memory::addr_t addr;
            if (readAddr(computer, s, addr))
            {
                if (not computer->memory->load(s, addr))
                    error << "Error" << endl;
                s.clear();
            }
            else
                error << "Bad address" << endl;
        }
        else if (found=="break")
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
                error << "Missing argument after " << found << endl;
            }
        }
        else if (found=="jp")
        {
            Memory::addr_t addr;
            if (readAddr(computer, s, addr))
            {
                computer->cpu->jp(addr);
                output << found << ": ok" << endl;
            }
            else {
                error << "Expected valid address" << endl;
            }
        }
        else if (found=="help")
        {
            for(const auto& it: cmds)
            {
                auto command=it.first;
                auto description=it.second;
                output << command << " : " << description << endl;
            }

        }
        else if (found=="while" or found=="until")
        {
            string expr=s;
            exprtype eval;
            if (parseExpression(s, eval, computer->cpu))
            {
                output << "running " << found << expr << endl;
                if (found=="while")
                    computer->cpu->setWhile(expr);
                else
                    computer->cpu->setUntil(expr);
                output << found << ": ok" << endl;
            }
            else {
                error << "Error in expression " << expr << endl;
            }
        }
        else if (found=="stop")
        {
            computer->cpu->stop();
            output << found << ": ok" << endl;
        }
        else if (found=="display")
        {
            string expr(s);
            exprtype eval;
            if (parseExpression(s, eval, computer->cpu))
            {
                output << "Inserting display(" << expr << ")" << endl;
                displays.insert(expr);
            }
        }
        else if (found=="undisplay")
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
                        output << found << ": ok" << endl;
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
        else if (found=="print" or cmd=="?")
        {
            if (cmd=="?")
            {
                found="print";
            }
            outexpr(output, s);
        }
        else if (found=="set" or (s[0]=='='))
        {
            string reg;
            if (s[0]=='=')
                reg=cmd;
            else
                reg=getlex(s);
            found="set";

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
        else if (found=="reset")
        {
            computer->reset();
            output << found << ": ok" << endl;
        }
        else if (found=="step")
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
                output << found << ' ' << count << ": ok" << endl;
            }
        }
        else if (found=="next")
        {
            computer->cpu->step_over();
            output << found << ": ok" << endl;
        }
        else if (found=="finish")
            computer->cpu->step_out();
        else if (found=="continue")
            computer->cpu->start();
        else if (found=="run")
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
        else if (found=="")
        {
            error << "Command not found: " << cmd << endl;
            s=cmd+' '+s;
            break;
        }
        else
        {
            error << "Shouldn't arrive here found(" << found << ") s(" << s << ") cmd(" << cmd << ") amb(" << ambiguous << ")" << endl;
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
