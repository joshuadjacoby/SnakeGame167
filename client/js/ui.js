/*
Joshua Jacoby
Steven Ratcliff
Jonathan Saavedra
*/

/* UI Object */
var ui = {

    // Submit button action: instantiates a GameNetwork and attempts to connect.
    submitButton: function() {
        // Hide the message panel
        $("#msg-panel").hide();
        $("form").hide();

        // Set player name (currently, global variable)
        myName = $("#player1").val();

        // Instantiate network and connect
        network = new GameNetwork($("#server-ip").val(), $("#port").val());
        network.connectServer();
    },
      
    
    /* Shows the message screen with the given contents */
    showMessagePanel: function(headline, body, buttonText) {
        $("#msg-headline").html(headline); // Set the H1 text
        $("#msg-body").html(body); // Set the p text
        $("#restart-btn").val(buttonText); // Set the restart button text
    
        $("canvas").remove(); // Delete the game canvas, if we have one
        $("form").hide(); // Hide the settings form
        
        $("#msg-panel").show(); // Show the message panel
    },
    
    /* Update the connection status message */
    showConnectionStatus: function(isConnected) {
        if (isConnected) {
            $("#connection-status").css("background-color", "green"); 
            $("#connection-status").html("Connected!");
        }
        else {
            $("#connection-status").css("background-color", "red"); 
            $("#connection-status").html("No server connection");        
        }
    },    
    
    /* Show the game over screen */
    endGame: function(player1, player2, score, score2) {
        // Show the endgame message
        var msg;
    
        if (score == score2) {
            msg = "The game was a " + score + "-" + score2 + " tie.";
        }
        else if (score > score2) {
            msg = "" + player1 + " won the game, " + score + "-" + score2;
        }
        else {
            msg = "" + player2 + " won the game, " + score2 + "-" + score;
        }
    
        this.showMessagePanel("Game over", msg, "Play again");
        
        // Disconnect from server
        server.disconnect();
    }   
};

/* Document ready: this code executes as soon as the DOM has loaded  */
$(document).ready(function() {
    // Set event handler for submit button
    $("#submit").click(function() {
        ui.submitButton();
    });
    
    // Set event handler for restart button
    $("#restart-btn").click(function() {
         $("#msg-panel").hide();
         $("#settings-form").show();
    });
    
    // Set event handler for return key
    $(document).keypress(function(e) {
        if(e.which == 13 && document.activeElement.tagName == "INPUT") {
            ui.submitButton();
        }
    });    
});


