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
*   "CURRENT_FRAME" = the client's current frame number
*   "CLIENT_DIRECTION" = int (up/down/left/right constants)
*   "COLLISION" = true or false: is the client reporting a collision
*/
json SnakeGame::handleClientInput(json clientData) {
	// Check for game-over condition
	if (!gameActive || clientData["COLLISION"]) {
		gameActive = false;
		return statusObject();
	}

	Player* p;
	if (clientData["PLAYER_NUMBER"].get<int>() == 1)
		p = player1;
	else
		p = player2;

	// update the player object
	p->direction = clientData["CLIENT_DIRECTION"];
	p->currentFrame = clientData["CURRENT_FRAME"];
	bool appleEaten = p->advance(applePosition);
	if (appleEaten) {
		setApple();
	}

	return statusObject();
}
    
/** Returns json server update object reflecting game's latest information.
* @return json - server update
*/
json SnakeGame::getUpdate() {
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
 *  "APPLE_POSITION" = a JSON object containing the (x,y) coords for the food
 *  "GAME_STATUS" = true/false whether the game is still active
 *  "PLAYER_1_NAME" = player 1's name
 *  "PLAYER_1_QUEUE" = a JSON object containing P1's queue
 *  "PLAYER_1_SCORE" = player 1's score
 *  "PLAYER_1_DIRECTION" = player 1's last direction
 *  "PLAYER_1_FRAME" = the frame # associated with Player 1's latest position
 *  "PLAYER_2_NAME" = player 2's name
 *  "PLAYER_2_QUEUE" = a JSON object containing P2's queue
 *  "PLAYER_2_SCORE" = player 2's score
 *  "PLAYER_2_DIRECTION" player 2's last direction
 *  "PLAYER_2_FRAME" = the frame # associated with Player 2's latest position
 */
json SnakeGame::statusObject(bool resync) const {
    json j;
    j["MESSAGE_TYPE"] = "SERVER_UPDATE";
    j["APPLE_POSITION"] = applePosition.getJSON();
    j["GAME_STATUS"] = gameActive;
	j["RESYNC"] = resync;

    j["PLAYER_1_NAME"] = player1->playerName;
    j["PLAYER_1_QUEUE"] = player1->getQueueJSON();
    j["PLAYER_1_SCORE"] = player1->score;
	j["PLAYER_1_DIRECTION"] = player1->direction;
	j["PLAYER_1_FRAME"] = player1->currentFrame;

    j["PLAYER_2_NAME"] = player2->playerName;
    j["PLAYER_2_QUEUE"] = player2->getQueueJSON();
    j["PLAYER_2_SCORE"] = player2->score;
	j["PLAYER_2_DIRECTION"] = player2->direction;
	j["PLAYER_2_FRAME"] = player2->currentFrame;
    return j;
}