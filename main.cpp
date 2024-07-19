#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include <stdio.h>
#include <atomic>
#include <thread>
#include <iostream>
std::atomic_int counter;

//Global Variables for threads to access


// class PetersonLock{
//     std::thread leftChild;
//     std::thread rightChild;
//     bool flag[2];
//     std::atomic_int turn;
// public:
//     PetersonLock(std::thread leftChildd = NULL, std::thread rightChildd = NULL){
//         leftChild = leftChildd;
//         rightChild = rightChildd;
//         flag = {false, false};
//         turn = -1;
//     }
// }

//Create a class for the other locks
class Lock{
public:
    bool flag[2] = {false,false};   //Used for tournament tree
    std::atomic_int atomicToken;//Used for fetchAndIncrement
    std::atomic_int atomicTurn;//Used for fetchAndIncrement and tournament tree
    std::atomic_flag lock = ATOMIC_FLAG_INIT;//Used for testAndSet

    //Constructor to initalize every variable
    Lock() : atomicToken(0), atomicTurn(0) {}

    void doWork(){

        //lock -> acquire
        acquireLock();
        //Run counter loop
        for(int i= 0; i < 100000; i++){
            counter++;
        }
        //lock -> release
        releaseLock();
    }

    // functions to be overwritten by the child classes
    virtual void acquireLock() = 0;
    virtual void releaseLock() = 0;
};

//----------------------------------------------------------------------------------
class FetchAndIncrementLock : public Lock{
public:
    FetchAndIncrementLock(): Lock() {}

    void acquireLock() override {
        //Each thread will have its own turn variable
        int myTurn = atomicToken.fetch_add(1);
        //Busy spin while it is not a threads turn
        while(myTurn != atomicTurn){}
        //Acquired Lock
    }

    void releaseLock() override {
        atomicTurn += 1;
    }
};

//----------------------------------------------------------------------------------
class TestAndSetLock : public Lock{
public:
    TestAndSetLock(): Lock() {}

    void acquireLock() override {
        while (lock.test_and_set()){}
        //Acquired Lock
    }

    void releaseLock() override {
        //Release lock
        lock.clear();     
    }
};

//----------------------------------------------------------------------------------


// void acquireLock(int threadNum){
//     //Announce intention to get lock
//     flag[threadNum] = true;
//     //Give the lock to the other thread first
//     atomicTurn = 1 - threadNum;

//     //Spin
//     while(flag[1-threadNum] && (atomicTurn != threadNum)){}
// }

// void releaseLock(int threadNum){
//     //Release lock
//     flag[threadNum] = false;
// }

void doWorkk(Lock& lock) {
    lock.doWork();
}



int main(){

    int algorithm;
    int numThreads;
    counter = 0;
    printf("What algorithm would you like to use\n0: TT\n1: TAS\n2: FAI\n");
    std::cin >> algorithm;


    printf("How many threads: ");
    std::cin >> numThreads;

    //Create an array to keep track of threads
    std::thread threadTracker[numThreads];
    
    //Initalize the global variables
    // counter = 0;
    // atomicToken = 0;
    // atomicTurn = 0;


    // if(algorithm == 0){
    //     // //Create the amount of threads specified and send them to function with the sharedData


    //     for(int i = 0; i < numThreads; i++){
    //         //Use lambda function to pass the threadID into the function
    //         threadTracker[i] = std::thread(std::bind(tournamentTreeDriver, i));
    //     }  
    //     //Once program is done, join threads back together
    //     for(int i = 0; i < numThreads; i++){
    //         threadTracker[i].join();
    //     }
    // }
    //Initalize pointer to a lock
    Lock* lock = nullptr;
    if(algorithm == 1){

        //Create the lock
        lock = new TestAndSetLock();
        // //Create the amount of threads specified and send them to function with the sharedData
        for(int i = 0; i < numThreads; i++){
            threadTracker[i] = std::thread(doWorkk, std::ref(*lock));
        }
        //Once program is done, join threads back together
        for(int i = 0; i < numThreads; i++){
            threadTracker[i].join();
        }
    }
    else if(algorithm == 2){
 
        //Create the lock
        lock = new FetchAndIncrementLock();
        //Create the amount of threads specified and send them to function with the lock
        for(int i = 0; i < numThreads; i++){
            threadTracker[i] = std::thread(doWorkk, std::ref(*lock));

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
