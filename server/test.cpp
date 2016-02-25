#include "position.h"
#include <iostream>
#include "json.hpp"
#include "player.h"

using namespace std;
using json = nlohmann::json;

Player *shawn;
Player *josh;

void showStatus(Player*);

int main() {

    shawn = new Player("Shawn", DOWN, Position(4,0));
    josh = new Player("Josh", UP, Position(4, 5));
    Position food(4,1);

    
    showStatus(shawn);
    showStatus(josh);

    
    shawn->advance(food);
    shawn->advance(food);
    shawn->advance(food);
    josh->advance(food);
    josh->advance(food);
    
    
    
    showStatus(shawn);
    showStatus(josh);
    
    cout << shawn->collidesWith(*josh) << endl;
    
    shawn->direction = RIGHT;
    shawn->advance(food);
    shawn->advance(food);
        shawn->advance(food);
        shawn->advance(food);
        shawn->advance(food);
    
    showStatus(shawn);
    showStatus(josh);
    
    cout << shawn->collidesWith(*josh) << endl;

    cout << josh->occupies(Position(4,3)) << endl;
    cout << josh->occupies(Position(0,0)) << endl;    

delete shawn;
return 0;
}

void showStatus(Player *p) {
	cout << p->playerName << endl;
	cout << "Score: " << p->score << endl;
	cout << "Direction: " << p->direction << endl;
	cout << "Queue: " << p->getQueueJSON() << endl;
	cout << "---------------------------------------" << endl;
}

