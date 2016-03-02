/*
Joshua Jacoby
Steven Ratcliff
Jonathan Saavedra
*/

var
/**
 * Constats
 */
COLS = 26,
ROWS = 26,
EMPTY = 0,
SNAKE1 = 1,
FRUIT = 2,
SNAKE2 = 3,
LEFT  = 0,
UP    = 1,
RIGHT = 2,
DOWN  = 3,
KEY_LEFT  = 37,
KEY_UP    = 38,
KEY_RIGHT = 39,
KEY_DOWN = 40,
KEY_W = 87,
KEY_S = 83,
KEY_A = 65,
KEY_D = 68,
UPDATE_CYCLE_LENGTH_MS = 300, /* Duration of each frame -- same as server */
/**
 * Game objects
 */
canvas,	  /* HTMLCanvas */
ctx,	  /* CanvasRenderingContext2d */
keystate, /* Object, used for keyboard inputs */
temp,

myName,
player1, 
player2, /* string, player names */

score1, /* int: player scores */
score2,

playerNumber, /* int : Player number (1 or 2) assigned from server */
newServerUpdate, /* message object: a server game update ready to be processed */ 
applePosition, /* (x, y) coordinate pair representing the apple location */

running, /* boolean, flags if game is running or not */
animationFrame, /* the current Window.animationframe (in case we need to kill it) */
network_latency, /* the most recent latency estimate */

frame, /* The current frame number for server synchronization */
lastUpdateTime, /* the time, in ms, when we last advanced the frame */

network; /* type: GameNetwork */

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
		if (x < this._grid.length && y < this._grid[x].length) {
		    this._grid[x][y] = val;
		}
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
 *  Snake 1 is the snake corresponding to Player 1 
 *  (which may or may not be the local player -- depends on the assignment from server) 
 */
