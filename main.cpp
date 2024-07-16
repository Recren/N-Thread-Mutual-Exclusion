#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include <stdio.h>
#include <atomic>
#include <thread>
#include <iostream>


//Global Variables for threads to access

std::atomic_int token;//Used for fetchAndIncrement
std::atomic_int turn;//Used for fetchAndIncrement
std::atomic_flag lock = ATOMIC_FLAG_INIT;//Used for testAndSet
int counter;


void testAndSet(){

    while (lock.test_and_set()){}

    //CRITCAL SECTION
    for(int i = 0; i < 10000000; i++){
        counter += 1;
    }
    //Release lock
    lock.clear();
}

void fetchAndIncrement(){

    //Each thread will have its own turn variable.
    //fetch_add to request the lock and increment the value all in one
    int myTurn = token.fetch_add(1);

    //Busy spin while it is not a threads turn
    while(myTurn != turn){}

    //CRITCAL SECTION
    for(int i = 0; i < 10000000; i++){
        counter += 1;
    }
    //Release lock
    turn += 1;

}

int main(){

    int algorithm;
    int numThreads;
    printf("What algorithm would you like to use\n0: TT\n1: TAS\n2: FAI\n");
    std::cin >> algorithm;


    printf("How many threads: ");
    std::cin >> numThreads;

    //Create an array to keep track of threads
    std::thread threadTracker[numThreads];
    
    //Initalize the global variables
    counter = 0;
    token = 0;
    turn = 0;

    if(algorithm == 0){
        printf("TT");
    }
    else if(algorithm == 1){

        // //Create the amount of threads specified and send them to function with the sharedData
        for(int i = 0; i < numThreads; i++){
            threadTracker[i] = std::thread(testAndSet);
        }
        //Once program is done, join threads back together
        for(int i = 0; i < numThreads; i++){
            threadTracker[i].join();
        }
    }
    else if(algorithm == 2){
 
        //Create the amount of threads specified and send them to function with the sharedData
        for(int i = 0; i < numThreads; i++){
            threadTracker[i] = std::thread(fetchAndIncrement);

        }
        //Once program is done, join threads back together
        for(int i = 0; i < numThreads; i++){
            threadTracker[i].join();
        }
    }


    //Check if value counter is valid
    std::cout << counter << std::endl;



    
    return 0;
}
