//
// Created by muktadir on 4/18/17.
//

#include "Producer.h"

void waitForSomeTime(int r){
    int x = 100000000 * r;
    for (int i = 0; i < x; ++i) ;
}

char str[] ="PC";

void Producer::produceInteger(){
    int product = 0;
    while (1){

        //produced some integer
        product++;
        printf("Producing %d\n", product);
        waitForSomeTime(3);

        //now time to put it into the buffer
        sharedData.lock->Acquire();

        if(sharedData.data.size()  >= SHARED_DATA_SIZE)
            sharedData.bedOfProducer->Wait(sharedData.lock);

        sharedData.data.push_back(product);

        sharedData.bedOfConsumer->Signal(sharedData.lock);

        sharedData.lock->Release();
    }
}