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
     return os << double(ts);
   }
}



