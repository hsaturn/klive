#pragma once
#include <sstream>
#include <map>

#include <common/observer.h>
#include <QLineEdit>
#include <QTextEdit>
#include <QTimer>

class Console : public QWidget, public Observable<Console>
{
    Q_OBJECT

public:
    Console();

    struct Message
    {
        std::string data;

        mutable std::stringstream error;
        mutable std::stringstream out;

        // observers should increment this if processed
        mutable uint8_t processed=0;
    };

    void update(const Console::Message&);

    void append(std::string, std::string color="black");
    void error(std::string err) { append(err, "red"); }

    using HelpContainer = std::map<std::string, std::string>;
    void registerHelp(std::string command, HelpContainer);

public slots:
    void onCmdLine();
    void gotoend();

private:
    void help(std::string what);
    void displayHelp(std::string /* module */, HelpContainer);

    QLineEdit* lineEdit_ = nullptr;
    QTextEdit* textEdit_ = nullptr;
    QTimer timer_;
    std::map<std::string, HelpContainer> help_;
};
