#ifndef MESSAGEDELAYER_H
#define MESSAGEDELAYER_H

#include <chrono>
#include <queue>
#include <stdlib.h>
#include <string>
#include <utility>
#include <iostream>
#include <random>

class MessageDelayer{
    
    public:
        void putMessage (int clientID, std::string message); // adds a new message to the queue
        std::pair <int, std::string> getMessage();                 // gets the next message from queue, if possible
        bool isMessageReady();                          // check whether next message can be retrieved yet
        void clear();                                   // erase everything in the queue
        MessageDelayer(int Delay);
    
    private:
        void updateReleaseTime();
        std::queue <std::pair <int, std::string>> messageQueue;  // FIFO queue holds pairs of (int, string)
        int maxDelay;                           // the max amount of delay we will create
        unsigned long long nextReleaseTime;          // don’t release message until this time (in ms, since epoch) 
};

#endif