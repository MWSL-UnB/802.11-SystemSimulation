#ifndef _timestamp_h
#define _timestamp_h 1

#include "long_integer.h"
#include <iostream>

using namespace std;

const double TimeUnit = 1.0e-6; // time unit is 1 microsec

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
