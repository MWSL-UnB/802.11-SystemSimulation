#ifndef _myexception_h
#define _myexception_h 1

#include "long_integer.h"
#include <string>

using namespace std;


typedef enum {CONFIG,
              EVENT,
              GENERAL,
              OPENFILE,
              SYNTAX,
              TS_OVERFLOW} exception_type;


class my_exception{
  exception_type exc;
  string comp_str;
  long_integer li;
public:
  my_exception () {exc = GENERAL;}
  my_exception (string s) {exc = GENERAL; comp_str = s;} 
  my_exception (exception_type e) : exc(e) {}
  my_exception (exception_type e, string s) {exc = e; comp_str = s;}
  my_exception (exception_type e, long_integer i, string s = "") {
    exc = e; li = i; comp_str = s;}

  string what();
};

#endif
