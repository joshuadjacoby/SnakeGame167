/*
Joshua Jacoby
Steven Ratcliff
Jonathan Saavedra
*/

/*  GameNetwork class encapsulates the network interface aspect of the game. 
    Constructor parameters: serverIP, port #
*/
var GameNetwork = function(serverIP, port) {
        
    /* Attempts new server connection to start a new game. */ 
    this.connectServer = function() {      
    
        // Create a new socket
        server = new FancyWebSocket('ws://' + serverIP + ':' + port);
        
        /*
         * BIND CALLBACKS FOR SOCKET EVENTS -- 3 BINDINGS:
         */
        
        // (1) Open event -- We're connected!
        server.bind('open', function() {
            ui.showConnectionStatus(true);
        });
        
        // (2) Disconnection event
        server.bind('close', function( data ) {
            ui.showConnectionStatus(false); // Update the notification strip
            // Stop the game, if it's running?
            // ...
            // ...
        });
        
        // (3) Message event -- message received from server
        server.bind('message', function( payload ) {
            
            // DEBUG
            console.log("RECEIVED: " + payload);
            
            // Deserialize the message object from JSON string
            var msgObject = JSON.parse(payload);
            
            // Process the message according to its type
            
            // Player assignment: this lets us know our player number, so we
            // can initialize things on the client side
            if (msgObject["MESSAGE_TYPE"] == "PLAYER_ASSIGNMENT") {
                initializePlayer(msgObject["PLAYER_NUMBER"]);
                console.log("Received player assignment as player " + playerNumber);
                
                // Send server the first status update
                console.log("SENDING: " + JSON.stringify(playerStatus()));
                server.send('message', JSON.stringify(playerStatus()));     
                
                // Start the game
                main();           
            }
            
            // Time stamps message: contains four time stamps to facilitate latency estimation
            // using the method described in the NTP protocol specification. Contains the following
            // fields:
            // "MESSAGE_TYPE" = "TIME_STAMP_REPLY"
            // "T1" = Time the request was first sent from the client
            // "T2" = Time the server received the request
            // "T3" = Time the server responded to the request
            // "T4" = Time the client received the server's response
            else if (msgObject["MESSAGE_TYPE"] == "TIME_STAMP_REPLY") {
                msgObject["T4" = new Date().getTime();] // Record the final time stamp
                network_latency = (msgObject["T4"] - msgObject["T1"]) - (msgObject["T3"] - msgObject["T2"]);
            }
            
            // A game status update (snake positions, scores, apple position, etc.)
            else if (msgObject["MESSAGE_TYPE"] == "SERVER_UPDATE") {
                // newServerUpdate is a global var declared and used in snake.js
                newServerUpdate = msgObject;    
            }
            
            // Oops! connection was rejected :-(
            else if (msgObject["MESSAGE_TYPE"] == "CONNECTION_REJECTED") {
                ui.showMessagePanel("Connection rejected by server", "Please try again later", "Try again");
                document.getElementById("restart-btn").focus();       
            }
            
            // Other error (e.g., other player disconnected)
            else if (msgObject["MESSAGE_TYPE"] == "ERROR") {
                running = false; // halt the game
                ui.showMessagePanel("An error occurred", msgObject["ERROR_MSG"], "Try again");
            }
        });
        
        // Try to connect...   
        server.connect();
    };
    
    /* Sends an update bundle to the server (as a serialized JSON string)
    */
    this.sendUpdate = function(clientUpdate) {
        clientUpdate["TIME_STAMP"] = new Date().getTime(); // Add time stamp
        var outString = JSON.stringify(clientUpdate);
        console.log("SENDING: " + outString);
	    server.send('message', outString);
    }

};