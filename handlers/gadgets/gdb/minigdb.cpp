#include "minigdb.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <string>
#include <sstream>
#include <QPushButton>
#include <common/expr.h>

using namespace std;


MiniGdb::MiniGdb(Computer* computer)
    : computer(computer)
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

bool readAddr(Computer* comp, string& s, uint32_t& addr)
{
    exprtype result;
    if (parseExpression(comp->cpu, s, result))
    {
        addr=static_cast<uint32_t>(result);
        // TODO if addr > max ram
        return true;
    }
    return false;
}

string getword(string &s, char sep=' ')
{
    string word;
    while(s.length() && s[0]!=sep)
    {
        word += s[0];
        s.erase(0,1);
    }
    while(s.length() && s[0]==sep) s.erase(0,1);
    return word;
}

void MiniGdb::cpuStep()
{
    computer->cpu->step();
}

void MiniGdb::update(Cpu* sender, const Cpu::Message& msg)
{
    switch(msg.event)
    {
        case Cpu::Message::UNTIL_REACHED:
            result->setText("Until condition reached");
            break;
        case Cpu::Message::WHILE_REACHED:
            result->setText("While condition reached");
            break;
        case Cpu::Message::STEP:
            break;
    }
}

void MiniGdb::observableDies(const Cpu *sender)
{
    // TODO what to do ?
}

void MiniGdb::onCmdLine()
{
    static string lastCmd;
    string s=cmdLine->text().toStdString();
    if (s=="")
    {
        s=lastCmd;
    }
    else
    {
        lastCmd=s;
    }
    stringstream error;
    stringstream answer;
    string cmd;
    result->setText("");

    while (s.length())
    {
        cmd=getlex(s);
        cout << "CMD " << cmd << endl;
        if (cmd=="b" || cmd=="d")
        {
            Memory::addr_t addr;
            if (readAddr(computer, s, addr))
            {
                if (cmd=="b")
                    computer->cpu->breaks.add(addr);
                else
                    computer->cpu->breaks.remove(addr);
                answer << "ok, breaks: " << computer->cpu->breaks.size() << endl;
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
            }
            else {
                error << "Expected valid address" << endl;
            }
        }
        else if (cmd=="help")
        {
            answer << "jp addr" << endl
                 << "set reg=value" << endl
                 << "b addr: add bp" << endl
                 << "d addr: remove bp" << endl
                 << "[n]ext, [c]ontinue" << endl
                 << "[s]tep, r reset" << endl
                 << "?[p]rint {expr}" << endl
                 << "[u]ntil {expr}" << endl
                 << "[w]while {expr}" << endl
                  ;
        }
        else if (cmd=="while" or cmd=="w" or cmd=="until" or cmd=="u")
        {
            string expr=s;
            exprtype result;
            if (parseExpression(computer->cpu, s, result))
            {
                answer << "running " << cmd << expr << endl;
                if (cmd=="while")
                    computer->cpu->setWhile(expr);
                else
                    computer->cpu->setUntil(expr);
            }
            else {
                error << "Error in expression " << expr << endl;
            }
        }
        else if (cmd=="p" or cmd=="print" or cmd=="?")
        {
            string expr(s);
            exprtype result;
            if (parseExpression(computer->cpu, s, result))
            {
                answer << expr << " = " << result
                << ", " << showbase << hex << result << dec << endl;
            }
            else
            {
                error << "Error in expression (" << expr << ")" << endl;
            }
        }
        else if (cmd=="set" or (cmd.find('=')!=string::npos))
        {
            if (cmd.find('=')!=string::npos) { s=cmd; cmd="set"; }
            string reg=getword(s,'=');
            cout << " reg=(" << reg << ") s=(" << s << ")" << endl;
            int32_t value;
            if (parseExpression(computer->cpu, s, value) && computer->cpu->regs()->set(reg, value))
            {
                answer << "SET " << reg  << '=' << value << endl;
            }
            else
            {
                error << "Unknown error (set " << reg << " ...)";
            }
        }
        else if (cmd=="reset")
        {
            computer->cpu->reset();
        }
        else if (cmd=="n")
        {
            int32_t count(1);
            if (s.length())
            {
                if (!parseExpression(computer->cpu, s, count))
                {
                    error << "Invalid expression" << endl;
                    count=0;
                }

            }
            computer->cpu->run_steps(count);
        }
        else if (cmd=="s")
            computer->cpu->step();	// TODO
        else if (cmd=="c")
            computer->cpu->start();
        else
        {
            error << "Unknown command (" << cmd << ")" << endl;
        }
    }
    if (error.str().length())
    {
        result->setStyleSheet("color: red;");
        result->setText(error.str().c_str());
    }
    if (answer.str().length())
    {
        result->setStyleSheet("color: black;");
        result->setText(answer.str().c_str());
    }
    cmdLine->setText("");
}
