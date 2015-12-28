/*
* Copyright (c) 2002-2015 by Microwave and Wireless Systems Laboratory, by Andre Barreto and Calil Queiroz
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

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

