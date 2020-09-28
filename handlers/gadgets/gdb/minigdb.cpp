#include "minigdb.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <string>
#include <sstream>
#include <QPushButton>

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

bool readAddr(string &str, Memory::addr_t& addrout)
{
    string hexa=getword(str);

    int base=10;
    if (hexa[0]=='x')
    {
        base=16;
        hexa.erase(0,1);
    }
    else if (hexa.length()>>2 && hexa.substr(0,2)=="0x")
    {
        hexa.erase(0,2);
        base=16;
    }
    try
    {
        if (hexa.length())
        {
            addrout=stoul(hexa, nullptr, base);
            cout << "hexa(" << hexa << ")=(" << addrout << ")" << endl;
            return true;
        }
    }
    catch(...)
    {

    }
    return false;
}

bool readValue(string &value, int32_t &valout)
{
    bool neg=false;
    if (value.length() && value[0]=='-')
    {
        neg=true;
        value.erase(0,1);
    }

    uint32_t addr;
    if (readAddr(value, addr))
    {
        if (neg)
            valout=-static_cast<int32_t>(addr);
        else {
            valout=addr;
        }
        return true;
    }
    return false;
}

void MiniGdb::cpuStep()
{
    computer->cpu->step();
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
        cmd=getword(s);
        cout << "CMD " << cmd << endl;
        if (cmd=="b" || cmd=="d")
        {
            Memory::addr_t addr;
            if (readAddr(s, addr))
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
            if (readAddr(s, addr))
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
                 << "n next, c continue" << endl
                 << "s stop, r reset" << endl;
        }
        else if (cmd=="set" or (cmd.find('=')!=string::npos))
        {
            if (cmd.find('=')!=string::npos) { s=cmd; cmd="set"; }
            string reg=getword(s,'=');
            cout << " reg=(" << reg << ") s=(" << s << ")" << endl;
            int32_t value;
            if (readValue(s, value) && computer->cpu->regs()->set(reg, value))
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
            computer->cpu->run_step();
        else if (cmd=="s")
            computer->cpu->stop();
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
