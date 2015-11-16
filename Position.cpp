#include <math.h>
#include <iomanip>

#include "Position.h"

double Position::distance (Position a) const {
  double xd = a.x - x;
  double yd = a.y - y;
  return sqrt(xd*xd + yd*yd);
}

////////////////////////////////////////////////////////////////////////////////
// output operator<< Position                                                 //
////////////////////////////////////////////////////////////////////////////////
ostream& operator << (ostream& os, const Position& p) {
  int w = os.width();
  return os << '(' << setw(w) << p.x << ';' << setw(w) << p.y << ')';
}

////////////////////////////////////////////////////////////////////////////////
// input operator>> Position                                                  //
////////////////////////////////////////////////////////////////////////////////
istream& operator>> (istream& is, Position& p) {

  char c;
  
  is >> c;
  if (c != '(') is.clear(ios::failbit);
  
  is >> p.x;

  is >> c;
  if (c != ';') is.clear(ios::failbit);
  
  is >> p.y;

  is >> c;
  if (c != ')') is.clear(ios::failbit);
  
  return is;
}

