/*
 * Josh Jacoby
 * Shawn Ratcliff
 * Jonathan Saavedra
 */

#ifndef position_h
#define position_h

#include "json.hpp"

using namespace std;
using json = nlohmann::json;

/* An (x, y) integer pair to represent game coordinates */
class Position {
    
public:
    
    int x;
    int y;
    
    // Constructors
    Position();
    Position(int, int);
    
    // Returns a json object in the form {"x" : value, "y" : value}
    json getJSON() const;
};

// Default constructor
Position::Position() {
    x = 0;
    y = 0;
}

// Constructor
Position::Position(int a, int b) {
    x = a;
    y = b;
}

// Returns JSON object
json Position::getJSON() const {
    return json({ {"x", x}, {"y", y} });
}

// Overloaded == operator for Position class
bool operator== (const Position& a, const Position& b) {
    return (a.x == b.x && a.y == b.y);
}

// Overloaded != operator for Position class
bool operator!= (const Position& a, const Position& b) {
    return !(a == b);
}


#endif /* position_h */
