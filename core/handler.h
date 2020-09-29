#ifndef HANDLER_H
#define HANDLER_H

#include <QMenu>
#include <core/mainwindow.h>

class Handler;
class MainWindow;

class Handlers
{
public:
    Handlers() = delete;

    static void registerHandler(Handler*);
    static void unregisterHandler(const Handler*);

    static void initialize(MainWindow*);

private:
    using container=std::multimap<std::string, Handler*>;
    static container* handlers;
};

class Handler
{
public:
    Handler(const std::string& name);

    const std::string name() const { return name_; }
    virtual ~Handler();

    virtual void initialize(MainWindow*) {};

private:
    std::string name_;
};

#endif // HANDLER_H
