#ifndef MESSAGEDELAYER_H
#define MESSAGEDELAYER_H

#include <chrono>
#include <stdlib.h>

class MessageDelayer{
    
    public:
        void putMessage (int clientID, string message); // adds a new message to the queue
        pair<int, string> getMessage();                 // gets the next message from queue, if possible
        bool isMessageReady();                          // check whether next message can be retrieved yet
        void clear();                                   // erase everything in the queue
        MessageDelayer(int maxDelayer);
    
    private:
        void updateReleaseTime();
        Queue<pair<int, string>> messageQueue;  // FIFO queue holds pairs of (int, string)
        int maxDelay;                           // the max amount of delay we will create
        umsigned long nextReleaseTime;          // don’t release message until this time (in ms, since epoch) 
};

#endif