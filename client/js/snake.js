//Joshua Jacoby
//Steven Ratcliff
//Jonathan Saavedra
var
/**
 * Constants
 */
COLS = 26,
ROWS = 26,
EMPTY = 0,
SNAKE = 1,
FRUIT = 2,
SNAKE2 = 3,
LEFT = 0,
UP = 1,
RIGHT = 2,
DOWN = 3,
KEY_LEFT = 37,
KEY_UP = 38,
KEY_RIGHT = 39,
KEY_DOWN = 40,
KEY_W = 87,
KEY_S = 83,
KEY_A = 65,
KEY_D = 68,
/**
 * Game objects
 */
canvas,	  /* HTMLCanvas */
ctx,	  /* CanvasRenderingContext2d */
keystate, /* Object, used for keyboard inputs */
frames,   /* number, used for animation */
score2,
score,	  /* number, keep track of the player score */
temp,

player1,
player2, /* string, player IDs */

name_request = false,
get_direction = false,
snake2_scored = false,
fruit_location = false,


serverIP, /* number */
port,

server; /* type: FancyWebSocket */

/**
 * Grid datastructor, usefull in games where the game world is
 * confined in absolute sized chunks of data or information.
 *
 * @type {Object}
 */
grid = {
	width: null,  /* number, the number of columns */
	height: null, /* number, the number of rows */
	_grid: null,  /* Array<any>, data representation */
	/**
	 * Initiate and fill a c x r grid with the value of d
	 * @param  {any}    d default value to fill with
	 * @param  {number} c number of columns
	 * @param  {number} r number of rows
	 */
	init: function(d, c, r) {
		this.width = c;
		this.height = r;
		this._grid = [];
		for (var x=0; x < c; x++) {
			this._grid.push([]);
			for (var y=0; y < r; y++) {
				this._grid[x].push(d);
			}
		}
	},
	/**
	 * Set the value of the grid cell at (x, y)
	 *
	 * @param {any}    val what to set
	 * @param {number} x   the x-coordinate
	 * @param {number} y   the y-coordinate
	 */
	set: function(val, x, y) {
		this._grid[x][y] = val;
	},
	/**
	 * Get the value of the cell at (x, y)
	 *
	 * @param  {number} x the x-coordinate
	 * @param  {number} y the y-coordinate
	 * @return {any}   the value at the cell
	 */
	get: function(x, y) {
		return this._grid[x][y];
	}
}
/**
 * The snake, works as a queue (FIFO, first in first out) of data
 * with all the current positions in the grid with the snake id
 *
 * @type {Object}
 */
snake = {
	direction: null, /* number, the direction */
	last: null,		 /* Object, pointer to the last element in
						the queue */
	_queue: null,	 /* Array<number>, data representation*/
	/**
	 * Clears the queue and sets the start position and direction
	 *
	 * @param  {number} d start direction
	 * @param  {number} x start x-coordinate
	 * @param  {number} y start y-coordinate
	 */
	init: function(d, x, y) {
		this.direction = d;
		this._queue = [];
		this.insert(x, y);
	},
	/**
	 * Adds an element to the queue
	 *
	 * @param  {number} x x-coordinate
	 * @param  {number} y y-coordinate
	 */
	insert: function(x, y) {
		// unshift prepends an element to an array
		this._queue.unshift({x:x, y:y});
		this.last = this._queue[0];
	},

	update: function(x){
	    this._queue = x;
	},
	/**
	 * Removes and returns the first element in the queue.
	 *
	 * @return {Object} the first element
	 */
	remove: function() {
		// pop returns the last element of an array
		return this._queue.pop();
	}
}
snake2 = {
    direction: null, /* number, the direction */
    last: null,		 /* Object, pointer to the last element in
						the queue */
    _queue: null,	 /* Array<number>, data representation*/
    /**
	 * Clears the queue and sets the start position and direction
	 *
	 * @param  {number} d start direction
	 * @param  {number} x start x-coordinate
	 * @param  {number} y start y-coordinate
	 */
    init: function (d, x, y) {
        this.direction = d;
        this._queue = [];
        this.insert(x, y);
    },
    
    update: function (x) {
        // 1. Iterate through current queue and set each corresponding grid cell empty
       
        for (var i  = 0; i < this._queue.length; i++ ) {
            grid.set(EMPTY, this._queue[i].x, this._queue[i].y);
        }
        // 2. Replace current queue with new queue, x
        this._queue = x;

        // 3. Iterate through new queue and set each corresponding grid cell to snake2
        for (var i  = 0; i < this._queue.length; i++ ) {
            grid.set(SNAKE2, this._queue[i].x, this._queue[i].y);
        }

        // 4. Update the 'last' pointer
        this.last = this._queue[0];
    },
    /**
	 * Adds an element to the queue
	 *
	 * @param  {number} x x-coordinate
	 * @param  {number} y y-coordinate
	 */
    insert: function (x, y) {
        // unshift prepends an element to an array
        this._queue.unshift({ x: x, y: y });
        this.last = this._queue[0];
    },
    /**
	 * Removes and returns the first element in the queue.
	 *
	 * @return {Object} the first element
	 */
    remove: function () {
        // pop returns the last element of an array
        return this._queue.pop();
    }
};



