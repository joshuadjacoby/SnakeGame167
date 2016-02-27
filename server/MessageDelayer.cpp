#include "MessageDelayer.h"

MessageDelayer::MessageDelayer(int Delay) {
    // Instantiate a binomial distribution with a maximum delay of Delay, and
    // a probability of 0.5 (i.e., average latency at the midpoint of [0, Delay]).
    latencyDistribution = std::binomial_distribution<int>(Delay, 0.5);
    nextReleaseTime = 0;   // So that we can identify a brand-new MessageDelayer
}

// Inserts a message into the MessageDelayer
void MessageDelayer::putMessage(int clientID, std::string message) {
	std::pair <int, std::string>  message_pair = std::make_pair(clientID, message);
    messageQueue.push(message_pair);
    if (nextReleaseTime == 0)
        updateReleaseTime();
}

// Fetches the next message from the MessageDelayer, if one is ready
// (If not, a special error value is sent. )
std::pair <int, std::string> MessageDelayer::getMessage() {
    if (isMessageReady() == true && !messageQueue.empty() ) {
        updateReleaseTime();
		std::pair <int, std::string> message = messageQueue.front();
        messageQueue.pop();
		return message;
    } else
        return std::pair <int, std::string>(-1, "");
}

bool MessageDelayer::isMessageReady() {
	return (!messageQueue.empty() && (std::chrono::duration_cast<std::chrono::milliseconds>
		(std::chrono::system_clock::now().time_since_epoch()).count() >= nextReleaseTime));
}

// Discard all messages in the buffer
void MessageDelayer::clear() {
    while (!messageQueue.empty()) {
        messageQueue.pop();
    }
}

// Calculate and set the next time a message will be ready to be released
void MessageDelayer::updateReleaseTime(){
    int delay = latencyDistribution(randomEngine);
    unsigned long long currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    nextReleaseTime = currentTime + delay;
}