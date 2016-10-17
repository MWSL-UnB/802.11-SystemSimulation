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

#ifndef _timestamp_h
#define _timestamp_h 1

#include "long_integer.h"
#include <iostream>

using namespace std;

const double TimeUnit = 0.4e-6; // time unit is 0.4 microseconds

class timestamp {
  long_integer t;

public:
  timestamp() {t=0;}
  timestamp(long_integer i) : t(i) {}
  timestamp(unsigned i) : t(i) {}
  explicit timestamp(int i);
  explicit timestamp(double d);


  bool is_not_a_timestamp () const {return t==not_a_long_integer;}

  operator double() const {return (double(t) * TimeUnit);}

  bool operator<  (timestamp b) const {return t<b.t;}
  bool operator<= (timestamp b) const {return t<=b.t;}
  bool operator>  (timestamp b) const {return t>b.t;}
  bool operator>= (timestamp b) const {return t>=b.t;}
  bool operator== (timestamp b) const {return t==b.t;}

  timestamp operator+ (timestamp b) const;
  timestamp operator- (timestamp b) const;
  timestamp operator* (timestamp b) const;
  long_integer operator/ (timestamp b) const;
  timestamp operator+ (int b) const;
  timestamp& operator+= (timestamp b);

  friend timestamp operator* (int a, timestamp b);
  friend timestamp operator+ (int a, timestamp b);

  friend ostream& operator << (ostream& os, const timestamp& ts);
};

inline timestamp not_a_timestamp() {return timestamp(not_a_long_integer);}
inline timestamp timestamp_max () {return timestamp(long_integer_max);}


#endif
