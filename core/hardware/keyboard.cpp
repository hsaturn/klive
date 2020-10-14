#include "keyboard.h"

#include <QApplication>

#include <QEvent>
#include <QKeyEvent>

#include <iostream>
#include <iomanip>
#include <bitset>
#include <set>
#include <core/hardware/cpu.h>
#include <core/hardware/memory.h>
#include <QFile>
#include <QTextStream>
#include <common/expr.h>
#include <sstream>

using namespace std;

namespace hw
{

Keyboard::Keyboard(Cpu* cpu_) : cpu(cpu_)
{
    size_t line_nr=0;
    exprtype addr;
    exprtype port;

    std::string filename=":/keyboard.txt";

    QApplication::instance()->installEventFilter(this);
    QFile file(filename.c_str());
    if (file.open(QFile::ReadOnly))
    {
        QTextStream stream(&file);
        while(!stream.atEnd())
        {
            line_nr++;
            string line = stream.readLine().toStdString();

            while(line[0]==' ' || line[0]=='\t') line.erase(0,1);
            if (line[0]=='#') continue;
            if (line[0]=='\'') continue;
            if (line.length()==0) continue;

            Ports* vports;

            std::string lex=getlex(line);

            if (lex.length()==1)	// Ascii
            {
                vports=&ascii[lex[0]];
            }
            else
            {
                static Ports dummy;
                vports=&dummy;	// TODO
            }

            while (line.length())
            {
                std::stringstream error;
                lex=getlex(line);
                if (line[0]==':' && lex[0]=='0' && lex[1]=='x')	// addr:port
                {
                        line.erase(0,1);	// erase :
                        if (!parseExpression(line, port))
                        {
                            error << "Expected port number";
                        }
                        else if (parseExpression(lex, addr))
                        {
                            uint8_t value = ~ (1 << port);
                            vports->push_back({static_cast<Memory::addr_t>(addr), value});
                            if (ports.count(addr)==0)
                            {
                                ports[addr]=~0;	// Init to no key pressed
                                std::cout << "  Adding " << hex << addr << '=' << ports[addr] << dec << std::endl;
                            }
                        }
                        else
                        {
                            error << "Bad address" << endl;
                        }
                }
                else if (lex.length()==0)
                {
                    error << "syntax error (" << line << ")";
                    line="";
                }
                else
                {
                    error << "syntax error (" << lex << ")";
                    lex="";
                }
                if (error.str().length())
                {
                    std::cerr << "ERROR in " << filename << " at line " << line_nr << ":" << error.str() << std::endl;
                    line="";
                    error.clear();
                }
            }
        }
    }
    else
    {
        std::cerr << "Unable to open keyboard mapping file (" << filename << ")" << std::endl;
    }

    std::cout << "Keyboard mapping : " << ports.size() << " mapping(s)" << std::endl;
}

Keyboard::~Keyboard()
{
    QApplication::instance()->removeEventFilter(this);
}

bool Keyboard::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* key=dynamic_cast<QKeyEvent*>(event);

            std::cout << endl;
            // key() is ascii if possible else key code
            // -> Left and Right (ctrl or shift) have the same code
            const auto& it=ascii.find(key->key());
            if (it != ascii.end())
            {
                for(const Port& port: it->second)
                {
                    ports[port.addr] &= port.value;
                    // std::cout << "setting " << hex << showbase << port.addr << " &= " << (uint16_t)port.value
                    //          << " = " << (uint16_t) ports[port.addr] << dec << std::endl;
                }
            }
            else
            {
                std::cout << "Key not mapped:" << key->key() << ' ';
                std::cout << "Key " << key->key() << ' ';
                std::cout << "Virtyual " << key->nativeVirtualKey() << ' ';
                std::cout << "Native " << key->nativeScanCode() << ' ';
                std::cout << "Mod: " << bitset<32>(key->nativeModifiers()) << ' ';
                std::cout << std::endl;
            }

    }
    if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent* key=dynamic_cast<QKeyEvent*>(event);
            const auto& it=ascii.find(key->key());
            if (it != ascii.end())
            {
                for(const Port& port: it->second)
                {
                    ports[port.addr] |= ~port.value;
                    // std::cout << "resetting " << hex << showbase << port.addr << " |= " << (uint16_t)port.value
                    //          << " = " << (uint16_t) ports[port.addr] << dec << std::endl;
                }
            }
    }
    return QObject::eventFilter(object, event);
}

void Keyboard::update(Cpu *sender, const typename Cpu::Message &msg)
{
   static std::unordered_map<hw::Memory::addr_t, int> view;
   if (sender==cpu && msg.event==Cpu::Message::INPORT)
   {
       const auto &it = ports.find(msg.port->port);
       if (it != ports.end())
       {
         if (view[msg.port->port] != it->second)
         {
             view[msg.port->port] = it->second;
             // std::cout << "INPORT(" << hex << showbase << it->first << "=" << it->second << dec << endl;
         }
         msg.port->value = it->second;
       }
       else
       {
           // std::cout << "INPUT = ? " << hex << showbase << msg.port->port << " " << dec << endl;
           msg.port->value=~0;
       }
       view[msg.port->port] = msg.port->value;
   }
}


void Keyboard::observableDies(const Cpu *sender)
{
    if (sender==cpu)
    {
        cpu = nullptr;
    }
}
}
