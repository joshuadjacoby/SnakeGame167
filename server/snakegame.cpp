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
	bool player1Collision = false;
	bool player2Collision = false;
	bool player1Wrong = false;
	bool player2Wrong = false;
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
	Player* p;
	if (clientData["PLAYER_NUMBER"].get<int>() == 1)
		p = player1;
	else
		p = player2;
	p->direction = clientData["CLIENT_DIRECTION"];
	if (clientData["GOT_QUEUE"]) {
		json queue = clientData["CLIENT_QUEUE"];
		p->update_queue(queue);
	}

	if (clientData["GOT_APPLE"]) {
		if (applePosition != p->head()) {
			if (p == player1) {
				player1Wrong = true;
			}
			else
				player2Wrong = true;
		}
		else {
			setApple();
		}
	}


	if (clientData["COLLISION"]) {
		if (p == player1) {
			player1Collision = true;
		}
		else
			player2Collision = true;
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
    
	bool appleEaten = false;

    /* Note: The sequence of steps is crucial here. One player must
     * advance first, then a collision check must occur, and then
     * the other player may go. If both players advanced simultaneously,
     * before the collision check, they could effectively pass through
     * each other without triggering a collision. Finally, at the end,
     * both players must be checked for a collision with the outer boundary.
     */
    /*
    // 1. Advance the first player, and check if apple was eaten.
    if (player1->advance(applePosition)) {
        setApple();
		appleEaten = true;
    }
	*/
    // 2. Check if collision happened
    if (Player::collisionCheck(*player1, *player2)) {
        gameActive = false;
        return statusObject();
    }
    /*
    // 3. Advance the second player, and check if apple was eaten.
    if (player2->advance(applePosition)) {
        setApple();
		appleEaten = true;
    }
    */
    // 4. Check if a collision occurred with the game boundary
    if (player1Collision || player2Collision) {
		gameActive = false;
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
    while (conflicted || applePosition.x <= 0 || applePosition.x >= (COLS-1) || applePosition.y <= 0 || applePosition.y >= (ROWS-1));
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
json SnakeGame::statusObject() {
    json j;

    j["MESSAGE_TYPE"] = "SERVER_UPDATE";
    j["CURRENT_FRAME"] = currentFrame;
    j["APPLE_POSITION"] = applePosition.getJSON();
    j["GAME_STATUS"] = gameActive;

	// Send a resync signal every n frames to
	// override client-side prediction

    j["PLAYER_1_NAME"] = player1->playerName;
    j["PLAYER_1_QUEUE"] = player1->getQueueJSON();
    j["PLAYER_1_SCORE"] = player1->score;
	j["PLAYER_1_DIRECTION"] = player1->direction;
	j["PLAYER_1_CORRECTION"] = player1Wrong;

    j["PLAYER_2_NAME"] = player2->playerName;
    j["PLAYER_2_QUEUE"] = player2->getQueueJSON();
    j["PLAYER_2_SCORE"] = player2->score;
	j["PLAYER_2_DIRECTION"] = player2->direction;
	j["PLAYER_2_CORRECTION"] = player2Wrong;

	player1Wrong = false;
	player2Wrong = false;
    return j;
}