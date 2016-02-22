/*
 * Josh Jacoby
 * Shawn Ratcliff
 * Jonathan Saavedra
 */

#include "json.hpp"
#include <deque>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "snakegame.h"

using json = nlohmann::json;
using namespace std;

/*
 Private members:
 
    Player player1;
    Player player2;
    int currentFrame; // The current logical frame of the game
    bool gameActive; // Whether the game is still active or has terminated
    Position applePosition; // The current location of the apple
*/

/* PUBLIC METHODS*/


/** Public constructor. Initializes new game.
*  @param json - Initial status update from player 1
*  @param json - Initial status update from player 2
*/
SnakeGame::SnakeGame(json msg1, json msg2) {
    
    // Initialize player 1
    player1.playerName = msg1["PLAYER_NAME"];
    player1.direction = UP;
    player1.score = 0;
    player1.queue.push_back(Position(CENTER_COLUMN, ROWS - 1)); // Bottom row, center column
    
    // Initialize player 2
    player2.playerName = msg2["PLAYER_NAME"];
    player2.direction = DOWN;
    player2.score = 0;
    player2.queue.push_back(Position(CENTER_COLUMN, 0)); // Top row, center column
    
    // Initialize game state
    currentFrame = 0;
    setApple();
    gameActive = true;
}
    
/** Receives JSON update from client and updates client's data (mainly, direction).
*   @param clientData - the json update message received from a client
*
*   Client update messages contain the following key/value data:
*   
*   "MESSAGE_TYPE" = "CLIENT_UPDATE"
*   "CLIENT_ID" = int (1 or 2)
*   "CLIENT_DIRECTION" = int (up/down/left/right constants)
*/
void SnakeGame::handleClientInput(json clientData) {
    if (clientData["CLIENT_ID"] == 1) {
        player1.direction = clientData["CLIENT_DIRECTION"];
    }
    else if (clientData["CLIENT_ID"] == 2) {
        player2.direction = clientData["CLIENT_DIRECTION"];
    }
}
    
/** Increments the frame counter and advances the players' position by one unit based on
 *  their last-known direction.
 *  @return a JSON object containing the complete game state after the update (to be broadcast to both players)
 */
json SnakeGame::update() {

    // Check to see if game is over. In this case, return a status message
    // but don't increment the game counter or change anything else.
    if (!gameActive) {
        return statusObject();
    }
    
    // But if the game is active, evolve the game state:
    
    // Increment the frame counter
    currentFrame++;
    
    // Advance the players
    player1.advance();
    player2.advance();
    
    // Check if collision happened
    if (collisionDetected()) {
        gameActive = false;
    }
    
    // Check if Player 1 scored
    if (player1.queue.back() == applePosition) {
        player1.score++;
        setApple();
    }
    else {
        player1.queue.pop_front(); // Snake 1 didn't grow this time
    }

    // Check if Player 2 scored
    if (player2.queue.back() == applePosition) {
        player2.score++;
        setApple();
    }
    else {
        player2.queue.pop_front(); // Snake 2 didn't grow this time
    }
    
    // Construct and return update bundle
    return statusObject();
}

/** Returns whether game is active or not
 *  @return bool active or inactive
 */
bool SnakeGame::isActive() {
    return gameActive;
}



/******** PRIVATE METHODS ************/


/** Sets a new apple position.
 */
void SnakeGame::setApple() {
    
    // Temporary algorithm: keep picking random positions
    // until we find one that isn't conflicted.
    bool conflicted = false;
    do {
        applePosition.x = rand() % COLS;
        applePosition.y = rand() % ROWS;

        // Check player 1's queue
        for (int i = 0; i < player1.queue.size() && !conflicted; i++) {
            if (player1.queue[i] == applePosition)
                conflicted = true;
        }

        // Check player 2's queue
        for (int i = 0; i < player2.queue.size() && !conflicted; i++) {
            if (player2.queue[i] == applePosition)
                conflicted = true;
        }
    }
    while (conflicted);
}

/** Checks whether collision has occurred.
 *  @return bool True if collision; false if OK
 */
bool SnakeGame::collisionDetected() const {
    
    Position p;
    
    // Check player 1's head
    p = player1.queue.back();
    if (p.x < 0 || p.x >= COLS || p.y < 0 || p.y >= ROWS) {
        return true;
    }
    for (int i = 0; i < player2.queue.size(); i++) {
        if (p == player2.queue[i]) {
            return true;
        }
    }

    // Check player 2's head
    p = player2.queue.back();
    if (p.x < 0 || p.x >= COLS || p.y < 0 || p.y >= ROWS) {
        return true;
    }
    for (int i = 0; i < player1.queue.size(); i++) {
        if (p == player1.queue[i]) {
            return true;
        }
    }
    return false; // Everything checked out OK
}

/** Builds JSON object reporting current game status.
 *  @return json the game status
 *
 *  Status object contains at least the following key-value pairs (but more can be added downstream):
 *
 *  "MESSAGE_TYPE" = "SERVER_UPDATE",
 *  "CURRENT_FRAME" = the current frame number
 *  "GAME_STATUS" = true/false whether the game is still active
 *  "PLAYER_1_NAME" = player 1's name
 *  "PLAYER_2_NAME" = player 2's name
 *  "PLAYER_1_SCORE" = player 1's score
 *  "PLAYER_2_SCORE" = player 2's score
 *  "PLAYER_1_QUEUE" = a JSON object containing P1's queue
 *  "PLAYER_2_QUEUE" = a JSON object containing P2's queue
 *  "APPLE_POSITION" = a JSON object containing the (x,y) coords for the food
 */
json SnakeGame::statusObject() const {
    json j;
    j["MESSAGE_TYPE"] = "SERVER_UPDATE";
    j["CURRENT_FRAME"] = currentFrame;
    j["GAME_STATUS"] = gameActive;
    j["PLAYER_1_NAME"] = player1.playerName;
    j["PLAYER_2_NAME"] = player2.playerName;
    j["PLAYER_1_SCORE"] = player1.score;
    j["PLAYER_2_SCORE"] = player2.score;
    
    // Add Player 1 queue
    json q1; // player 1's queue as a json object
    for (int i = player1.queue.size() - 1; i >= 0; i--) {
        q1.push_back(json({ {"x", player1.queue[i].x}, {"y", player1.queue[i].y} }));
    }
    j["PLAYER_1_QUEUE"] = q1;
    
    //Add Player 2 queue
    json q2; // player 2's queue as a json object
    for (int i = player2.queue.size() - 1; i >= 0; i--) {
        q2.push_back(json({ {"x", player2.queue[i].x}, {"y", player2.queue[i].y} }));
    }
    j["PLAYER_2_QUEUE"] = q2;
    j["APPLE_POSITION"] = json({ {"x", applePosition.x}, {"y", applePosition.y} });

    return j;
}