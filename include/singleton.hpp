#ifndef _SINGLETON_HPP_
#define _SINGLETON_HPP_

#include <cstdlib>  /* atexit */
#include <cstring>
#include <unistd.h>

// #ifdef HANDLETON
#include <functional>
#include <mutex>
#include <atomic>
// #endif

#define ATOMIC_PTR std::atomic<T*> /* std::atomic<std::shared_ptr<T>> */

template <class T, class... CtorArgs>
class Singleton
{
public:
    Singleton() = delete;
    ~Singleton() = default;
    Singleton(const Singleton& other_) = delete;
    Singleton& operator=(const Singleton& other_) = delete;

    static T* GetInstance(CtorArgs... args);

private:
// #ifdef HANDLETON  
    static void Dtor() { if(m_instance != nullptr){delete m_instance;} }
    static T* Init(CtorArgs... args);
    static T* Supply();
    static std::function<T*(CtorArgs...)> m_func_init;
    static std::function<T*()> m_func_supply;
    static bool m_supply_flag;
    static ATOMIC_PTR m_instance;
    static std::mutex m_mutex;
// #endif
};

// #ifdef HANDLETON

template <class T, class... CtorArgs>
std::function<T*(CtorArgs...)> Singleton<T, CtorArgs...>::m_func_init = [](CtorArgs... args) { return Init(args...); };

template <class T, class... CtorArgs>
std::function<T*()> Singleton<T, CtorArgs...>::m_func_supply = []() { return Supply(); };


template <class T, class... CtorArgs>
bool Singleton<T, CtorArgs...>::m_supply_flag = false;
template <class T, class... CtorArgs>
ATOMIC_PTR Singleton<T, CtorArgs...>::m_instance;

template <class T, class... CtorArgs>
std::mutex Singleton<T, CtorArgs...>::m_mutex;

template <class T, class... CtorArgs>
T* Singleton<T, CtorArgs...>::GetInstance(CtorArgs... args)
{
    return m_supply_flag == false ? m_func_init(args...): m_func_supply();
}

template <class T, class... CtorArgs>
T* Singleton<T, CtorArgs...>::Init(CtorArgs... args)
{
    T* temp = m_instance.load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);

    if (NULL == temp)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        temp = m_instance.load(std::memory_order_relaxed);
        if (NULL == temp)
        {
            temp = new T(args...); 
            std::atomic_thread_fence(std::memory_order_release);
            m_instance.store(temp, std::memory_order_relaxed);
            // m_func_init = Supply;
            m_supply_flag = true;
            std::atexit(Dtor);
        }
    }
    return temp;
}

template <class T, class... CtorArgs>
T* Singleton<T, CtorArgs...>::Supply()
{
    return m_instance.load(std::memory_order_relaxed);
}

#endif

// #endif