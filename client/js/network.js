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
        server = new FancyWebSocket('ws://'+ serverIP + ':' + port);
        
        /*
         * BIND CALLBACKS FOR SOCKET EVENTS -- 3 BINDINGS:
         */
        
        // (1) Open event -- We're connected!
        server.bind('open', function() {
            // Don't start the game yet -- we're going to wait to receive a "connection ready" message from the server
            // This prevents us from launching the game too soon, because the server might need to 
            // disconnect us if too many players are connected already 
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
            
            // Connection ready message: let's start playing the game
            if (payload == "CONNECTION_READY") {
                server.send('message', player1);  // Send player id to server
                server.send('message', player2);  // ""
                ui.showConnectionStatus(true); // Update the notification strip          
                main(); // start the game 
            }
            
            // Connection rejected message: let's display the message panel
            // to inform the user
            if (payload == "CONNECTION_REJECTED") {
                ui.showMessagePanel("Connection rejected by server", "Please try again later", "Try again");
                document.getElementById("restart-btn").focus();    
            }
        	});
        
        server.bind('player1scored', function (payload) {
            score = payload;
        });
        
        server.bind('player2scored', function (payload) {
            score2 = payload;
        });
        
        // Try to connect...   
        server.connect();
    };
    
    /* Reports Player 1's score event to server.
        (This client is not responsible for computing Player 2's score, 
        so that is never reported from this client.) 
    */
    this.reportScore = function() {
        server.change('player1scored');
	    server.send('player1scored','p1score');
    }
      
};