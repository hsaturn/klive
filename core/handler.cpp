#include <handler.h>
#include <QMessageBox>
#include <QApplication>
#include <QObject>

#include <iostream>

Handlers::container* Handlers::handlers=nullptr;

void Handlers::registerHandler(Handler* handler)
{
    if (handlers == nullptr)
    {
        handlers = new container();
    }
    handlers->insert({handler->name(), handler});
}

void Handlers::unregisterHandler(const Handler *handler)
{
    for(container::iterator it = handlers->begin(); it!=handlers->end(); it++)
    {
        if (it->second==handler)
        {
            std::cout << "Unregistering handler '" << it->first << '\'' << ' ' << handler << std::endl;
            handlers->erase(it);
            break;
        }
    }
}

void Handlers::initialize(MainWindow *main)
{
    std::cout << "Initializing handlers" << std::endl;
    if (handlers == nullptr)
    {
        std::cout << "No handlers registered. Nothing will probably happen now." << std::endl;
        return;
    }
    for(auto it: *handlers)
    {
        std::cout << "  Initializing '" << it.second->name() << "'." << std::endl;
        it.second->initialize(main);
    }
}

Handler::Handler(const std::string& name) : name_(name)
{
    Handlers::registerHandler(this);
}

Handler::~Handler()
{
    Handlers::unregisterHandler(this);
}
