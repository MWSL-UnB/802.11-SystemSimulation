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

#include <string>
#include <iostream>

#include "myexception.h"
#include "log.h"

const char LOG_FILE_NAME[] = "sim.log";

int log_type::phy       = 0x001;
int log_type::mac       = 0x002;
int log_type::channel   = 0x004;
int log_type::traffic   = 0x008;
int log_type::adapt     = 0x010;
int log_type::setup     = 0x020;
int log_type::debug     = 0x040;

////////////////////////////////
// input operator >> log_type //
////////////////////////////////
istream& operator>> (istream& is, log_type& lt) {
  string str;
  is >> str;

  if (str == "SETUP") lt = log_type::setup;
  else if (str == "PHY") lt = log_type::phy;
  else if (str == "MAC") lt = log_type::mac;
  else if (str == "TRAFFIC") lt = log_type::traffic;
  else if (str == "ADAPT") lt = log_type::adapt;
  else if (str == "CHANNEL") lt = log_type::channel;
  else if (str == "DEBUG") lt = log_type::debug;
  else is.clear(ios::failbit);

  return is;
}

////////////////////////////////
// output operator<< log_type //
////////////////////////////////
ostream& operator<< (ostream& os, const log_type& lt) {
  bool out = false;
  if (lt & log_type::setup) {os << "setup"; out = true;}
  if (lt & log_type::phy) {if (out) os << ", "; os << "phy"; out = true;}
  if (lt & log_type::mac) {if (out) os << ", "; os << "mac"; out = true;}
  if (lt & log_type::traffic) {if (out) os << ", "; os << "traffic"; out=true;}
  if (lt & log_type::adapt) {if (out) os << ", "; os << "adapt"; out = true;}
  if (lt & log_type::channel) {if (out) os << ", "; os << "channel"; out=true;}
  if (lt & log_type::debug) {if (out) os << ", "; os << "debug"; out=true;}
  
  return os;                                
}

//////////////////////////
// log_file constructor //
//////////////////////////
log_file::log_file(string s, log_type i) {
  log_flag = i;
  if (i) {
    s = s + '\\' + LOG_FILE_NAME;
    ofstream::open(s.c_str());
    
    if (!is_open()) throw(my_exception(OPENFILE, s));
  }
  
}

////////////////////
// log_file::open //
////////////////////
log_file& log_file::open(string s, log_type i) {

  log_flag = i;
  if (i) {
    s = s + '\\' + LOG_FILE_NAME;
    ofstream::open(s.c_str());

    if (!is_open()) throw(my_exception(OPENFILE, s));
  }  
  return *this;
}

////////////////////
// log_file::open //
////////////////////
log_file& log_file::open(string s, vector<log_type>& iv) {

  log_flag = 0;
  for (vector<log_type>::const_iterator it = iv.begin(); it != iv.end(); ++it)
    log_flag |= *it;
       
  if (log_flag) {
    s = s + '\\' + LOG_FILE_NAME;
    ofstream::open(s.c_str());

    if (!is_open()) throw(my_exception(OPENFILE, s));
  }  
  return *this;
}