/**
 * Set a food id at a random free cell in the grid
 */
function setFood() {
	//var empty = [];
	// iterate through the grid and find all empty cells
    get_direction = false;
    fruit_location = true;
    server.send('message', 'fruit');
    
    /*
	for (var x=0; x < grid.width; x++) {
		for (var y=0; y < grid.height; y++) {
			if (grid.get(x, y) === EMPTY) {
				empty.push({x:x, y:y});
			}
		}
	}
	//chooses a random cell
	var randpos = empty[Math.round(Math.random()*(empty.length - 1))];
    */
	//grid.set(FRUIT, randpos.x, randpos.y);
    
}
/**
 * Starts the game
 */
function main() {
	// create and initiate the canvas element
	canvas = document.createElement("canvas");
	canvas.width = COLS*20;
	canvas.height = ROWS*20;
	ctx = canvas.getContext("2d");
	// add the canvas element to the body of the document
	document.body.appendChild(canvas);
	// sets an base font for bigger score display
	ctx.font = "12px Helvetica";
	frames = 0;
	keystate = {};
	// keeps track of the keybourd input
	document.addEventListener("keydown", function(evt) {
		keystate[evt.keyCode] = true;
	});
	document.addEventListener("keyup", function(evt) {
		delete keystate[evt.keyCode];
	});
	// intatiate game objects and starts the game loop
	init();
}
/**
 * Resets and inits game objects
 */
function init() {
    name_request = true;
    score = 0;
    score2 = 0;
    temp = ' ';
	grid.init(EMPTY, COLS, ROWS);
    
    
}
/**
 * The game loop function, used for game updates and rendering
 */
function loop() {
	update();
	draw();
	// When ready to redraw the canvas call the loop function
	// first. Runs about 60 frames a second
	window.requestAnimationFrame(loop, canvas);
}
/**
 * Updates the game logic
 */
function update() {
	frames++;
	// changing direction of the snake depending on which keys
	// that are pressed
	if (keystate[KEY_LEFT] && snake.direction !== RIGHT) {
		snake.direction = LEFT;
	}
	if (keystate[KEY_UP] && snake.direction !== DOWN) {
		snake.direction = UP;
	}
	if (keystate[KEY_RIGHT] && snake.direction !== LEFT) {
		snake.direction = RIGHT;
	}
	if (keystate[KEY_DOWN] && snake.direction !== UP) {
		snake.direction = DOWN;
	}
	if (keystate[KEY_A] && snake.direction !== RIGHT) {
	    snake.direction = LEFT;
	}
	if (keystate[KEY_W] && snake.direction !== DOWN) {
	    snake.direction = UP;
	}
	if (keystate[KEY_D] && snake.direction !== LEFT) {
	    snake.direction = RIGHT;
	}
	if (keystate[KEY_S] && snake.direction !== UP) {
	    snake.direction = DOWN;
    }
    // each  frames update the game state.

	
	server.send('message', JSON.stringify(snake._queue));
	if (frames%20 === 0) {
		// pop the last element from the snake queue i.e. the
		// head
		var nx = snake.last.x;
		var ny = snake.last.y;
		var nx2 = snake2.last.x;
		var ny2 = snake2.last.y;

		// updates the position depending on the snake direction
		switch (snake.direction) {
			case LEFT:
				nx--;
				break;
			case UP:
				ny--;
				break;
			case RIGHT:
				nx++;
				break;
			case DOWN:
				ny++;
				break;
		}
        
		// checks all gameover conditions
		if (0 > nx || nx > grid.width-1  ||
			0 > ny || ny > grid.height-1 ||
			grid.get(nx, ny) === SNAKE || 0 > nx2 || nx2 > grid.width - 1 ||
			0 > ny2 || ny2 > grid.height - 1 || grid.get(nx, ny) === SNAKE2 ||
			grid.get(nx2, ny2) === SNAKE
		) {
			endGame();
		}
		// check wheter the new position are on the fruit item
		if (grid.get(nx, ny) === FRUIT) {
		    // increment the score and sets a new fruit position
		    server.send('message', 'p1score');
		} else {
			// take out the first item from the snake queue i.e
			// the tail and remove id from grid
			var tail = snake.remove();
			grid.set(EMPTY, tail.x, tail.y);
		}

		// add a snake id at the new position and append it to
		// the snake queue
		grid.set(SNAKE, nx, ny);
		snake.insert(nx, ny);
        
        
	}
}
/**
 * Render the grid to the canvas.
 */
