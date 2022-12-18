// scheduler.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "debug.h"
#include "scheduler.h"
#include "main.h"

//----------------------------------------------------------------------
// Compare function for ready list.
//----------------------------------------------------------------------

int SJF_cmp(Thread* t1, Thread* t2) {
	double bt1=t1->getBurstTime(), bt2=t2->getBurstTime();
	if(bt1==bt2)
		return 0;
	return bt1<bt2?-1:1;
}

int Priority_cmp(Thread* t1, Thread* t2) {
	int p1=t1->getPriority(), p2=t2->getPriority();
	if(p1==p2)
		return 0;
	return p1>p2?-1:1;
}

int RR_cmp(Thread* t1, Thread* t2) {
	return 1;
}

void Jump(Thread* t) {
	Scheduler* scheduler = kernel->scheduler;
	int p=t->getPriority(), op=p-10;
	if(op>=50&&op<=99) {
		if(p>=100&&p<=149) {
			scheduler->L2->Remove(t);
			DEBUG(dbgSchedule, "Tick [" << kernel->stats->totalTicks << "]: Thread [" << t->getID() << "] is removed from queue L[2]");
			scheduler->L1->Append(t);
			DEBUG(dbgSchedule, "Tick [" << kernel->stats->totalTicks << "]: Thread [" << t->getID() << "] is inserted into queue L[1]");
		} else {
			scheduler->L2->Remove(t);
			scheduler->L2->Append(t);
		}
	} else if(op>=0&&op<=49) {
		if(p>=50&&p<=99) {
			scheduler->L3->Remove(t);
			DEBUG(dbgSchedule, "Tick [" << kernel->stats->totalTicks << "]: Thread [" << t->getID() << "] is removed from queue L[3]");
			scheduler->L2->Append(t);
			DEBUG(dbgSchedule, "Tick [" << kernel->stats->totalTicks << "]: Thread [" << t->getID() << "] is inserted into queue L[2]");
		}
	}
}

void UpdateWaitingTime(Thread* t) {
	int currentTime = kernel->stats->totalTicks;
	t->setWaitingTime(t->getWaitingTime()+currentTime-t->getStartWaitingTime());
	int waitingTime = t->getWaitingTime();
	if(waitingTime>=1500) {
		int p=t->getPriority();
		if(p+10<=149)
			t->setPriority(p+10);
		else
			t->setPriority(149);
		DEBUG(dbgSchedule, "Tick [" << kernel->stats->totalTicks << "]: Thread [" << t->getID() << "] changes its priority from [" << p << "] to [" << t->getPriority() << "]");
		t->setWaitingTime(waitingTime-1500);
		Jump(t);
	}
	t->setStartWaitingTime(currentTime);
}

void Scheduler::Aging() {
	L1->Apply(UpdateWaitingTime);
	L2->Apply(UpdateWaitingTime);
	L3->Apply(UpdateWaitingTime);
}

bool Scheduler::CheckIfPreempt() {
	Thread* currentThread = kernel->currentThread;
	int p=currentThread->getPriority();
	if(p>=0&&p<=49) {
		if(L1->IsEmpty()==false || L2->IsEmpty()==false || L3->IsEmpty()==false)
			return true;
		else
			return false;
	} else if(p>=50&&p<=99) {
		if(L1->IsEmpty()==false)
			return true;
		else
			return false;
	} else if(p>=100&&p<=149) {
		if(L1->IsEmpty()==false && L1->Front()->getBurstTime()<currentThread->getBurstTime())
			return true;
		else
			return false;
	}
	return false;
}

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------

Scheduler::Scheduler()
{ 
	L1 = new SortedList<Thread*>(SJF_cmp);
	L2 = new SortedList<Thread*>(Priority_cmp);
	L3 = new SortedList<Thread*>(RR_cmp);
    toBeDestroyed = NULL;
} 

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{ 
	delete L1;
	delete L2;
	delete L3;
} 

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void
Scheduler::ReadyToRun (Thread *thread)
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    DEBUG(dbgThread, "Putting thread on ready list: " << thread->getName());

    thread->setStatus(READY);
	thread->setStartWaitingTime(kernel->stats->totalTicks);

	int p=thread->getPriority();
	if(p>=0&&p<=49) {
		L3->Append(thread);
		DEBUG(dbgSchedule, "Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is inserted into queue L[3]");
	}
	else if(p>=50&&p<=99) {
		L2->Append(thread);
		DEBUG(dbgSchedule, "Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is inserted into queue L[2]");
	}
	else if(p>=100&&p<=149) {
		L1->Append(thread);
		DEBUG(dbgSchedule, "Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is inserted into queue L[1]");
	}
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun ()
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);

	if(L1->IsEmpty()==false) {
		DEBUG(dbgSchedule, "Tick [" << kernel->stats->totalTicks << "]: Thread [" << L1->Front()->getID() << "] is removed from queue L[1]");
		return L1->RemoveFront();
	}
	else if(L2->IsEmpty()==false) {
		DEBUG(dbgSchedule, "Tick [" << kernel->stats->totalTicks << "]: Thread [" << L2->Front()->getID() << "] is removed from queue L[2]");
		return L2->RemoveFront();
	}
	else if(L3->IsEmpty()==false) {
		DEBUG(dbgSchedule, "Tick [" << kernel->stats->totalTicks << "]: Thread [" << L3->Front()->getID() << "] is removed from queue L[3]");
		return L3->RemoveFront();
	}
	else
		return NULL;
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable kernel->currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//	"finishing" is set if the current thread is to be deleted
//		once we're no longer running on its stack
//		(when the next thread starts running)
//----------------------------------------------------------------------

void
Scheduler::Run (Thread *nextThread, bool finishing)
{
    Thread *oldThread = kernel->currentThread;
	Statistics* stats = kernel->stats;
    
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if (finishing) {	// mark that we need to delete current thread
		ASSERT(toBeDestroyed == NULL);
	 	toBeDestroyed = oldThread;
    }
    
    if (oldThread->space != NULL) {	// if this thread is a user program,
        oldThread->SaveUserState(); 	// save the user's CPU registers
		oldThread->space->SaveState();
    }
    
    oldThread->CheckOverflow();		    // check if the old thread
					    // had an undetected stack overflow

    kernel->currentThread = nextThread;  // switch to the next thread
    nextThread->setStatus(RUNNING);      // nextThread is now running
	nextThread->setStartTime(stats->totalTicks);
	nextThread->setWaitingTime(0);
    
    DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".

    SWITCH(oldThread, nextThread);

    // we're back, running oldThread
      
    // interrupts are off when we return from switch!
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    DEBUG(dbgThread, "Now in thread: " << oldThread->getName());

    CheckToBeDestroyed();		// check if thread we were running
					// before this one has finished
					// and needs to be cleaned up
    
    if (oldThread->space != NULL) {	    // if there is an address space
        oldThread->RestoreUserState();     // to restore, do it.
		oldThread->space->RestoreState();
    }
}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
// 	If the old thread gave up the processor because it was finishing,
// 	we need to delete its carcass.  Note we cannot delete the thread
// 	before now (for example, in Thread::Finish()), because up to this
// 	point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void
Scheduler::CheckToBeDestroyed()
{
    if (toBeDestroyed != NULL) {
        delete toBeDestroyed;
		toBeDestroyed = NULL;
    }
}
 
//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void
Scheduler::Print()
{
    cout << "Ready list contents:\n";
    readyList->Apply(ThreadPrint);
}
