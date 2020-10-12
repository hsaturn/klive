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

using namespace std;

namespace hw
{

Keyboard::Keyboard(Cpu* cpu_) : cpu(cpu_)
{
    QApplication::instance()->installEventFilter(this);
}

Keyboard::~Keyboard()
{
    QApplication::instance()->removeEventFilter(this);
}

static bool a=false;

bool Keyboard::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* key=dynamic_cast<QKeyEvent*>(event);

        if (key->key()==65)
        {
            std::cout << "Key " << key->key() << ' ';
            std::cout << "Virtyual " << key->nativeVirtualKey() << ' ';
            std::cout << "Native " << key->nativeScanCode() << ' ';
            std::cout << "Mod: " << bitset<32>(key->nativeModifiers()) << ' ';
            std::cout << endl;
            // key() is ascii if possible else key code
            // -> Left and Right (ctrl or shift) have the same code
            a=key->key()==65;
        }

    }
    if (event->type() == QEvent::KeyRelease)
    {
        a=false;
    }
    return QObject::eventFilter(object, event);
}

void Keyboard::update(Cpu *sender, const typename Cpu::Message &msg)
{
   static set<hw::Memory::addr_t> view;
   if (sender==cpu && msg.event==Cpu::Message::INPORT)
   {
       if (view.count(msg.port->port)==0)
       {
           view.insert(msg.port->port);
           std::cout << hex << showbase << "INPORT " << msg.port->port << dec << std::endl;
       }
       if (msg.port->port==0xFDFE && a)
          msg.port->value = ~1;
       else
          msg.port->value = ~0;
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
