//Joshua Jacoby
//Steven Ratcliff
//Jonathan Saavedra
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <time.h>
#include <vector>
#include <chrono>
#include "websocket.h"
#include "snakegame.h"
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

webSocket server;

SnakeGame *game_p = NULL; // A pointer to access the SnakeGame we'll eventually instantiate

// This container holds player 1 and player 2's pre-game status updates, before the
// game is instantiated. We use this container to hold the messages, and to
// determine when both players are ready.
vector<json> pregame_player_msgs(2);

unsigned long lastUpdateTime = 0; // Keep track of when we last advanced the game state


/** Resets, deletes any pointers and resets to NULL */
void resetGame() {
    delete game_p;
    game_p = NULL;
    pregame_player_msgs.clear();
}


/** Send json object as string to specified client
 *  @param int clientID
 *  @param json JSON object message
 *
 * FOR MILESTONE 3: this must be delayed by a random amount
 */
void send_json(int clientID, json msg) {
    
    // Add time stamp:
    // Example:
    // msg["TIME_STAMP"] = timeInMs();
    
    // Send the message
    server.wsSend(clientID, msg.dump(), false);

}


/* called when a client connects */
void openHandler(int clientID){

    json msg; // Our first message to the client
    
    // If Player 1 just connected...
    if (server.getClientIDs().size() == 1) {
        // Send msg: you've been assigned player 1
        msg["MESSAGE_TYPE"] = "PLAYER_ASSIGNMENT";
        msg["PLAYER_NUMBER"] = 1;
        send_json(clientID, msg);
    }
    
    // If Player 2 just connected...
    else if (server.getClientIDs().size() == 2) {
        // Send msg: you've been assigned player 2
        msg["MESSAGE_TYPE"] = "PLAYER_ASSIGNMENT";
        msg["PLAYER_NUMBER"] = 2;
        send_json(clientID, msg);
    }
    
    // Or if there are too many connections, reject it:
    else {
        msg["MESSAGE_TYPE"] = "CONNECTION_REJECTED";
        send_json(clientID, msg);
        server.wsClose(clientID);
    }
}


/* called when a client disconnects */
void closeHandler(int clientID){

    // If game is ongoing, kill it and send out
    // an error to whomever is still connected:
    if (game_p != NULL && game_p->isActive()) {
        resetGame();
        json errorMsg;
        errorMsg["MESSAGE_TYPE"] = "ERROR";
        errorMsg["ERROR_MSG"] = "Other player disconnected";

        vector<int> clientIDs = server.getClientIDs();
        for (int i = 0; i < clientIDs.size(); i++) {
            send_json(clientIDs[i], errorMsg);
        }
    }
}

/* called when a client sends a message to the server */
void messageHandler(int clientID, string message){

    // DEBUG
    // DEBUG
    // DEBUG
    cout << "Message received from socket client ID " << clientID << " :" << message << endl;
    // DEBUG
    // DEBUG
    // DEBUG
    
    
    // Deserialize message from the string
    json msg = json::parse(message);
    
    
    // Step 1: Process the message
    if (msg["MESSAGE_TYPE"] == "CLIENT_UPDATE") {
        
        // If the game object has already been instantiated,
        // pass the message to the game
        if (game_p != NULL) {
            game_p->handleClientInput(msg);
        }
        
        // Otherwise, this is a pre-game message; hold it in the
        // appropriate slot (index 0 or 1, depending on player 1 or player 2)
        else {
            pregame_player_msgs[clientID] = msg;
        }
    }
    
    // Step 2: Let's now check if both players have sent us a pre-game status
    // update. If they have, we're ready to start the game!
    if (pregame_player_msgs[0] != NULL && pregame_player_msgs[1] != NULL) {
        
        // Instantiate a new game using the pre-game client update messages
        game_p = new SnakeGame(pregame_player_msgs[0], pregame_player_msgs[1]);
        
        // Flush the pre-game messages, since we don't need them any more
        pregame_player_msgs.clear();
    }
}



/* Once the server is started, this function is called repeatedly throughout 
 * the server's life-cycle. We'll use it to run the game loop (when there is
 * a game pending. */
void periodicHandler(){

    // If there's no game object, do nothing.
    if (game_p == NULL) {
        return;
    }
    
    // If the game is active, update the game state.
    // We want this to occur no sooner than 100 milliseconds (10 updates per second).
    // WARNING: This sets the pace of the game, so any future client features that
    // perform movement prediction will have to use the same clock speed.
    if (game_p->isActive()) {
        unsigned long currentTime = chrono::system_clock::now().time_since_epoch() / chrono::milliseconds(1);
        if (currentTime - lastUpdateTime >= 500) {
            // Update the game
            json msg = game_p->update();
            
            // Broadcast the update to all clients
            vector<int> clientIDs = server.getClientIDs();
            for (int i = 0; i < clientIDs.size(); i++) {
                send_json(clientIDs[i], msg);
            }
            
            // Update the clock
            lastUpdateTime = chrono::system_clock::now().time_since_epoch() / chrono::milliseconds(1);
        }
    }
    
    // If there is a game object but the status is INACTIVE,
    // update the clients and then DESTROY the game object.
    if (!game_p->isActive()) {
        // Get the final status message
        json msg = game_p->update();
        
        // Broadcast the final update to all clients
        // and then disconnect clients
        cout << "GAME OVER BROADCASTING FINAL UPDATE" << endl;
        vector<int> clientIDs = server.getClientIDs();
        for (int i = 0; i < clientIDs.size(); i++) {
            send_json(clientIDs[i], msg);
            server.wsClose(clientIDs[i]);
        }
        
        // Delete the game object from memory
        delete game_p;
        game_p = NULL;
    }
}




int main(int argc, char *argv[]){
    
    game_p = NULL;
    
    int port;

    cout << "Please set server port: ";
    cin >> port;

    /* set event handler */
    server.setOpenHandler(openHandler);
    server.setCloseHandler(closeHandler);
    server.setMessageHandler(messageHandler);
    server.setPeriodicHandler(periodicHandler);

    /* start the chatroom server, listen to ip '127.0.0.1' and port '8000' */
    server.startServer(port);

    return 1;
}
