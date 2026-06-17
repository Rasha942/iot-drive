#include "ai.hpp"
#include "scheduler.hpp"
#include "singleton.hpp"

AI::AITask::AITask(AI& ai): m_ai(ai) {}

void AI::AITask::Execute() { m_ai.Perform();}


AI::AI(std::function<bool()> func, std::chrono::milliseconds timeToCheck): 
                                                m_aiTask(std::make_shared<AI::AITask>(*this)), 
                                                m_func(func), m_timeToCheck(timeToCheck)
{
    Scheduler* scd = Singleton<Scheduler>::GetInstance();
    scd->AddTask(m_aiTask, m_timeToCheck);
}

void AI::Perform()
{
    if (false == m_func()) 
    {
        Scheduler* scd = Singleton<Scheduler>::GetInstance();
        scd->AddTask(m_aiTask, m_timeToCheck);
    } 
    else 
    {
        delete  this;
    }
}
