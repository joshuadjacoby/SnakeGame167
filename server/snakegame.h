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

typedef struct Position Position;
typedef struct Player Player;


/* An (x, y) pair to represent game coordinates */
struct Position {
    int x;
    int y;
    
    /** Default constructor */
    Position() {
        x = 0;
        y = 0;
    }
    
    /** Constructor */
    Position(int xValue, int yValue) {
        x = xValue;
        y = yValue;
    }
    
    /** Overloaded equality operator */
    bool operator==(const Position& other) const
    {
        return (x == other.x && y == other.y);
    }
};


/* Representation of a player and his current state */
struct Player {
    int score;
    int direction;
    deque<Position> queue;
    string playerName;
    
    /** Advances the player's snake 1 unit based on current direction
     */
    void advance() {

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
        queue.pop_front(); // Delete the old tail
    }
};


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
     */
    void handleClientInput(json clientData);
    
    /** Increments the frame counter and advances the players' position by one unit based on
     *  their last-known direction.
     *  @return a JSON object containing the complete game state after the update (to be broadcast to both players)
     */
    json update();
    
    /** Returns whether game is active or not
     *  @return bool active or inactive
     */
    bool isActive();
    
    
    private:
    
    Player player1;
    Player player2;
    int currentFrame; // The current logical frame of the game
    bool gameActive; // Whether the game is still active or has terminated
    Position applePosition; // The current location of the apple
    
    /** Sets a new apple position.
     */
    void setApple();
    
    /** Checks whether collision has occurred.
     *  @return bool True if collision; false if OK
     */
    bool collisionDetected() const;
    
    /** Builds JSON object reporting current game status.
     *  @return json the game status
     */
    json statusObject() const;

    
};


#endif /* snakegame_h */
