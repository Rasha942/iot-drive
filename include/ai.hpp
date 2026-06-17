#ifndef __ASYNC_INGECTION__
#define __ASYNC_INGECTION__

#include "scheduler.hpp"
#include <functional>
#include <memory>

class AI            
{
    public:
        AI(std::function<bool()> func, std::chrono::milliseconds timeToCheck);
        void Perform();
    
    private:
        ~AI() = default;

        class AITask: public Scheduler::ISchedTask
            {
                public:
                    AITask(AI& ai);
                    virtual void Execute(); 

                private:
                    AI& m_ai;
        };

        std::shared_ptr<AITask> m_aiTask;
        std::function<bool()> m_func;
        std::chrono::milliseconds m_timeToCheck;
};

#endif
