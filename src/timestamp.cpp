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

#include "timestamp.h"
#include "long_integer.h"
#include "myexception.h"


const double double_max = double(long_integer_max-1)/TimeUnit;

//////////////////
// Constructors //
//////////////////

timestamp::timestamp(int i) {
  if (i < 0) throw(my_exception(TS_OVERFLOW));
  t = i;
}

timestamp::timestamp(double d) {
  if (d < 0 || d > double_max)  throw(my_exception(TS_OVERFLOW));
  t = (long_integer)(d/TimeUnit);
}

////////////////////////////////////////////////////////////////////////////////
// Member and friend functions of timestamp
////////////////////////////////////////////////////////////////////////////////

//////////////////////////
// arithmetic operators //
//////////////////////////
timestamp timestamp::operator+ (timestamp b) const{
  long_integer sum = t + b.t;
  if (sum < t || sum < b.t) throw (my_exception(TS_OVERFLOW));
  return timestamp(sum);
}

timestamp timestamp::operator+ (int b) const {
  long_integer sum = t + b;
  if (sum < t || sum < b) throw (my_exception(TS_OVERFLOW));
  return timestamp(sum);
}

timestamp operator+ (int a, timestamp b) {
  long_integer sum = a + b.t;
  if (sum < a || sum < b.t) throw (my_exception(TS_OVERFLOW));
  return timestamp(sum);
}

timestamp timestamp::operator- (timestamp b) const {
  if (t >= b.t) return timestamp (t - b.t);
  else throw (my_exception(TS_OVERFLOW));
}
timestamp timestamp::operator* (timestamp b) const {
  if (t && b.t >= long_integer_max / t) throw (my_exception(TS_OVERFLOW));
  else return timestamp(t*b.t);
}

timestamp operator* (int a, timestamp b) {
  timestamp c;
  if (a < 0 || (a && b.t >= long_integer_max / a))
    throw (my_exception(TS_OVERFLOW));
  else return timestamp(a*b.t);
}

timestamp& timestamp::operator+= (timestamp b) {
  long_integer aux = t;
  t += b.t;
  if (t < aux || t < b.t) throw (my_exception(TS_OVERFLOW));
  return *this;
}

long_integer timestamp::operator/ (timestamp b) const {
  return t/b.t;
}

////////////////////////
// output operator << //
////////////////////////
ostream& operator << (ostream& os, const timestamp& ts) {
   if (ts.is_not_a_timestamp()) {
    return os << "not_a_timestamp";
   } else {
//    return os << ts.t << '(' << double(aux_ts) << "sec.)";
     return os <<  double(ts);
   }
}



