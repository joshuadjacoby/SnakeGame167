/*
 * Josh Jacoby
 * Shawn Ratcliff
 * Jonathan Saavedra
 */

#include "json.hpp"
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "snakegame.h"
#include "position.h"
#include "player.h"

using json = nlohmann::json;
using namespace std;

/* PUBLIC METHODS*/

/** Public constructor. Initializes new game.
*  @param json - Initial status update from player 1
*  @param json - Initial status update from player 2
*/
SnakeGame::SnakeGame(json msg1, json msg2) {
    
    // Instantiate players
    player1 = new Player(msg1["PLAYER_NAME"], UP, Position(CENTER_COLUMN, ROWS-1));
    player2 = new Player(msg2["PLAYER_NAME"], DOWN, Position(CENTER_COLUMN, 0));
    
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
*   "PLAYER_NUMBER" = int (1 or 2)
*   "CLIENT_DIRECTION" = int (up/down/left/right constants)
*/
void SnakeGame::handleClientInput(json clientData) {
    if (clientData["PLAYER_NUMBER"] == 1) {
        player1->direction = clientData["CLIENT_DIRECTION"];
    }
    else if (clientData["PLAYER_NUMBER"] == 2) {
        player2->direction = clientData["CLIENT_DIRECTION"];
    }
}
    
/** Increments the frame counter and advances the players' position by one unit based on
 *  their last-known direction.
 *  @return a JSON object containing the complete game state after the update (to be broadcast to both players)
 */
json SnakeGame::update() {

    // Check to see if game is over. In this case, return a status message
    // but don't increment the game counter or do anything else.
    if (!gameActive) {
        return statusObject();
    }
    
    // Increment the frame counter
    currentFrame++;
    
    // Advance the players and record whether apple
    // was eaten.
    bool appleEaten = player1->advance(applePosition) || player2->advance(applePosition);
    
    // Check if collision happened
    if (player1->collidesWith(*player2)) {
        gameActive = false;
    }
    
    // Reset the apple, if necessary
    if (appleEaten) {
        setApple();
    }
    
    // Construct and return JSON update bundle
    return statusObject();
}

/** Returns whether game is active or not
 *  @return bool active or inactive
 */
bool SnakeGame::isActive() const {
    return gameActive;
}

/** Destructor */
SnakeGame::~SnakeGame() {
    // Free heap-allocated memory
    delete player1;
    delete player2;
}


/******** PRIVATE METHODS ************/


/** Sets a new apple position.
 */
void SnakeGame::setApple() {
    // Choose a random location and check if it's occupied. Repeat as needed.
    bool conflicted = false;
    do {
        applePosition.x = rand() % COLS;
        applePosition.y = rand() % ROWS;
        conflicted = player1->occupies(applePosition) || player2->occupies(applePosition);
    }
    while (conflicted);
}

/** Builds JSON object reporting current game status.
 *  @return json the game status
 *
 *  Status object contains at least the following key-value pairs (but more can be added downstream):
 *
 *  "MESSAGE_TYPE" = "SERVER_UPDATE",
 *  "CURRENT_FRAME" = the current frame number
 *  "APPLE_POSITION" = a JSON object containing the (x,y) coords for the food
 *  "GAME_STATUS" = true/false whether the game is still active
 *  "PLAYER_1_NAME" = player 1's name
 *  "PLAYER_1_QUEUE" = a JSON object containing P1's queue
 *  "PLAYER_1_SCORE" = player 1's score
 *  "PLAYER_2_NAME" = player 2's name
 *  "PLAYER_2_QUEUE" = a JSON object containing P2's queue
 *  "PLAYER_2_SCORE" = player 2's score
 */
json SnakeGame::statusObject() const {
    json j;

    j["MESSAGE_TYPE"] = "SERVER_UPDATE";
    j["CURRENT_FRAME"] = currentFrame;
    j["APPLE_POSITION"] = applePosition.getJSON();
    j["GAME_STATUS"] = gameActive;

    j["PLAYER_1_NAME"] = player1->playerName;
    j["PLAYER_1_QUEUE"] = player1->getQueueJSON();
    j["PLAYER_1_SCORE"] = player1->score;

    j["PLAYER_2_NAME"] = player2->playerName;
    j["PLAYER_2_QUEUE"] = player2->getQueueJSON();
    j["PLAYER_2_SCORE"] = player2->score;
    return j;
}