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
string player1Move = "3";
string player2Move = "3";



/* called when a client connects */
void openHandler(int clientID){
    // If we have few enough connections, accept it. Otherwise, reject.
    if (server.getClientIDs().size() == 2) {
        player_scores[0] = 0;
		player_scores[1] = 0;
		player1_name = " ";
		player2_name = " ";
		vector<int> clientIDs = server.getClientIDs();
		for (int i = 0; i < clientIDs.size(); i++)
			server.wsSend(clientIDs[i], "CONNECTION_READY", false);
    }
    if (server.getClientIDs().size() > 2) {
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
void messageHandler(int clientID, string message) {
	ostringstream os;
	vector<int> clientIDs = server.getClientIDs();


	vector<pair<int, int>> locs;

	cout << message << endl;

	string s = message;
	string delimiter = ":";

	size_t pos = 0;
	string x;
	string y;
	while ((pos = s.find(delimiter)) != string::npos) {
		x = s[pos + delimiter.length()];
		s.erase(0, pos + delimiter.length());
		pos = s.find(delimiter);
		y = s[pos + delimiter.length()];
		s.erase(0, pos + delimiter.length());
		locs.push_back(make_pair(stoi(x), stoi(y)));
	}

	for (int i = 0; i < locs.size(); i++)
	{
		cout << locs[i].first << ", " << locs[i].second << endl;
	}

	if (message == "p1score") {
		if (clientID == 0) {
			player_scores[0]++;
			os << player_scores[0];
		}
		else {
			player_scores[1]++;
			os << player_scores[1];
		}
			server.wsSend(clientID, os.str());
		return;
	}
	

	if (message == "p2score") {
		if (clientID == 0)
			os << player_scores[1];
		else { os << player_scores[0]; }

		for (int i = 0; i < clientIDs.size(); i++) {
			if (clientIDs[i] == clientID)
				server.wsSend(clientIDs[i], os.str());
		}
		return;
	}

	if (message == "namerequest") {
		cout << "name requested" << endl;
		if (clientID == 0)
			os << player2_name;
		else { os << player1_name; }
		server.wsSend(clientID, os.str());
		return;
	}


	if (message == "0") {
		if (clientID == 0)
			player1Move = "2";
		else { player2Move = "2"; }
	}

	if (message == "1") {
		if (clientID == 0)
			player1Move = "3";
		else { player2Move = "3"; }
	}

	if (message == "2") {
		if (clientID == 0)
			player1Move = "0";
		else { player2Move = "0"; }
	}

	if (message == "3") {
		if (clientID == 0)
			player1Move = "1";
		else { player2Move = "1"; }
	}

	else {
		if (clientID == 0)
			player1_name = message;
		else {
			player2_name = message;
		}
	}
	
	if (clientID == 0)
		server.wsSend(clientID, player2Move);
	else { server.wsSend(clientID, player1Move); }
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
