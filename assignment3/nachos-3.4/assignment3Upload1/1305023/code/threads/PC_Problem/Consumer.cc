//
// Created by muktadir on 4/18/17.
//
#include "Consumer.h"


void Consumer::consumeInteger(){
    int product;
    while (1) {
        ///take data from the list to consume
        sharedData.lock->Acquire();
        printf("A\n" );
        while (sharedData.data.size() < 1)
            sharedData.bedOfConsumer->Wait(sharedData.lock);
        
        product = sharedData.data.front();
        sharedData.data.pop_front();
        sharedData.bedOfProducer->Signal(sharedData.lock);
        sharedData.lock->Release();

        printf("Consuming: %d\n", product);
        waitForSomeTime(2);
    }
}
