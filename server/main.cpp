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
json pregame_player_msgs; // Holding place for messages recevied from clients before game starts
unsigned long lastUpdateTime = 0; // Keep track of when we last advanced the game state


/** Resets, deletes any pointers and resets to NULL */
void resetGame() {
    delete game_p;
    game_p = NULL;
    pregame_player_msgs.clear();
    lastUpdateTime = 0;
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

        // Send the message to whomever is connected
        vector<int> clientIDs = server.getClientIDs();
        for (int i = 0; i < clientIDs.size(); i++) {
            send_json(clientIDs[i], errorMsg);
        }

        // Close all open connections (must be done separately)
        clientIDs = server.getClientIDs();
        for (int i = 0; i < clientIDs.size(); i++) {
            server.wsClose(i);
        }
    }
}

/* called when a client sends a message to the server */
void messageHandler(int clientID, string message){
    
    // Deserialize message from the string
    json msg = json::parse(message);
    
    if (msg["MESSAGE_TYPE"] == "CLIENT_UPDATE") {

        // Scenario A: We don't have a game ready yet. This is a pre-game message.
        if (game_p == NULL) {
        
            // Step 1: Put the message in the correct bin.
            pregame_player_msgs[clientID] = msg;
        
            // Step 2: If both bins are filled (2 players ready),
            // then start a new game.
            if (pregame_player_msgs.size() == 2) {
                game_p = new SnakeGame(pregame_player_msgs[0], pregame_player_msgs[1]);
                pregame_player_msgs.clear();
            }
        }

        // Scenario B: A game already exists. Just forward the message to it.
        else {
            game_p->handleClientInput(msg);
        }
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
    // We want this to occur no sooner than 200 milliseconds (5 updates per second).
    // WARNING: This sets the pace of the game, so any future client features that
    // perform movement prediction will have to use the same clock speed.
    if (game_p->isActive()) {
        unsigned long currentTime = chrono::system_clock::now().time_since_epoch() / chrono::milliseconds(1);
        if (currentTime - lastUpdateTime >= 200) {
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
        
        // Reset everything; delete old game object; etc.
        resetGame();
    }
}




int main(int argc, char *argv[]){
    
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
