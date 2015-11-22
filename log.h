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
