// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "PC_Problem/Consumer.h"
// testnum is set in main.cc
int testnum = 2;

SharedData sharedData;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}



//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

/**
 * Producer consumer problem
 */


void producer(int id){
    printf("inside producer\n");
    Producer p;
    p.produceInteger();
}

void consumer(int id){
    printf("inside consumer\n");
    Consumer c;
    c.consumeInteger();
}

#define NUM_PRODUCER 3
#define NUM_CONSUMER 3

void ThreadTest2(){
    sharedData.lock = new Lock(NULL);
    sharedData.bedOfConsumer = new Condition("C");
    sharedData.bedOfProducer = new Condition("P");

    Thread* p[NUM_PRODUCER];
    Thread* c[NUM_CONSUMER];

    int x = NUM_PRODUCER > NUM_CONSUMER ? NUM_PRODUCER: NUM_CONSUMER ;

    for (int i = 0; i < x; ++i)
    {
        if(i < NUM_PRODUCER){
            p[i] =  new Thread("Producer");
            p[i]->Fork (producer, i);
        }
        if(i < NUM_CONSUMER){
            c[i] = new Thread("Consumer");
        c[i]->Fork(consumer, i);
        }

    }
    
}


/**
 * condition testing
 */
void ThreadTest3(){

}
//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
        case 1:
            ThreadTest1();
            break;
        case 2:
            ThreadTest2();
            break;
        default:
        printf("No test specified.\n");
        break;
    }
}

