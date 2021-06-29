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
        std::map<std::string, int> str_to_key=
                {{"enter", 16777220},
                 {"space", 32},
                 {"back", 16777219},
                 {"altgr", 16777251},
                 {"lshift", 16777248}};	// TODO Modifier is not taken in account

        std::map<std::string, std::string> equ;
        QTextStream stream(&file);
        while(!stream.atEnd())
        {
            line_nr++;
            string line = stream.readLine().toStdString();

            while(line[0]==' ' || line[0]=='\t') line.erase(0,1);
            if (line.length()==0) continue;

            Ports* vports;

            std::string lex=getlex(line);
            if (lex=="//")
            {
                continue;
            }
            else if (line[0]=='=')
            {
                line.erase(0,1);
                std::cout << "  equ " << lex << '=' << line << std::endl;
                equ[lex]=line;
            }
            else
            {
                if (lex.length()==1)	// Ascii
                {
                    vports=&ascii[lex[0]];
                }
                else if (str_to_key.count(lex))
                {
                    vports=&ascii[str_to_key[lex]];
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
                    if (equ.count(lex))
                    {
                        lex=equ[lex];
                    }

                    if (lex=="//")
                    {
                        continue;
                    }
                    else if (line[0]==':' && lex[0]=='0' && lex[1]=='x')	// addr:port
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
   if (sender != cpu) return;

   if (msg.event==Cpu::Message::INPORT)
   {
       const auto &it = ports.find(msg.port->port);
       if (it != ports.end())
       {
         msg.port->value = it->second;
         if (msg.port->value != 255)
         {
            std::cout << "in " << msg.port->port << "=" << msg.port->value << std::endl;
         }
         msg.port->filled = true;
       }
   }
   else if (msg.event == Cpu::Message::MACROSTEP)
   {
       // check shift key

   }
   else if (msg.event == Cpu::Message::RESET)
   {
       for(auto& port: ports)
       {
           std::cout << "Resetting kbd #" << hex << port.first << dec << std::endl;
           port.second = ~0;
       }
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
