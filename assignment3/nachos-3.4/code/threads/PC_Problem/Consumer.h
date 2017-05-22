//
// Created by muktadir on 4/18/17.
//

#ifndef CODE_CONSUMER_H
#define CODE_CONSUMER_H

#include "../synch.h"
#include "Producer.h"

extern SharedData sharedData;
class Consumer{
public:
    void consumeInteger();
};

#endif //CODE_CONSUMER_H
