//Joshua Jacoby
//Steven Ratcliff
//Jonathan Saavedra
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <time.h>
#include "websocket.h"

using namespace std;

webSocket server;
int player_scores[2];
string player1_name;
string player2_name;



/* called when a client connects */
void openHandler(int clientID){
    // If we have few enough connections, accept it. Otherwise, reject.
    if (server.getClientIDs().size() < 2) {
        player_scores[0] = 0;
	player_scores[1] = 0;
	player1_name = " ";
	player2_name = " ";
        server.wsSend(clientID, "CONNECTION_READY", false); // Send ok msg
    }
    else {
        server.wsSend(clientID, "CONNECTION_REJECTED", false); // Send rejected msg
        server.wsClose(clientID);
    }

}

/* called when a client disconnects */
void closeHandler(int clientID){
    ostringstream os;
    os << "Stranger " << clientID << " has leaved.";

    vector<int> clientIDs = server.getClientIDs();
    for (int i = 0; i < clientIDs.size(); i++){
        if (clientIDs[i] != clientID)
            server.wsSend(clientIDs[i], os.str());
    }
}

/* called when a client sends a message to the server */
void messageHandler(int clientID, string message){
    ostringstream os;

	if (message == "p2score") {
		player_scores[1]++;
		os << player_scores[1];
	}

	if (message == "p1score") {
		player_scores[0]++;
		os << player_scores[0];

	}
	else {
		if (player1_name == " ")
			player1_name = message;
		else {
			player2_name = message;
		}
	}
	


    vector<int> clientIDs = server.getClientIDs();
    for (int i = 0; i < clientIDs.size(); i++){
        if (clientIDs[i] == clientID)
            server.wsSend(clientIDs[i], os.str());
    }
}

/* called once per select() loop */
void periodicHandler(){
    static time_t next = time(NULL) + 10;
    time_t current = time(NULL);
    if (current >= next){
        ostringstream os;
        string timestring = ctime(&current);
        timestring = timestring.substr(0, timestring.size() - 1);
        os << timestring;

        vector<int> clientIDs = server.getClientIDs();
        for (int i = 0; i < clientIDs.size(); i++)
            server.wsSend(clientIDs[i], os.str());

        next = time(NULL) + 10;
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
    //server.setPeriodicHandler(periodicHandler);

    /* start the chatroom server, listen to ip '127.0.0.1' and port '8000' */
    server.startServer(port);

    return 1;
}
