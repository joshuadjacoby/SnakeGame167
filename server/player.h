/*
 * Josh Jacoby
 * Shawn Ratcliff
 * Jonathan Saavedra
 */

#ifndef player_h
#define player_h

#include "json.hpp"
#include <string>
#include <deque>
#include <set>
#include <algorithm>
#include "position.h"

using namespace std;
using json = nlohmann::json;

/* Declare constants: these must be identical on the client side.
 * By convention, we will say that Player 1 always starts at the bottom facing up, and
 * Player 2 starts at the top, facing down. (As it was originally implemented in the JS snake game.)
 */
const int COLS = 26;
const int ROWS = 26;
const int CENTER_COLUMN = 13;
const int LEFT  = 0;
const int UP    = 1;
const int RIGHT = 2;
const int DOWN  = 3;

/* Representation of a player and his current state */
class Player {
    
public:
    
    string playerName;
    int direction;
    int score;
    
    /** Constructor 
     *  @param string - player name
     *  @param int - direction
     *  @param Position - the player's starting position
     */
    Player(string, int, Position);

    /** Returns the position of this player's head */
    Position head() const;
    
    /** Advances the player's snake 1 unit based on current direction,
     *  and grows the snake if it eats food.
     *  @param const Position& - the current food Position
     */
    void advance(const Position&);
    
    /** Returns the player's queue as a JSON object. */
    json getQueueJSON() const;
    
    
    /** Detects whether a collision has just occurred in the current frame.
     *  NOTE: This only detects a head-to-body or head-to-head collision. (T or L junction).
     *  It will not detect a body-to-body overlap (X junction), which represents a
     *  collision that occurred in a previous frame.
     *  @param const Player& - the other player
     */
    bool collidesWith(const Player&) const;

    
protected:
    
    // We use a FIFO queue to model the snake. Counterintuitively,
    // The front of the queue is the TAIL of the snake; while the
    // back of the queue is the HEAD (i.e., the node that will be
    // last to pop off the queue as the snake moves.)
    deque<Position> queue;
    
};


Player::Player(string nom, int dir, Position startPos) {
    playerName = nom;
    direction = dir;
    queue.push_back(startPos);
    score = 0;
}

Position Player::head() const {
    return queue.back(); // The snake's head is at the back of the queue
}


void Player::advance(const Position& food) {
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


#endif /* player_h */
