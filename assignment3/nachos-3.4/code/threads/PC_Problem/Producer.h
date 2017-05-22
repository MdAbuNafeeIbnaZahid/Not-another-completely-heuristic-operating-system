//
// Created by muktadir on 4/18/17.
//

#ifndef CODE_PRODUCER_H
#define CODE_PRODUCER_H

#include <list>
#include "../synch.h"
using namespace std;
void waitForSomeTime(int );

#define SHARED_DATA_SIZE 15


struct SharedData{
    list<int> data;

    Lock* lock;

    Condition* bedOfProducer;
    Condition* bedOfConsumer;

};

extern SharedData sharedData;

class Producer{
public:
    void produceInteger();
};

#endif //CODE_PRODUCER_H
