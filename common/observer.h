#pragma once
#include <list>
#include <algorithm>

template<class T>
class Observer
{
public:
    Observer(){}
    virtual ~Observer(){}

    virtual void update(T* sender, const typename T::Message& msg)=0;
    virtual void observableDies(const T* sender) = 0;
};

template<class T>
class Observable
{
public:
    Observable(){}
    virtual ~Observable()
    {
        // TODO should use walkOnObservers(lamda)
        for(auto& observer: observers)
            observer->observableDies(static_cast<const T*>(this));
    }

    void attach(Observer<T>* observer)
    {
        for(const auto& it: observers)
            if (it==observer)
                return;

        observers.push_front(observer);
    }

    void detach(Observer<T>* observer)
    {
        for(auto it=observers.begin(); it!=observers.end(); it++)
            if (*it==observer)
            {
                observers.erase(it);
                return;
            }
    }

    template<class Message>
    void notify(const Message& msg)
    {
        for(const auto& observer: observers)
        {
            observer->update(static_cast<T*>(this), msg);
        }
    }

private:
    std::list<Observer<T>*> observers;
};
