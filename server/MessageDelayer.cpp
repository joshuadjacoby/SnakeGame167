#include MessageDelayer.h

MessageDelayer::MessageDelayer(int maxDelay){
    this.maxDelay = maxDelay
    nextReleaseTime = 0   // So that we can identify a brand-new MessageDelayer
}

// Inserts a message into the MessageDelayer
void MesageDelayer::putMessage(int clientID, string message) {
    messageQueue.push(pair<int, string.(clientID, message));
    if (nextReleaseTime == 0)
        updateReleaseTime();
}

// Fetches the next message from the MessageDelayer, if one is ready
// (If not, a special error value is sent. )
pair<int, string> MessageDelayer::getMessage(){
    updateReleaseTime();
    
    if (isMessageReady)
        return message;
    else
        return pair<int, string>(-1, "");
}

bool MessageDelayer::isMessageReady(){
    return (!messageQueue().empty() && >= nextReleaseTime)
}