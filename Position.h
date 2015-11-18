#ifndef _Position_h
#define _Position_h 1

#include <ostream>
#include <istream>

using namespace std;

class Position{
      double x;
      double y;

public:
  Position() {x = 0; y = 0;}
  Position(double a, double b) : x(a), y(b) {}

  Position operator- (Position b) const {return Position(x-b.x,y-b.y);}
  Position operator+ (Position b) const {return Position(x+b.x,y+b.y);}

  double distance (Position a = Position(0,0)) const;

  friend ostream& operator<< (ostream& os, const Position& p);
  friend istream& operator>> (istream& is, Position& p);
  
};

#endif
