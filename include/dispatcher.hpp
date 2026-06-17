#ifndef __DISPATCHER_HPP__
#define __DISPATCHER_HPP__
#define TPS
#define CALL_MEMBER_FN(object,ptrToMember) ((object).*(ptrToMember))

#include <cstddef>
#include <list>
template <class EVENT>
class ACallback;

template<class EVENT>
class Dispatcher
{
public:
    Dispatcher() = default;
    ~Dispatcher();
    void Register(ACallback<EVENT>* callback);
    void Unregister(ACallback<EVENT>* callback);
    void Notify(const EVENT& event);
private:
    std::list<ACallback<EVENT>*> m_callback_list;
};
template<class EVENT>
Dispatcher<EVENT>::~Dispatcher(){for (auto i : m_callback_list){i->NotifyDeath();}}

template<class EVENT>
void Dispatcher<EVENT>::Register(ACallback<EVENT>* callback){m_callback_list.push_back(callback); callback->SetDispatcher(this);}

template<class EVENT>
void Dispatcher<EVENT>::Unregister(ACallback<EVENT>* callback){m_callback_list.remove(callback);}

template<class EVENT>
void Dispatcher<EVENT>::Notify(const EVENT& event)
{
    std::list<ACallback<EVENT>*> temp = m_callback_list;
    for (auto i : temp)
    {
        i->Notify(event);
    }
}
template <class EVENT>
class ACallback
{
public:
    ACallback();
    virtual ~ACallback();
    virtual void Notify(const EVENT& event) =0;
    virtual void NotifyDeath();
    virtual void SetDispatcher(Dispatcher<EVENT>* dispatcher_);

private:
    Dispatcher<EVENT>* m_dispatcher;
};
template <class EVENT>
ACallback<EVENT>::ACallback():m_dispatcher(NULL)
{

}

template <class EVENT>
ACallback<EVENT>::~ACallback(){if(m_dispatcher!= NULL){m_dispatcher->Unregister(this);}}
template <class EVENT>
void ACallback<EVENT>::NotifyDeath(){m_dispatcher = NULL;}
template <class EVENT>
void ACallback<EVENT>::SetDispatcher(Dispatcher<EVENT>* dispatcher_){m_dispatcher = dispatcher_;}

template<class Observer, class EVENT>
class Callback:public ACallback<EVENT>
{
public:
    Callback(Observer& observer, void (Observer::*func)(const EVENT&), void (Observer::*func_death)() = NULL);
    virtual ~Callback();
    virtual void Notify(const EVENT& event) override;
    virtual void NotifyDeath() override;
private:
    Observer& m_observer;
    void (Observer::*m_func_ptr)(const EVENT&);
    void (Observer::*m_func_death)();

};

template<class Observer, class EVENT>
Callback<Observer,EVENT>::Callback(Observer& observer, void (Observer::*func_ptr)(const EVENT&),void (Observer::*func_death)()):m_observer(observer),m_func_ptr(func_ptr),m_func_death(func_death)
{

}

template<class Observer, class EVENT>
Callback<Observer,EVENT>::~Callback()
{

}

template<class Observer, class EVENT>
void Callback<Observer,EVENT>::Notify(const EVENT& event)
{
    CALL_MEMBER_FN(m_observer, m_func_ptr)(event);
}
template<class Observer, class EVENT>
void Callback<Observer,EVENT>:: NotifyDeath()
{
    if(NULL != m_func_death)
    {
        CALL_MEMBER_FN(m_observer, m_func_death)();
    }
    ACallback<EVENT>::NotifyDeath();

  
}



#endif