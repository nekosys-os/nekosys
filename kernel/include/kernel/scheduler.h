#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <kernel/interrupts.h>
#include <kernel/thread.h>
#include <nk/vector.h>
#include <nk/singleton.h>

namespace Kernel
{

    class Scheduler : public InterruptHandler
    {
        DECLARE_SINGLETON(Scheduler)

    private:
        void *tss;
        nk::Vector<Thread *> threads;
        int ctr = 0;

    public:
        void *GetTssPtr();

        void Start(Thread *thread);

        void HandleInterrupt(unsigned int interrupt, RegisterStates *regs) override;

        void Yield();

        void PreemptCurrent(RegisterStates *regs);

    private:
        void Initialize();

        void ContextSwitch(Thread *oldThread, Thread *newThread, RegisterStates *regs);

        Thread *FindNextThread();

        bool CanRun(Thread *thread);
    };

}; // namespace Kernel

#endif