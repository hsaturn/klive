#include "minigdb.h"
#include <QVBoxLayout>
#include <QLabel>
#include <string>

using namespace std;

MiniGdb::MiniGdb(Computer* computer)
    : computer(computer)
{
    QVBoxLayout* layout=new QVBoxLayout;

    cmdLine = new QLineEdit;
    connect(cmdLine, SIGNAL(returnPressed()), this, SLOT(onCmdLine()));

    layout->addWidget(cmdLine);
    result = new QLabel;
    layout->addWidget(result);
    setLayout(layout);
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
    string error;
    string answer;

    if (s.length())
    {
        result->setText("");
        if (s[0]=='b' || s[0]=='d')
        {
            string hexa=s.substr(1);
            while(hexa.length() && hexa[0]==' ') hexa.erase(0,1);
            int base=10;
            if (hexa.length())
            {
                if (hexa.substr(0,2)=="0x") base=16;
                Memory::addr_t addr=stoul(hexa, nullptr, base);
                if (s[0]=='b')
                    computer->cpu->breaks.add(addr);
                else
                    computer->cpu->breaks.remove(addr);
                answer="ok "+s;
            }
            else
            {
                error="Missing argument";
            }
        }
        else if (s[0]=='n')
            computer->step();
        else if (s[0]=='s')
            computer->stop();
        else if (s[0]=='c')
            computer->start();
        else
        {
            error="Unknown command";
        }
        if (error.length())
        {
            result->setStyleSheet("color: red;");
            result->setText(error.c_str());
        }
        else if (answer.length())
        {
            result->setStyleSheet("color: black;");
            result->setText(answer.c_str());
        }
    }
    cmdLine->setText("");
}
