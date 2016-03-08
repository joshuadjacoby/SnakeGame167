/*
 * Josh Jacoby
 * Shawn Ratcliff
 * Jonathan Saavedra
 */
 
#ifndef snakegame_h
#define snakegame_h

#include "json.hpp"
#include <deque>
#include <string>
#include "player.h"
#include "position.h"

using namespace std;
using json = nlohmann::json;

/* Snake Game class. Encapsulates game logic and state. */
class SnakeGame {

    public:
    
    /** Public constructor. Initializes new game.
     *  @param json - Initial status update from player 1
     *  @param json - Initial status update from player 2
     */
    SnakeGame(json, json);
    
    /** Receives JSON update from client and updates client's data.
     *  @param clientData - the json update message received from a client
	 *  @return json - server update reflecting the latest information
     */
    json handleClientInput(json clientData);
    
	/** Returns json server update object reflecting game's latest information. 
	  * @return json - server update
	  */
	json getUpdate();

    /** Returns whether game is active or not
     *  @return bool active or inactive
     */
    bool isActive() const;
    
    /** Destructor */
    ~SnakeGame();
    
    private:
    
    Player* player1;
    Player* player2;
    bool gameActive; // Whether the game is still active or has terminated
    Position applePosition; // The current location of the apple
	
    
    /** Sets a new apple position.
     */
    void setApple();
    
    /** Builds JSON object reporting current game status.
	 *  @param bool - force clients to resync/override all local prediction
     *  @return json the game status
     */
    json statusObject(bool resync = false) const;
    
};

#endif /* snakegame_h */
