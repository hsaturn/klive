#include "console.h"
#include <iostream>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QScrollBar>
#include <common/expr.h>

Console::Console()
{
    QVBoxLayout* layout = new QVBoxLayout;
    lineEdit_ = new QLineEdit;
    textEdit_ = new QTextEdit;
    connect(lineEdit_, SIGNAL(returnPressed()), this, SLOT(onCmdLine()));
    layout->addWidget(textEdit_);
    layout->addWidget(lineEdit_);
    setLayout(layout);

    registerHelp("console" , {{ "help", "Display modules commands help" }});
}

void Console::onCmdLine()
{
    static std::string lastCmd;

    std::string s=lineEdit_->text().toStdString();
    if (s=="")
        s = lastCmd;
    else
        lastCmd = s;

    Message msg;

    std::string cmd=getlex(s);
    std::string ambiguous;
    std::string found;

    for(auto [ module, help ] : help_)
    {
        for(auto [ command_in, text] : help)
        {
            std::string command = command_in;
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
        error("Ambiguous command : " + cmd);
        error("Candidates: " + ambiguous);
        return;
    }

    if (found.length())
        msg.data = found + ' ' +s;
    else
        msg.data = cmd + ' ' + s;

    update(msg);	// self execution
    notify(msg);

    if (msg.processed == 0)
        error("Unknown command: "+msg.data);
    if (msg.error.str().length())
        error(msg.error.str());
    if (msg.out.str().length())
        append(msg.out.str());

    textEdit_->setPlainText(textEdit_->toPlainText()+msg.out.str().c_str());
    lineEdit_->setText("");
}

void Console::append(std::string text, std::string color)
{
    textEdit_->setTextColor(QColor(color.c_str()));
    textEdit_->append(text.c_str());
   // timer_.singleShot(1, this, &Console::gotoend);
    textEdit_->scroll(0,10);
}

void Console::gotoend()
{
    QTextCursor cursor = textEdit_->textCursor();
    cursor.movePosition(QTextCursor::End);
    textEdit_->setTextCursor(cursor);
    std::cout << "curpos: " << cursor.position() << std::endl;
    textEdit_->ensureCursorVisible();
    textEdit_->update();
    textEdit_->verticalScrollBar()->setValue(textEdit_->verticalScrollBar()->minimum());

    // textEdit_->moveCursor(QTextCursor::End);
    textEdit_->ensureCursorVisible();
}

void Console::registerHelp(std::string module, HelpContainer helps)
{
    help_[module] = helps;
}

void Console::help(std::string what)
{
    if (help_.find(what) != help_.end())
    {
        displayHelp(what, help_[what]);
        return;
    }
    for(auto [ module , help ] : help_)
    {
        displayHelp(module, help);
        append("");
    }
}

void Console::displayHelp(std::string module, HelpContainer help)
{
    append(module, "green");
    for(auto [cmd, text ]: help)
        append(cmd+" : " + text);
}

void Console::update(const Console::Message& msg)
{
    std::string line = msg.data;
    std::string cmd = getlex(line);

    if (cmd == "clear")
    {
        textEdit_->clear();
        msg.processed++;
    }
    else if (cmd == "help")
    {
        help(line);
        msg.processed++;
    }
}
