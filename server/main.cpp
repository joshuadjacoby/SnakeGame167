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
string player1queue;
string player2queue;
bool player1nameset;
bool player2nameset;
bool p1ready;
bool p2ready;

typedef pair<int, int> location;

vector<location> player1_locs;
vector<location> player2_locs;

const int COLS = 26;
const int ROWS = 26;

/* called when a client connects */
void openHandler(int clientID){
    // If we have few enough connections, accept it. Otherwise, reject.
    if (server.getClientIDs().size() == 2) {
        player_scores[0] = 0;
		player_scores[1] = 0;
		player1queue = "[{\"x\":13, \"y\" : 25}]";
		player2queue = "[{\"x\":13, \"y\" : 0}]";
		player1nameset = false;
		player2nameset = false;
		p1ready = false;
		p2ready = false;
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
    os << "Stranger " << clientID << " has left.";

    vector<int> clientIDs = server.getClientIDs();
    for (int i = 0; i < clientIDs.size(); i++){
        if (clientIDs[i] != clientID)
            server.wsSend(clientIDs[i], os.str());
    }
}


/**
* Set a food id at a random free cell in the grid
*/
location setFood(vector<location> a, vector<location> b) {
	vector<location> empty;

	for (int x = 0; x < ROWS; x++) {
		for (int y = 0; y < COLS; y++) {
			if ((find(a.begin(), a.end(), location(x, y)) == a.end()) && (find(b.begin(), b.end(), location(x, y)) == b.end()))
					empty.push_back(location(x, y));
		}
	}

	//chooses a random cell
	location randpos = empty[rand() % empty.size()];
	return randpos;

}

/* called when a client sends a message to the server */
void messageHandler(int clientID, string message) {

	if (clientID == 0 && player1nameset == false) {
		player1_name = message;
		player1nameset = true;
		if (player1nameset == true && player2nameset == true) {
			server.wsSend(0, player2_name);
			server.wsSend(1, player1_name);
		}
		return;
	}
	if (clientID == 1 && player2nameset == false) {
		player2_name = message;
		player2nameset = true;
		if (player1nameset == true && player2nameset == true) {
			server.wsSend(0, player2_name);
			server.wsSend(1, player1_name);
		}
		return;
	}

	ostringstream os;
	vector<int> clientIDs = server.getClientIDs();

	if (message == "ready") {
		if (clientID == 0) {
			p1ready = true;
		}
		if (clientID == 1) {
			p2ready = true;
		}
		if (p1ready == true && p2ready == true) {
			server.wsSend(0, "READY");
			server.wsSend(1, "READY");
		}
		return;
	}



	if (message == "p1score") {
		if (clientID == 0) {
			player_scores[0]++;
			server.wsSend(0, "p1scored");
			server.wsSend(1, "p2scored");
		}
		else {
			player_scores[1]++;
			server.wsSend(1, "p1scored");
			server.wsSend(0, "p2scored");
		}

		return;
	}

	if (message == "client" && clientID == 0) {
		server.wsSend(0, "0");
		return;
	}

	if (message == "client" && clientID == 1) {
		server.wsSend(1, "1");
		return;
	}

	if (message == "fruit") {
		string s;
		if (clientID == 0)
			s = player1queue;
		else
			s = player2queue;
		string delimiter = ":";

		size_t startPos = 0;
		size_t endPos = 0;
		string x;
		string y;
		while ((startPos = s.find(delimiter)) != string::npos) {
			endPos = s.find(",");
			x = s.substr(startPos + delimiter.length(), endPos);
			s.erase(0, endPos);
			startPos = s.find(delimiter);
			endPos = s.find("}");
			y = s.substr(startPos + delimiter.length(), endPos);
			s.erase(0, endPos);
			if (clientID == 0)
				player1_locs.push_back(make_pair(stoi(x), stoi(y)));
			else
				player2_locs.push_back(make_pair(stoi(x), stoi(y)));
		}
		if (!player1_locs.empty() && !player2_locs.empty()) {
			location fruit = setFood(player1_locs, player2_locs);
			string fruitString = to_string(fruit.first) + "/" + to_string(fruit.second);
			server.wsSend(0, fruitString);
			server.wsSend(1, fruitString);
		}
	}
	
	else {
		if (clientID == 0) {
			player1queue = message;
			server.wsSend(0, player2queue);
		}
		if (clientID == 1) {
			player2queue = message;
			server.wsSend(1, player1queue);
		}
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