function draw() {
	// calculate tile-width and -height
	var tw = canvas.width/grid.width;
	var th = canvas.height/grid.height;
	// iterate through the grid and draw all cells
	for (var x=0; x < grid.width; x++) {
		for (var y=0; y < grid.height; y++) {
			// sets the fillstyle depending on the id of
			// each cell
			switch (grid.get(x, y)) {
				case EMPTY:
					ctx.fillStyle = "#fff";
					break;
				case SNAKE:
					ctx.fillStyle = "#00f";
					break;
				case FRUIT:
					ctx.fillStyle = "#f83";
					break;
			    case SNAKE2:
			        ctx.fillStyle = "#f00";
			        break;
			}
			ctx.fillRect(x*tw, y*th, tw, th);
		}
	}
	// changes the fillstyle once more and draws the score
	// message to the canvas
	ctx.fillStyle = "#000";
	ctx.fillText(temp, 10, canvas.height - 10);
	ctx.fillText(player1 + " score: " + score, 10, canvas.height - 10);
	ctx.fillText(player2 + " score: " + score2, 180, canvas.height - 10);

}


// Connect to server and prepare to start game
function connectServer() {

    // Initialize variables
    serverIP = $("#server-ip").val();
    port = $("#port").val();
    player1 = $("#player1").val();
      
    // Hide the message panel
    $("#msg-panel").hide();
    $("form").hide();
    
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
        showConnectionStatus(false); // Update the notification strip
        // Stop the game, if it's running?
        // ...
        // ...
	});

	// (3) Message event -- message received from server
	server.bind('message', function (payload) {
        if (name_request == true) {
	        player2 = payload;
	        name_request = false;
	        server.send('message', 'client');
        }
        else if (message == "READY") {

        }
        else if (payload == "p1scored") {
            score++;
            setFood();
        }
        else if (payload == "p2scored") {
            score2++;
            get_direction = false;
            fruit_location = true;
        }
        else if (get_direction == true) {
	        var queue = JSON.parse(payload);
	        snake2.update(queue);
	    }
        else if (fruit_location == true) {
            var xy = payload.split("/");
            grid.set(FRUIT, parseInt(xy[0]), parseInt(xy[1]));
            fruit_location = false;
            get_direction = true;
        }
        else if (payload == "0") {
           var sp = { x: Math.floor(COLS / 2), y: ROWS - 1 };
	       var sp2 = { x: Math.floor(COLS / 2), y: 0 };
	       snake.init(UP, sp.x, sp.y);
	       snake2.init(DOWN, sp2.x, sp2.y);
	       grid.set(SNAKE, sp.x, sp.y);
	       grid.set(SNAKE2, sp2.x, sp2.y);
	       setFood();
           loop();
        }
	    else if (payload == "1") {
           var sp2 = { x: Math.floor(COLS / 2), y: ROWS - 1 };
	       var sp = { x: Math.floor(COLS / 2), y: 0 };
	       snake.init(DOWN, sp.x, sp.y);
	       snake2.init(UP, sp2.x, sp2.y);
	       grid.set(SNAKE2, sp.x, sp.y);
	       grid.set(SNAKE, sp2.x, sp2.y);
	       setFood();
           loop();
        }
        // Connection ready message: let's start playing the game
        else if (payload == "CONNECTION_READY") {
            server.send('message', player1);  // Send player id to server
            showConnectionStatus(true); // Update the notification strip          
            main(); // start the game 
        }
        
        // Connection rejected message: let's display the message panel
        // to inform the user
        else if (payload == "CONNECTION_REJECTED") {
            showMessagePanel("Connection rejected by server", "Please try again later", "Try again");
            document.getElementById("restart-btn").focus();
        }

 	});

    
    // Try to connect...   
    server.connect();
}

// Game over
function endGame() {
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

    showMessagePanel("Game over", msg, "Play again");
    
    // Disconnect from server
    server.disconnect();
}


// Shows the message screen with the given contents
function showMessagePanel(headline, body, buttonText) {
    $("#msg-headline").html(headline); // Set the H1 text
    $("#msg-body").html(body); // Set the p text
    $("#restart-btn").val(buttonText); // Set the restart button text

    $(canvas).remove(); // Delete the game canvas, if we have one
    $("form").hide(); // Hide the settings form
    
    $("#msg-panel").show(); // Show the message panel
}

// Show the current connection status
function showConnectionStatus(isConnected) {
    if (isConnected) {
        $("#connection-status").css("background-color", "green"); 
        $("#connection-status").html("Connected!");
    }
    else {
        $("#connection-status").css("background-color", "red"); 
        $("#connection-status").html("No server connection");        
    }
}

// Set click event handlers
$(document).ready(function(e) {

    $("#submit").click(function() {
        connectServer();
    });
    
    $("#restart-btn").click(function() {
       location.reload();
    });
});

$(document).keypress(function(e) {
    if(e.which == 13 && document.activeElement.tagName == "INPUT") {
        connectServer();
    }
});