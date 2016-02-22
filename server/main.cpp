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

vector<json> clientMessages; // A place to hold client messages received before a game is instantiated
SnakeGame *game_p = NULL; // A pointer to access the SnakeGame we'll eventually instantiate
unsigned long lastUpdateTime = 0;


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
        msg["ASSIGNMENT"] = 1;
        send_json(clientID, msg);
    }
    
    // If Player 2 just connected...
    else if (server.getClientIDs().size() == 2) {
        // Send msg: you've been assigned player 2
        msg["MESSAGE_TYPE"] = "PLAYER_ASSIGNMENT";
        msg["ASSIGNMENT"] = 2;
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
        delete game_p;
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

    // Deserialize message from the string
    json msg = json::parse(message);
    
    
    // Step 1: Process the message
    if (msg["MESSAGE_TYPE"] == "CLIENT_UPDATE") {
        
        // If the game is active, pass the message to the game
        if (game_p != NULL) {
            game_p->handleClientInput(msg);
        }
        
        // Otherwise, hold the message in storage
        else {
            clientMessages.push_back(msg);
        }
    }
    
    // Step 2: If the clientMessages vector contains 2 messages, that means
    // both players are ready and it's time to start the game!
    
    if (clientMessages.size() == 2) {
        // Instantiate a new game with the messages in the correct sequence
        if (clientMessages[0]["CLIENT_ID"] == 1) {
            game_p = new SnakeGame(clientMessages[0], clientMessages[1]);
        }
        else {
            game_p = new SnakeGame(clientMessages[1], clientMessages[0]);
        }
        
        // Clear the stored messages
        clientMessages.clear();
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
        if (currentTime - lastUpdateTime >= 100) {
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
        vector<int> clientIDs = server.getClientIDs();
        for (int i = 0; i < clientIDs.size(); i++) {
            send_json(clientIDs[i], msg);
            server.wsClose(clientIDs[i]);
        }
        
        // Delete the game object from memory
        delete game_p;
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
