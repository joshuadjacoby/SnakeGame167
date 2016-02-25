/*
 * Josh Jacoby
 * Shawn Ratcliff
 * Jonathan Saavedra
 */

#include "player.h"

using json = nlohmann::json;

Player::Player(std::string nom, int dir, Position startPos) {
    playerName = nom;
    direction = dir;
    queue.push_back(startPos);
    score = 0;
}

Position Player::head() const {
    return queue.back(); // The snake's head is at the back of the queue
}


bool Player::advance(const Position& food) {
    Position newHead;
    newHead = queue.back();
    switch(direction) {
        case UP:
            newHead.y--;
            break;
        case DOWN:
            newHead.y++;
            break;
        case LEFT:
            newHead.x--;
            break;
        case RIGHT:
            newHead.x++;
            break;
    }
    queue.push_back(newHead); // Add the new head
    if (food != head()) {
        queue.pop_front(); // Delete the old tail if we didn't get food
        return false;
    }
    else {
        score++;
        return true;
    }
}

json Player::getQueueJSON() const {
    // Iterate over the Positions in the queue, and add their respective JSON objects
    // to a JSON container object
    json output;
    for (auto iter = queue.begin(); iter != queue.end(); iter++ ) {
        output.push_back(iter->getJSON());
    }
    return output;
}

bool Player::collidesWith(const Player &otherPlayer) const {
    // Check our head against the other snake's queue
    for (auto iter = otherPlayer.queue.begin(); iter != otherPlayer.queue.end(); iter++) {
        if (head() == *iter)
            return true;
    }
    
    // Check the other snake's head against our queue
    for (auto iter = queue.begin(); iter != queue.end(); iter++) {
        if (otherPlayer.head() == *iter)
            return true;
    }
    
    // Otherwise, return false
    return false;
}

bool Player::occupies(const Position& pos) const {
    for (auto iter = queue.begin(); iter != queue.end(); iter++) {
        if (pos == *iter)
            return true;
    }
    return false;
}
