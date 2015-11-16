#ifndef _log_h
#define _log_h 1

#include <fstream>
#include <string>
#include <vector>

class log_type {
  int t;
  
public:
  static int setup;
  static int phy;
  static int mac;
  static int traffic;
  static int adapt;
  static int channel;
  static int debug;
  
  log_type() {t = 0;}
  log_type(int i) {t = i;}

  operator bool() const {return t;}
  operator int() const {return t;}
  
  inline log_type operator& (log_type t2) {return t & t2;}
  inline log_type operator| (log_type t2) {return t | t2;}  
  inline log_type& operator&= (log_type t2) {t &= t2; return *this;}
  inline log_type& operator|= (log_type t2) {t |= t2; return *this;}              
};

istream& operator>> (istream& is, log_type& lt);    
ostream& operator<< (ostream& os, const log_type& lt);

////////////////////
// class log_file //
////////////////////
class log_file : public ofstream {
  log_type log_flag;
    
public:
  log_file() {}
  log_file(string s, log_type i); 
  
  log_file& open(string s, log_type i);
  log_file& open(string s, vector<log_type>& iv);

  
  inline bool operator() (log_type i) const {return i & log_flag;}  
  
};

#endif
