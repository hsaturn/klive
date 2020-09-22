#pragma once
#include <list>

template<class T>
class Observer
{
public:
    Observer(){}
    virtual ~Observer(){}

    virtual void update(T* sender, typename T::Message* msg)=0;
};

template<class T>
class Observable
{
public:
    Observable(){}
    virtual ~Observable(){}

    void attach(Observer<T>* observer)
    {
        if (observers.find(observer)==observers.end())
        {
            observers.push_front(observer);
        }
    }

    void detach(Observer<T>* observer)
    {
        observers.remove(observer);
    }

    template<class Message>
    void notify(Message* msg)
    {
        for(const auto& observer: observers)
        {
            observer->update(static_cast<T*>(this), msg);
        }
    }

private:
    std::list<Observer<T>*> observers;
};