snake1 = {
	direction: null, /* number, the direction */
	last: null,		 /* Object, pointer to the last element in
						the queue */
	_queue: null,	 /* Array<number>, data representation*/
	history: {},   /* List of the direction for each frame. E.g., history[2] = direction @ frame 2 */
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

/*  Snake 2 is the snake corresponding to Player 2 
 *  (which may very well be this client - depends on server assignment) 
 */
    snake2 = {
    direction: null, /* number, the direction */
    last: null,		 /* Object, pointer to the last element in
						the queue */
    _queue: null,	 /* Array<number>, data representation*/
    history: {},   /* List of the direction for each frame. E.g., history[2] = direction @ frame 2 */
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
	frame = 0;
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
	running = true;
	loop();
}

/**
 * Resets and inits game objects
 */
function init() {
    score1 = 0;
    score2 = 0;
    temp = ' ';
	grid.init(EMPTY, COLS, ROWS);
}

/**
 * The game loop function, used for game updates and rendering
 */
function loop() {
    // Only run this loop if game is running
    if (!running) {
        return;
    }
	
	checkKeyState();
	update();
	draw();
	
	// When ready to redraw the canvas call the loop function
	// first. Runs about 60 frames a second
	animationFrame = window.requestAnimationFrame(loop, canvas);
}

/**
 * Checks the key state and updates the local player's direction.
 */
function checkKeyState() {
	
    // CAUTION: This section is a little tricky, becasue we need to know
    // which snake we are "driving." Are we snake1, or snake2?
    // That depends on the assignment we received from the server.

    // First, let's determine our current direction, since it affects
    // where the snake is allowed to turn
    var direction;
    
    if (playerNumber == 1) {
        direction = snake1.direction;     
    }
    else {
        direction = snake2.direction;
    }

    var newDirection = direction;

    // Read the keystate and choose a direction    
	if (keystate[KEY_LEFT] && direction !== RIGHT) {
		newDirection = LEFT;
	}
	if (keystate[KEY_UP] && direction !== DOWN) {
		newDirection = UP;
	}
	if (keystate[KEY_RIGHT] && direction !== LEFT) {
		newDirection = RIGHT;
	}
	if (keystate[KEY_DOWN] && direction !== UP) {
		newDirection = DOWN;
	}
	
	// If the direction has changed, we need to update our 
	// status and then tell the server.
	if (direction != newDirection) {
    	    // Update status
    	    if (playerNumber == 1) {
        	    snake1.direction = newDirection;
    	    }
    	    else {
        	    snake2.direction = newDirection;
    	    }
    	    
    	    // Build a player status object and send it to the server
    	    network.sendUpdate(playerStatus());
  	}	
}

/** Updates the game state, if an update is available from the server.
 *
 *  Status object contains at least the following key-value pairs 
 *  (more can be added later, as client functionality grows):
 *
 *  "MESSAGE_TYPE" = "SERVER_UPDATE",
 *  "CURRENT_FRAME" = the current frame number
 *  "GAME_STATUS" = true/false whether the game is still active
 *  "PLAYER_1_NAME" = player 1's name
 *  "PLAYER_2_NAME" = player 2's name
 *  "PLAYER_1_SCORE" = player 1's score
 *  "PLAYER_2_SCORE" = player 2's score
 *  "PLAYER_1_QUEUE" = a JSON object containing P1's queue
 *  "PLAYER_2_QUEUE" = a JSON object containing P2's queue
 *  "PLAYER_1_DIRECTION" = player 1's direction
 *  "PLAYER_2_DIRECTION" = player 2's direction
 */
function update() {
    // If it's too soon for the next frame, do nothing
    if (new Date().getTime() - lastUpdateTime < UPDATE_CYCLE_LENGTH_MS) {
        return;
    }
    
    frame++;
    
    // Record the current player's direction in the history
    mySnake().history[frame] = mySnake().direction;
   
    // If no news from server, just advance both snakes 1 unit for simulation
    if (newServerUpdate == null || newServerUpdate == undefined) {
        advanceSnake(snake1);
        advanceSnake(snake2);   
        console.log("Local frame " + frame + ": extrapolating...");
    }
    
    // If we have an update from the server, apply it.
    else {
        // First, check if game is over
        if (newServerUpdate["GAME_STATUS"] == false) {
            ui.endGame(player1, player2, score1, score2);
            newServerUpdate = null;
            running = false;
            return;
        }
                
        // Apply the data and update the opponent's direction
        player1 = newServerUpdate["PLAYER_1_NAME"];
        player2 = newServerUpdate["PLAYER_2_NAME"];
        score1 = newServerUpdate["PLAYER_1_SCORE"];
        score2 = newServerUpdate["PLAYER_2_SCORE"];
        snake1._queue = newServerUpdate["PLAYER_1_QUEUE"];
        snake2._queue = newServerUpdate["PLAYER_2_QUEUE"];
        applePosition = newServerUpdate["APPLE_POSITION"];
        if (playerNumber == 1) { 
            snake2.direction = newServerUpdate["PLAYER_2_DIRECTION"];
        } else {
            snake1.direction = newServerUpdate["PLAYER_1_DIRECTION"];
        }
        
        // The server update is always arriving from the past. So, let's 
        // fast-forward a certain number of frames to compensate.
        var lag = frame - newServerUpdate["CURRENT_FRAME"];
        compensateLag(lag);
        
        // Delete the server update; we don't need it anymore
        newServerUpdate = null;        
    }
    
    // Clear the grid
    	grid.init(EMPTY, COLS, ROWS);

    	// Write snake 1 onto the grid
    	for (var i = 0; snake1._queue != null && i < snake1._queue.length; i++) {
        	grid.set(SNAKE1, snake1._queue[i]["x"], snake1._queue[i]["y"]);
    	}

    	// Write snake 2 onto the grid
    	for (var i = 0; snake2._queue != null && i < snake2._queue.length; i++) {
        	grid.set(SNAKE2, snake2._queue[i]["x"], snake2._queue[i]["y"]);
    	}
                    
    // Write the apple onto the grid
    grid.set(FRUIT, applePosition["x"], applePosition["y"]);
    
    // Update the clock
    lastUpdateTime = new Date().getTime();
}

/** Advances the snake object one unit for client-side
 *  prediction purposes.
 *  @param snakeObject - the snake object to advance
 */
function advanceSnake(snake) {
    // Grab the current head location
	var nx = snake._queue[0].x;
	var ny = snake._queue[0].y;

	// Determine new head position (nx, ny)
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
         
    snake.insert(nx, ny);    
    	snake.remove();
}

/** Compensates for lag in server update by fast-forwarding. 
 *  For the opponent, we simply advance the snake by the 
 *  number of lag frames, as a guess. But for the player,
 *  we can do better: we'll replay the forward history
 *  of the player's actual turns, starting from the frame
 *  immediately after the old frame in which the server 
 *  update originated.
 */
function compensateLag(lag) {
    var old_frame = frame - lag;
    
    // Replay the local player's forward history (known)
    for (var i = old_frame + 1; i <= frame; i++) {
        mySnake().direction = mySnake().history[i];
        advanceSnake(mySnake());
    }
    
    // Fast-forward the opponent straight ahead (predicted)  
    var opponentSnake;
    if (playerNumber == 1) {
        opponentSnake = snake2;
    } else {
        opponentSnake = snake1;
    }
    for (var i = 0; i < lag; i++) {
        advanceSnake(opponentSnake);
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
				case SNAKE1:
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
	ctx.fillText(player1 + " score: " + score1, 10, canvas.height - 10);
	ctx.fillText(player2 + " score: " + score2, 180, canvas.height - 10);
    ctx.fillText(" Network latency: " + network_latency + "ms", 300, canvas.height - 10);

}

/** Initializes new player 
    @param playernum - the player number assigned from the server (1 or 2)
*/
function initializePlayer(playerNum) {
    // Set the player number
    playerNumber = playerNum;

    // Set the initial direction on the appropriate snake
    // The local player could be either snake1 or snake2, depending.
    if (playerNumber == 1) {        
        snake1.direction = UP;
    }
    else if (playerNumber == 2) {
        snake2.direction = DOWN;
    }
}


/** Returns an object containing a Client Update message bundle.
  * This has at least the following fields (more can be added later):
  * "MESSAGE_TYPE" = "CLIENT_UPDATE"
  * "PLAYER_NUMBER" = 1 or 2
  * "PLAYER_NAME" = (player name)
  * "CLIENT_DIRECTION" = int (direction)
*/
function playerStatus() {
    var msg = {};
    msg["MESSAGE_TYPE"] = "CLIENT_UPDATE";
    msg["PLAYER_NUMBER"] = playerNumber;
    msg["PLAYER_NAME"] = myName;

    if (playerNumber == 1) {
        msg["CLIENT_DIRECTION"] = snake1.direction;
    }    
    
    else if (playerNumber == 2) {
        msg["CLIENT_DIRECTION"] = snake2.direction;
    }
    return msg;
}

/** Returns a reference to the local player's Snake object  
*/
function mySnake() {
    if (playerNumber == 1) {
        return snake1;
    }
    else {
        return snake2;
    }
}